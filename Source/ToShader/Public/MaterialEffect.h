#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MaterialEffect.generated.h"

#pragma region MaterialEffect Structs

class UMaterialEffectLib;

UENUM(BlueprintType)
enum class EMPType : uint8
{
	Key,
	Float,
	Float4,
	Texture,
};

//可选键名存储的地方
USTRUCT(BlueprintType)
struct FMPTableProp : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	//使用RawName作为Name
	// UPROPERTY(EditAnywhere,BlueprintReadWrite)
	// FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMPType Type = EMPType::Float;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CustomPrimitiveDataIndex = -1;
};

USTRUCT(BlueprintType)
struct FMPKey
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Key"))
	FName Name;
	UPROPERTY(EditAnywhere)
	bool bIsEnabled;
};

USTRUCT(BlueprintType)
struct FMPFloat
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float"))
	FName Name;
	UPROPERTY(EditAnywhere)
	float Val;
};

USTRUCT(BlueprintType)
struct FMPFloatCurve
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float"))
	FName Name;
	UPROPERTY(EditAnywhere)
	UCurveFloat* Curve;
};

USTRUCT(BlueprintType)
struct FMPVector
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float4"))
	FName Name;
	UPROPERTY(EditAnywhere)
	FVector Val;
};

USTRUCT(BlueprintType)
struct FMPColor
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float4"))
	FName Name;
	UPROPERTY(EditAnywhere)
	FLinearColor Val;
};

USTRUCT(BlueprintType)
struct FMPColorCurve
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Float4"))
	FName Name;
	UPROPERTY(EditAnywhere)
	UCurveLinearColor* Curve;
};

USTRUCT(BlueprintType)
struct FMPTexture
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Texture"))
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
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="bUseEffectMeshTags", EditConditionHides))
	TSet<FString> MeshTags;

	UPROPERTY(EditDefaultsOnly)
	TArray<FMPKey> Keys;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMPFloat> Floats;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMPFloatCurve> FloatCurves;
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

	bool operator==(const UEffectData& Other) const
	{
		return Other.EffectName == EffectName;
	}
};

USTRUCT(BlueprintType)
struct FMPTemplate 
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere)
	FName Name = "";
	UPROPERTY(EditAnywhere)
	float Timer = 0;;
	UPROPERTY(EditAnywhere)
	int CustomPrimitiveDataIndex;
	UPROPERTY(EditAnywhere)
	EMPType Type;
	
	UPROPERTY(EditAnywhere)
	float FloatVal;
	UPROPERTY(EditAnywhere)
	FVector4 Float4Val;
	UPROPERTY(EditAnywhere)
	UTexture* TextureVal;
};

struct FMPData
{
	bool bIsCustomPrimitiveData = true;
	int CustomPrimitiveIndex = -1;
	
	float Data;
	UTexture* Data_Tex;
};

#pragma endregion

UCLASS()
class TOSHADER_API UMaterialEffectLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	TArray<FMPTemplate> ConstructMPFromEffectData(UEffectData* Data);

	UFUNCTION(BlueprintCallable)
	FMPTemplate ConstructOriMPFromMesh(UPrimitiveComponent* Mesh);
	
	UFUNCTION(BlueprintCallable)
	void UpdateOriMPCache(TMap<FName, FMPTemplate>& OriCache,TArray<FMPTemplate> MPFromNewEff);
	
	//call when new effect apply
	UFUNCTION(BlueprintCallable)
	void ApplyNewEffect(UPrimitiveComponent* AMeshHasEffect,TMap<FName, FMPTemplate>& OriCache, TMap<FName, FMPTemplate>& CurData,
	                    TArray<UEffectData*>& EffectList, UEffectData* NewEffect);

	//call for every effect apply
	UFUNCTION(BlueprintCallable)
	void UpdateMPData(TArray<UPrimitiveComponent*> Meshes,TMap<FName, FMPTemplate>& CurData, UEffectData* NewEffect);

	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Key();
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Float();
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Float4();
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Texture();
};
