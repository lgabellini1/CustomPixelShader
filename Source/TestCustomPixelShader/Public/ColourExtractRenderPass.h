#pragma once

#include "CoreMinimal.h"
#include "SceneTexturesConfig.h"
#include "ShaderParameterStruct.h"

/*
 * This file includes a shader parameter struct paired with the FShader.
 */


BEGIN_SHADER_PARAMETER_STRUCT(FColourExtractParams,)
	SHADER_PARAMETER(FVector3f, TargetColour)
	
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColourTexture)

	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)

	// Reference to uniform buffer's textures (for SceneColor)
    SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureShaderParameters, SceneTextures)

	// Needed if outputting to a render target
    RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()


class FColourExtractPS : public FGlobalShader
{
	DECLARE_EXPORTED_GLOBAL_SHADER(FColourExtractPS, )
	
	// Generates a constructor for the class 
	SHADER_USE_PARAMETER_STRUCT(FColourExtractPS, FGlobalShader);
	
	using FParameters = FColourExtractParams;

	
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		SET_SHADER_DEFINE(OutEnvironment, USE_UNLIT_SCENE_COLOUR, 0);
	}	
};
