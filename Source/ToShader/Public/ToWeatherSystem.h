// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ToWeatherSystem.generated.h"

UCLASS()
class TOSHADER_API AToWeatherSystem : public AActor
{
	GENERATED_BODY()

public:
	AToWeatherSystem();

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	void Init();
	
	virtual void PostInitProperties() override;

private:
	UDirectionalLightComponent* DirectionalLight;
	USkyLightComponent* SkyLight;
	USkyAtmosphereComponent* SkyAtmosphere;
	
};
