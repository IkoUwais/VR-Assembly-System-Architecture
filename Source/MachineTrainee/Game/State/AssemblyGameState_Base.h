#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Game.h"



#include "AssemblyGameState_Base.generated.h"


class UAssemblyDataSubsystem;
class AAssemblyManager;
class APart_Base;


// New delegate — fired when async session load completes. GameMode subscribes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateSessionReady, bool, bSuccess);

// Broadcast when a new step becomes active and its silhouette is shown.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateStepStarted, int32, StepIndex, FPartID, ExpectedPartID);

// Fires on any failed placement attempt (wrong part or out of tolerance).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateStepFailed, int32, StepIndex, FPartID, AttemptedPartID);

// Fires when the next part needs to be shown on the HUD.
// (Already have OnStepStarted, but this can be for specific UI updates)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGameStateStepAdvanced, int32, StepIndex, FPartID, PlacedPartID, APart_Base*, PlacedPart);

// Fires when a full sequence completes. Carries the final result for UI.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateSequenceCompleted, FAssemblySessionResult, AssemblySessionResult);


/**
 * The project's "asset-manager" and outcome tracker.
 *
 * Owns:
 *  - The runtime lookup: FPartID -> UPartDefinition (extended later for AssemblyID -> AssemblyDefinition).
 *  - The single validation gate: sequences are validated here ONCE and firmly.
 *    If validation fails, the sequence never starts — the AssemblyManager never sees it.
 *  - Runtime outcome state: FailedAttempts, SequenceStartTime, LastResult.
 *  - Event subscription to the AssemblyManager. The assembly-manager broadcasts; GameState holds values.
 *
 * Does NOT:
 *  - Spawn actors (GameMode drives the Spawner).
 *  - Drive the step machine (Assembly-Manager owns it).
 *  - Know about GameMode (dependency is one-way: GameMode -> GameState).
 *
 * Data flow:
 *   GameMode -> GameState (start sequence)
 *   GameState -> Assembly-Manager (StartSequence, after validation passes)
 *   AssemblyManager -> GameState (delegate broadcasts on step events)
 *   GameState -> UI (delegate broadcasts, later)
 *
 * Pattern: Mediator between GameMode and Assembly-Manager.
 * Pattern: Service Locator for part data (single shared lookup).
 * Pattern: Observer — subscribes to Assembly-Manager, rebroadcasts to UI.
 */
UCLASS(BlueprintType)
class MACHINETRAINEE_API AAssemblyGameState_Base : public AGameState {
	
	
	GENERATED_BODY()
	
	
public:
	
	AAssemblyGameState_Base();
	
protected:
	
	virtual void BeginPlay() override;
	
public:
	
	// - Events
	UPROPERTY(BlueprintAssignable)
	FOnGameStateSessionReady OnSessionReady;

	UPROPERTY(BlueprintAssignable)
	FOnGameStateStepStarted OnStepStarted;
	
	UPROPERTY(BlueprintAssignable)
	FOnGameStateStepFailed OnStepFailed;

	UPROPERTY(BlueprintAssignable)
	FOnGameStateStepAdvanced OnStepAdvanced;
	
	UPROPERTY(BlueprintAssignable)
	FOnGameStateSequenceCompleted OnSequenceCompleted;
	
	
	
	// - Setup — called by GameMode - //
	
	// Called by GameMode. Kicks off async load via subsystem, then spawn + start.
	UFUNCTION(BlueprintCallable)
	void StartGameSession(const FAssemblySequenceID In_AssemblyID);
	
	// Hand the GameState it's assembly-manager for this level. Binds delegates.
	// Must be called before BeginSequence.
	UFUNCTION(BlueprintCallable)
	void RegisterAssemblyManager(AAssemblyManager* In_Manager);

	
	
	// - Execution — called by GameMode - //

	// Preconditions: PrepareSequence returned true, assembly-manager is registered.
	// Captures start time, marks active, calls Assembly-Manager->StartSequence().
	UFUNCTION(BlueprintCallable)
	void BeginAssemblySequence();

