#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(ToShader, Log, All);

class FToShaderHelpers
{
	public:
	static void log(FString msg);
	static void log(FString msg,float i);
	static void log(FString msg,int b,bool printAsBool = false);
	static void modifyConifg(FString path,FString section, TMap<FString,FString> key_val);
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
