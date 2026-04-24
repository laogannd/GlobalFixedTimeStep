// Copyright DGOne. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GlobalFixedTimeStepSubsystem.generated.h"

/** Fired once per fixed substep. Parameter is the fixed delta time in seconds. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlobalFixedTick, float, FixedDeltaTime);

/** Fired once per frame after all substeps complete. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalFixedTickFinished);

/**
 * Global fixed timestep update system.
 * Auto-created per UWorld as a UTickableWorldSubsystem.
 * Implements the classic accumulator pattern: accumulates frame DeltaTime,
 * fires OnFixedTick N times when accumulated time >= fixed step size.
 * Max substeps per frame are capped to prevent spiral of death.
 */
UCLASS()
class GLOBALFIXEDTIMESTEP_API UGlobalFixedTimeStepSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UGlobalFixedTimeStepSubsystem();

	//~ USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override;

	// --- Delegates (Blueprint-bindable) ---

	/** Subscribe to receive fixed-rate ticks. Fires N times per frame (once per substep). */
	UPROPERTY(BlueprintAssignable, Category="Fixed TimeStep")
	FOnGlobalFixedTick OnFixedTick;

	/** Fires once per frame after all fixed substeps complete. Useful for interpolation/reconciliation. */
	UPROPERTY(BlueprintAssignable, Category="Fixed TimeStep")
	FOnGlobalFixedTickFinished OnFixedTickFinished;

	// --- Runtime Configuration ---

	/** Set the fixed time step interval at runtime. Resets the accumulator. */
	UFUNCTION(BlueprintCallable, Category="Fixed TimeStep")
	void SetFixedTimeStep(float NewFixedTimeStep);

	/** Set the maximum substeps per frame. */
	UFUNCTION(BlueprintCallable, Category="Fixed TimeStep")
	void SetMaxSubStepsPerFrame(int32 NewMaxSubSteps);

	/** Enable or disable the fixed tick system at runtime. Resets accumulator when re-enabling. */
	UFUNCTION(BlueprintCallable, Category="Fixed TimeStep")
	void SetEnabled(bool bNewEnabled);

	/** Zero the accumulator. Call after loading screens or long pauses. */
	UFUNCTION(BlueprintCallable, Category="Fixed TimeStep")
	void ResetAccumulator();

	// --- Read-only Accessors ---

	/** Returns the current fixed time step interval in seconds. */
	UFUNCTION(BlueprintPure, Category="Fixed TimeStep")
	float GetFixedTimeStep() const;

	/** Returns the max substeps per frame. */
	UFUNCTION(BlueprintPure, Category="Fixed TimeStep")
	int32 GetMaxSubStepsPerFrame() const;

	/** Returns true if the system is enabled. */
	UFUNCTION(BlueprintPure, Category="Fixed TimeStep")
	bool IsEnabled() const;

	/** Returns 0..1 interpolation alpha for rendering smoothing between fixed steps. */
	UFUNCTION(BlueprintPure, Category="Fixed TimeStep")
	float GetInterpolationAlpha() const;

	/** Returns total number of fixed ticks fired since world start. */
	UFUNCTION(BlueprintPure, Category="Fixed TimeStep")
	int64 GetFixedTickCount() const;

	/** Returns how many substeps fired in the most recent frame. */
	UFUNCTION(BlueprintPure, Category="Fixed TimeStep")
	int32 GetSubStepsThisFrame() const;

private:
	/** Current fixed time step in seconds. */
	float CurrentFixedTimeStep;

	/** Current max substeps cap. */
	int32 CurrentMaxSubSteps;

	/** Whether the system is active. */
	bool bSystemEnabled;

	/** Accumulator for time (double precision for long-session stability). */
	double Accumulator;

	/** Cached interpolation alpha (0..1). */
	float CachedInterpolationAlpha;

	/** Total fixed ticks fired since Initialize. */
	int64 TotalFixedTickCount;

	/** Substeps that fired in the most recent Tick call. */
	int32 LastSubStepsThisFrame;
};
