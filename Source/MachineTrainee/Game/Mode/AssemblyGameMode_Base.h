#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Game.h"



#include "AssemblyGameMode_Base.generated.h"


class UPartDefinition;
class APart_Base;
class UAssemblySequenceDefinition;
class ASilhouette_Base;
class AAssemblySpawner;
class AAssemblyManager;
class AAssemblyGameState_Base;


UCLASS()
class MACHINETRAINEE_API AAssemblyGameMode_Base : public AGameMode {
	
	
	GENERATED_BODY()
	
	
public:
	
	// overrides
	
protected:
	
	virtual void BeginPlay() override;
	
public:
	
	// Single entry point. Starts the whole assembly session.
	// Order: register catalog -> register manager -> validate -> spawn -> begin.
	// If validation fails, nothing downstream runs. Loud log, clean abort.
	UFUNCTION(BlueprintCallable)
	void StartAssemblySession();

	// Ends the current session cleanly. Safe to call anytime.
	UFUNCTION(BlueprintCallable)
	void EndAssemblySession();

protected:

	// Identifier for the Assembly that this Level and Game-Mode is going to host
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAssemblySequenceID AssemblySequenceID;
	
	// The actor class used to spawn all parts.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<APart_Base> PartActorClass;
	
	// How many table parts spawn at sequence start. Rest trickle in later.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 InitialTableSpawnCount = 3;

	// The manager to be placed in the level for this station.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAssemblyManager> AssemblyManagerClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform AssemblyManagerSpawnTransform;
		
	
	
	// The spawner to be placed in the level.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAssemblySpawner> AssemblySpawnerClass;
	
	
	// The silhouette actor to be placed in the level.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ASilhouette_Base> SilhouetteActorClass;
	
	
	
	// The manager that is placed in the level for this station.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AAssemblyManager> AssemblyManager;
	
	// The spawner that is placed in the level.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AAssemblySpawner> AssemblySpawner;
	
	// The silhouette actor that is placed in the level.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ASilhouette_Base> SilhouetteActor;
	
	
	
	// Cached at BeginPlay. Null if World Settings points to a different GameState class.
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AAssemblyGameState_Base> CachedGameState;
	
	UPROPERTY(BlueprintReadOnly)
	bool bSessionStarted = false;


	void SpawnActorsForGameplayAssembly();
	
	UFUNCTION()
	void HandleSessionReady(bool bSuccess);
	
	// Pushes editor references into runtime systems. Called once from StartAssemblySession.
	// Returns false if any required reference is missing.
	bool IsAssemblyInitializedForGameplay();

	// Reads the sequence and fills Out_Data with base + table IDs and transforms.
	bool GetBuiltSpawnDataFromSequence(FSpawnData& Out_Data) const;
	
private:
	
	//
	
public:
	
	UFUNCTION(BlueprintPure)
	bool HasSessionStarted() const { return bSessionStarted; }
	
};