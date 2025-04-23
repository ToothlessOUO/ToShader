#include "MaterialEffect.h"

#include "ToShaderSubsystem.h"

void UEffectData::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UToShaderSubsystem::GetSubsystem()->CallUpdate_MaterialEffectPropertyTable();
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
