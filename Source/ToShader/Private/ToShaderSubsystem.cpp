#include "ToShaderSubsystem.h"
#include "ToShaderModule.h"
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

void UToShaderSubsystem::AddModuleToSubsystem(UToShaderModule* Module)
{
	if (!Module) return;
	if (! Modules.Contains(Module))
		Modules.Add(Module);
	bShouldUpdateShowLists = true;
}

void UToShaderSubsystem::AddMeshRendererToSubsystem(AMeshRenderer* Actor)
{
	if (!Actor || MeshRenderers.Contains(Actor)) return;
	if (!MeshRenderers.Contains(Actor))
		MeshRenderers.Add(Actor);
	bShouldUpdateShowLists = true;
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
	Modules.RemoveAll([](UToShaderModule* Module)
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

bool UToShaderSubsystem::Tick(float DeltaTime)
{
	SetShowLists();
	return true;
}
