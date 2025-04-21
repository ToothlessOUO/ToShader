// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToShaderSubsystem.h"
#include "Components/ActorComponent.h"
#include "ToShaderComponent.generated.h"

#pragma region Interfaces
// This class does not need to be modified.
UINTERFACE()
class UAlwaysTick : public UInterface
{
	GENERATED_BODY()
};

class TOSHADER_API IAlwaysTick
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnAlwaysTick(float Dt);
};
#pragma endregion

#pragma region MaterialEffect

USTRUCT(BlueprintType)
struct FMaterialParamKey
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Name;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bIsEnabled;
};

USTRUCT(BlueprintType)
struct FMaterialParamFloat
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly)
	FString Name;
	UPROPERTY(EditDefaultsOnly)
	float Val;
};

USTRUCT(BlueprintType)
struct FMaterialParamFloat4
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly)
	FString Name;
	UPROPERTY(EditDefaultsOnly)
	FVector4 Val;
};

USTRUCT(BlueprintType)
struct FMaterialParamTexture
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly)
	FString Name;
	UPROPERTY(EditDefaultsOnly)
	UTexture* Tex = nullptr;
};

UCLASS()
class TOSHADER_API UEffectData : public UDataAsset{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
	FName EffectName;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMaterialParamKey>		KeyParams;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMaterialParamFloat>		FloatParams;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMaterialParamFloat4>	Float4Params;
	UPROPERTY(EditDefaultsOnly)
	TArray<FMaterialParamTexture>	TextureParams;
};
#pragma endregion

#pragma region ToShaderComponent
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOSHADER_API UToShaderComponent : public UActorComponent,public IAlwaysTick
{
	GENERATED_BODY()

public:
	
	UToShaderComponent();

	UPROPERTY()
	TMap<ERendererTag,FMeshGroup> RendererGroup;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PostInitProperties() override;
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	
	void CollectTargetsAndCallSubsystem();
	

private:
	UToShaderSubsystem* GetSubsystem();

	
};

#pragma endregion

#pragma region AlwaysTickComponent

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOSHADER_API UToAlwaysTickComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UToAlwaysTickComponent();
protected:
	virtual void PostInitProperties() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool bIsOwnerImplementInterface = false;
};

#pragma endregion
