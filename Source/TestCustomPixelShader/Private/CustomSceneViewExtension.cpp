#include "CustomSceneViewExtension.h"
#include "ColourExtractRenderPass.h"
#include "PixelShaderUtils.h"
#include "RenderGraphEvent.h"
#include "SceneRendering.h"
#include "PostProcess/PostProcessInputs.h"
#include "TestCustomPixelShader/Public/Utils.h"

DECLARE_GPU_DRAWCALL_STAT(ColourExtract);

FCustomSceneViewExtension::FCustomSceneViewExtension(const FAutoRegister& AutoRegister)
	: FSceneViewExtensionBase{AutoRegister}
{}

void FCustomSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& InView,
																const FPostProcessingInputs& Inputs)
{
	FSceneViewExtensionBase::PrePostProcessPass_RenderThread(GraphBuilder, InView, Inputs);

	checkSlow(InView.bIsViewInfo);
	const FIntRect Viewport = static_cast<const FViewInfo&>(InView).ViewRect;
	const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

	RDG_GPU_STAT_SCOPE(GraphBuilder, ColourExtract); // for stat gpu
	RDG_EVENT_SCOPE(GraphBuilder, "Colour Extract"); // for RenderDoc

	const FSceneTextureShaderParameters SceneTextures = CreateSceneTextureShaderParameters(GraphBuilder, InView,
		ESceneTextureSetupMode::SceneColor | ESceneTextureSetupMode::GBuffers);
	
	const FScreenPassTexture SceneColourTexture{(*Inputs.SceneTextures)->SceneColorTexture, Viewport};

	// Allocate shader parameters 
	FColourExtractPS::FParameters* ShaderParameters = GraphBuilder.AllocParameters<FColourExtractPS::FParameters>();
	ShaderParameters->SceneColourTexture = SceneColourTexture.Texture;
	ShaderParameters->SceneTextures = SceneTextures;
	ShaderParameters->TargetColour = RGBToLab(FVector3f{1.0f, 0.0f, 0.0f});
	ShaderParameters->View = InView.ViewUniformBuffer;
	ShaderParameters->RenderTargets[0] = FRenderTargetBinding{(*Inputs.SceneTextures)->SceneColorTexture,
		ERenderTargetLoadAction::ELoad};
	
	TShaderMapRef<FColourExtractPS> PixelShader{GlobalShaderMap};
	FPixelShaderUtils::AddFullscreenPass(GraphBuilder, GlobalShaderMap, FRDGEventName{TEXT("Colour Extract Pass")},
		PixelShader, ShaderParameters, Viewport);
}
