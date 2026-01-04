#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "CustomSceneViewExtension.h"
#include "CustomShaderSubsystem.generated.h"

UCLASS()
class UCustomShaderSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
private:
	TSharedPtr<FCustomSceneViewExtension> CustomSceneViewExtension;
};