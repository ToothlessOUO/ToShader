// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToShaderSubsystem.h"
#include "Components/ActorComponent.h"
#include "ToShaderComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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

	void CollectTargetsAndCallSubsystem();
	

private:
	UToShaderSubsystem* GetSubsystem();

	
};
