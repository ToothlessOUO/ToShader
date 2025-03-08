#include "ToShaderComponent.h"

#include "ToShader.h"

#define tolog FToShaderHelpers::log

UToShaderComponent::UToShaderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UToShaderComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UToShaderComponent::PostInitProperties()
{
	Super::PostInitProperties();
	CollectTargetsAndCallSubsystem();
}

UToShaderSubsystem* UToShaderComponent::GetSubsystem()
{
	return UToShaderSubsystem::GetSubsystem();
}

void UToShaderComponent::CollectTargetsAndCallSubsystem()
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
				// if (UToShaderSubsystem::IsMeshContainsTag(P,ERendererTag::VisInCaptureOnly))
				// 	P->bVisibleInSceneCaptureOnly = true;
				Group.Components.Add(P);
			}
		}
		if (Group.Components.IsEmpty()) continue;
		RendererGroup.Emplace(E, Group);
	}
	GetSubsystem()->AddModuleToSubsystem(this);
}


void UToShaderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

