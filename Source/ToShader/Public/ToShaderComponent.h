// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MaterialEffect.h"
#include "ToShaderSubsystem.h"
#include "ToStructs.h"
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

#pragma region ToShaderComponent
UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class TOSHADER_API UToShaderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UToShaderComponent();

	UPROPERTY()
	TMap<ERendererTag, FMeshGroup> RendererGroup;
	
	UFUNCTION(BlueprintCallable)
	void ApplyNewEffect(UEffectDataAsset* NewEffect);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PostInitProperties() override;
	virtual void DestroyComponent(bool bPromoteChildren = false) override;

private:
	bool bHasBeginPlay = false;
	UToShaderSubsystem* GetSubsystem();

	void Init();

	FMeshGroupDyMat Meshes;
	TMap<FName, FMeshGroup> MeshTags;
	void CacheTagMeshes();

	void CollectTargetsAndCallSubsystem();

	UPrimitiveComponent* GetFirstMeshTag(FName Tag);
	TMap<FName, TMap<UEffectDataAsset*, FEffectData>> MaterialEffectData;
	TMap<FName, FMPDGroup> LastMPD;
	TMap<FName, int> MPDCounter;
	void UpdateMaterialEffect(float Dt);
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
