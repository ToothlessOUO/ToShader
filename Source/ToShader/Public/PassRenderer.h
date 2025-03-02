#pragma once

#include "CoreMinimal.h"
#include "AlwaysTickComponent.h"
#include "GameFramework/Actor.h"
#include "PassRenderer.generated.h"

UCLASS()
class TOSHADER_API APassRenderer : public AActor
{
	GENERATED_BODY()

public:
	APassRenderer();
	
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)
	TArray<UPrimitiveComponent*> GetShowObjs();
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)
	void ExecuteAfterInitComponents();

	UPROPERTY(BlueprintReadOnly)
	TArray<USceneCaptureComponent2D*> Captures;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAlwaysTickComponent* AlwaysTickComponent;
	

protected:
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	UFUNCTION()
	void T(float DeltaTime);
	virtual void Tick(float DeltaTime) override;

private:
	double TimeVersion;
	
};
