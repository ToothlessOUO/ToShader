#pragma once

#include "CoreMinimal.h"
#include "ToShaderSubsystem.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "MeshRenderer.generated.h"

UCLASS()
class TOSHADER_API AMeshRenderer : public AActor
{
	GENERATED_BODY()

public:
	AMeshRenderer();

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="MeshRenderer")
	bool bUseShowOnlyList = true;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="MeshRenderer")
	TSet<ERendererTag> TargetMeshTags;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="MeshRenderer")
	TSet<ERendererTag> HiddenMeshTags;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="MeshRenderer",
		meta=(ToolTip="开启后默认会启用actor tick，其他情况下使用也需要确保tick开启"))
	bool bShouldFollowTheView = false;

	UPROPERTY(BlueprintReadOnly)
	TArray<USceneCaptureComponent*> Captures;
	
	UFUNCTION(BlueprintCallable,BlueprintPure)
	TArray<UPrimitiveComponent*> GetShowList();
	
	virtual void SetShowList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList);
	virtual void SetHiddenList(TArray<TWeakObjectPtr<UPrimitiveComponent>> NewList);
	

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void Setup();

	UToShaderSubsystem* GetSubsystem();
	void UpdateTransform();
};

UCLASS()
class TOSHADER_API AScreenOverlayMesh : public AActor
{
	GENERATED_BODY()
public:
	AScreenOverlayMesh();

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)
	void SetEnabled(bool bEnabled);

	protected:
	virtual void BeginPlay() override;
	UPROPERTY()
	TArray<UStaticMeshComponent*> Meshes;
	
};
USTRUCT(BlueprintType)
struct FScreenOverlayMeshParam
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite,VisibleAnywhere)
	bool bEnabled;
	UPROPERTY(BlueprintReadWrite,VisibleAnywhere)
	AScreenOverlayMesh* Mesh;

	bool operator==(const FScreenOverlayMeshParam& InParam) const
	{
		return this->Mesh == InParam.Mesh;
	}

	bool operator==(const TSubclassOf<AScreenOverlayMesh>& Type) const
	{
		if (!Mesh) return false;
		return Mesh->GetClass() == Type;
	}

	friend uint32 GetTypeHash(const FScreenOverlayMeshParam& MyStruct)
	{
		return GetTypeHash(MyStruct.Mesh);
	}
};
UCLASS()
class TOSHADER_API AScreenOverlayMeshManager : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly,Category="ScreenOverlayMeshManager")
	TSet<FScreenOverlayMeshParam> Meshes;

	UFUNCTION(BlueprintCallable)
	void SetScreenOverlayMeshEnabled(TSubclassOf<AScreenOverlayMesh> Type,bool bE);
	
	AScreenOverlayMeshManager();
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
};




