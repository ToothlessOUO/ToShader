#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "ToShaderSubsystem.generated.h"

USTRUCT()
struct FPrimitiveComponents
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UPrimitiveComponent*> Components;
};

UCLASS()
class TOSHADER_API UToShaderSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UToShaderSubsystem();

	bool ShouldUpdateEyeBrows(double TimeVersion,double& CurTimerVersion) const;
	void AddEyeBrowComponents(AActor* Actor,TArray<UPrimitiveComponent*> Components);
	UFUNCTION(BlueprintCallable,Blueprintable)
	TArray<UPrimitiveComponent*> GetEyeBrowComponents();
	

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:

	double EyeBrowTimeVersion;
	UPROPERTY()
	TMap<AActor*,FPrimitiveComponents> EyeBrowMap;
	bool bIsEyeBrowComponentsChanged = true;
	UPROPERTY()
	TArray<UPrimitiveComponent*> EyeBrowComponents;
	void UpdateEyeBrowComponentsFromMap();
	
	bool Tick(float DeltaTime);
	FTSTicker::FDelegateHandle TickDelegateHandle;
};
