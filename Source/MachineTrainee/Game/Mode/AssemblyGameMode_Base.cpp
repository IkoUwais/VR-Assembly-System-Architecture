
#include "AssemblyGameMode_Base.h"
#include "MachineTrainee/Actors/AssemblyManager/AssemblyManager.h"
#include "MachineTrainee/Actors/Part/Part_Base.h"
#include "MachineTrainee/Actors/Silhouette/Silhouette_Base.h"
#include "MachineTrainee/Actors/Spawner/AssemblySpawner/AssemblySpawner.h"
#include "MachineTrainee/Game/State/AssemblyGameState_Base.h"

void AAssemblyGameMode_Base::BeginPlay() {
	
	Super::BeginPlay();
	
	// Passive setup only. Nothing starts here.
	CachedGameState = GetGameState<AAssemblyGameState_Base>();
	if (!CachedGameState) return;

}

void AAssemblyGameMode_Base::StartAssemblySession() {
	
	if (bSessionStarted || !IsAssemblyInitializedForGameplay()) return;
	
	// Hook the ready callback before requesting.
	CachedGameState->OnSessionReady.AddDynamic(this, &AAssemblyGameMode_Base::HandleSessionReady);
	
	CachedGameState->StartGameSession(AssemblySequenceID);
	
	// Execution continues in HandleSessionReady.
	
}

bool AAssemblyGameMode_Base::IsAssemblyInitializedForGameplay() {
	
	SpawnActorsForGameplayAssembly();
	
	if (!CachedGameState || !AssemblyManager || !AssemblySpawner || !SilhouetteActor || !AssemblySequenceID.IsValid() || !PartActorClass) return false;

	// Initialize the Game-State with Assembly-Manager
	CachedGameState->RegisterAssemblyManager(AssemblyManager);
	
	// Initialize Manager for Game.
	AssemblyManager->InitializeAssemblyManagerForGameplay(CachedGameState, SilhouetteActor);

	// Initialize Manager for Game.
	AssemblySpawner->InitializeSpawnerForGameplay(AssemblyManager, CachedGameState, PartActorClass);

	return true;
	
}

void AAssemblyGameMode_Base::SpawnActorsForGameplayAssembly() {
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	if (!AssemblyManager) {
		
		AssemblyManager = World->SpawnActor<AAssemblyManager>(AssemblyManagerClass, AssemblyManagerSpawnTransform);
		
	}
	
	if (!AssemblySpawner) {
		
		AssemblySpawner = World->SpawnActor<AAssemblySpawner>(AssemblySpawnerClass, FTransform());
		
	}
	
	if (!SilhouetteActor) {
		
		SilhouetteActor = World->SpawnActor<ASilhouette_Base>(SilhouetteActorClass, FTransform());
		
	}
	
}

void AAssemblyGameMode_Base::HandleSessionReady(bool bSuccess) {
	
	CachedGameState->OnSessionReady.RemoveDynamic(this, &AAssemblyGameMode_Base::HandleSessionReady);

	if (!bSuccess) return;

	// Build spawn data + spawn parts + begin.
	FSpawnData SpawnData;
	if (!GetBuiltSpawnDataFromSequence(SpawnData)) return;

	AssemblySpawner->SpawnAssemblyFromData(SpawnData);
	CachedGameState->BeginAssemblySequence();

	bSessionStarted = true;
	UE_LOG(LogTemp, Log, TEXT("AAssemblyGameMode: session started."));
	
}

bool AAssemblyGameMode_Base::GetBuiltSpawnDataFromSequence(FSpawnData& Out_Data) const {
	
	// GameState exposes the cached session data — no definition access here.
	const FAssemblySequenceData* SeqData = CachedGameState->GetCachedSessionData();
	if (!SeqData) return false;

	Out_Data.BasePartID = SeqData->BasePartID;
	Out_Data.BaseTransform = SeqData->BaseTransform;
	Out_Data.InitialSpawnCount = InitialTableSpawnCount;
	Out_Data.TablePartIDs.Empty();

	// Table parts follow the sequence order. Base part is separate.
	for (const FAssemblyStep& Step : SeqData->Steps) {
		
		Out_Data.TablePartIDs.Add(Step.PartID);
		
	}

	return Out_Data.TablePartIDs.Num() > 0;
	
}

void AAssemblyGameMode_Base::EndAssemblySession() {
	
	/*
	* User presses 1 (or BP button)
			  │
			  ▼
			StartAssemblySession()
			  ├─ bSessionStarted = false → passes the guard ✓
			  ├─ IsAssemblyInitializedForGameplay()
			  │     ├─ AssemblyManager != null → skips spawn
			  │     ├─ Re-wires: RegisterAssemblyManager, InitializeAssemblyManagerForGameplay, InitializeSpawnerForGameplay
			  │     └─ returns true
			  ├─ Binds HandleSessionReady
			  ├─ GameState->StartGameSession(AssemblySequenceID)
			  │     ├─ subsystem loads (cached, near-instant)
			  │     └─ fires OnSessionReady(true)
			  ├─ HandleSessionReady(true)
			  │     ├─ GetBuiltSpawnDataFromSequence
			  │     ├─ Spawner->SpawnAssemblyFromData  ← fresh parts spawn
			  │     └─ GameState->BeginAssemblySequence
			  └─ Loop resumes
	 * 
	 */
	 
	if (!bSessionStarted) return;
	
	// 1. Stop the sequence (hides silhouette, clears run flag).
	if (CachedGameState) {
		
		CachedGameState->AbortAssemblySequence();
		
	}
	
	// 2. Destroy all parts (spawner delegates to manager).
	if (AssemblySpawner) {

		AssemblySpawner->CleanupSpawnedAssembly();
  
	} else {

		AssemblyManager->CleanupAssembly();
  
	}

	// 3. Clear GameState session counters.
	if (CachedGameState) {
		
		CachedGameState->CleanupSession();
		
	}

	// 4. Allow a fresh StartAssemblySession.
	bSessionStarted = false;
	
	UE_LOG(LogTemp, Log, TEXT("AAssemblyGameMode: session ended and cleaned up."));
	
}