	// Cancels the active sequence. Assembly-Manager stops, GameState clears active flag.
	UFUNCTION(BlueprintCallable)
	void AbortAssemblySequence();
	
	
	// Clear session-scoped runtime state. Called by GameMode on EndAssemblySession.
	UFUNCTION(BlueprintCallable)
	void CleanupSession();
	
	
	
	// - Lookup — called by Spawner - //

	// Resolve a PartID to its data payload. Nullptr if not registered.
	// This is what the spawner hands to APart_Base::InitializePartWithData.
	const FPartData* GetResolvedPartData(const FPartID& In_PartID) const;
	
	const FAssemblySequenceData* GetCachedSessionData() const {
		
		return CachedSessionData.Steps.Num() > 0 ? &CachedSessionData : nullptr;
		
	}
	
	UFUNCTION(BlueprintPure)
	bool GetPartVisual(const FPartID& In_PartID , FPartVisual& Out_PartVisual);
	
	// - U I - //
	UFUNCTION(BlueprintImplementableEvent, Category = "Assembly|UI")
	void ShowLoadingUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Assembly|UI")
	void HideLoadingUI();
	
	
	// - T R A C K I N G - //
	// C++-friendly variant. Returns nullptr if not found — zero copy, no Out param.
	const FStepFailureRecord* FindFailureRecordByExpectedPart(const FPartID In_PartID) const;
	
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAssemblyDataSubsystem> CachedAssemblyDataSubsystem;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FAssemblySequenceID PendingSessionID;
	
	// Cached session-scoped data — populated by HandleSessionDataLoaded.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FAssemblySequenceData CachedSessionData;
	

	// Setup references. Assembly-Manager is assigned via RegisterAssemblyManager.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AAssemblyManager> AssemblyManager;

	// Runtime outcome state. This is where the values live — the assembly-manager only broadcasts.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSequenceActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 FailedAttempts = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float SequenceStartTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FAssemblySessionResult LastResult;
	
	
	// - T R A C K I N G - //
	// Session-scoped failure tracking. Populated by HandleStepFailed.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FAssemblyFailureSummary AssemblyFailureSummary;
		
	// Async complete callback from the subsystem.
	void HandleSessionDataLoaded(bool bSuccess);
	
	
	UFUNCTION()
	void HandleStepStarted(int32 In_StepIndex, FPartID In_ExpectedPartID);

	UFUNCTION()
	void HandleStepValidated(int32 In_StepIndex, FPartID In_PlacedPartID, APart_Base* PlacedPart);

	UFUNCTION()
	void HandleStepFailed(int32 In_StepIndex, FPartID In_AttemptedPartID);

	UFUNCTION()
	void HandleSequenceCompleted();
	
	
private:
	
	void BroadcastOnSessionReady(const bool In_Success);
	
	
public:
	
	UFUNCTION(BlueprintPure)
	bool IsSequenceActive() const { return bSequenceActive; }

	UFUNCTION(BlueprintPure)
	int32 GetFailedAttempts() const { return FailedAttempts; }

	UFUNCTION(BlueprintPure)
	float GetSequenceElapsedSeconds() const;

	UFUNCTION(BlueprintPure)
	FAssemblySessionResult GetLastResult() const { return LastResult; }
	
	// - T R A C K I N G - //
	// Find the failure record for the step whose expected part is In_PartID.
	// Returns true and fills Out_Record if found.
	// Blueprint-friendly: use this from the UI.
	UFUNCTION(BlueprintPure)
	bool GetFailureRecordByExpectedPart(const FPartID In_PartID, FStepFailureRecord& Out_Record) const;

	// Find the failure record for a specific step index.
	UFUNCTION(BlueprintPure)
	bool GetFailureRecordByStepIndex(const int32 In_StepIndex, FStepFailureRecord& Out_Record) const;
	
};