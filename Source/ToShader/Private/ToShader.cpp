#include "ToShader.h"

#include "ISettingsModule.h"

DEFINE_LOG_CATEGORY(ToShader);

#define LOCTEXT_NAMESPACE "FToShaderModule"

#define tolog FToShaderHelpers::log

#pragma region ToShaderHelpers
void FToShaderHelpers::log(FString msg)
{
	UE_LOG(ToShader, Warning, TEXT("%s"), *msg);
}

void FToShaderHelpers::log(FString msg, int b, bool printAsBool)
{
	if (printAsBool)
	{
		if (b == 0 || b == 1)
		{
			msg += b ? FString("true") : FString("false");
			log(msg);
			return;
		}
	}
	msg += FString::FromInt(b);
	log(msg);
}

void FToShaderHelpers::log(FString msg, float i)
{
	msg += FString::SanitizeFloat(i);
	log(msg);
}

void FToShaderHelpers::modifyConifg(FString path, FString section, TMap<FString,FString> key_val)
{
	FConfigFile ConfigFile;
	ConfigFile.Read(path);
	// 读取配置文件
	if (ConfigFile.IsEmpty())
	{
		tolog("can't read file : "+ path);
		return;
	}
	// 添加或更新配置项
	for (auto k_v : key_val)
	{
		ConfigFile.SetString(*section, *k_v.Key, *k_v.Value);
	}
	// 写回配置文件
	if (!ConfigFile.Write(path))
	{
		tolog("can't write file : "+ path);
		return;
	}
	tolog("update success "+ path);
}

#pragma endregion

#pragma region ModuleFunctions
void FToShaderModule::StartupModule()
{
	TickDelegate = FTickerDelegate::CreateRaw(this, &FToShaderModule::Tick);
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);

	ApplyEngineConfigs();
}

void FToShaderModule::ShutdownModule()
{
	FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void FToShaderModule::ApplyEngineConfigs()
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const FString Section = "/Script/Engine.RendererSettings";
	TMap<FString,FString> ConfigKV;
	ConfigKV.Emplace("r.Nanite.Allowtessellation","1");
	ConfigKV.Emplace("r.Nanite.Tessellation","1");
	ConfigKV.Emplace("r.VirtualTextures","False");
	FToShaderHelpers::modifyConifg(ConfigPath,Section,ConfigKV);
}

#pragma endregion

bool FToShaderModule::Tick(float DeltaTime)
{
	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToShaderModule, ToShader)
