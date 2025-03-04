#include "MeshRenderer.h"

#include "ToShader.h"
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

void AMeshRenderer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CollectCaptures();
	if (GetSubsystem())
		GetSubsystem()->AddMeshRendererToSubsystem(this);
}

void AMeshRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMeshRenderer::CollectCaptures()
{
	Captures.Empty();
	auto Components = GetComponents();
	for (const auto Component : Components)
	{
		if (!Component) continue;
		auto Capture = Cast<USceneCaptureComponent2D>(Component);
		if (!Capture) continue;
		Captures.Add(Capture);
	}
}

UToShaderSubsystem* AMeshRenderer::GetSubsystem()
{
	if (!GEngine) return nullptr;
	return GEngine->GetEngineSubsystem<UToShaderSubsystem>();
}


