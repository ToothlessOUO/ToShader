#include "ToShaderSubsystem.h"

#include "EngineAnalytics.h"
#include "ToShader.h"
#include "Kismet/KismetSystemLibrary.h"

#define tolog FToShaderHelpers::log

UToShaderSubsystem::UToShaderSubsystem()
{
}

bool UToShaderSubsystem::ShouldUpdateEyeBrows(double TimeVersion, double& CurTimerVersion) const
{
	CurTimerVersion = EyeBrowTimeVersion;
	return FMath::IsNearlyEqual(TimeVersion,EyeBrowTimeVersion);
}

void UToShaderSubsystem::AddEyeBrowComponents(AActor* Actor, TArray<UPrimitiveComponent*> Components)
{
	if (Actor == nullptr || Components.IsEmpty()) return;
	FPrimitiveComponents PrimitiveComponents;
	PrimitiveComponents.Components = Components;
	EyeBrowMap.FindOrAdd(Actor);
	EyeBrowMap[Actor] = PrimitiveComponents;
	tolog("NewActorCompAdd :"+Actor->GetName()+" ",Components.Num());
	bIsEyeBrowComponentsChanged = true;
}

TArray<UPrimitiveComponent*> UToShaderSubsystem::GetEyeBrowComponents()
{
	return EyeBrowComponents;
}

void UToShaderSubsystem::UpdateEyeBrowComponentsFromMap()
{
	if (!bIsEyeBrowComponentsChanged) return;
	EyeBrowMap.Remove(nullptr);
	EyeBrowComponents.Empty();
	for (auto Ele : EyeBrowMap)
	{
		if (!Ele.Value.Components.IsEmpty())
			EyeBrowComponents.Append(Ele.Value.Components);
	}

	bIsEyeBrowComponentsChanged = false;
	EyeBrowTimeVersion = UKismetSystemLibrary::GetGameTimeInSeconds(GetWorld());
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

bool UToShaderSubsystem::Tick(float DeltaTime)
{
	UpdateEyeBrowComponentsFromMap();
	return true;
}
