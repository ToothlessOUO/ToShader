#include "ToShaderModule.h"

#include "ToShader.h"

#define tolog FToShaderHelpers::log

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
	CollectTargetsAndCallSubsystem();
}

UToShaderSubsystem* UToShaderModule::GetSubsystem()
{
	return GEngine->GetEngineSubsystem<UToShaderSubsystem>();
}

void UToShaderModule::CollectTargetsAndCallSubsystem()
{
	if (!GetOwner() || !GetSubsystem()) return;
	RendererGroup.Empty();
	const auto EnumPtr = StaticEnum<ERendererTag>();
	if (!EnumPtr) return;
	for (ERendererTag E : TEnumRange<ERendererTag>())
	{
		const auto TagName = FName(EnumPtr->GetNameStringByValue(static_cast<int64>(E)));
		auto Components = GetOwner()->GetComponentsByTag(UPrimitiveComponent::StaticClass(),TagName);
		if (Components.IsEmpty())
		{
			continue;
		}
		FMeshGroup Group;   
		for (const auto Component : Components)
		{
			if (auto P = Cast<UPrimitiveComponent>(Component))
			{
				Group.Components.Add(P);
			}
		}
		if (Group.Components.IsEmpty()) continue;
		RendererGroup.Emplace(E, Group);
	}
	GetSubsystem()->AddModuleToSubsystem(this);
}


void UToShaderModule::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

