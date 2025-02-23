#include "ToShaderSubsystem.h"

#define tolog FToShaderHelpers::log

UToShaderSubsystem::UToShaderSubsystem()
{
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
	return true;
}
