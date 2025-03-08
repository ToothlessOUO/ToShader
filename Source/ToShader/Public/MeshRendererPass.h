#pragma once
#include "SceneViewExtension.h"


class AMeshRendererPro;

class FMeshRendererPass : public FWorldSceneViewExtension
{
public:
	FMeshRendererPass(const FAutoRegister& AutoReg, UWorld* InWorld);

	void Setup(AMeshRendererPro* MeshRendererPro);
	
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& View, const FRenderTargetBindingSlots& RenderTargets, TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures) override;
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override;

	struct FAdditionalData {
		FVector ExampleColorProperty;
	};
	
	TArray<TWeakObjectPtr<UPrimitiveComponent>> ShowList;
	TMap<const FPrimitiveSceneProxy*, FAdditionalData> AdditionalData;
	FIntPoint RTSize;
	FTextureRenderTargetResource* OutputRT;

	private:
	AMeshRendererPro* PassOwner;
};
