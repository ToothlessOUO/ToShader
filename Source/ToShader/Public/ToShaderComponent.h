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

#pragma region ToShaderComponent
UCLASS(ClassGroup=(Custom),BlueprintType,Blueprintable, meta=(BlueprintSpawnableComponent))
class TOSHADER_API UToShaderComponent : public UActorComponent
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

private:
	UToShaderSubsystem* GetSubsystem();

	void Init();

	FMeshGroup Meshes;
	TMap<FName,FMeshGroup> MeshTags;
	void CacheMeshTags();

	void CollectTargetsAndCallSubsystem();
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
