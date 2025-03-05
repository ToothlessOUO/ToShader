#include "MeshRenderer.h"

#include "ToShader.h"
#include "ToShaderSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"

#define tolog FToShaderHelpers::log

AMeshRenderer::AMeshRenderer()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	PreTick = CreateDefaultSubobject<UPreTick>(TEXT("PreTick"));
	PostTick = CreateDefaultSubobject<UPostTick>(TEXT("PostTick"));
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
		Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		Captures.Add(Capture);
	}
}

UToShaderSubsystem* AMeshRenderer::GetSubsystem()
{
	if (!GEngine) return nullptr;
	return GEngine->GetEngineSubsystem<UToShaderSubsystem>();
}

#pragma region TickStage

UPreTick::UPreTick()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
	SetTickGroup(TG_PrePhysics);
}

void UPreTick::PostInitProperties()
{
	Super::PostInitProperties();
}

void UPreTick::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Interface)
	{
		Interface->Execute_OnPreTick(GetOwner(), DeltaTime);
	}
	else
	{
		if (!GetOwner()) return;
		if (GetOwner()->GetClass()->ImplementsInterface(UTickStageInterface::StaticClass()))
		{
			Interface = Cast<ITickStageInterface>(GetOwner());
		}
	}
}

UPostTick::UPostTick()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
	SetTickGroup(TG_PostPhysics);
}

void UPostTick::PostInitProperties()
{
	Super::PostInitProperties();
}

void UPostTick::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Interface)	
	{
		Interface->Execute_OnPostTick(GetOwner(), DeltaTime);
	}
	else
	{
		if (!GetOwner()) return;
		if (GetOwner()->GetClass()->ImplementsInterface(UTickStageInterface::StaticClass()))
		{
			Interface = Cast<ITickStageInterface>(GetOwner());
		}
	}
}

#pragma endregion
