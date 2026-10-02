#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Game.h"



#include "AssemblyManager.generated.h"


class UAssemblySequenceDefinition;
class AAssemblyGameState_Base;
class APart_Base;
class ASilhouette_Base;


// Broadcast when a new step becomes active and its silhouette is shown.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAssemblyStepStarted, int32, StepIndex, FPartID, ExpectedPartID);

// Broadcast when the trainee successfully places a part at the current step.
// PlacedPart is the actor that was just snapped — the spawner uses it to free its marker.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAssemblyStepValidated, int32, StepIndex, FPartID, PlacedPartID, APart_Base*, PlacedPart);

// Broadcast when the full sequence completes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAssemblySequenceCompleted);

// Fires on any failed placement attempt — wrong part OR out of tolerance.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAssemblyStepFailed, int32, StepIndex, FPartID, AttemptedPartID);


/**
 * Runs the assembly step machine for a single station.
 *
 * Responsibilities (post-trim):
 *  - Owns CurrentStepIndex and the step lifecycle.
 *  - Drives the silhouette via ASilhouette_Base.
 *  - Validates placement position/rotation against the current step.
 *  - Broadcasts step events. Never holds outcome counters — those live on GameState.
 *
 * Does NOT:
 *  - Validate sequence data (GameState owns this — one-shot, firm).
 *  - Own the PartID -> UPartDefinition lookup (GameState owns it).
 *  - Spawn actors (Spawner owns this, driven by GameMode).
 *
 * Pattern: Facade / Mediator (Tier 1) — single coordination point.
 * Pattern: State (Tier 1) — implicit machine over CurrentStepIndex + bIsRunningSequence.
 * Pattern: Observer (Tier 1) — broadcasts without knowing subscribers.
 */
UCLASS(BlueprintType)
class MACHINETRAINEE_API AAssemblyManager : public AActor {
	
	
	GENERATED_BODY()

	
public:

	AAssemblyManager();

protected:
	
	// overrides
	
public:
	
	// - Events - //

	UPROPERTY(BlueprintAssignable)
	FOnAssemblyStepStarted OnStepStarted;

	UPROPERTY(BlueprintAssignable)
	FOnAssemblyStepValidated OnStepValidated;

	UPROPERTY(BlueprintAssignable)
	FOnAssemblySequenceCompleted OnSequenceCompleted;
	
	UPROPERTY(BlueprintAssignable)
	FOnAssemblyStepFailed OnStepFailed;
	
	// - Events - //
	
	
	
	// - Setup — called by GameState / level wiring - //

	UFUNCTION(BlueprintCallable)
	void InitializeAssemblyManagerForGameplay(AAssemblyGameState_Base* In_GameState, ASilhouette_Base* In_Silhouette);

	// Register the pre-placed base part. Spawner calls this after locking the base.
	UFUNCTION(BlueprintCallable)
	void RegisterBasePart(APart_Base* In_BasePart);

	// Register a table part (spawned at sequence start or via conveyor later).
	UFUNCTION(BlueprintCallable)
	void RegisterTablePart(APart_Base* In_TablePart);
	
	// - Setup — called by GameState / level wiring - //
	
	
	
	// - Sequence Control - //

	UFUNCTION(BlueprintCallable)
	void StartSequence(const FAssemblySequenceData& In_AssemblySequenceData);

	UFUNCTION(BlueprintCallable)
	void StartCurrentSequence();
	
	UFUNCTION(BlueprintCallable)
	void EndCurrentSequence();

	UFUNCTION(BlueprintCallable)
	void AdvanceStepForNextPart();

	// Entry point from APart_Base::OnReleased. Manager decides snap vs drop.
	void OnPartReleased(APart_Base* In_ReleasedPart);
	
	// - Sequence Control - //

	
	// - called by the part-base to start glowing the silhouette green!
	void PartGrabbedForAssembly(APart_Base* In_Part);
	
	
	// Destroy every tracked part and reset manager state. Called by GameMode on EndAssemblySession.
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	void CleanupAssembly();
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> Silhouette_SuccessGlowMaterial;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> Silhouette_SuccessOverlayMaterial;
	
	
	// - Cached References - //

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AAssemblyGameState_Base> GameState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ASilhouette_Base> SilhouetteActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FAssemblySequenceData CachedAssemblySequenceData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<APart_Base> BasePart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<APart_Base>> TrackedParts;

	
	
	// - Runtime State - //

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentStepIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsRunningSequence = false;
	
	
	FTimerHandle TH_PartGlowing;
	
	// Identity + position + rotation check against the current step.
	bool IsPlacementValid(APart_Base* In_Part) const;

	// Returns the current step or nullptr if index is out of range.
	const FAssemblyStep* GetCurrentStep() const;

	// Resolves mesh for the step via GameState and shows the silhouette.
	// Returns false if the mesh or GameState reference is missing.
	bool ShowSilhouetteForStep(const FAssemblyStep& In_Step);

	// Finalizes the sequence. Broadcasts OnSequenceCompleted. Does not build the result struct.
	void CompleteSequence();
	
	void RegisterAndTrackPart(APart_Base* In_Part);

	void StopGlowingTimer();
	
private:
	
	
	
public:
	
	UFUNCTION(BlueprintPure)
	int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	UFUNCTION(BlueprintPure)
	int32 GetTotalStepCount() const;

	UFUNCTION(BlueprintPure)
	bool IsRunningSequence() const { return bIsRunningSequence; }

	UFUNCTION(BlueprintPure)
	APart_Base* GetBasePart() const { return BasePart; }

};