#include "MeshRendererPass.h"

#include "MeshRenderer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "InstanceCulling/InstanceCullingContext.h"
#include "EngineModule.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/Material.h"
#include "MeshPassProcessor.inl"
#include "RenderGraphUtils.h"
#include "RHIGPUReadback.h"
#include "RenderingThread.h"
#include "SceneInterface.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "SceneTexturesConfig.h"
#include "SimpleMeshDrawCommandPass.h"
#include "SceneRendererInterface.h"
#include "StaticMeshSceneProxy.h"
#include "ToShader.h"
#include "Runtime/Renderer/Private/BasePassRendering.h"

#define tolog FToShaderHelpers::log

#pragma region Shaders

class FMyMeshPassMaterialShader : public FMeshMaterialShader
{
public:
	FMyMeshPassMaterialShader(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{
		PassUniformBuffer.Bind(Initializer.ParameterMap, FSceneTextureUniformParameters::FTypeInfo::GetStructMetadata()->GetShaderVariableName());
	}

	FMyMeshPassMaterialShader()
	{
	}

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};

class FMyMeshPassMaterialShaderVS : public FMyMeshPassMaterialShader
{
	DECLARE_SHADER_TYPE(FMyMeshPassMaterialShaderVS, MeshMaterial);

public:
	FMyMeshPassMaterialShaderVS()
	{
	}

	FMyMeshPassMaterialShaderVS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer)
		: FMyMeshPassMaterialShader(Initializer)
	{
	}
};

IMPLEMENT_MATERIAL_SHADER_TYPE(, FMyMeshPassMaterialShaderVS, TEXT("/Plugin/Runtime/ToShader/MyMeshPassMaterialShader.usf"), TEXT("VSMain"), SF_Vertex);

struct FMyMeshPassShaderElementData : public FMeshMaterialShaderElementData
{
	FMyMeshPassShaderElementData()
	{
	}

	FVector3f ExampleColorProperty;
};

class FMyMeshPassMaterialShaderPS : public FMyMeshPassMaterialShader
{
	DECLARE_SHADER_TYPE(FMyMeshPassMaterialShaderPS, MeshMaterial);

public:
	FMyMeshPassMaterialShaderPS()
	{
	}

	FMyMeshPassMaterialShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMyMeshPassMaterialShader(Initializer)
	{
		ExampleColorProperty.Bind(Initializer.ParameterMap, TEXT("ExampleColorProperty"));
	}

	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMyMeshPassShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, ShaderElementData, ShaderBindings);

		ShaderBindings.Add(ExampleColorProperty, ShaderElementData.ExampleColorProperty);
	}

	void GetElementShaderBindings(
		const FShaderMapPointerTable& PointerTable,
		const FScene* Scene,
		const FSceneView* ViewIfDynamicMeshCommand,
		const FVertexFactory* VertexFactory,
		const EVertexInputStreamType InputStreamType,
		const FStaticFeatureLevel FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMeshBatch& MeshBatch,
		const FMeshBatchElement& BatchElement,
		const FMyMeshPassShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const
	{
		FMeshMaterialShader::GetElementShaderBindings(PointerTable, Scene, ViewIfDynamicMeshCommand, VertexFactory, InputStreamType, FeatureLevel, PrimitiveSceneProxy, MeshBatch, BatchElement, ShaderElementData,
		                                              ShaderBindings, VertexStreams);
	}

	LAYOUT_FIELD(FShaderParameter, ExampleColorProperty);
};

IMPLEMENT_MATERIAL_SHADER_TYPE(, FMyMeshPassMaterialShaderPS, TEXT("/Plugin/Runtime/ToShader/MyMeshPassMaterialShader.usf"), TEXT("PSMain"), SF_Pixel);

#pragma endregion

#pragma region MeshPassProcessor
class FMyMeshPassProcessor : public FMeshPassProcessor
{
public:
	FMyMeshPassProcessor(
		const FScene* InScene,
		const FSceneView* InViewIfDynamicMeshCommand,
		FMeshPassDrawListContext* InDrawListContext)
		: FMeshPassProcessor(
			EMeshPass::BasePass, //<-- Is this OK?
			InScene,
			InViewIfDynamicMeshCommand->GetFeatureLevel(), InViewIfDynamicMeshCommand, InDrawListContext
		)
	{
		PassDrawRenderState.SetBlendState(TStaticBlendState<>::GetRHI());
		PassDrawRenderState.SetDepthStencilState(TStaticDepthStencilState<>::GetRHI());
	}

	virtual void AddMeshBatch(const FMeshBatch& RESTRICT MeshBatch, uint64 BatchElementMask, const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy, int32 StaticMeshId = -1) override;

	FMeshPassProcessorRenderState PassDrawRenderState;

