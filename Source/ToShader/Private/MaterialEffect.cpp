#include "MaterialEffect.h"

#include "ToShaderSubsystem.h"
#include "Curves/CurveLinearColor.h"

void UEffectDataAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UToShaderSubsystem::GetSubsystem()->CallUpdate_MaterialEffectPropertyTable();
}

FMPDGroup UMaterialEffectLib::MakeMPDGroup(UEffectDataAsset* Asset, const FEffectData& Data,bool& bIsValid)
{
	bIsValid = false;
	if (!Asset) return {};

	FMPDGroup MPD;

	float t = Data.Timer - FMath::Fmod(Data.Timer, Asset->Duration);

	if (!Asset->Keys.IsEmpty())
	{
		for (const auto K : Asset->Keys)
		{
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Key);
			FMPDKey Key{K.Name, Asset, R->CustomPrimitiveDataIndex};
			MPD.Floats.Emplace(Key, K.bIsEnabled ? 1 : 0);
		}
	}
	if (!Asset->Floats.IsEmpty())
	{
		for (const auto K : Asset->Floats)
		{
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float);
			FMPDKey Key{K.Name, Asset, R->CustomPrimitiveDataIndex};
			MPD.Floats.Emplace(Key, K.Val);
		}
	}
	if (!Asset->FloatCurves.IsEmpty())
	{
		for (const auto K : Asset->FloatCurves)
		{
			if (!K.Curve) continue;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float);
			FMPDKey Key{K.Name, Asset, R->CustomPrimitiveDataIndex};
			MPD.Floats.Emplace(Key, K.Curve->GetFloatValue(t));
		}
	}
	if (!Asset->Vectors.IsEmpty())
	{
		for (const auto K : Asset->Vectors)
		{
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float4);
			FMPDKey Key{K.Name, Asset, R->CustomPrimitiveDataIndex};
			MPD.Float4s.Emplace(Key, K.Val);
		}
	}
	if (!Asset->Colors.IsEmpty())
	{
		for (const auto K : Asset->Colors)
		{
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float4);
			FMPDKey Key{K.Name, Asset, R->CustomPrimitiveDataIndex};
			MPD.Float4s.Emplace(Key, K.Val);
		}
	}
	if (!Asset->ColorCurves.IsEmpty())
	{
		for (const auto K : Asset->ColorCurves)
		{
			if (!K.Curve) continue;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Float4);
			FMPDKey Key{K.Name, Asset, R->CustomPrimitiveDataIndex};
			MPD.Float4s.Emplace(Key, K.Curve->GetLinearColorValue(t));
		}
	}
	if (!Asset->Textures.IsEmpty())
	{
		for (const auto K : Asset->Textures)
		{
			if (!K.Tex) continue;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name, EMPType::Texture);
			FMPDKey Key{K.Name, Asset, R->CustomPrimitiveDataIndex};
			MPD.Textures.Emplace(Key, K.Tex);
		}
	}

	if (!(MPD.Floats.IsEmpty()&&MPD.Float4s.IsEmpty()&&MPD.Textures.IsEmpty())) bIsValid = true;
	return MPD;
}

bool UMaterialEffectLib::UpdateEffectData(float Dt, UEffectDataAsset* Asset, FEffectData& Data)
{
	Data.Timer += Dt;
	if (!Asset->bIsLoop)
	{
		if (Data.Timer > Asset->Duration) return false;
	}
	bool bSuccess = false;
	Data.Group = MakeMPDGroup(Asset,Data,bSuccess);
	return bSuccess;
}

void UMaterialEffectLib::CombineMPD(FMPDGroup& CurMPD, const FMPDGroup& EffectMPD)
{
	//此处进入的数据已经排过序了，如果CurMPD已有记录，说明已被最高优先级的修改了，就不需要改了
	if (!EffectMPD.Floats.IsEmpty())
	{
		for (auto E : EffectMPD.Floats)
		{
			if (!CurMPD.Floats.Contains(E.Key))
			{
				CurMPD.Floats.Emplace(E);
				break;
			}
		}
	}
	if (!EffectMPD.Float4s.IsEmpty())
	{
		for (auto E : EffectMPD.Float4s)
		{
			if (!CurMPD.Float4s.Contains(E.Key))
			{
				CurMPD.Float4s.Emplace(E);
				break;
			}
		}
	}
	if (!EffectMPD.Textures.IsEmpty())
	{
		for (auto E : EffectMPD.Textures)
		{
			if (!CurMPD.Textures.Contains(E.Key))
			{
				CurMPD.Textures.Emplace(E);
				break;
			}
		}
	}
}

