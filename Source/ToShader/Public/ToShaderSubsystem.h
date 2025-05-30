#pragma once

#include "CoreMinimal.h"
#include "MaterialEffect.h"
#include "Subsystems/EngineSubsystem.h"
#include "ToStructs.h"
#include "ToShaderSubsystem.generated.h"

class UToShaderComponent;
class AMeshRenderer;
class AScreenOverlayMeshManager;

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
	
	bool IsMeshContainsRenderTag(UPrimitiveComponent* Mesh,ERendererTag Tag);

	void CallUpdateMeshRenderers();

	void SetScreenOverlayMeshManager(AScreenOverlayMeshManager* Manager);
	
	UFUNCTION(BlueprintCallable,BlueprintPure)
	void GetScreenOverlayMeshManager(bool& bSuccess,AScreenOverlayMeshManager* &RetManager);

	void CallUpdate_MaterialEffectPropertyTable();
	TArray<FName> GetMaterialEffectPropertyTableRowNames(EMPType Type);
	FMPTableProp* GetMP(FName Name,EMPType Type);
	
	TArray<FName> GetMaterialEffectTag();

	UFUNCTION(BlueprintCallable,BlueprintPure)
	UMaterial* GetOverlayEffectMaterial();

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

	//不同的系統可能使用不同的Enum，但是Enum对应的tag都是一样的，Enum是tag的子集，XXXTagNames提前缓存了Enum和Tag之间的关系
	void CacheTagNames();
	
	UPROPERTY()
	TMap<ERendererTag,FName> RendererTagNames;
	bool GetRenderTagName(ERendererTag Tag,FName& TagName);
	
	TWeakObjectPtr<AScreenOverlayMeshManager> ScreenMeshManager;
	
	TMap<EMaterialEffectActionScope,FName> MaterialEffectTagNames;
	TMap<FName,FMPTableProp*> MPKeyCache;
	TMap<FName,FMPTableProp*> MPFloatCache;
	TMap<FName,FMPTableProp*> MPFloat3Cache;
	TMap<FName,FMPTableProp*> MPTextureCache;
	UPROPERTY()
	TWeakObjectPtr<UDataTable> MaterialEffectPropertyTable;
	void UpdateMaterialEffectPropertyTable();

	void CacheObjs();
	TObjectPtr<UMaterial> OverlayEffectMat = nullptr;
	const FString Path_OverlayEffectMaterial = "/ToShader/Shaders/ToShading/M_OverlayEffect.M_OverlayEffect";
	
	bool Tick(float DeltaTime);
	FTSTicker::FDelegateHandle TickDelegateHandle;
};
