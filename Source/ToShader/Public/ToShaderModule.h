// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToShaderSubsystem.h"
#include "Components/ActorComponent.h"
#include "ToShaderModule.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOSHADER_API UToShaderModule : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="EyeBrow")
	FName EyeBrowTag = TEXT("EyeBrow");
	
	UToShaderModule();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void PostInitProperties() override;

private:
	UToShaderSubsystem* GetSubsystem();

	void InitEyeBrow();
	UPROPERTY()
	TArray<UPrimitiveComponent*> EyeBrowTargets;
	
};
