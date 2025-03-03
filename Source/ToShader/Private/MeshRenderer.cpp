#include "MeshRenderer.h"

#include "ToShaderSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"

#define tolog FToShaderHelpers::log

AMeshRenderer::AMeshRenderer()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AMeshRenderer::SetShowList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList)
{
	for (auto Capture : Captures)
	{
		Capture->ShowOnlyComponents = NewList;
	}
}

void AMeshRenderer::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMeshRenderer::PostInitProperties()
{
	Super::PostInitProperties();

	CleanTargetTags();

	CollectCaptures();
	if (GetSubsystem())
		GetSubsystem()->AddMeshRendererToSubsystem(this);
}

void AMeshRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMeshRenderer::CleanTargetTags()
{
	TArray<ERendererTag> NewTags;
	for (auto Tag : TargetMeshTags)
	{
		NewTags.AddUnique(Tag);
	}
	TargetMeshTags = NewTags;
}

void AMeshRenderer::CollectCaptures()
{
	auto Components = K2_GetComponentsByClass(USceneComponent::StaticClass());
	for (const auto Component : Components)
	{
		auto Capture = Cast<USceneCaptureComponent2D>(Component);
		if (!Capture) continue;
		Captures.Add(Capture);
	}
}

UToShaderSubsystem* AMeshRenderer::GetSubsystem()
{
	return GEngine->GetEngineSubsystem<UToShaderSubsystem>();
}


