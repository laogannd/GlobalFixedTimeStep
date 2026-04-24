// Copyright DGOne. All Rights Reserved.

#include "GlobalFixedTimeStepSettings.h"

UGlobalFixedTimeStepSettings::UGlobalFixedTimeStepSettings()
	: FixedTimeStep(1.0f / 60.0f)
	, MaxSubStepsPerFrame(8)
	, bEnableFixedTick(true)
{
}

const UGlobalFixedTimeStepSettings* UGlobalFixedTimeStepSettings::Get()
{
	return GetDefault<UGlobalFixedTimeStepSettings>();
}

FName UGlobalFixedTimeStepSettings::GetContainerName() const
{
	return TEXT("Project");
}

FName UGlobalFixedTimeStepSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FName UGlobalFixedTimeStepSettings::GetSectionName() const
{
	return TEXT("GlobalFixedTimeStep");
}

#if WITH_EDITOR
FText UGlobalFixedTimeStepSettings::GetSectionText() const
{
	return NSLOCTEXT("GlobalFixedTimeStep", "SettingsSection", "Global Fixed TimeStep");
}

FText UGlobalFixedTimeStepSettings::GetSectionDescription() const
{
	return NSLOCTEXT("GlobalFixedTimeStep", "SettingsDescription",
		"Configure the global fixed timestep update system: tick rate, max substeps, and enable/disable.");
}

void UGlobalFixedTimeStepSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FixedTimeStep = FMath::Clamp(FixedTimeStep, 0.0001f, 1.0f);
	MaxSubStepsPerFrame = FMath::Clamp(MaxSubStepsPerFrame, 1, 64);
}
#endif
