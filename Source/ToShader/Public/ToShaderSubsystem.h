#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "ToShaderSubsystem.generated.h"

class UToShaderModule;
class AMeshRenderer;

USTRUCT()
struct FMeshGroup
{
	GENERATED_BODY()
	UPROPERTY()
	UToShaderModule* Module;
	UPROPERTY()
	TArray<UPrimitiveComponent*> Components;
};

USTRUCT()
struct FShowList
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<TWeakObjectPtr<UPrimitiveComponent>> List;
};

//Enum same as Tag name
UENUM(BlueprintType)
enum class ERendererTag : uint8
{
	EyeBrow,Face,Hair,
	Max
};
ENUM_RANGE_BY_COUNT(ERendererTag, ERendererTag::Max);

UCLASS()
class TOSHADER_API UToShaderSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UToShaderSubsystem();
	
	TArray<TWeakObjectPtr<UPrimitiveComponent>> GetShowList(ERendererTag Tag);
	TArray<TWeakObjectPtr<UPrimitiveComponent>> GetShowList(TArray<ERendererTag> Tags);
	
	void AddModuleToSubsystem(UToShaderModule* Module);
	void AddMeshRendererToSubsystem(AMeshRenderer* Actor);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


private:

	bool bShouldUpdateShowLists = true;
	UPROPERTY()
	TArray<UToShaderModule*> Modules;
	UPROPERTY()
	TArray<AMeshRenderer*> MeshRenderers;

	void SetShowLists();
	
	
	bool Tick(float DeltaTime);
	FTSTicker::FDelegateHandle TickDelegateHandle;
};
