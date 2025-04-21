#include "MeshRenderer.h"
#include "ToShader.h"
#include "ToShaderSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"

#pragma region MeshRenderer
AMeshRenderer::AMeshRenderer()
{

	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	if (bShouldFollowTheView)
	{
		PrimaryActorTick.bCanEverTick = true;
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetTickGroup(TG_PostUpdateWork);
	}
}

TArray<UPrimitiveComponent*> AMeshRenderer::GetShowList()
{
	if (Captures.IsEmpty()) return {};
	TArray<UPrimitiveComponent*> ShowList;
	for (auto Ele : Captures[0]->ShowOnlyComponents)
	{
		if (Ele.IsValid())
		{
			ShowList.Add(Ele.Get());
		}
	}
	return ShowList;
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
	Setup();
}

void AMeshRenderer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	tolog("OnConstruction");
	Setup();
}

void AMeshRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTransform();
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
	//收集并注册至子系统
	if (GetSubsystem())
		GetSubsystem()->AddMeshRendererToSubsystem(this);
}

UToShaderSubsystem* AMeshRenderer::GetSubsystem()
{
	if (!GEngine) return nullptr;
	return GEngine->GetEngineSubsystem<UToShaderSubsystem>();
}

void AMeshRenderer::UpdateTransform()
{
	if (bShouldFollowTheView)
	{
		auto T = UToShaderHelpers::getMainCameraTransform();
		T.SetScale3D(FVector(1.f, 1.f, 1.f));
		SetActorTransform(T);
	}
}
#pragma endregion

#pragma region ScreenOverlayMesh Renderer

AScreenOverlayMesh::AScreenOverlayMesh()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AScreenOverlayMesh::BeginPlay()
{
	Super::BeginPlay();

	for (auto L = K2_GetComponentsByClass(UStaticMeshComponent::StaticClass());
		const auto C : L)
	{
		Meshes.Add(Cast<UStaticMeshComponent>(C));
	}
}

void AScreenOverlayMesh::SetEnabled_Implementation(bool bEnabled)
{
	for (auto Element : Meshes)
	{
		Element->SetHiddenInGame(bEnabled);
	}
}

void AScreenOverlayMeshManager::SetScreenOverlayMeshEnabled(const TSubclassOf<AScreenOverlayMesh> Type,bool bE)
{
	for (auto Element : Meshes)
	{
		if (Element == Type)
		{
			Element.Mesh->SetEnabled(bE);
			return;
		}
	}
}

AScreenOverlayMeshManager::AScreenOverlayMeshManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SetTickGroup(TG_PostUpdateWork);
}

void AScreenOverlayMeshManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UToShaderSubsystem::GetSubsystem()->SetScreenOverlayMeshManager(this);
}

void AScreenOverlayMeshManager::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorld()) return;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FTransform SpawnTransform = GetTransform();
	for (auto [bEnabled, Mesh] : Meshes)
	{
		if (!Mesh) continue;
		AScreenOverlayMesh* NewMesh = GetWorld()->SpawnActor<AScreenOverlayMesh>(Mesh->GetClass(), SpawnTransform, SpawnParams);
		NewMesh->AttachToActor(this,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Mesh = NewMesh;
		Mesh->SetEnabled(bEnabled);
	}
}

void AScreenOverlayMeshManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	auto T = UToShaderHelpers::getMainCameraTransform();
	T.SetScale3D(FVector(1.f, 1.f, 1.f));
	SetActorTransform(T);
}

#pragma endregion






