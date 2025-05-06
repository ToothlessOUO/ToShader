#include "ToShaderComponent.h"

#include "ToShader.h"

#pragma region ToShaderComponent
UToShaderComponent::UToShaderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UToShaderComponent::ApplyNewEffect(UEffectDataAsset* NewEffect)
{
	if (!NewEffect || !bHasBeginPlay) return;
	if (NewEffect->ActionScope > EMaterialEffectActionScope::Outline) return;
	const int ASID = (int)NewEffect->ActionScope;
	const FName ASName = GetSubsystem()->GetMaterialEffectTag()[ASID];
	if (!MeshTags.Contains(ASName)) return; //所有需要的Mesh请务必提前在Actor中添加并设置好，避免运行时开销
	const auto Tag = ASName;
	if (!MaterialEffectData.Contains(Tag))
	{
		MaterialEffectData.Emplace(Tag);
		LastMPD.Emplace(Tag);
		MPDCounter.Emplace(Tag);
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
	MaterialEffectData[Tag].Emplace(NewEffect, NewEffectData);
	//将新元素加载进LastGroup
	UMaterialEffectLib::CacheLastMPDGroupProp(GetFirstMeshTag(Tag), LastMPD[Tag], NewEffectData);
	//整理优先级
	UMaterialEffectLib::SortEffectDataMap(MaterialEffectData[Tag]);
}

//Game & PIE
void UToShaderComponent::BeginPlay()
{
	Super::BeginPlay();
	bHasBeginPlay = true;
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
	TArray<UMeshComponent*> M;
	GetOwner()->GetComponents<UMeshComponent>(M);
	for (auto Element : M)
	{
		if (!Element) continue;
		Meshes.Components.Emplace(Element);
		if (bHasBeginPlay)
		{
			Meshes.MeshDyMaterial.Emplace(Element);
			Meshes.MeshDyMaterial[Element] = UToShaderHelpers::makeAndApplyMeshMaterialsDynamic(Element);
			if (Element->OverlayMaterial != nullptr)
			{
				Meshes.OverlayMaterials.Emplace(Element);
				Meshes.OverlayMaterials[Element] = UToShaderHelpers::makeAndApplyMeshOverlayMaterialDynamic(Element);
			}
		}
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

void UToShaderComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateMaterialEffect(DeltaTime);
}

void UToShaderComponent::UpdateMaterialEffect(float Dt)
{
	for (auto& Data : MaterialEffectData)
	{
		auto CurModifyTag = Data.Key;
		FMPDGroup CurMPD;
		TArray<UEffectDataAsset*> RemoveList;
		for (auto D : Data.Value)
		{
			if (UMaterialEffectLib::IsEffectDataEnd(Dt, D.Key, D.Value))
			{
				for (auto E : D.Key->Keys) //结束时关闭key
				{
					if (auto Prop = UToShaderSubsystem::GetSubsystem()->GetMP(E.Name, EMPType::Key);
						Prop->CustomPrimitiveDataIndex != -1)
					{
						for (auto Mesh : MeshTags[CurModifyTag].Components)
						{
							Mesh->SetCustomPrimitiveDataFloat(Prop->CustomPrimitiveDataIndex, 0);
						}
					}
					else
					{
						for (auto Mesh : MeshTags[CurModifyTag].Components)
						{
							UToShaderHelpers::setDynamicMaterialGroupFloatParam(E.Name, 0, Meshes.MeshDyMaterial[Mesh]);
						}
					}
					//修改LastGroup
					LastMPD[CurModifyTag].Floats[FMPDKey(E.Name)] = 0;
				}
				RemoveList.Emplace(D.Key);
			}
			if (D.Key == nullptr) RemoveList.Emplace(nullptr);
		}
		if (!RemoveList.IsEmpty())
		{
			for (auto E : RemoveList)
			{
				Data.Value.Remove(E);
			}
			UMaterialEffectLib::SortEffectDataMap(Data.Value); //更新排序
		}
		if (Data.Value.IsEmpty()) continue;
		for (auto& E : Data.Value)
		{
			UMaterialEffectLib::UpdateEffectData(Dt, E.Key, E.Value);
			UMaterialEffectLib::CombineMPD(CurMPD, E.Value.Group);
		}
		//遍历last并作比较同时 更新Last 修改数据
		for (auto& E : LastMPD[CurModifyTag].Floats)
		{
			float TargetVal;
			if (CurMPD.Floats.Contains(E.Key))
				TargetVal = CurMPD.Floats[E.Key];
			else
				TargetVal = 0; //CustomPrimitiveData的默认值都是0
			if (!FMath::IsNearlyEqual(E.Value, TargetVal))
			{
				//tolog(E.Key.Name.ToString() + " : ", E.Value);
				if (E.Key.bExeImmediately) //直接设置val
					E.Value = TargetVal;
				else
					E.Value = FMath::Lerp(E.Value, TargetVal, 0.1f);

				if (E.Key.CustomPrimitiveIndex != -1)
				{
					for (auto Mesh : MeshTags[CurModifyTag].Components)
					{
						Mesh->SetCustomPrimitiveDataFloat(E.Key.CustomPrimitiveIndex, E.Value);
					}
				}
				else // 说明不是存在CustomPrimitiveData里的，而是PerMaterial数据
				{
					for (auto Mesh : MeshTags[CurModifyTag].Components)
					{
						UToShaderHelpers::setDynamicMaterialGroupFloatParam(E.Key.Name, E.Value, Meshes.MeshDyMaterial[Mesh]);
						//设置覆层参数
						if (Mesh->OverlayMaterial == nullptr) continue;
						if (!Meshes.OverlayMaterials.Contains(Mesh)) continue;
						Meshes.OverlayMaterials[Mesh]->SetScalarParameterValue(E.Key.Name, E.Value);
					}
				}
			}
		}
		for (auto& E : LastMPD[CurModifyTag].Float3s)
		{
			FVector TargetVal;
			if (CurMPD.Float3s.Contains(E.Key))
				TargetVal = CurMPD.Float3s[E.Key];
			else
				TargetVal = FVector::Zero();
			if (!(FMath::IsNearlyEqual(E.Value.X, TargetVal.X)
				&& FMath::IsNearlyEqual(E.Value.Y, TargetVal.Y)
				&& FMath::IsNearlyEqual(E.Value.Z, TargetVal.Z)))
			{
				if (E.Key.bExeImmediately)
					E.Value = TargetVal;
				else
					E.Value = FMath::Lerp(E.Value, TargetVal, 0.1f);

				if (E.Key.CustomPrimitiveIndex != -1)
				{
					for (auto Mesh : MeshTags[CurModifyTag].Components)
					{
						Mesh->SetCustomPrimitiveDataVector3(E.Key.CustomPrimitiveIndex, FVector(E.Value.X, E.Value.Y, E.Value.Z));
					}
				}
				else
				{
					for (auto Mesh : MeshTags[CurModifyTag].Components)
					{
						UToShaderHelpers::setDynamicMaterialGroupFloat3Param(E.Key.Name, E.Value, Meshes.MeshDyMaterial[Mesh]);

						if (Mesh->OverlayMaterial == nullptr) continue;
						if (!Meshes.OverlayMaterials.Contains(Mesh)) continue;
						Meshes.OverlayMaterials[Mesh]->SetVectorParameterValue(E.Key.Name, E.Value);
					}
				}
			}
		}
		for (auto& E : LastMPD[CurModifyTag].Textures)
		{
			UTexture* TargetVal;
			if (CurMPD.Textures.Contains(E.Key))
				TargetVal = CurMPD.Textures[E.Key];
			else
				TargetVal = nullptr;
			if (E.Value != TargetVal)
			{
				E.Value = TargetVal;
				for (auto Mesh : MeshTags[CurModifyTag].Components)
				{
					UToShaderHelpers::setDynamicMaterialGroupTextureParam(E.Key.Name, E.Value, Meshes.MeshDyMaterial[Mesh]);

					if (Mesh->OverlayMaterial == nullptr) continue;
					if (!Meshes.OverlayMaterials.Contains(Mesh)) continue;
					Meshes.OverlayMaterials[Mesh]->SetTextureParameterValue(E.Key.Name, E.Value);
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

void UToAlwaysTickComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsOwnerImplementInterface)
	{
		IAlwaysTick::Execute_OnAlwaysTick(GetOwner(), DeltaTime);
		//tolog("AlwaysTickComponent::TickComponent");
	}
}

#pragma endregion AlwaysTickComponent
