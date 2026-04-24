// Copyright DGOne. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GlobalFixedTimeStepSettings.generated.h"

/**
 * Project-wide default settings for the Global Fixed TimeStep system.
 * Accessible via Project Settings → Plugins → Global Fixed TimeStep.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Global Fixed TimeStep"))
class GLOBALFIXEDTIMESTEP_API UGlobalFixedTimeStepSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGlobalFixedTimeStepSettings();

	/** Fixed time step interval in seconds. Default: 1/60 (~0.01667s). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Fixed TimeStep",
		meta=(ClampMin="0.0001", ClampMax="1.0", UIMin="0.001", UIMax="0.1"))
	float FixedTimeStep;

	/** Maximum substeps per frame to prevent spiral of death. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Fixed TimeStep",
		meta=(ClampMin="1", ClampMax="64"))
	int32 MaxSubStepsPerFrame;

	/** Master switch to globally enable/disable the fixed tick system. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Fixed TimeStep")
	bool bEnableFixedTick;

	/** Singleton accessor. */
	static const UGlobalFixedTimeStepSettings* Get();

	//~ UDeveloperSettings interface
	virtual FName GetContainerName() const override;
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
