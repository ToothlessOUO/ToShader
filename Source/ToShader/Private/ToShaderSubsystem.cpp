#include "ToShaderSubsystem.h"
#include "MaterialEffect.h"
#include "ToShaderComponent.h"
#include "MeshRenderer.h"
#include "ToShader.h"

UToShaderSubsystem::UToShaderSubsystem()
{
}

TArray<TWeakObjectPtr<UPrimitiveComponent>> UToShaderSubsystem::GetShowList(ERendererTag Tag)
{
	TArray<TWeakObjectPtr<UPrimitiveComponent>> ShowList;
	for (const auto M : Modules)
	{
		if (!M.IsValid()) continue;
		if (!M->RendererGroup.Contains(Tag)) continue;
		for (const auto G : M->RendererGroup[Tag].Components)
		{
			TWeakObjectPtr<UPrimitiveComponent> P = MakeWeakObjectPtr(G);
			ShowList.Add(P);
		}
	}
	return ShowList;
}

TArray<TWeakObjectPtr<UPrimitiveComponent>> UToShaderSubsystem::GetShowList(TArray<ERendererTag> Tags)
{
	TArray<TWeakObjectPtr<UPrimitiveComponent>> ShowList;
	for (auto Tag : Tags)
	{
		auto List = GetShowList(Tag);
		if (List.IsEmpty()) continue;
		ShowList.Append(List);
	}
	return ShowList;
}

void UToShaderSubsystem::AddModuleToSubsystem(UToShaderComponent* Module)
{
	if (!Module) return;
	if (!Modules.Contains(Module))
		Modules.Add(Module);
	CallUpdateMeshRenderers();
}

void UToShaderSubsystem::RemoveModuleFromSubsystem(UToShaderComponent* Module)
{
	if (!Module) return;
	if (Modules.Contains(Module))
		Modules.Remove(Module);
	CallUpdateMeshRenderers();
}


void UToShaderSubsystem::AddMeshRendererToSubsystem(AMeshRenderer* Actor)
{
	if (!Actor) return;
	if (MeshRenderers.Contains(Actor)) return;

	MeshRenderers.Add(Actor);
	CallUpdateMeshRenderers();
}

UToShaderSubsystem* UToShaderSubsystem::GetSubsystem()
{
	if (GEngine)
	{
		return GEngine->GetEngineSubsystem<UToShaderSubsystem>();
	}
	return nullptr;
}

bool UToShaderSubsystem::IsMeshContainsTag(UPrimitiveComponent* Mesh, ERendererTag Tag)
{
	if (Mesh == nullptr) return false;
	const auto S = GetSubsystem();
	if (!S) return false;
	FName TagName;
	if (!S->GetTagName(Tag, TagName)) return false;
	for (auto Element : Mesh->ComponentTags)
	{
		if (Element == TagName)
		{
			return true;
		}
	}
	return false;
}

//调用后更新所有MeshRenderer
void UToShaderSubsystem::CallUpdateMeshRenderers()
{
	bShouldUpdateMeshRenderers = true;
}

void UToShaderSubsystem::SetScreenOverlayMeshManager(AScreenOverlayMeshManager* Manager)
{
	if (Manager == nullptr || ScreenMeshManager.IsValid()) return;
	ScreenMeshManager = Manager;
}

void UToShaderSubsystem::GetScreenOverlayMeshManager(bool& bSuccess, AScreenOverlayMeshManager*& RetManager)
{
	if (!ScreenMeshManager.IsValid())
	{
		bSuccess = false;
		RetManager = nullptr;
		return;
	}

	bSuccess = true;
	RetManager = ScreenMeshManager.Get();
}

void UToShaderSubsystem::CallUpdate_MaterialEffectPropertyTable()
{
	MaterialEffectPropertyTable = nullptr;
}

TArray<FName> UToShaderSubsystem::GetMaterialEffectPropertyTableRowNames(EMPType Type)
{
	if (!MaterialEffectPropertyTable.IsValid()) return TArray<FName>();
	TArray<FName> RetArray;

	switch (Type)
	{
	case EMPType::Key:
		for (auto Data : MPKeyCache)
		{
			RetArray.Emplace(Data.Key);
		}
		break;
	case EMPType::Float:
		for (auto Data : MPFloatCache)
		{
			RetArray.Emplace(Data.Key);
		}
		break;
	case EMPType::Float4:
		for (auto Data : MPFloat4Cache)
		{
			RetArray.Emplace(Data.Key);
		}
		break;
	case EMPType::Texture:
		for (auto Data : MPTextureCache)
		{
			RetArray.Emplace(Data.Key);
		}
		break;
	}

	return RetArray;
}