	//FMeshRendererPass::FAdditionalData MyMeshPassPrimitivesData;
};

void FMyMeshPassProcessor::AddMeshBatch(const FMeshBatch& RESTRICT MeshBatch, uint64 BatchElementMask, const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy, int32 StaticMeshId)
{
	const FMaterialRenderProxy* MaterialRenderProxy = MeshBatch.MaterialRenderProxy;
	while (MaterialRenderProxy)
	{
		const FMaterial* Material = MaterialRenderProxy->GetMaterialNoFallback(FeatureLevel);
		if (Material && PrimitiveSceneProxy)
		{
			const FVertexFactory* VertexFactory = MeshBatch.VertexFactory;

			TMeshProcessorShaders<
				FMyMeshPassMaterialShaderVS,
				FMyMeshPassMaterialShaderPS> PassShaders;

			FMaterialShaderTypes ShaderTypes;
			ShaderTypes.AddShaderType<FMyMeshPassMaterialShaderVS>();
			ShaderTypes.AddShaderType<FMyMeshPassMaterialShaderPS>();

			FMaterialShaders Shaders;
			if (!Material->TryGetShaders(ShaderTypes, VertexFactory->GetType(), Shaders))
			{
				MaterialRenderProxy = MaterialRenderProxy->GetFallback(FeatureLevel);
				continue;
			}
			Shaders.TryGetVertexShader(PassShaders.VertexShader);
			Shaders.TryGetPixelShader(PassShaders.PixelShader);

			const FMeshDrawingPolicyOverrideSettings OverrideSettings = ComputeMeshOverrideSettings(MeshBatch);
			const ERasterizerFillMode MeshFillMode = ComputeMeshFillMode(*Material, OverrideSettings);
			const ERasterizerCullMode MeshCullMode = CM_None;

			const FMeshDrawCommandSortKey SortKey = CalculateMeshStaticSortKey(PassShaders.VertexShader, PassShaders.PixelShader);

			FMyMeshPassShaderElementData ShaderElementData;
			ShaderElementData.InitializeMeshMaterialData(ViewIfDynamicMeshCommand, PrimitiveSceneProxy, MeshBatch, -1, true);
			// ShaderElementData.ExampleColorProperty = FVector3f(MyMeshPassPrimitivesData.ExampleColorProperty.X, MyMeshPassPrimitivesData.ExampleColorProperty.Y,
			//                                                    MyMeshPassPrimitivesData.ExampleColorProperty.Z);

			BuildMeshDrawCommands(
				MeshBatch,
				BatchElementMask,
				PrimitiveSceneProxy,
				*MaterialRenderProxy,
				*Material,
				PassDrawRenderState,
				PassShaders,
				MeshFillMode,
				MeshCullMode,
				SortKey,
				EMeshPassFeatures::Default,
				ShaderElementData
			);

			break;
		}

		MaterialRenderProxy = MaterialRenderProxy->GetFallback(FeatureLevel);
	}
}

#pragma endregion

#pragma region FMeshRendererPass
FMeshRendererPass::FMeshRendererPass(const FAutoRegister& AutoReg, UWorld* InWorld)
	: FWorldSceneViewExtension(AutoReg, InWorld)
{
}

void FMeshRendererPass::Setup(AMeshRendererPro* MeshRendererPro)
{
	if (MeshRendererPro)
	{
		PassOwner = MeshRendererPro;
		if (PassOwner->OutputRT != nullptr)
		{
			OutputRT = PassOwner->OutputRT->GameThread_GetRenderTargetResource();
			//check(OutputRT);
		}
	}
}

void FMeshRendererPass::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
	check(IsInGameThread());
	if (PassOwner == nullptr) return;
	if (PassOwner->GetWorld())
	{
		// for (auto Component :ShowList)
		// {
		//
		// 	if (Component.IsValid() && Component->IsVisible())
		// 	{
		// 		Meshes.Add(Component->GetSceneProxy());
		//
		// 		// FAdditionalData Data;
		// 		// Data.ExampleColorProperty = Component->ExampleColorProperty;
		// 		// MyMeshPassPrimitivesData.Add(Component->GetSceneProxy(), Data);
		// 	}
		//
		// }
		const FIntPoint RenderTargetSize = InViewFamily.RenderTarget->GetSizeXY();
		RTSize = RenderTargetSize;
		
		UTextureRenderTarget2D* MyMeshPassOutputRT = PassOwner->OutputRT;
		if (MyMeshPassOutputRT->SizeX != RTSize.X
			|| MyMeshPassOutputRT->SizeY != RTSize.Y)
		{
			MyMeshPassOutputRT->ResizeTarget(RTSize.X, RTSize.Y);
			//We must flush rendering commands here to make sure the render target is ready for rendering
			FlushRenderingCommands();
		}
	}
}

