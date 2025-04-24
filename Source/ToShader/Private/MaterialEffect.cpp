#include "MaterialEffect.h"

#include "ToShaderSubsystem.h"
#include "Curves/CurveLinearColor.h"

void UEffectData::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UToShaderSubsystem::GetSubsystem()->CallUpdate_MaterialEffectPropertyTable();
}

TArray<FMPTemplate> UMaterialEffectLib::ConstructMPFromEffectData(UEffectData* Data)
{
	if(!Data) return {};
	TArray<FMPTemplate> Res;
	
	if(!Data->Keys.IsEmpty())
	{
		for(const auto K:Data->Keys)
		{
			FMPTemplate t;
			t.Name = K.Name;
			t.Type = EMPType::Key;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name,t.Type);
			t.CustomPrimitiveDataIndex = R->CustomPrimitiveDataIndex;
			t.FloatVal = K.bIsEnabled?1:0;
			Res.Emplace(t);
		}
	}
	if(!Data->Floats.IsEmpty())
	{
		for(const auto K:Data->Floats)
		{
			FMPTemplate t;
			t.Name = K.Name;
			t.Type = EMPType::Float;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name,t.Type);
			t.CustomPrimitiveDataIndex = R->CustomPrimitiveDataIndex;
			t.FloatVal = K.Val;
			Res.Emplace(t);
		}
	}
	if(!Data->FloatCurves.IsEmpty())
	{
		for(const auto K:Data->FloatCurves)
		{
			if(!K.Curve) continue;
			FMPTemplate t;
			t.Name = K.Name;
			t.Type = EMPType::Float;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name,t.Type);
			t.CustomPrimitiveDataIndex = R->CustomPrimitiveDataIndex;
			t.FloatVal = K.Curve->GetFloatValue(0);
			Res.Emplace(t);
		}
	}
	if(!Data->Vectors.IsEmpty())
	{
		for(const auto K:Data->Vectors)
		{
			FMPTemplate t;
			t.Name = K.Name;
			t.Type = EMPType::Float4;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name,t.Type);
			t.CustomPrimitiveDataIndex = R->CustomPrimitiveDataIndex;
			t.Float4Val = K.Val;
			Res.Emplace(t);
		}
	}
	if(!Data->Colors.IsEmpty())
	{
		for(const auto K:Data->Colors)
		{
			FMPTemplate t;
			t.Name = K.Name;
			t.Type = EMPType::Float4;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name,t.Type);
			t.CustomPrimitiveDataIndex = R->CustomPrimitiveDataIndex;
			t.Float4Val = K.Val;
			Res.Emplace(t);
		}
	}
	if(!Data->ColorCurves.IsEmpty())
	{
		for(const auto K:Data->ColorCurves)
		{
			if(!K.Curve) continue;
			FMPTemplate t;
			t.Name = K.Name;
			t.Type = EMPType::Float4;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name,t.Type);
			t.CustomPrimitiveDataIndex = R->CustomPrimitiveDataIndex;
			t.Float4Val = K.Curve->GetLinearColorValue(0);
			Res.Emplace(t);
		}
	}
	if(!Data->Textures.IsEmpty())
	{
		for(const auto K:Data->Textures)
		{
			FMPTemplate t;
			t.Name = K.Name;
			t.Type = EMPType::Texture;
			auto R = UToShaderSubsystem::GetSubsystem()->GetMP(K.Name,t.Type);
			t.CustomPrimitiveDataIndex = R->CustomPrimitiveDataIndex;
			t.TextureVal = K.Tex;
			Res.Emplace(t);
		}
	}

	return Res;
}

FMPTemplate UMaterialEffectLib::ConstructOriMPFromMesh(UPrimitiveComponent* Mesh)
{
	FMPTemplate Ret = MPFromNewEff;
	auto  d = Mesh->GetDefaultCustomPrimitiveData();
	
}

void UMaterialEffectLib::UpdateOriMPCache(TMap<FName, FMPTemplate>& OriCache, TArray<FMPTemplate> MPFromNewEff)
{
	for (auto Element : MPFromNewEff)
	{
		if(OriCache.Contains(Element.Name)) continue;
		OriCache.Emplace(Element.Name,)
	}
}

void UMaterialEffectLib::ApplyNewEffect(UPrimitiveComponent* AMeshHasEffect,TMap<FName, FMPTemplate>& OriCache, TMap<FName, FMPTemplate>& CurData,
                                        TArray<UEffectData*>& EffectList, UEffectData* NewEffect)
{
	if(!NewEffect) return;
	if(EffectList.Contains(NewEffect))
	{
		EffectList.Remove(NewEffect);
	}
	EffectList.Emplace(NewEffect);

	//获取Effect的MPTemplate
	auto MPs = ConstructMPFromEffectData(NewEffect);
	//增量更新OriCache

	
	
}

void UMaterialEffectLib::UpdateMPData(TArray<UPrimitiveComponent*> Meshes,TMap<FName, FMPTemplate>& CurData, UEffectData* NewEffect)
{
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
