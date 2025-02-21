// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToShader.h"

DEFINE_LOG_CATEGORY(LogToShader);

#define LOCTEXT_NAMESPACE "FToShaderModule"

void FToShaderModule::StartupModule()
{
	TickDelegate = FTickerDelegate::CreateRaw(this, &FToShaderModule::Tick);
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);
}

void FToShaderModule::ShutdownModule()
{
}

void FToShaderModule::tolog(FString msg)
{
	UE_LOG(LogToShader, Warning, TEXT("%s"), *msg);
}

void FToShaderModule::ApplyEngineConfigs()
{
	if (GEngine != nullptr && !bHasApplyEnginConfigsDone)
	{
		// 检查 r.Nanite.AllowTessellation 是否已经设置为 1
		bool bAllowTessellationSet = false;
		GConfig->GetBool(TEXT("/Script/Engine.RendererSettings"), TEXT("r.Nanite.AllowTessellation"), bAllowTessellationSet, GGameIni);
		tolog("GGameTessellation enabled "+bAllowTessellationSet?"True":"False");
		if (!bAllowTessellationSet)
		{
			GEngine->Exec(nullptr, TEXT("r.Nanite.AllowTessellation = 1"));
			GConfig->SetBool(TEXT("/Script/Engine.RendererSettings"), TEXT("r.Nanite.AllowTessellation"), true, GGameIni);
		}

		// 检查 r.Nanite.Tessellation 是否已经设置为 1
		bool bTessellationSet = false;
		GConfig->GetBool(TEXT("/Script/Engine.RendererSettings"), TEXT("r.Nanite.Tessellation"), bTessellationSet, GGameIni);
		if (!bTessellationSet)
		{
			GEngine->Exec(nullptr, TEXT("r.Nanite.Tessellation = 1"));
			GConfig->SetBool(TEXT("/Script/Engine.RendererSettings"), TEXT("r.Nanite.Tessellation"), true, GGameIni);
		}
		bHasApplyEnginConfigsDone = true;
	}
	
}

bool FToShaderModule::Tick(float DeltaTime)
{
	ApplyEngineConfigs();
	return false;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToShaderModule, ToShader)
