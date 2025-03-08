#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "GameFramework/Actor.h"
#include "ToShaderSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"
#include "MeshRenderer.generated.h"

UCLASS()
class TOSHADER_API AMeshRenderer : public AActor
{
	GENERATED_BODY()

public:
	AMeshRenderer();

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="MeshRenderer")
	TSet<ERendererTag> TargetMeshTags;

	UPROPERTY(BlueprintReadOnly)
	TArray<USceneCaptureComponent2D*> Captures;
	
	virtual void SetShowList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void Setup();

	UToShaderSubsystem* GetSubsystem();
};


UCLASS()
class TOSHADER_API AMeshRendererPro : public AMeshRenderer
{
	GENERATED_BODY()

public:
	AMeshRendererPro();
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="MeshRenderer|Pro")
	FName PassName = FName();
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="MeshRenderer|Pro")
	UTextureRenderTarget2D* OutputRT = nullptr;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="MeshRenderer|Pro")
	ERTSizeScale RTScale;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="MeshRenderer|Pro")
	TMap<ERendererTag,UMaterialInterface*> TagMeshMaterialWhenRendering;

	virtual void SetShowList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList) override;

	void CallBackWhenAddToSubsystemSuccess(FPassContainer Pass);

	//返回mesh的tag在TagMeshMaterialWhenRendering中的值，当其中不存在mesh的tag时bIsMeshTagValid为false
	void SaveMeshMaterialWhenRendering(UPrimitiveComponent* Mesh,TMap<UPrimitiveComponent*,FMaterialGroup>& Saved);
	void ResetMeshMaterialAfterRendering(UPrimitiveComponent* Mesh,TMap<UPrimitiveComponent*,FMaterialGroup> Saved);

protected:
	virtual void Setup() override;
	
	FPassContainer TargetPass;
};