bool FMeshRendererPass::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
{
	return true;
}

BEGIN_SHADER_PARAMETER_STRUCT(FMeshRendererPassParameters,)
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
	SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneUniformParameters, Scene)
	SHADER_PARAMETER_STRUCT_INCLUDE(FInstanceCullingDrawParams, InstanceCullingDrawParams)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

void FMeshRendererPass::PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& View, const FRenderTargetBindingSlots& RenderTargets,
                                                                TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures)
{
	if (!PassOwner) return;
	RDG_GPU_MASK_SCOPE(GraphBuilder, View.GPUMask);
	RDG_EVENT_SCOPE(GraphBuilder, "RenderMeshRendererPass");

	TArray< const FPrimitiveSceneProxy* > Meshes;
	TMap<UPrimitiveComponent*,FMaterialGroup> Saved;
	//todo 将Mesh的材质设置为目标材质
	if (PassOwner->GetWorld())
	{
		for (auto Component : ShowList)
		{
			if (Component.IsValid() && Component->IsVisible())
			{
				PassOwner->SaveMeshMaterialWhenRendering(Component.Get(),Saved);
				
				if (Component->GetSceneProxy())
				{
					Meshes.Add(Component->GetSceneProxy());
					// FAdditionalData Data;
					// Data.ExampleColorProperty = FVector::ZeroVector;
					// AdditionalData.Add(Component->GetSceneProxy(), Data);
				}
			}
		}
	}
	tolog("SavedNum : ",Saved.Num());

	if (Meshes.IsEmpty()) return;
	
	if (RTSize.X <= 0 || RTSize.Y <= 0)
	{
		return;
	}
	if (!OutputRT)
	{
		return;
	}
	if (OutputRT->GetSizeXY() != RTSize)
	{
		return;
	}

	FRDGTextureRef DepthTexture = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D(RTSize, PF_DepthStencil, FClearValueBinding::DepthFar, TexCreate_DepthStencilTargetable),
		TEXT("MeshRendererPassTempDepth"));
	const FTextureRHIRef SourceTexture = OutputRT->GetTexture2DRHI();
	FRDGTextureRef MyMeshPassOutputTexture = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(SourceTexture, TEXT("OutputRT")));
	auto PassParameters = GraphBuilder.AllocParameters<FMeshRendererPassParameters>();
	PassParameters->View = View.ViewUniformBuffer;
	PassParameters->Scene = GetSceneUniformBufferRef(GraphBuilder, View);
	PassParameters->RenderTargets[0] = FRenderTargetBinding(MyMeshPassOutputTexture, ERenderTargetLoadAction::EClear);
	PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(DepthTexture, ERenderTargetLoadAction::EClear, ERenderTargetLoadAction::ENoAction, FExclusiveDepthStencil::DepthWrite_StencilNop);

	TArray<const FPrimitiveSceneProxy*> RenderedPrimitives = Meshes;
	//TMap<const FPrimitiveSceneProxy*, FAdditionalData> RenderedPrimitivesData = this->AdditionalData;

	if (!PassParameters || !View.Family->Scene->GetRenderScene()) return;
	
	AddSimpleMeshPass(
		GraphBuilder,
		PassParameters,
		View.Family->Scene->GetRenderScene(),
		View,
		nullptr,
		RDG_EVENT_NAME("RenderMeshRendererPass"),
		View.UnscaledViewRect,
		[View, RenderedPrimitives](FDynamicPassMeshDrawListContext* DynamicMeshPassContext)
		{
			FMyMeshPassProcessor PassMeshProcessor(nullptr, &View, DynamicMeshPassContext);
			if (RenderedPrimitives.IsEmpty()) return;
			for (const FPrimitiveSceneProxy* primitive : RenderedPrimitives)
			{
				if (primitive == nullptr) continue;
				//const FStaticMeshSceneProxy* MeshProxy = static_cast<const FStaticMeshSceneProxy*>(primitive);
				//if (!MeshProxy) continue;
			
				int32 LODIndex = 0;
				TArray<FMeshBatch> MeshElements;
				primitive->GetMeshDescription(LODIndex, MeshElements);
				if (MeshElements.IsEmpty()) continue;
				
				// if (!RenderedPrimitivesData.IsEmpty() && RenderedPrimitivesData.Contains(primitive))
				// 	PassMeshProcessor.MyMeshPassPrimitivesData = RenderedPrimitivesData[primitive];
				PassMeshProcessor.AddMeshBatch(MeshElements[0], 1, primitive);
			}
		});

	if (!Saved.IsEmpty())
	{
		//todo 将Mesh材质替换为原先材质
		for (auto Component : ShowList)
		{
			PassOwner->ResetMeshMaterialAfterRendering(Component.Get(),Saved);
		}
	}
	
}
#pragma endregion
