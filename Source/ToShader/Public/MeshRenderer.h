#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ToShaderSubsystem.h"
#include "MeshRenderer.generated.h"

class UPreTick;
class UPostTick;


#pragma region TickStage Interface
UINTERFACE()
class UTickStageInterface : public UInterface
{
	GENERATED_BODY()
};

class TOSHADER_API ITickStageInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPreTick(float DeltaTime);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPostTick(float DeltaTime);
};
#pragma endregion

UCLASS()
class TOSHADER_API AMeshRenderer : public AActor,public ITickStageInterface
{
	GENERATED_BODY()

public:
	AMeshRenderer();

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	TSet<ERendererTag> TargetMeshTags;

	UPROPERTY(BlueprintReadOnly)
	TArray<USceneCaptureComponent2D*> Captures;

	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	UPreTick* PreTick;
	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	UPostTick* PostTick;

	void SetShowList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList);
	
	bool HasInit = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	
	void CollectCaptures();

	UToShaderSubsystem* GetSubsystem();
};

UCLASS()
class TOSHADER_API UPreTick : public UActorComponent
{
	GENERATED_BODY()
	ITickStageInterface* Interface;
public:
	UPreTick();
private:
	virtual void PostInitProperties() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

UCLASS()
class TOSHADER_API UPostTick : public UActorComponent
{
	GENERATED_BODY()
	ITickStageInterface* Interface;
public:
	UPostTick();
protected:
	virtual void PostInitProperties() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};