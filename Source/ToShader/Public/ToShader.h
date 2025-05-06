#pragma once

#include "ToShaderSubsystem.h"
#include "Modules/ModuleManager.h"
#include "ToShader.generated.h"

class FMeshRendererPass;
DECLARE_LOG_CATEGORY_EXTERN(LogToShader, Log, All);

UCLASS()
class TOSHADER_API UToShaderHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	public:
	static void log(FString msg);
	static void log(FString msg,float i);
	static void log(FString msg,FName i);
	static void log(FString msg,int b,bool printAsBool = false);
	static void modifyConifg(FString path,FString section, TMap<FString,FString> key_val);
	static void getMeshMaterials(UPrimitiveComponent* mesh,FMaterialGroup& outMaterials);
	static void setMeshMaterials(UPrimitiveComponent* mesh,UMaterialInterface* mat);
	static void setMeshMaterials(UPrimitiveComponent* mesh,TArray<UMaterialInterface*> mats);
	static int getRTSizeScale(ERTSizeScale scale);
	static bool isEditor();
	//Convert all material of mesh to dynamic material instance if it is not dynamic
	static FDynamicMaterialGroup makeAndApplyMeshMaterialsDynamic(UPrimitiveComponent* mesh);
	static UMaterialInstanceDynamic* makeAndApplyMeshOverlayMaterialDynamic(UMeshComponent* mesh);
	static void setDynamicMaterialGroupFloatParam(FName name,float val,const FDynamicMaterialGroup& group);
	static void setDynamicMaterialGroupFloat3Param(FName name,FVector val,const FDynamicMaterialGroup& group);
	static void setDynamicMaterialGroupTextureParam(FName name,UTexture* val,const FDynamicMaterialGroup& group);

#if WITH_EDITOR
	static FLevelEditorViewportClient* GetViewPortClient()
	{
		return	GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient :
			GLastKeyLevelEditingViewportClient ? GLastKeyLevelEditingViewportClient :
			nullptr;
	}
#endif
	UFUNCTION(BlueprintCallable,BlueprintPure)
	static FTransform getMainCameraTransform();

	UFUNCTION(BlueprintCallable,Category="ToShader",meta=(ToolTip="将该Mesh的所有材质设置为新的材质，并返回Mesh原来的材质"))
	static TArray<UMaterialInterface*> SetMeshMaterial(UPrimitiveComponent* Mesh,UMaterialInterface* NewMat);
	UFUNCTION(BlueprintCallable,Category="ToShader",meta=(ToolTip="将该Mesh的材质按照数组进行设置"))
	static void SetMeshMaterials(UPrimitiveComponent* Mesh,TArray<UMaterialInterface*> NewMats);

};

#define tolog UToShaderHelpers::log

class FToShaderModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	private:
	
	void ApplyEngineConfigs();

	FTickerDelegate TickDelegate;
	FTSTicker::FDelegateHandle TickDelegateHandle;
	bool Tick(float DeltaTime);
};


