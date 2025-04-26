#pragma once

#include "CoreMinimal.h"
#include "MaterialEffect.h"
#include "Subsystems/EngineSubsystem.h"
#include "ToShaderSubsystem.generated.h"

class UToShaderComponent;
class AMeshRenderer;
class AScreenOverlayMeshManager;

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
	ScreenOverlay,
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
	void RemoveModuleFromSubsystem(UToShaderComponent* Module);
	void AddMeshRendererToSubsystem(AMeshRenderer* Actor);

	static UToShaderSubsystem* GetSubsystem();
	
	static bool IsMeshContainsTag(UPrimitiveComponent* Mesh,ERendererTag Tag);

	void CallUpdateMeshRenderers();

	void SetScreenOverlayMeshManager(AScreenOverlayMeshManager* Manager);
	
	UFUNCTION(BlueprintCallable,BlueprintPure)
	void GetScreenOverlayMeshManager(bool& bSuccess,AScreenOverlayMeshManager* &RetManager);

	void CallUpdate_MaterialEffectPropertyTable();
	TArray<FName> GetMaterialEffectPropertyTableRowNames(EMPType Type);
	FMPTableProp* GetMP(FName Name,EMPType Type);

	TArray<FName> MaterialEffectTag{"Body","Outline","Weapon"};

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


private:
	TWeakObjectPtr<APlayerCameraManager> PlayerCameraManager  = nullptr;
	
	TArray<TWeakObjectPtr<UToShaderComponent>> Modules;
	
	bool bShouldUpdateMeshRenderers = true;
	TArray<TWeakObjectPtr<AMeshRenderer>> MeshRenderers;
	void SetShowList(AMeshRenderer* MeshRenderer);
	void UpdateMeshRenderersShowLists();
	
	UPROPERTY()
	TMap<ERendererTag,FName> TagNames;
	bool GetTagName(ERendererTag Tag,FName& TagName);
	void CacheTagNames();

	TWeakObjectPtr<AScreenOverlayMeshManager> ScreenMeshManager;
	
	TMap<FName,FMPTableProp*> MPKeyCache;
	TMap<FName,FMPTableProp*> MPFloatCache;
	TMap<FName,FMPTableProp*> MPFloat4Cache;
	TMap<FName,FMPTableProp*> MPTextureCache;
	UPROPERTY()
	TWeakObjectPtr<UDataTable> MaterialEffectPropertyTable;
	void UpdateMaterialEffectPropertyTable();

	
	bool Tick(float DeltaTime);
	FTSTicker::FDelegateHandle TickDelegateHandle;
};
