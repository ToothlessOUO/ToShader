#pragma once

#include "CoreMinimal.h"
#include "MeshRendererPass.h"
#include "Subsystems/EngineSubsystem.h"
#include "ToShaderSubsystem.generated.h"

class UToShaderComponent;
class AMeshRenderer;

#pragma region structs enums
USTRUCT(BlueprintType)
struct FMeshGroup
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UPrimitiveComponent*> Components;
};

USTRUCT(BlueprintType)
struct FMaterialGroup
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInterface*> Materials;
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
	VisInCaptureOnly,HairMask,FaceMask,EyeBrowMask,
	Max
};

UENUM(BlueprintType)
enum class ERTSizeScale : uint8
{
	Default = 0
	,Down2X = 2
	,Down4X = 4
	,Down6X = 6
};

ENUM_RANGE_BY_COUNT(ERendererTag, ERendererTag::Max);

USTRUCT()
struct FPassContainer
{
	GENERATED_BODY()
	TSharedPtr<FMeshRendererPass, ESPMode::ThreadSafe> Pass;
};
#pragma endregion


UCLASS()
class TOSHADER_API UToShaderSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UToShaderSubsystem();
	
	TArray<TWeakObjectPtr<UPrimitiveComponent>> GetShowList(ERendererTag Tag);
	TArray<TWeakObjectPtr<UPrimitiveComponent>> GetShowList(TArray<ERendererTag> Tags);
	
	void AddModuleToSubsystem(UToShaderComponent* Module);
	void AddMeshRendererToSubsystem(AMeshRenderer* Actor);

	static UToShaderSubsystem* GetSubsystem();
	
	static bool IsMeshContainsTag(UPrimitiveComponent* Mesh,ERendererTag Tag);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


private:

	bool bShouldUpdateShowLists = true;
	UPROPERTY()
	TArray<UToShaderComponent*> Modules;
	UPROPERTY()
	TArray<AMeshRenderer*> MeshRenderers;
	UPROPERTY()
	TMap<FName,FPassContainer> Passes;

	void SetShowList(AMeshRenderer* MeshRenderer);
	void SetShowLists();

	UPROPERTY()
	TMap<ERendererTag,FName> TagNames;
	bool GetTagName(ERendererTag Tag,FName& TagName);
	void CacheTagNames();
	
	bool Tick(float DeltaTime);
	FTSTicker::FDelegateHandle TickDelegateHandle;
};
