#include "ToShader.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "ToShaderSubsystem.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Slate/SceneViewport.h"

DEFINE_LOG_CATEGORY(LogToShader);

#define LOCTEXT_NAMESPACE "FToShaderModule"

#pragma region ToShaderHelpers
void UToShaderHelpers::log(FString msg)
{
	UE_LOG(LogToShader, Warning, TEXT("%s"), *msg);
}

void UToShaderHelpers::log(FString msg, int b, bool printAsBool)
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

void UToShaderHelpers::log(FString msg, FName i)
{
	msg += i.ToString();
	log(msg);
}

void UToShaderHelpers::log(FString msg, float i)
{
	msg += FString::SanitizeFloat(i);
	log(msg);
}

void UToShaderHelpers::modifyConifg(FString path, FString section, TMap<FString, FString> key_val)
{
	FConfigFile ConfigFile;
	ConfigFile.Read(path);
	// 读取配置文件
	if (ConfigFile.IsEmpty())
	{
		tolog("can't read file : " + path);
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
		tolog("can't write file : " + path);
		return;
	}
	tolog("update success " + path);
}

void UToShaderHelpers::getMeshMaterials(UPrimitiveComponent* mesh, FMaterialGroup& outMaterials)
{
	for (int i = 0; i < mesh->GetNumMaterials(); i++)
	{
		outMaterials.Materials.Add(mesh->GetMaterial(i));
	}
}

void UToShaderHelpers::setMeshMaterials(UPrimitiveComponent* mesh, UMaterialInterface* mat)
{
	if (mesh == nullptr || mat == nullptr) return;
	for (int i = 0; i < mesh->GetNumMaterials(); i++)
	{
		mesh->SetMaterial(i, mat);
	}
}

void UToShaderHelpers::setMeshMaterials(UPrimitiveComponent* mesh, TArray<UMaterialInterface*> mats)
{
	if (mats.Num() < mesh->GetNumMaterials()) return;
	for (int i = 0; i < mesh->GetNumMaterials(); i++)
	{
		mesh->SetMaterial(i, mats[i]);
	}
}

int UToShaderHelpers::getRTSizeScale(ERTSizeScale scale)
{
	if (scale == ERTSizeScale::Default) return 1;
	return static_cast<int>(scale);
}

bool UToShaderHelpers::isEditor()
{
	// 如果没有传入World，尝试获取当前World
	const UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	if (!World)
	{
		// 如果还是无法获取World，使用基础判断
		return !GIsEditor || GIsPlayInEditorWorld;
	}
	// 根据WorldType判断
	switch (World->WorldType)
	{
	case EWorldType::Game:       // 打包游戏
	case EWorldType::PIE:         // PIE模式
	case EWorldType::GamePreview: // 游戏预览
		return true;
            
	case EWorldType::Editor:      // 纯编辑器
	case EWorldType::EditorPreview: // 编辑器预览
	default:
		return false;
	}
}

FDynamicMaterialGroup UToShaderHelpers::makeAndApplyMeshMaterialsDynamic(UPrimitiveComponent* mesh)
{
	FDynamicMaterialGroup Group;
	for (int i = 0; i < mesh->GetNumMaterials(); i++)
	{
		if (const auto LMat = mesh->GetMaterial(i); LMat != nullptr)
		{
			if (LMat->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				auto DLMat = Cast<UMaterialInstanceDynamic>(LMat);
				Group.Materials.Emplace(DLMat);
				break;
			}
			auto M = UMaterialInstanceDynamic::Create(LMat, mesh);
			mesh->SetMaterial(i, M);
			Group.Materials.Emplace(M);
		}
		else
		{
			Group.Materials.Emplace(nullptr);
		}
	}
	return Group;
}

void UToShaderHelpers::setDynamicMaterialGroupFloatParam(FName name, const float val,const FDynamicMaterialGroup& group)
{
	for (auto M : group.Materials)
	{
		if(!M) continue;
		M->SetScalarParameterValue(name,val);
	}
}

void UToShaderHelpers::setDynamicMaterialGroupFloat4Param(FName name, FVector4f val, const FDynamicMaterialGroup& group)
{
	for (auto M : group.Materials)
	{
		if(!M) continue;
		M->SetVectorParameterValue(name,val);
	}
}

void UToShaderHelpers::setDynamicMaterialGroupTextureParam(FName name, UTexture* val, const FDynamicMaterialGroup& group)
{
	if(!val) return;
	for (auto M : group.Materials)
	{
		if(!M) continue;
		M->SetTextureParameterValue(name,val);
	}
}

FTransform UToShaderHelpers::getMainCameraTransform()
{
#if WITH_EDITOR
	if (!GEditor->PlayWorld)
	{
		if (FLevelEditorViewportClient* Client = GetViewPortClient())
		{
			FRotator ViewportRotation(0, 0, 0);
			FVector ViewportLocation(0, 0, 0);

			if (!Client->IsOrtho())
			{
				ViewportRotation = Client->GetViewRotation();
			}

			ViewportLocation = Client->GetViewLocation();
			return FTransform(ViewportRotation, ViewportLocation, FVector::OneVector);
		}
		return FTransform();
	}
	if (GEditor->PlayWorld->GetFirstPlayerController())
	{
		if (GEditor->PlayWorld->GetFirstPlayerController()->PlayerCameraManager)
		{
			//cameraManager = GEditor->PlayWorld->GetFirstPlayerController()->PlayerCameraManager;
			return GEditor->PlayWorld->GetFirstPlayerController()->PlayerCameraManager->GetTransform();
		}
	}
	return FTransform();
#endif
}

TArray<UMaterialInterface*> UToShaderHelpers::SetMeshMaterial(UPrimitiveComponent* Mesh, UMaterialInterface* NewMat)
{
	if (!Mesh || !NewMat) return {};
	TArray<UMaterialInterface*> RetMaterials;
	for (int i = 0; i < Mesh->GetNumMaterials(); i++)
	{
		RetMaterials.Add(Mesh->GetMaterial(i));
		Mesh->SetMaterial(i, NewMat);
	}
	return RetMaterials;
}

void UToShaderHelpers::SetMeshMaterials(UPrimitiveComponent* Mesh, TArray<UMaterialInterface*> NewMats)
{
	if (!Mesh || NewMats.IsEmpty() || NewMats.Num() < Mesh->GetNumMaterials()) return;
	for (int i = 0; i < Mesh->GetNumMaterials(); i++)
	{
		Mesh->SetMaterial(i, NewMats[i]);
	}
}

#pragma endregion

#pragma region ModuleFunctions

void FToShaderModule::StartupModule()
{
	TickDelegate = FTickerDelegate::CreateRaw(this, &FToShaderModule::Tick);
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ToShader"))->GetBaseDir(),
	                                          TEXT("Shaders"));
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
	TMap<FString, FString> ConfigKV;
	ConfigKV.Emplace("r.Nanite.Allowtessellation", "1");
	ConfigKV.Emplace("r.Nanite.Tessellation", "1");
	ConfigKV.Emplace("r.VirtualTextures", "False");
	ConfigKV.Emplace("r.DistanceFieldAO", "0");
	UToShaderHelpers::modifyConifg(ConfigPath, Section, ConfigKV);
}

bool FToShaderModule::Tick(float DeltaTime)
{
	return true;
}

#pragma endregion

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToShaderModule, ToShader)
