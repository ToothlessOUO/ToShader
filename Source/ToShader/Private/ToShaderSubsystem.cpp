#include "ToShaderSubsystem.h"
#include "ToShaderComponent.h"
#include "MeshRenderer.h"

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
	if (! Modules.Contains(Module))
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
	if (!S->GetTagName(Tag,TagName)) return false;
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
			}else
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
		TagNames.Emplace(E,TagName);
	}
}

bool UToShaderSubsystem::Tick(float DeltaTime)
{
	UpdateMeshRenderersShowLists();
	return true;
}
