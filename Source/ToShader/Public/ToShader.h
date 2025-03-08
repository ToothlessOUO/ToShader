#pragma once

#include "ToShaderSubsystem.h"
#include "Modules/ModuleManager.h"

class FMeshRendererPass;
DECLARE_LOG_CATEGORY_EXTERN(LogToShader, Log, All);

class FToShaderHelpers
{
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
};

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
