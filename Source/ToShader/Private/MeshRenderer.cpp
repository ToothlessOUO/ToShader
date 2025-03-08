#include "MeshRenderer.h"

#include "ToShader.h"
#include "ToShaderSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"

#define tolog FToShaderHelpers::log

#pragma region MeshRenderer
AMeshRenderer::AMeshRenderer()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

}

void AMeshRenderer::SetShowList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList)
{
	for (auto Capture : Captures)
	{
		Capture->ShowOnlyComponents = NewList;
	}
}

void AMeshRenderer::BeginPlay()
{
	Super::BeginPlay();
}

void AMeshRenderer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Setup();
}

void AMeshRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMeshRenderer::Setup()
{
	Captures.Empty();
	auto Components = GetComponents();
	for (const auto Component : Components)
	{
		if (!Component) continue;
		auto Capture = Cast<USceneCaptureComponent2D>(Component);
		if (!Capture) continue;
		Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		Captures.Add(Capture);
	}
	if (GetSubsystem())
		GetSubsystem()->AddMeshRendererToSubsystem(this);
}

UToShaderSubsystem* AMeshRenderer::GetSubsystem()
{
	if (!GEngine) return nullptr;
	return GEngine->GetEngineSubsystem<UToShaderSubsystem>();
}

#pragma endregion

#pragma region MeshRenderer Pro

AMeshRendererPro::AMeshRendererPro()
{
}

void AMeshRendererPro::SetShowList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList)
{
	//更新 pass mesh
	if (!TargetPass.Pass.IsValid())
	{
		Setup();
		return;
	}
	TargetPass.Pass->ShowList = NewList;
}

void AMeshRendererPro::CallBackWhenAddToSubsystemSuccess(FPassContainer Pass)
{
	Pass.Pass->Setup(this);
	TargetPass = Pass;
}

void AMeshRendererPro::SaveMeshMaterialWhenRendering(UPrimitiveComponent* Mesh, TMap<UPrimitiveComponent*, FMaterialGroup>& Saved)
{
	if (!Mesh) return;
	if (TagMeshMaterialWhenRendering.IsEmpty()) return;
	for (const auto Element : TagMeshMaterialWhenRendering)
	{
		if (UToShaderSubsystem::IsMeshContainsTag(Mesh, Element.Key))
		{
			FMaterialGroup Group;
			FToShaderHelpers::getMeshMaterials(Mesh,Group);
			FToShaderHelpers::setMeshMaterials(Mesh,TagMeshMaterialWhenRendering[Element.Key]);
			Saved.Emplace(Mesh,Group);
			return;
		}
	}
}

void AMeshRendererPro::ResetMeshMaterialAfterRendering(UPrimitiveComponent* Mesh, TMap<UPrimitiveComponent*, FMaterialGroup> Saved)
{
	if (!Mesh) return;
	if (!Saved.Contains(Mesh)) return;
	FToShaderHelpers::setMeshMaterials(Mesh,Saved[Mesh].Materials);
}

void AMeshRendererPro::Setup()
{
	if (GetSubsystem())
		GetSubsystem()->AddMeshRendererToSubsystem(this);
}
#pragma endregion





