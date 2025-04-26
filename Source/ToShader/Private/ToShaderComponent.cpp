#include "ToShaderComponent.h"

#include "ToShader.h"

#pragma region ToShaderComponent
UToShaderComponent::UToShaderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UToShaderComponent::ApplyNewEffect(UEffectDataAsset* NewEffect)
{
	if (!NewEffect) return;
	if (!MeshTags.Contains(NewEffect->Tag)) return;
	const auto Tag = NewEffect->Tag;
	if (!MaterialEffectData.Contains(Tag))
	{
		MaterialEffectData.Add(Tag);
		OriMPD.Add(Tag);
		LastMPD.Add(Tag);
		MPDCounter.Add(Tag);
	}
	if (MaterialEffectData[Tag].Contains(NewEffect))
	{
		if (NewEffect->bIsLoop) return;
		MaterialEffectData[Tag].Remove(NewEffect);
	}
	bool bSuccess;
	const auto NewEffectData = UMaterialEffectLib::MakeEffectData(NewEffect, bSuccess);
	if (!bSuccess) return;
	MPDCounter[Tag]++;
	NewEffect->Counter = MPDCounter[Tag];
	MaterialEffectData[Tag].Add(NewEffect, NewEffectData);
	UMaterialEffectLib::UpdateOriMPDGroup(GetFirstMeshTag(Tag), OriMPD[Tag], LastMPD[Tag], NewEffectData);
	//整理优先级
	MaterialEffectData[Tag].KeySort([](const UEffectDataAsset* A1, const UEffectDataAsset* A2)
	{
		if (A1->EffectPriority > A2->EffectPriority) return true;
		if (A1->EffectPriority == A2->EffectPriority)
		{
			if (A1->Counter > A2->Counter) return true;
		}
		return false;
	});
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

	if (!GetOwner()) return;
	TArray<UPrimitiveComponent*> M;
	GetOwner()->GetComponents<UPrimitiveComponent>(M);
	for (auto Element : M)
	{
		if (!Element) continue;
		Meshes.Components.Emplace(Element);
		for (auto Tag : Element->ComponentTags)
		{
			if (!MeshTags.Contains(Tag))
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
		if (!MeshTags.Contains(TagName)) continue;
		RendererGroup.Emplace(E, MeshTags[TagName]);
	}
	GetSubsystem()->AddModuleToSubsystem(this);
}

UPrimitiveComponent* UToShaderComponent::GetFirstMeshTag(FName Tag)
{
	if (MeshTags.Contains(Tag) && !MeshTags[Tag].Components.IsEmpty()) return MeshTags[Tag].Components[0];
	return nullptr;
}

void UToShaderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateMaterialEffect(DeltaTime);
}

void UToShaderComponent::UpdateMaterialEffect(float Dt)
{
	for (auto Data : MaterialEffectData)
	{
		//Update Data and
		FMPDGroup CurMPD;
		for (auto It = Data.Value.CreateIterator(); It; ++It)
		{
			if (!UMaterialEffectLib::UpdateEffectData(Dt, It.Key(), It.Value()))
			{
				It.RemoveCurrent(); // 安全删除当前元素
			}
			else
			{
				UMaterialEffectLib::CombineMPD(CurMPD, It.Value().Group);
			}
		}
		//遍历last并作比较同时修改数据
		for (FMPDGroup& L : LastMPD[Data.Key])
		{
			for (auto& E : L.Floats)
			{
				float TargetVal;
				if (CurMPD.Floats.Contains(E.Key))
					TargetVal = CurMPD.Floats[E.Key];
				else
					TargetVal = OriMPD[Data.Key].Floats[E.Key];
				if (!FMath::IsNearlyEqual(E.Value, TargetVal))
				{
					E.Value = FMath::Lerp(E.Value, TargetVal, 0.1f);
					if (E.Key.CustomPrimitiveIndex!=-1)
					{
						for (auto Mesh : MeshTags[Data.Key].Components)
						{
							Mesh->SetCustomPrimitiveDataFloat(E.Key.CustomPrimitiveIndex, E.Value);
						}
					}
					//ToDo:创建材质实例，支持设置PerMaterial参数
				}
			}
			for (auto& E : L.Float4s)
			{
				FVector4 TargetVal;
				if (CurMPD.Float4s.Contains(E.Key))
					TargetVal = CurMPD.Float4s[E.Key];
				else
					TargetVal = OriMPD[Data.Key].Float4s[E.Key];
				if (!(FMath::IsNearlyEqual(E.Value.X, TargetVal.X)
					&& FMath::IsNearlyEqual(E.Value.Y, TargetVal.Y)
					&& FMath::IsNearlyEqual(E.Value.Z, TargetVal.Z)
					&& FMath::IsNearlyEqual(E.Value.W, TargetVal.W)))
				{
					E.Value = FMath::Lerp(E.Value, TargetVal, 0.1f);
					if (E.Key.CustomPrimitiveIndex!=-1)
					{
						for (auto Mesh : MeshTags[Data.Key].Components)
						{
							Mesh->SetCustomPrimitiveDataVector4(E.Key.CustomPrimitiveIndex, E.Value);
						}
					}
				}
			}
			for (auto& E : L.Textures)
			{
				UTexture* TargetVal;
				if (CurMPD.Textures.Contains(E.Key))
					TargetVal = CurMPD.Textures[E.Key];
				else
					TargetVal = OriMPD[Data.Key].Textures[E.Key];
				if (!(CurMPD.Textures[E.Key] == E.Value))
				{
					E.Value = TargetVal;
				}
			}
		}
	}
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
