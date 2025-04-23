#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MaterialEffect.generated.h"

#pragma region MaterialEffect Structs

class UMaterialEffectLib;

UENUM(BlueprintType)
enum class EMPType : uint8
{
	Key,Float,Float4,Texture,
};

//可选键名存储的地方
USTRUCT(BlueprintType)
struct FMPTableProp : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	//使用RawName作为Name
	// UPROPERTY(EditAnywhere,BlueprintReadWrite)
	// FName Name;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	EMPType Type = EMPType::Float;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int CustomPrimitiveDataIndex;
};

struct FMPTableRow
{
	FName RowName;
	FMPTableProp* Prop;
};

USTRUCT(BlueprintType)
struct FMPKey
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly,meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Key"))
	FName Name;
	UPROPERTY(EditAnywhere)
	bool bIsEnabled;
};

USTRUCT(BlueprintType)
struct FMPFloat
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly,meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float"))
	FName Name;
	UPROPERTY(EditAnywhere)
	float Val;
};

USTRUCT(BlueprintType)
struct FMPFloatCurve
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly,meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float"))
	FName Name;
	UPROPERTY(EditAnywhere)
	UCurveFloat* Curve;
};

USTRUCT(BlueprintType)
struct FMPVector
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly,meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float4"))
	FName Name;
	UPROPERTY(EditAnywhere)
	FVector Val;
};

USTRUCT(BlueprintType)
struct FMPColor
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly,meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float4"))
	FName Name;
	UPROPERTY(EditAnywhere)
	FLinearColor Val;
};

USTRUCT(BlueprintType)
struct FMPColorCurve
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly,meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float4"))
	FName Name;
	UPROPERTY(EditAnywhere)
	UCurveLinearColor* Curve;
};

USTRUCT(BlueprintType)
struct FMPTexture
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly,meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Texture"))
	FName Name;
	UPROPERTY(EditAnywhere)
	UTexture* Tex = nullptr;
};

UCLASS(Blueprintable)
class TOSHADER_API UEffectData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FName EffectName;
	UPROPERTY(EditDefaultsOnly)
	float Duration;
	UPROPERTY(EditDefaultsOnly)
	bool bIsLoop;

	UPROPERTY(EditDefaultsOnly)
	bool bUseEffectMeshTags;
	UPROPERTY(EditDefaultsOnly,meta=(EditCondition="bUseEffectMeshTags",EditConditionHides))
	TSet<FString> MeshTags;

	UPROPERTY(EditDefaultsOnly)
	TArray<FMPKey> Keys;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMPFloat> Floats;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMPFloat> FloatCurves;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMPVector> Vectors;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMPColor> Colors;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMPColorCurve> ColorCurves;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMPTexture> Textures;

protected:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
};

#pragma endregion

UCLASS()
class TOSHADER_API UMaterialEffectLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(CallInEditor,BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Key();
	UFUNCTION(CallInEditor,BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Float();
	UFUNCTION(CallInEditor,BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Float4();
	UFUNCTION(CallInEditor,BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Texture();
	
};

