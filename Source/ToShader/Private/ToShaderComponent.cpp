#include "ToShaderComponent.h"

#include "ToShader.h"

#pragma region ToShaderComponent
UToShaderComponent::UToShaderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

//Game & PIE
void UToShaderComponent::BeginPlay()
{
	Super::BeginPlay();
	Init();
}

//Editor
void UToShaderComponent::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

void UToShaderComponent::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
	GetSubsystem()->RemoveModuleFromSubsystem(this);
}

UToShaderSubsystem* UToShaderComponent::GetSubsystem()
{
	return UToShaderSubsystem::GetSubsystem();
}

void UToShaderComponent::Init()
{
	CacheMeshTags();
	CollectTargetsAndCallSubsystem();
}

void UToShaderComponent::CacheMeshTags()
{
	Meshes.Components.Empty();
	MeshTags.Empty();
	
	if(!GetOwner()) return;
	TArray<UPrimitiveComponent*> M;
	GetOwner()->GetComponents<UPrimitiveComponent>(M);
	for (auto Element : M)
	{
		if(!Element) continue;
		Meshes.Components.Emplace(Element);
		for (auto Tag : Element->ComponentTags)
		{
			if(!MeshTags.Contains(Tag))
			{
				MeshTags.Emplace(Tag);
			}
			MeshTags[Tag].Components.Emplace(Element);
		}
	}
}

void UToShaderComponent::CollectTargetsAndCallSubsystem()
{
	if (!GetOwner() || !GetSubsystem() || MeshTags.IsEmpty()) return;
	RendererGroup.Empty();
	const auto EnumPtr = StaticEnum<ERendererTag>();
	if (!EnumPtr) return;
	for (ERendererTag E : TEnumRange<ERendererTag>())
	{
		const auto TagName = FName(EnumPtr->GetNameStringByValue(static_cast<int64>(E)));
		if(!MeshTags.Contains(TagName)) continue;
		RendererGroup.Emplace(E, MeshTags[TagName]);
	}
	GetSubsystem()->AddModuleToSubsystem(this);
}

void UToShaderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

#pragma endregion

#pragma region AlwaysTickComponent

UToAlwaysTickComponent::UToAlwaysTickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UToAlwaysTickComponent::PostInitProperties()
{
	Super::PostInitProperties();
	if (GetOwner() && GetOwner()->GetClass()->ImplementsInterface(UAlwaysTick::StaticClass()))
	{
		bIsOwnerImplementInterface = true;
	}
}

void UToAlwaysTickComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsOwnerImplementInterface)
	{
		IAlwaysTick::Execute_OnAlwaysTick(GetOwner(), DeltaTime);
		//tolog("AlwaysTickComponent::TickComponent");
	}
}

#pragma endregion AlwaysTickComponent

