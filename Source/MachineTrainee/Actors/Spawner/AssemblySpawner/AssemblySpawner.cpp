
#include "AssemblySpawner.h"
#include "EngineUtils.h"
#include "MachineTrainee/Actors/AssemblyManager/AssemblyManager.h"
#include "MachineTrainee/Actors/Part/Part_Base.h"
#include "MachineTrainee/Actors/Spawner/SpawnPoint/PartSpawnPoint.h"
#include "MachineTrainee/Game/State/AssemblyGameState_Base.h"


AAssemblySpawner::AAssemblySpawner() {
	
	PrimaryActorTick.bCanEverTick = false;
	
}

void AAssemblySpawner::BeginPlay() {
	
	Super::BeginPlay();
	
	CollectSpawnPointsFromLevelWorld();
	
}

void AAssemblySpawner::CollectSpawnPointsFromLevelWorld() {
	
	if (SpawnPoints.IsEmpty() && GetWorld()) {
	
		for (TActorIterator<APartSpawnPoint> It(GetWorld()); It; ++It) {
		
			SpawnPoints.Add(*It);
		
		}
		
	}	
	
}

void AAssemblySpawner::InitializeSpawnerForGameplay(AAssemblyManager* In_AssemblyManager, AAssemblyGameState_Base* In_AssemblyGameState, TSubclassOf<APart_Base> In_PartClass) {
	
	// remove previous delegate-binding
	if (AssemblyManager) {
		
		AssemblyManager->OnStepStarted.RemoveDynamic(this, &AAssemblySpawner::HandleStepStarted);
		AssemblyManager->OnStepValidated.RemoveDynamic(this, &AAssemblySpawner::HandleStepValidated);
		
	}
	
	AssemblyManager = In_AssemblyManager;
	AssemblyManager->OnStepValidated.AddDynamic(this, &AAssemblySpawner::HandleStepValidated);
	AssemblyManager->OnStepStarted.AddDynamic(this, &AAssemblySpawner::HandleStepStarted);
	
	GameState = In_AssemblyGameState;
	PartActorClass = In_PartClass;
	
}

void AAssemblySpawner::SpawnAssemblyFromData(const FSpawnData& In_SpawnData) {
	
	if (!AssemblyManager || !GameState || !PartActorClass) return;

	ResetSpawnState();
	
	// - Base part - //
	APart_Base* BasePart = SpawnSinglePart(In_SpawnData.BasePartID, FTransform());
	if (!BasePart) return;

	// Base part is locked in place immediately.
	BasePart->PartAppliedToAssembly(In_SpawnData.BaseTransform * AssemblyManager->GetActorTransform());
	AssemblyManager->RegisterBasePart(BasePart);

	// - Fill Conveyor Queue
	PendingPartIDs = In_SpawnData.TablePartIDs;
	
	// - Initial table parts (K) - //
	const int32 SpawnCount = FMath::Min(In_SpawnData.InitialSpawnCount, PendingPartIDs.Num());

	for (int32 i = 0; i < SpawnCount; ++i) {
		
		SpawnNextPendingPart();
		
	}
	
}

void AAssemblySpawner::ResetSpawnState() {
	
	ClearAllHighlights();
	PendingPartIDs.Empty();
	PartMarkerMap.Empty();
	ResetAllMarkers();
	
}

void AAssemblySpawner::CleanupSpawnedAssembly() {
	
	// The manager owns the authoritative list of parts. Ask it to clean up.
	// Then clear our own bookkeeping.
	if (AssemblyManager) {
		
		AssemblyManager->CleanupAssembly();
		
	}

	// Reset spawner state.
	ResetSpawnState();

	UE_LOG(LogTemp, Log, TEXT("AAssemblySpawner: cleanup complete."));
	
}

void AAssemblySpawner::ResetAllMarkers() {
		
	for (APartSpawnPoint* SpawnPointMarker : SpawnPoints) {

		if (SpawnPointMarker) {
			
			SpawnPointMarker->SetOccupied(false);
			
		}
  
	}
	
}

void AAssemblySpawner::HandleStepStarted(int32 In_StepIndex, FPartID In_ExpectedPartID) {

	HighlightPartsByID(In_ExpectedPartID);
	
}

