#include "ToShader.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "ToShaderSubsystem.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

DEFINE_LOG_CATEGORY(LogToShader);

#define LOCTEXT_NAMESPACE "FToShaderModule"

#define tolog FToShaderHelpers::log

#pragma region ToShaderHelpers
void FToShaderHelpers::log(FString msg)
{
	UE_LOG(LogToShader, Warning, TEXT("%s"), *msg);
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

void FToShaderHelpers::log(FString msg, FName i)
{
	msg+=i.ToString();
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

void FToShaderHelpers::getMeshMaterials(UPrimitiveComponent* mesh,FMaterialGroup& outMaterials)
{
	for (int i=0;i<mesh->GetNumMaterials();i++)
	{
		outMaterials.Materials.Add(mesh->GetMaterial(i));
	}
}

void FToShaderHelpers::setMeshMaterials(UPrimitiveComponent* mesh, UMaterialInterface* mat)
{
	if (mesh == nullptr || mat == nullptr) return;
	for (int i=0;i<mesh->GetNumMaterials();i++)
	{
		mesh->SetMaterial(i,mat);
	}
}

void FToShaderHelpers::setMeshMaterials(UPrimitiveComponent* mesh, TArray<UMaterialInterface*> mats)
{
	if (mats.Num() < mesh->GetNumMaterials()) return;
	for (int i=0;i<mesh->GetNumMaterials();i++)
	{
		mesh->SetMaterial(i,mats[i]);
	}
}

int FToShaderHelpers::getRTSizeScale(ERTSizeScale scale)
{
	if (scale == ERTSizeScale::Default) return 1;
	return static_cast<int>(scale);
}

#pragma endregion

#pragma region ModuleFunctions

void FToShaderModule::StartupModule()
{
	TickDelegate = FTickerDelegate::CreateRaw(this, &FToShaderModule::Tick);
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ToShader"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Runtime/ToShader"), PluginShaderDir);

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
	ConfigKV.Emplace("r.DistanceFieldAO","0");
	FToShaderHelpers::modifyConifg(ConfigPath,Section,ConfigKV);
}

bool FToShaderModule::Tick(float DeltaTime)
{
	return true;
}

#pragma endregion

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToShaderModule, ToShader)
