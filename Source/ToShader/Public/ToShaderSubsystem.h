#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "ToShaderSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class TOSHADER_API UToShaderSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UToShaderSubsystem();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	bool Tick(float DeltaTime);
	FTSTicker::FDelegateHandle TickDelegateHandle;
};
