#pragma once

#include "ToStructs.generated.h"

USTRUCT(BlueprintType)
struct FDynamicMaterialGroup
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInstanceDynamic*> Materials;
};

USTRUCT(BlueprintType)
struct FMaterialGroup
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInterface*> Materials;
};

USTRUCT(BlueprintType)
struct FMeshGroup
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UMeshComponent*> Components;
};

USTRUCT(BlueprintType)
struct FMeshGroupDyMat
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UMeshComponent*> Components;
	UPROPERTY(BlueprintReadOnly)
	TMap<UMeshComponent*,FDynamicMaterialGroup> MeshDyMaterial;
	UPROPERTY(BlueprintReadOnly)
	TMap<UMeshComponent*,UMaterialInstanceDynamic*> OverlayMaterials;
};

USTRUCT()
struct FShowList
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<TWeakObjectPtr<UPrimitiveComponent>> List;
};

//Enum same as Tag name
UENUM(BlueprintType)
enum class ERendererTag : uint8
{
	EyeBrow,Face,Hair,Outline,Overlay,
	VisInCaptureOnly,HairMask,FaceMask,EyeBrowMask,
	ScreenOverlay,
	Max
};

UENUM(BlueprintType)
enum class EMaterialEffectActionScope : uint8
{
	Body,Weapon,Outline,
	PP_Uber,
	Max
};


UENUM(BlueprintType)
enum class ERTSizeScale : uint8
{
	Default = 0
	,Down2X = 2
	,Down4X = 4
	,Down6X = 6
};

ENUM_RANGE_BY_COUNT(ERendererTag, ERendererTag::Max);
ENUM_RANGE_BY_COUNT(EMaterialEffectActionScope, EMaterialEffectActionScope::Max);

UENUM(BlueprintType)
enum class EMaterialParamType : uint8
{
	None,Scalar,Vector,Texture,StaticSwitch
};

