#include "MaterialEffect.h"


#include "ToShader.h"
#include "ToShaderSubsystem.h"
#include "Curves/CurveLinearColor.h"
#include "Editor/MaterialEditor/Public/MaterialEditingLibrary.h"

void UEffectDataAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UToShaderSubsystem::GetSubsystem()->CallUpdate_MaterialEffectPropertyTable();
}

FMPDGroup UMaterialEffectLib::Log_CannotFindInDataTable(FName Name,bool& bIsValid)
{
	tolog("Can't find in MPDataTable : "+Name.ToString());
	bIsValid = false;
	return FMPDGroup();
}

FMPDGroup UMaterialEffectLib::MakeMPDGroup(UEffectDataAsset* Asset, bool& bIsValid)
{
	bIsValid = false;
	if (!Asset) return {};

	FMPDGroup MPD;

	if (!Asset->Keys.IsEmpty())
	{
		for (const auto K : Asset->Keys)
		{
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Key);
			if (!R)
			{
				return Log_CannotFindInDataTable(K.Name, bIsValid);
			}
			FMPDKey Key{K.Name, R->CustomPrimitiveDataIndex, true};
			MPD.Floats.Emplace(Key, 1);
		}
	}
	if (!Asset->Floats.IsEmpty())
	{
		for (const auto K : Asset->Floats)
		{
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float);
			if (!R)
			{
				return Log_CannotFindInDataTable(K.Name, bIsValid);
			}
			FMPDKey Key{K.Name, R->CustomPrimitiveDataIndex, K.bExeImmediately};
			MPD.Floats.Emplace(Key, K.Val);
		}
	}
	if (!Asset->FloatCurves.IsEmpty())
	{
		for (const auto K : Asset->FloatCurves)
		{
			if (!K.Curve) continue;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float);
			if (!R)
			{
				return Log_CannotFindInDataTable(K.Name, bIsValid);
			}
			FMPDKey Key{K.Name, R->CustomPrimitiveDataIndex};
			MPD.Floats.Emplace(Key, K.Curve->GetFloatValue(0) * K.CurveScale);
		}
	}
	if (!Asset->Vectors.IsEmpty())
	{
		for (const auto K : Asset->Vectors)
		{
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float3);
			if (!R)
			{
				return Log_CannotFindInDataTable(K.Name, bIsValid);
			}
			FMPDKey Key{K.Name, R->CustomPrimitiveDataIndex, K.bExeImmediately};
			MPD.Float3s.Emplace(Key, FVector4f(K.Val.X, K.Val.Y, K.Val.Z, 1));
		}
	}
	if (!Asset->Colors.IsEmpty())
	{
		for (const auto K : Asset->Colors)
		{
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float3);
			if (!R)
			{
				return Log_CannotFindInDataTable(K.Name, bIsValid);
			}
			FMPDKey Key{K.Name, R->CustomPrimitiveDataIndex, K.bExeImmediately};
			MPD.Float3s.Emplace(Key, FVector4f(K.Val.R, K.Val.G, K.Val.B, K.Val.A));
		}
	}
	if (!Asset->ColorCurves.IsEmpty())
	{
		for (const auto K : Asset->ColorCurves)
		{
			if (!K.Curve) continue;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float3);
			if (!R)
			{
				return Log_CannotFindInDataTable(K.Name, bIsValid);
			}
			FMPDKey Key{K.Name, R->CustomPrimitiveDataIndex};
			auto Color = K.Curve->GetLinearColorValue(0);
			MPD.Float3s.Emplace(Key, FVector4f(Color.R, Color.G, Color.B, Color.A) * K.CurveScale);
		}
	}
	if (!Asset->Textures.IsEmpty())
	{
		for (const auto K : Asset->Textures)
		{
			if (!K.Tex) continue;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Texture);
			if (!R)
			{
				return Log_CannotFindInDataTable(K.Name, bIsValid);
			}
			FMPDKey Key{K.Name, R->CustomPrimitiveDataIndex};
			MPD.Textures.Emplace(Key, K.Tex);
		}
	}

	if (!(MPD.Floats.IsEmpty() && MPD.Float3s.IsEmpty() && MPD.Textures.IsEmpty())) bIsValid = true;
	return MPD;
}

