// Copyright DGOne. All Rights Reserved.

#include "GlobalFixedTimeStepSubsystem.h"
#include "GlobalFixedTimeStepSettings.h"

DECLARE_STATS_GROUP(TEXT("GlobalFixedTimeStep"), STATGROUP_GlobalFixedTimeStep, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("GlobalFixedTimeStep Tick"), STAT_GlobalFixedTimeStepTick, STATGROUP_GlobalFixedTimeStep);

UGlobalFixedTimeStepSubsystem::UGlobalFixedTimeStepSubsystem()
	: CurrentFixedTimeStep(1.0f / 60.0f)
	, CurrentMaxSubSteps(8)
	, bSystemEnabled(true)
	, Accumulator(0.0)
	, CachedInterpolationAlpha(0.0f)
	, TotalFixedTickCount(0)
	, LastSubStepsThisFrame(0)
{
}

void UGlobalFixedTimeStepSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UGlobalFixedTimeStepSettings* Settings = UGlobalFixedTimeStepSettings::Get();
	if (Settings)
	{
		CurrentFixedTimeStep = FMath::Clamp(Settings->FixedTimeStep, 0.0001f, 1.0f);
		CurrentMaxSubSteps = FMath::Clamp(Settings->MaxSubStepsPerFrame, 1, 64);
		bSystemEnabled = Settings->bEnableFixedTick;
	}

	Accumulator = 0.0;
	CachedInterpolationAlpha = 0.0f;
	TotalFixedTickCount = 0;
	LastSubStepsThisFrame = 0;
}

void UGlobalFixedTimeStepSubsystem::Deinitialize()
{
	OnFixedTick.Clear();
	OnFixedTickFinished.Clear();

	Super::Deinitialize();
}

void UGlobalFixedTimeStepSubsystem::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_GlobalFixedTimeStepTick);

	if (!bSystemEnabled || CurrentFixedTimeStep <= 0.0f)
	{
		LastSubStepsThisFrame = 0;
		return;
	}

	// Accumulate frame time
	Accumulator += static_cast<double>(DeltaTime);

	// Cap accumulator to prevent spiral of death
	const double MaxAccumulator = static_cast<double>(CurrentFixedTimeStep) * static_cast<double>(CurrentMaxSubSteps);
	if (Accumulator > MaxAccumulator)
	{
		Accumulator = MaxAccumulator;
	}

	// Fire fixed ticks
	int32 StepsThisFrame = 0;
	const double FixedStep = static_cast<double>(CurrentFixedTimeStep);

	while (Accumulator >= FixedStep)
	{
		OnFixedTick.Broadcast(CurrentFixedTimeStep);
		Accumulator -= FixedStep;
		++StepsThisFrame;
		++TotalFixedTickCount;
	}

	LastSubStepsThisFrame = StepsThisFrame;

	// Compute interpolation alpha (0..1) for rendering smoothing
	CachedInterpolationAlpha = static_cast<float>(Accumulator / FixedStep);

	// Notify that all substeps for this frame are done
	if (StepsThisFrame > 0)
	{
		OnFixedTickFinished.Broadcast();
	}
}

TStatId UGlobalFixedTimeStepSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UGlobalFixedTimeStepSubsystem, STATGROUP_GlobalFixedTimeStep);
}

bool UGlobalFixedTimeStepSubsystem::IsTickable() const
{
	return bSystemEnabled;
}

bool UGlobalFixedTimeStepSubsystem::IsTickableInEditor() const
{
	return false;
}

// --- Runtime Configuration ---

void UGlobalFixedTimeStepSubsystem::SetFixedTimeStep(float NewFixedTimeStep)
{
	CurrentFixedTimeStep = FMath::Clamp(NewFixedTimeStep, 0.0001f, 1.0f);
	Accumulator = 0.0;
	CachedInterpolationAlpha = 0.0f;
}

void UGlobalFixedTimeStepSubsystem::SetMaxSubStepsPerFrame(int32 NewMaxSubSteps)
{
	CurrentMaxSubSteps = FMath::Clamp(NewMaxSubSteps, 1, 64);
}

void UGlobalFixedTimeStepSubsystem::SetEnabled(bool bNewEnabled)
{
	if (!bSystemEnabled && bNewEnabled)
	{
		// Re-enabling: reset accumulator to avoid burst of ticks
		Accumulator = 0.0;
		CachedInterpolationAlpha = 0.0f;
	}
	bSystemEnabled = bNewEnabled;
}

void UGlobalFixedTimeStepSubsystem::ResetAccumulator()
{
	Accumulator = 0.0;
	CachedInterpolationAlpha = 0.0f;
}

// --- Read-only Accessors ---

float UGlobalFixedTimeStepSubsystem::GetFixedTimeStep() const
{
	return CurrentFixedTimeStep;
}

int32 UGlobalFixedTimeStepSubsystem::GetMaxSubStepsPerFrame() const
{
	return CurrentMaxSubSteps;
}

bool UGlobalFixedTimeStepSubsystem::IsEnabled() const
{
	return bSystemEnabled;
}

float UGlobalFixedTimeStepSubsystem::GetInterpolationAlpha() const
{
	return CachedInterpolationAlpha;
}

int64 UGlobalFixedTimeStepSubsystem::GetFixedTickCount() const
{
	return TotalFixedTickCount;
}

int32 UGlobalFixedTimeStepSubsystem::GetSubStepsThisFrame() const
{
	return LastSubStepsThisFrame;
}
