#include "ToShaderSubsystem.h"
#include "ToShaderComponent.h"
#include "MeshRenderer.h"
#include "ToShader.h"
#include "Components/SceneCaptureComponent2D.h"

#define tolog FToShaderHelpers::log

UToShaderSubsystem::UToShaderSubsystem()
{
}

TArray<TWeakObjectPtr<UPrimitiveComponent>> UToShaderSubsystem::GetShowList(ERendererTag Tag)
{
	TArray<TWeakObjectPtr<UPrimitiveComponent>> ShowList;
	for (const auto M : Modules)
	{
		if (!M) continue;
		if (!M->RendererGroup.Contains(Tag)) continue;
		for (auto G : M->RendererGroup[Tag].Components)
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
	bShouldUpdateShowLists = true;
}

void UToShaderSubsystem::AddMeshRendererToSubsystem(AMeshRenderer* Actor)
{
	if (!Actor) return;
	if (Actor->IsA(AMeshRendererPro::StaticClass()))
	{
		if (!Actor->GetWorld()) return;
		const auto MeshRendererPro = Cast<AMeshRendererPro>(Actor);
		if (MeshRendererPro->PassName.IsNone() || Passes.Contains(MeshRendererPro->PassName)) return;
		FPassContainer NewPass;
		NewPass.Pass = FSceneViewExtensions::NewExtension<FMeshRendererPass>(Actor->GetWorld());
		Passes.Emplace(MeshRendererPro->PassName, NewPass);
		MeshRendererPro->CallBackWhenAddToSubsystemSuccess(NewPass);
	}
	if (MeshRenderers.Contains(Actor)) return;

	MeshRenderers.Add(Actor);
	bShouldUpdateShowLists = true;
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


void UToShaderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick));

	
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

void UToShaderSubsystem::SetShowLists()
{
	if (MeshRenderers.IsEmpty()) return;
	Modules.RemoveAll([](UToShaderComponent* Module)
	{
		return Module == nullptr;
	});
	MeshRenderers.RemoveAll([](AMeshRenderer* Renderer)
	{
		return Renderer == nullptr;
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
	bShouldUpdateShowLists = false;
}

bool UToShaderSubsystem::GetTagName(ERendererTag Tag, FName& TagName)
{
	if (TagNames.IsEmpty()) return false;
	TagName = TagNames[Tag];
	return true;
}

void UToShaderSubsystem::CacheTagNames()
{
	if (!TagNames.IsEmpty()) return;
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
	CacheTagNames();
	SetShowLists();
	return true;
}
