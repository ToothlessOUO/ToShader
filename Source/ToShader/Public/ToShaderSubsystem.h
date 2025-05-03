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
struct FDynamicMaterialGroup
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInstanceDynamic*> Materials;
};

USTRUCT(BlueprintType)
struct FMaterialGroup
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInterface*> Materials;
};

USTRUCT(BlueprintType)
struct FMeshGroup
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UPrimitiveComponent*> Components;
};

USTRUCT(BlueprintType)
struct FMeshGroupDyMat
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	TArray<UPrimitiveComponent*> Components;
	UPROPERTY(BlueprintReadOnly)
	TMap<UPrimitiveComponent*,FDynamicMaterialGroup> MeshDyMaterial;
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
	VisInCaptureOnly,HairMask,FaceMask,EyeBrowMask,Outline,
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

UENUM(BlueprintType)
enum class EMaterialEffectActionScope : uint8
{
	Body,Weapon,Outline,Overlay,
	PP_Uber,
	Max
};

ENUM_RANGE_BY_COUNT(ERendererTag, ERendererTag::Max);
ENUM_RANGE_BY_COUNT(EMaterialEffectActionScope, EMaterialEffectActionScope::Max);

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
	
	TArray<FName> GetMaterialEffectTag();

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

	void CacheTagNames();
	
	UPROPERTY()
	TMap<ERendererTag,FName> RendererTagNames;
	bool GetTagName(ERendererTag Tag,FName& TagName);
	
	TWeakObjectPtr<AScreenOverlayMeshManager> ScreenMeshManager;
	
	TMap<EMaterialEffectActionScope,FName> MaterialEffectTagNames;
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
