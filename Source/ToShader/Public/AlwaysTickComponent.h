// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AlwaysTickComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentTick,float,DeltaTime);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOSHADER_API UAlwaysTickComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAlwaysTickComponent();

	UPROPERTY(BlueprintReadOnly,VisibleAnywhere,BlueprintAssignable)
	FOnComponentTick OnComponentTick;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