void UMaterialEffectLib::SortEffectDataMap(TMap<UEffectDataAsset*, FEffectData>& M)
{
	M.KeySort([](const UEffectDataAsset& A, const UEffectDataAsset& B)
	{
		if (A.EffectPriority != B.EffectPriority)
		{
			return A.EffectPriority > B.EffectPriority;
		}
		return A.Counter < B.Counter;//先加入优先级越高
	});
	// tolog("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv");
	// for (auto Element : M)
	// {
	// 	tolog(Element.Key->EffectName.ToString());
	// }
	// tolog("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}

void UMaterialEffectLib::CombineMPD(FMPDGroup& CurMPD, const FMPDGroup& EffectMPD)
{
	//！！！此处进入的数据应当是已经排过序了的，如果CurMPD已有记录，说明已被最高优先级的修改了，就不需要改了
	if (!EffectMPD.Floats.IsEmpty())
	{
		for (auto E : EffectMPD.Floats)
		{
			if (!CurMPD.Floats.Contains(E.Key))
			{
				CurMPD.Floats.Emplace(E.Key, E.Value);
			}
		}
	}
	if (!EffectMPD.Float3s.IsEmpty())
	{
		for (auto E : EffectMPD.Float3s)
		{
			if (!CurMPD.Float3s.Contains(E.Key))
			{
				CurMPD.Float3s.Emplace(E.Key, E.Value);
			}
		}
	}
	if (!EffectMPD.Textures.IsEmpty())
	{
		for (auto E : EffectMPD.Textures)
		{
			if (!CurMPD.Textures.Contains(E.Key))
			{
				CurMPD.Textures.Emplace(E.Key, E.Value);
			}
		}
	}
}

void UMaterialEffectLib::CacheLastMPDGroupProp(UPrimitiveComponent* Mesh, FMPDGroup& LastGroup, const FEffectData& InNewEffect)
{
	if (!Mesh) return;
	//tolog("LastGroupFloat Num ",LastGroup.Floats.Num());
	if (!InNewEffect.Group.Floats.IsEmpty())
	{
		for (auto Element : InNewEffect.Group.Floats)
		{
			if (LastGroup.Floats.Contains(Element.Key)) continue;
			bool bIsValid = false;
			float Ori;
			if (Element.Key.CustomPrimitiveIndex != -1)
			{
				if (Mesh->GetCustomPrimitiveData().Data.IsValidIndex(Element.Key.CustomPrimitiveIndex))
				{
					bIsValid = true;
					Ori = Mesh->GetCustomPrimitiveData().Data[Element.Key.CustomPrimitiveIndex];
				}
			}
			else
			{
				if (const auto M = Mesh->GetMaterial(0))
				{
					if (M->GetScalarParameterValue(FMaterialParameterInfo(Element.Key.Name), Ori))
					{
						bIsValid = true;
					}
				}
			}
			FMPDKey NewOriKey = Element.Key;
			if (bIsValid)
				LastGroup.Floats.Emplace(NewOriKey, Ori);
			else
				LastGroup.Floats.Emplace(NewOriKey, 0);
		}
	}
	if (!InNewEffect.Group.Float3s.IsEmpty())
	{
		for (auto Element : InNewEffect.Group.Float3s)
		{
			if (LastGroup.Float3s.Contains(Element.Key)) continue;
			bool bIsValid = false;
			FLinearColor Ori = FLinearColor::White;
			if (Element.Key.CustomPrimitiveIndex != -1)
			{
				if (Mesh->GetCustomPrimitiveData().Data.IsValidIndex(Element.Key.CustomPrimitiveIndex + 3))
				{
					bIsValid = true;
					Ori = FLinearColor(
						Mesh->GetCustomPrimitiveData().Data[Element.Key.CustomPrimitiveIndex],
						Mesh->GetCustomPrimitiveData().Data[Element.Key.CustomPrimitiveIndex + 1],
						Mesh->GetCustomPrimitiveData().Data[Element.Key.CustomPrimitiveIndex + 2]);
				}
			}
			else
			{
				if (const auto M = Mesh->GetMaterial(0))
				{
					if (M->GetVectorParameterValue(FMaterialParameterInfo(Element.Key.Name), Ori))
					{
						bIsValid = true;
					}
				}
			}
			FMPDKey NewOriKey = Element.Key;
			if (bIsValid)
				LastGroup.Float3s.Emplace(NewOriKey, FVector(Ori.R, Ori.G, Ori.B));
			else
				LastGroup.Float3s.Emplace(NewOriKey, FVector::Zero());
		}
	}
	if (!InNewEffect.Group.Textures.IsEmpty())
	{
		for (auto Element : InNewEffect.Group.Textures)
		{
			if (LastGroup.Textures.Contains(Element.Key)) continue;
			bool bIsValid = false;
			UTexture* Ori;
			if (const auto M = Mesh->GetMaterial(0))
			{
				if (M->GetTextureParameterValue(FMaterialParameterInfo(Element.Key.Name), Ori))
				{
					bIsValid = Ori != nullptr;
				}
			}
			FMPDKey NewOriKey = Element.Key;
			if (bIsValid)
				LastGroup.Textures.Emplace(NewOriKey, Ori);
			else
				LastGroup.Textures.Emplace(NewOriKey, nullptr);
		}
	}
}

FEffectData UMaterialEffectLib::MakeEffectData(UEffectDataAsset* Asset, bool& bIsValid)
{
	FEffectData Data;
	Data.Timer = 0;
	Data.Group = MakeMPDGroup(Asset, bIsValid);
	return Data;
}

bool UMaterialEffectLib::IsEffectDataEnd(const float Dt, const UEffectDataAsset* Asset, const FEffectData& Data)
{
	const float t = Data.Timer + Dt;
	if (!Asset->bIsLoop)
	{
		if (t > Asset->Duration) return true;
	}
	return false;
}

void UMaterialEffectLib::UpdateMPDGroup(UEffectDataAsset* Asset, FEffectData& Data)
{
	const float T = FMath::Fmod(Data.Timer, Asset->Duration);
	const float NormalizedT = T / Asset->Duration;
	if (!Asset->FloatCurves.IsEmpty())
	{
		for (const auto K : Asset->FloatCurves)
		{
			if (!K.Curve) continue;
			FMPDKey Key{K.Name};
			Data.Group.Floats[Key] = K.Curve->GetFloatValue(K.bUseNormalizedDuration ? NormalizedT : T) * K.CurveScale;
		}
	}
	if (!Asset->ColorCurves.IsEmpty())
	{
		for (const auto K : Asset->ColorCurves)
		{
			if (!K.Curve) continue;
			FMPDKey Key{K.Name};
			auto Color = K.Curve->GetLinearColorValue(K.bUseNormalizedDuration ? NormalizedT : T);
			Data.Group.Float3s[Key] = FVector(Color.R, Color.G, Color.B) * FVector(K.CurveScale.R,K.CurveScale.G,K.CurveScale.B);
		}
	}
}


void UMaterialEffectLib::UpdateEffectData(const float Dt, UEffectDataAsset* Asset, FEffectData& Data)
{
	Data.Timer += Dt;
	//更新curve
	UpdateMPDGroup(Asset, Data);
}


TArray<FName> UMaterialEffectLib::MaterialEffect_GetValidName_Key()
{
	return UToShaderSubsystem::GetSubsystem()->GetMaterialEffectPropertyTableRowNames(EMPType::Key);
}

TArray<FName> UMaterialEffectLib::MaterialEffect_GetValidName_Float()
{
	return UToShaderSubsystem::GetSubsystem()->GetMaterialEffectPropertyTableRowNames(EMPType::Float);
}

TArray<FName> UMaterialEffectLib::MaterialEffect_GetValidName_Float4()
{
	return UToShaderSubsystem::GetSubsystem()->GetMaterialEffectPropertyTableRowNames(EMPType::Float3);
}

TArray<FName> UMaterialEffectLib::MaterialEffect_GetValidName_Texture()
{
	return UToShaderSubsystem::GetSubsystem()->GetMaterialEffectPropertyTableRowNames(EMPType::Texture);
}

TArray<FName> UMaterialEffectLib::MaterialEffect_GetValidName_Tag()
{
	return UToShaderSubsystem::GetSubsystem()->GetMaterialEffectActionScopeTagNames();
}

EMaterialParamType UMaterialEffectLib::GetMaterialParamType(UMaterialInterface* M, FName Name)
{
	if (M == nullptr) return EMaterialParamType::None;
	TArray<FName> ScalarNames;
	UMaterialEditingLibrary::GetScalarParameterNames(M,ScalarNames);
	if (ScalarNames.Contains(Name)) return EMaterialParamType::Scalar;
	TArray<FName> VectorNames;
	UMaterialEditingLibrary::GetVectorParameterNames(M,VectorNames);
	if (ScalarNames.Contains(Name)) return EMaterialParamType::Vector;
	TArray<FName> TextureNames;
	UMaterialEditingLibrary::GetTextureParameterNames(M,TextureNames);
	if (ScalarNames.Contains(Name)) return EMaterialParamType::Texture;
	TArray<FName> SwitchNames;
	UMaterialEditingLibrary::GetStaticSwitchParameterNames(M,SwitchNames);
	if (ScalarNames.Contains(Name)) return EMaterialParamType::StaticSwitch;
	return EMaterialParamType::None;
}