void AAssemblySpawner::HandleStepValidated(int32 In_StepIndex, FPartID In_PlacedPartID, APart_Base* In_PlacedPart) {

	if (!IsValid(In_PlacedPart)) return;
	
	// Free the marker this part came from.
	if (TObjectPtr<APartSpawnPoint>* FoundSpawnPoint_Ptr = PartMarkerMap.Find(In_PlacedPart)) {
		
		// Map stores a pointer-to the pointer(TObjectPtr)
		if (APartSpawnPoint* SpawnPoint = FoundSpawnPoint_Ptr->Get()) {
			
			SpawnPoint->SetOccupied(false);
			  
		}
		
		PartMarkerMap.Remove(In_PlacedPart);		
		
	}
	
	// Pull the next part from the queue onto the freed marker.
	if (PendingPartIDs.Num() > 0) {
		
		SpawnNextPendingPart();
		
	}
	
}

APart_Base* AAssemblySpawner::SpawnSinglePart(const FPartID& In_PartID, const FTransform& In_Transform) {
	
	if (!GetWorld() || !GameState) return nullptr;

	const FPartData* Data = GameState->GetResolvedPartData(In_PartID);
	if (!Data) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	APart_Base* Spawned = GetWorld()->SpawnActor<APart_Base>(PartActorClass, In_Transform, SpawnParams);
	if (!Spawned) return nullptr;

	FPartVisual PartVisual;
	if (!GameState->GetPartVisual(In_PartID, PartVisual)) {
		
		UE_LOG(LogTemp, Warning, TEXT("No PartVisual for the SpawnPart : '%s'."), *In_PartID.GetTag().ToString());
		
	}
	
	Spawned->InitializePartWithData(*Data, PartVisual);
	return Spawned;
	
}

void AAssemblySpawner::SpawnNextPendingPart() {
	
	if (PendingPartIDs.Num() == 0) return;
	
	APartSpawnPoint* Marker = AcquireFreeSpawnPoint();
	if (!Marker) return;

	// "P O P" from the Map
	const FPartID NextPartID = PendingPartIDs[0];
	PendingPartIDs.RemoveAt(0);
	
	const FTransform SpawnTransform = Marker->GetActorTransform();
	
	APart_Base* Part = SpawnSinglePart(NextPartID, SpawnTransform);
	if (!Part) {
		
		// "P U S H" in the Map again if NOT-Valid
		PendingPartIDs.Insert(NextPartID, 0);
		return;
		
	}

	Marker->SetOccupied(true);
	Part->SetRespawnTransform(SpawnTransform);
	AssemblyManager->RegisterTablePart(Part);
	
	PartMarkerMap.Add(Part, Marker);
	
}

APartSpawnPoint* AAssemblySpawner::AcquireFreeSpawnPoint() {
	
	for (APartSpawnPoint* Marker : SpawnPoints) {
		
		if (Marker && !Marker->IsOccupied()) return Marker;
		
	}
	
	return nullptr;
	
}

void AAssemblySpawner::HighlightPartsByID(const FPartID In_PartID) {

	ClearAllHighlights();
	
	if (!In_PartID.IsValid()) return;
	
	// PartMarkerMap holds only currently-spawned, uninstalled table parts.
	// Installed parts are already removed from the map, so they are never highlighted.
	for (const TPair<TObjectPtr<APart_Base>, TObjectPtr<APartSpawnPoint>>& Pair : PartMarkerMap) {

		APart_Base* Part = Pair.Key;
		if (!IsValid(Part)) continue;
		
		if (Part->GetPartID() != In_PartID) continue;

		Part->Toggle_Marker(true);
		HighlightedParts.Add(Part);

	}
	
}

void AAssemblySpawner::ClearAllHighlights() {
	
	for (TObjectPtr<APart_Base>& Part : HighlightedParts) {

		if (IsValid(Part)) {
			
			Part->Toggle_Marker(false);
			
		}

	}

	HighlightedParts.Empty();
	
}

void AAssemblySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason) {

	ResetSpawnState();
	
	if (AssemblyManager) {

		AssemblyManager->OnStepStarted.RemoveDynamic(this, &AAssemblySpawner::HandleStepStarted);
		AssemblyManager->OnStepValidated.RemoveDynamic(this, &AAssemblySpawner::HandleStepValidated);
		
	}

	Super::EndPlay(EndPlayReason);
	
}