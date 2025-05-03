#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MaterialEffect.generated.h"

class UMaterialEffectLib;
class UEffectDataAsset;

#pragma region MPDTable
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
	//是否暴露给MaterialEffect去配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExposeToMaterialEffect = true;
};
#pragma endregion

#pragma region MP
USTRUCT(BlueprintType)
struct FMPKey
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Key"))
	FName Name;
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
	float CurveScale = 1;
	UPROPERTY(EditAnywhere)
	bool bUseNormalizedDuration;
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
	FLinearColor CurveScale = FLinearColor(1,1,1,1);
	UPROPERTY(EditAnywhere)
	bool bUseNormalizedDuration;
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
#pragma endregion

#pragma region MPD
struct FMPDKey
{
	FName Name;
	int CustomPrimitiveIndex = -1;
	bool bIsKey = false;

	bool operator==(const FMPDKey& Other) const
	{
		if (Other.Name == "") return false;
		return Other.Name == Name;
	}

	friend uint32 GetTypeHash(const FMPDKey& Other)
	{
		return GetTypeHash(Other.Name);
	}
};

struct FMPDGroup
{
	TMap<FMPDKey, float>  Floats;
	TMap<FMPDKey, FVector4f> Float4s;
	TMap<FMPDKey, UTexture*> Textures;
};
#pragma endregion

#pragma region EffectData
UENUM(BlueprintType)
enum class EMaterialEffectPriority : uint8
{
	Low = 0, Normal = 1, High = 2
};

UCLASS(Blueprintable)
class TOSHADER_API UEffectDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FName EffectName;
	UPROPERTY(EditDefaultsOnly ,meta=(GetOptions="ToShader.MaterialEffectLib.MaterialEffect_GetValidName_Tag"))
	FName Tag;
	UPROPERTY(EditDefaultsOnly)
	EMaterialEffectPriority EffectPriority = EMaterialEffectPriority::Normal;
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

	//Runtime
	int Counter;//添加顺序  关乎排序，请在apply的位置维护

protected:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	bool operator==(const UEffectDataAsset& Other) const
	{
		return Other.EffectName == EffectName;
	}
};

struct FEffectData
{
	float Timer;
	FMPDGroup Group;
};
#pragma endregion

UCLASS()
class TOSHADER_API UMaterialEffectLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	static FMPDGroup MakeMPDGroup(UEffectDataAsset* Asset,bool& bIsValid);
	static void UpdateMPDGroup(UEffectDataAsset* Asset,FEffectData& Data);
public:
	static void SortEffectDataMap(TMap<UEffectDataAsset*,FEffectData>& M);
	static void CacheLastMPDGroupProp(UPrimitiveComponent* Mesh,FMPDGroup& LastGroup,const FEffectData& InNewEffect);
	//以下的bool valid一旦为false说明 Effect 不可用、已结束
	static FEffectData MakeEffectData(UEffectDataAsset* Asset,bool& bIsValid);
	static bool IsEffectDataEnd(float Dt, const UEffectDataAsset* Asset, const FEffectData& Data);
	static void UpdateEffectData(float Dt,UEffectDataAsset* Asset,FEffectData& Data);
	static void CombineMPD(FMPDGroup& CurMPD,const FMPDGroup& EffectMPD);
	
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Key();
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Float();
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Float4();
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Texture();
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> MaterialEffect_GetValidName_Tag();
};
