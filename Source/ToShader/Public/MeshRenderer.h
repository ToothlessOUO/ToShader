#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ToShaderSubsystem.h"
#include "MeshRenderer.generated.h"

UCLASS()
class TOSHADER_API AMeshRenderer : public AActor
{
	GENERATED_BODY()

public:
	AMeshRenderer();

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	TSet<ERendererTag> TargetMeshTags;

	UPROPERTY(BlueprintReadOnly)
	TArray<USceneCaptureComponent2D*> Captures;

	void SetShowList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList);

	bool HasInit = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	
	void CollectCaptures();

	UToShaderSubsystem* GetSubsystem();
};
