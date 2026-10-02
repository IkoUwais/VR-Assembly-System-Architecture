#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Game.h"



#include "AssemblySpawner.generated.h"


class APart_Base;
class APartSpawnPoint;
class AAssemblyManager;
class AAssemblyGameState_Base;


/**
 * Spawns parts for an assembly session.
 *
 * Every level must contain one spawner. GameMode calls SpawnAssemblyFromData once
 * at sequence start. Validation is already done by the time this runs — the spawner
 * trusts FSpawnData entirely.
 *
 * Responsibilities:
 *  - Spawn the base part and lock it in place via PartAppliedToAssembly.
 *  - Spawn the initial K table parts on available APartSpawnPoint markers.
 *  - Register every spawned part with the AssemblyManager.
 *  - Resolve FPartData through the GameState (single lookup owner).
 *
 * Does NOT:
 *  - Validate the sequence (GameState owns this — validation is a precondition).
 *  - Drive the step machine (Manager owns it).
 *  - Handle conveyor spawning yet (D13 — deferred; hook is the same entry point).
 *
 * Pattern: Builder — constructs the assembly from a description.
 * Pattern: Dependency Injection — Manager + GameState + PartActorClass are pushed in by GameMode.
 */
UCLASS(BlueprintType)
class MACHINETRAINEE_API AAssemblySpawner : public AActor {
	
	
	GENERATED_BODY()

	
public:

	AAssemblySpawner();
	
protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	
	// Spawn the whole assembly described by In_SpawnData.
	// Called by GameMode once PrepareSequence has succeeded.
	UFUNCTION(BlueprintCallable)
	void SpawnAssemblyFromData(const FSpawnData& In_SpawnData);

	// - Wiring — set by GameMode before calling SpawnAssemblyFromData - //

	void InitializeSpawnerForGameplay(AAssemblyManager* In_AssemblyManager, AAssemblyGameState_Base* In_AssemblyGameState, TSubclassOf<APart_Base> In_PartClass);
	
	
	// Destroy every part this spawner created and clear all spawner state.
	// Called by GameMode on EndAssemblySession.
	UFUNCTION(BlueprintCallable)
	void CleanupSpawnedAssembly();
	
	
	// Highlight every currently spawned, uninstalled part matching In_PartID.
	// Clears any previously highlighted set first.
	UFUNCTION(BlueprintCallable)
	void HighlightPartsByID(const FPartID In_PartID);

	// Clear highlights from every part this spawner is currently tracking.
	UFUNCTION(BlueprintCallable)
	void ClearAllHighlights();
	
		
protected:
	
	// Spawn point markers.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<APartSpawnPoint>> SpawnPoints;
	
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AAssemblyManager> AssemblyManager;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AAssemblyGameState_Base> GameState;

	// Class used to spawn every part. Designers set in editor.
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<APart_Base> PartActorClass;
	
	
	// Parts still waiting to be spawned (conveyor queue). Front = next to spawn.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FPartID> PendingPartIDs;
	
	// Which marker each currently-spawned table part came from.
	// Freed when that part is successfully placed.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<TObjectPtr<APart_Base>, TObjectPtr<APartSpawnPoint>> PartMarkerMap;
	
	
	// Parts currently highlighted by this spawner. Cleared on step change or session end.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<APart_Base>> HighlightedParts;
	
	
	// Fires when a new step begins. Highlights the matching table parts.
	UFUNCTION()
	void HandleStepStarted(int32 In_StepIndex, FPartID In_ExpectedPartID);
	
	// Fires when the trainee successfully places a part. Frees that part's marker,
	// then pulls the next pending part onto the freed marker.
	UFUNCTION()
	void HandleStepValidated(int32 In_StepIndex, FPartID In_PlacedPartID, APart_Base* In_PlacedPart);
	
	// Spawn one part. Resolves FPartData via GameState. Returns nullptr on failure.
	APart_Base* SpawnSinglePart(const FPartID& In_PartID, const FTransform& In_Transform);
	
	// Pop the front of PendingPartIDs and spawn it on the first free marker.
	// No-op if queue empty or no free marker.
	void SpawnNextPendingPart();
	
	
	// Cache all APartSpawnPoint markers in the world.
	void CollectSpawnPointsFromLevelWorld();
	
	// Returns the first free spawn point, or nullptr if all are occupied.
	APartSpawnPoint* AcquireFreeSpawnPoint();
	
	
	// Reset all markers to unoccupied. Called at the start of SpawnAssemblyFromData.
	void ResetAllMarkers();

	// Clear queue + marker map + free all markers. Called at start of SpawnAssemblyFromData.
	void ResetSpawnState();
	
	
private:
	
	
public:
	
	UFUNCTION(BlueprintPure)
	int32 GetPendingCount() const { return PendingPartIDs.Num(); }
	
};