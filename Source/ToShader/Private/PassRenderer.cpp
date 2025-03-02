#include "PassRenderer.h"

#include "ToShaderSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"

#include "ToShader.h"
#define tolog FToShaderHelpers::log

APassRenderer::APassRenderer()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AlwaysTickComponent = CreateDefaultSubobject<UAlwaysTickComponent>(TEXT("AlwaysTickComponent"));
}

TArray<UPrimitiveComponent*> APassRenderer::GetShowObjs_Implementation()
{
	return TArray<UPrimitiveComponent*>();
}

void APassRenderer::ExecuteAfterInitComponents_Implementation()
{
	auto ShaderSubsystem = GEngine->GetEngineSubsystem<UToShaderSubsystem>();
	if (ShaderSubsystem != nullptr)
	{
		double CurVersion;
		if (ShaderSubsystem->ShouldUpdateEyeBrows(TimeVersion,CurVersion))
		{
			TimeVersion = CurVersion;
		}
		else
		{
			return;
		}
	}else
	{
		return;
	}
	Captures.Empty();
	auto C = K2_GetComponentsByClass(USceneCaptureComponent2D::StaticClass());
	for (auto ActorComponent : C)
	{
		if (ActorComponent==nullptr) continue;
		auto Capture = Cast<USceneCaptureComponent2D>(ActorComponent);
		if (Capture == nullptr) continue;
		Captures.Add(Capture);
	}
	if (GetShowObjs().IsEmpty()) return;
	for (const auto Capture : Captures)
	{
		Capture->ShowOnlyComponents.Empty();
		for (const auto O : GetShowObjs())
		{
			if (O)
				Capture->ShowOnlyComponents.Add(O);
		}
	}
}

void APassRenderer::BeginPlay()
{
	Super::BeginPlay();
	
}

void APassRenderer::PostInitProperties()
{
	Super::PostInitProperties();
	AlwaysTickComponent->OnComponentTick.AddDynamic(this,&ThisClass::T);
}

void APassRenderer::T(float DeltaTime)
{
	tolog("Tick");
	Tick(DeltaTime);
}

void APassRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ExecuteAfterInitComponents();
}

