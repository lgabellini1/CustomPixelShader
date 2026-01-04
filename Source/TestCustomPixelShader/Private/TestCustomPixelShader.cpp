#include "TestCustomPixelShader.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FTestCustomPixelShaderModule"

void FTestCustomPixelShaderModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	const FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(
		TEXT("TestCustomPixelShader"))->GetBaseDir(), TEXT("Shaders"));
	if (!AllShaderSourceDirectoryMappings().Contains(TEXT("/CustomShaders")))
	{
		AddShaderSourceDirectoryMapping(TEXT("/CustomShaders"), PluginShaderDir);
	}
}

void FTestCustomPixelShaderModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTestCustomPixelShaderModule, TestCustomPixelShader)