FMPTableProp* UToShaderSubsystem::GetMP(const FName Name, const EMPType Type)
{
	switch (Type) {
	case EMPType::Key:
		if(MPKeyCache.Contains(Name))
			return MPKeyCache[Name];
		break;
	case EMPType::Float:
		if(MPFloatCache.Contains(Name))
			return MPFloatCache[Name];
		break;
	case EMPType::Float4:
		if(MPFloat4Cache.Contains(Name))
			return MPFloat4Cache[Name];
		break;
	case EMPType::Texture:
		if(MPTextureCache.Contains(Name))
			return MPTextureCache[Name];
		break;
	}
	return nullptr;
}

void UToShaderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick));

	CacheTagNames();
}

void UToShaderSubsystem::Deinitialize()
{
	Super::Deinitialize();
	FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void UToShaderSubsystem::SetShowList(AMeshRenderer* MeshRenderer)
{
	MeshRenderer->SetShowList(GetShowList(MeshRenderer->TargetMeshTags.Array()));
}

void UToShaderSubsystem::UpdateMeshRenderersShowLists()
{
	if (!bShouldUpdateMeshRenderers) return;
	if (MeshRenderers.IsEmpty()) return;
	MeshRenderers.RemoveAll([](TWeakObjectPtr<AMeshRenderer> Renderer)
	{
		return !Renderer.IsValid();
	});
	TMap<ERendererTag, FShowList> SavedList;
	for (auto Renderer : MeshRenderers)
	{
		FShowList CurList;
		if (Renderer->TargetMeshTags.IsEmpty()) continue;

		for (ERendererTag Tag : Renderer->TargetMeshTags)
		{
			if (SavedList.Contains(Tag))
			{
				CurList.List.Append(SavedList[Tag].List);
			}
			else
			{
				auto NewList = GetShowList(Tag);
				CurList.List.Append(NewList);
				SavedList.Emplace(Tag, NewList);
			}
		}
		Renderer->SetShowList(CurList.List);
	}
	bShouldUpdateMeshRenderers = false;
}

bool UToShaderSubsystem::GetTagName(ERendererTag Tag, FName& TagName)
{
	if (TagNames.IsEmpty()) return false;
	TagName = TagNames[Tag];
	return true;
}

void UToShaderSubsystem::CacheTagNames()
{
	//if (!TagNames.IsEmpty()) return;
	const auto EnumPtr = StaticEnum<ERendererTag>();
	if (!EnumPtr) return;
	for (ERendererTag E : TEnumRange<ERendererTag>())
	{
		const auto TagName = FName(EnumPtr->GetNameStringByValue(static_cast<int64>(E)));
		TagNames.Emplace(E, TagName);
	}
}

void UToShaderSubsystem::UpdateMaterialEffectPropertyTable()
{
	if (MaterialEffectPropertyTable.IsValid()) return;
	const FString DataTablePath = TEXT("/ToShader/DataTable/DT_MaterialEffectProperty.DT_MaterialEffectProperty");
	UDataTable* Table = LoadObject<UDataTable>(nullptr, *DataTablePath);
	if (!Table)
	{
		tolog("Material effect table not found in /ToShader/DataTable/DT_MaterialEffectProperty.DT_MaterialEffectProperty");
		return;
	}

	MaterialEffectPropertyTable = Table;

	// 获取RowMap的const引用
	const TMap<FName, uint8*>& RowMap = MaterialEffectPropertyTable.Get()->GetRowMap();

	MPKeyCache.Empty();
	MPFloatCache.Empty();
	MPFloat4Cache.Empty();
	MPTextureCache.Empty();
	// 遍历RowMap
	for (const auto& Pair : RowMap)
	{
		FName RowName = Pair.Key;
		FMPTableProp* RowData = reinterpret_cast<FMPTableProp*>(Pair.Value);
		switch (RowData->Type)
		{
		case EMPType::Key:
			MPKeyCache.Add(RowName,RowData);
			break;
		case EMPType::Float:
			MPFloatCache.Add(RowName,RowData);
			break;
		case EMPType::Float4:
			MPFloat4Cache.Add(RowName,RowData);
			break;
		case EMPType::Texture:
			MPTextureCache.Add(RowName,RowData);
			break;
		}
	}
}

bool UToShaderSubsystem::Tick(float DeltaTime)
{
	UpdateMeshRenderersShowLists();
	UpdateMaterialEffectPropertyTable();
	return true;
}
