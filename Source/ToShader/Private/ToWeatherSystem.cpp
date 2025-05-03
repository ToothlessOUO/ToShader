// Fill out your copyright notice in the Description page of Project Settings.


#include "ToWeatherSystem.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"


// Sets default values
AToWeatherSystem::AToWeatherSystem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AToWeatherSystem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AToWeatherSystem::Init()
{
	TArray<AActor*> A;
	GetAllChildActors(A);
	for (const auto a : A)
	{
		if (!DirectionalLight)
		{
			a->GetComponentByClass(UDirectionalLightComponent::StaticClass());
			if (a)
			{
				DirectionalLight = Cast<UDirectionalLightComponent>(a);
			}
		}
		if (!SkyLight)
		{
			a->GetComponentByClass(USkyLightComponent::StaticClass());
			if (a)
			{
				SkyLight = Cast<USkyLightComponent>(a);
			}
		}
		if (!SkyAtmosphere)
		{
			a->GetComponentByClass(USkyAtmosphereComponent::StaticClass());
			if (a)
			{
				SkyAtmosphere = Cast<USkyAtmosphereComponent>(a);
			}
		}
	}
}

void AToWeatherSystem::PostInitProperties()
{
	Super::PostInitProperties();
}

// Called every frame
void AToWeatherSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

