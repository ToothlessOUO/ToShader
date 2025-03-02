#include "ToShaderModule.h"

UToShaderModule::UToShaderModule()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UToShaderModule::BeginPlay()
{
	Super::BeginPlay();
	
}

void UToShaderModule::PostInitProperties()
{
	Super::PostInitProperties();
	InitEyeBrow();
}

UToShaderSubsystem* UToShaderModule::GetSubsystem()
{
	return GEngine->GetEngineSubsystem<UToShaderSubsystem>();
}

void UToShaderModule::InitEyeBrow()
{
	if (!GetOwner()) return;
	auto T = GetOwner()->GetComponentsByTag(UPrimitiveComponent::StaticClass(),EyeBrowTag);
	if (T.IsEmpty()) return;
	for (const auto ActorComponent : T)
	{
		if (ActorComponent==nullptr) continue;
		auto Primitive = Cast<UPrimitiveComponent>(ActorComponent);
		if (Primitive==nullptr) continue;
		EyeBrowTargets.Add(Primitive);
	}
	auto S = GetSubsystem();
	if (EyeBrowTargets.IsEmpty() || S==nullptr) return;
	S->AddEyeBrowComponents(GetOwner(), EyeBrowTargets);
}


void UToShaderModule::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

