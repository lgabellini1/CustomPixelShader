#include "CustomSceneViewExtension.h"
#include "ColourExtractRenderPass.h"
#include "PixelShaderUtils.h"
#include "RenderGraphEvent.h"
#include "SceneRendering.h"
#include "PostProcess/PostProcessInputs.h"

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

FVector3f sRGBToLinearRGB(const FVector3f& Colour)
{
	float R = Colour.X <= 0.04045 ? Colour.X / 12.92 : FMath::Pow((Colour.X + 0.055) / 1.055, 2.4);

	float G = Colour.Y <= 0.04045 ? Colour.Y / 12.92 : FMath::Pow((Colour.Y + 0.055) / 1.055, 2.4);

	float B = Colour.Z <= 0.04045 ? Colour.Z / 12.92 : FMath::Pow((Colour.Z + 0.055) / 1.055, 2.4);

	return FVector3f{R, G, B};
}

FVector3f RGBToXYZ(const FVector3f& Colour)
{
	float X = 0.4124564 * Colour.X + 0.3575761 * Colour.Y + 0.1804375 * Colour.Z;

	float Y = 0.2126729 * Colour.X + 0.7151522 * Colour.Y + 0.0721750 * Colour.Z;

	float Z = 0.0193339 * Colour.X + 0.1191920 * Colour.Y + 0.9503041 * Colour.Z;

	return FVector3f{X * 100, Y * 100, Z * 100};
}

FVector3f XYZToLab(const FVector3f& Colour)
{
	const FVector3f D65{0.95047, 1.00000, 1.08883};

	float X = Colour.X / D65.X;
	float Y = Colour.Y / D65.Y;
	float Z = Colour.Z / D65.Z;

	X = X > 0.008856 ? FMath::Pow(X, 1/3) : (7.787 * X + 16/116.0);
	Y = Y > 0.008856 ? FMath::Pow(Y, 1/3) : (7.787 * Y + 16/116.0);
	Z = Z > 0.008856 ? FMath::Pow(Z, 1/3) : (7.787 * Z + 16/116.0);

	float L = 116 * Y - 16;
	float a = 500 * (X - Y);
	float b = 500 * (Y - Z);

	return FVector3f{L, a, b};
}

FVector3f RGBToLab(const FVector3f& Colour)
{
	return XYZToLab(RGBToXYZ(sRGBToLinearRGB(Colour)));
}