void UMaterialEffectLib::UpdateOriMPDGroup(UPrimitiveComponent* Mesh, FMPDGroup& OriGroup,FMPDGroup& LastGroup, const FEffectData& InNewEffect)
{
	if (!Mesh) return;
	if (!InNewEffect.Group.Floats.IsEmpty())
	{
		for (auto Element : InNewEffect.Group.Floats)
		{
			if (OriGroup.Floats.Contains(Element.Key)) continue;
			bool bIsValid = false;
			float Ori;
			if (Element.Key.CustomPrimitiveIndex!=-1)
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
					FMaterialParameterInfo ParamInfo(Element.Key.Name);
					if (M->GetScalarParameterValue(ParamInfo,Ori))
					{
						bIsValid = true;
					}
				}
			}
			if (bIsValid)
			{
				FMPDKey NewOriKey = Element.Key;
				NewOriKey.Modifier = nullptr;
				OriGroup.Floats.Emplace(NewOriKey, Ori);
				LastGroup.Floats.Emplace(NewOriKey, Ori);
			}
		}
	}
	if (!InNewEffect.Group.Float4s.IsEmpty())
	{
		for (auto Element : InNewEffect.Group.Float4s)
		{
			if (OriGroup.Float4s.Contains(Element.Key)) continue;
			bool bIsValid = false;
			FLinearColor Ori;
			if (Element.Key.CustomPrimitiveIndex!=-1)
			{
				if (Mesh->GetCustomPrimitiveData().Data.IsValidIndex(Element.Key.CustomPrimitiveIndex+3))
				{
					bIsValid = true;
					Ori = FLinearColor(
						Mesh->GetCustomPrimitiveData().Data[Element.Key.CustomPrimitiveIndex],
						Mesh->GetCustomPrimitiveData().Data[Element.Key.CustomPrimitiveIndex+1],
						Mesh->GetCustomPrimitiveData().Data[Element.Key.CustomPrimitiveIndex+2],
						Mesh->GetCustomPrimitiveData().Data[Element.Key.CustomPrimitiveIndex+3]);
				}
			}
			else
			{
				if (const auto M = Mesh->GetMaterial(0))
				{
					FMaterialParameterInfo ParamInfo(Element.Key.Name);
					if (M->GetVectorParameterValue(ParamInfo,Ori))
					{
						bIsValid = true;
					}
				}
			}
			if (bIsValid)
			{
				FMPDKey NewOriKey = Element.Key;
				NewOriKey.Modifier = nullptr;
				OriGroup.Float4s.Emplace(NewOriKey, FVector4(Ori.R,Ori.G,Ori.B,Ori.A));
				LastGroup.Float4s.Emplace(NewOriKey, FVector4(Ori.R,Ori.G,Ori.B,Ori.A));
			}
		}
	}
	if (!InNewEffect.Group.Textures.IsEmpty())
	{
		for (auto Element : InNewEffect.Group.Textures)
		{
			if (OriGroup.Textures.Contains(Element.Key)) continue;
			bool bIsValid = false;
			UTexture* Ori;
			if (const auto M = Mesh->GetMaterial(0))
			{
				FMaterialParameterInfo ParamInfo(Element.Key.Name);
				if (M->GetTextureParameterValue(ParamInfo,Ori))
				{
					bIsValid = Ori!=nullptr;
				}
			}
			if (bIsValid)
			{
				FMPDKey NewOriKey = Element.Key;
				NewOriKey.Modifier = nullptr;
				OriGroup.Float4s.Emplace(NewOriKey, Ori);
				LastGroup.Float4s.Emplace(NewOriKey, Ori);
			}
		}
	}
}

FEffectData UMaterialEffectLib::MakeEffectData(UEffectDataAsset* Asset,bool& bIsValid)
{
	FEffectData Data;
	Data.Timer = 0;
	Data.Group = MakeMPDGroup(Asset,Data,bIsValid);
	return Data;
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
	return UToShaderSubsystem::GetSubsystem()->GetMaterialEffectPropertyTableRowNames(EMPType::Float4);
}

TArray<FName> UMaterialEffectLib::MaterialEffect_GetValidName_Texture()
{
	return UToShaderSubsystem::GetSubsystem()->GetMaterialEffectPropertyTableRowNames(EMPType::Texture);
}

TArray<FName> UMaterialEffectLib::MaterialEffect_GetValidName_Tag()
{
	return UToShaderSubsystem::GetSubsystem()->MaterialEffectTag;
}
