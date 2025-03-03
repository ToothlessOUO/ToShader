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
	TArray<ERendererTag> TargetMeshTags;

	UPROPERTY(BlueprintReadOnly)
	TArray<USceneCaptureComponent2D*> Captures;

	void SetShowList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList);


protected:
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void Tick(float DeltaTime) override;

	void CleanTargetTags();
	void CollectCaptures();

	UToShaderSubsystem* GetSubsystem();
};
