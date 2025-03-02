
#include "AlwaysTickComponent.h"
#include "ToShader.h"
#define tolog FToShaderHelpers::log

UAlwaysTickComponent::UAlwaysTickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
}


void UAlwaysTickComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UAlwaysTickComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OnComponentTick.IsBound())
	{
		tolog("Broad");
		OnComponentTick.Broadcast(DeltaTime);
	}
}


