
#include "AssemblyManager.h"
#include "MachineTrainee/Actors/Part/Part_Base.h"
#include "MachineTrainee/Actors/Silhouette/Silhouette_Base.h"
#include "MachineTrainee/Game/State/AssemblyGameState_Base.h"


AAssemblyManager::AAssemblyManager() {
	
	PrimaryActorTick.bCanEverTick = false;
	
}

void AAssemblyManager::InitializeAssemblyManagerForGameplay(AAssemblyGameState_Base* In_GameState, ASilhouette_Base* In_Silhouette) {
	
	GameState = In_GameState;
	SilhouetteActor = In_Silhouette;
		
}

void AAssemblyManager::RegisterBasePart(APart_Base* In_BasePart) {
	
	if (!In_BasePart) return;

	BasePart = In_BasePart;
	
	RegisterAndTrackPart(BasePart);

}

void AAssemblyManager::RegisterTablePart(APart_Base* In_TablePart) {
	
	if (!In_TablePart) return;
	
	RegisterAndTrackPart(In_TablePart);

}

void AAssemblyManager::RegisterAndTrackPart(APart_Base* In_Part) {
	
	In_Part->UpdateAssemblyManager(this);

	if (!TrackedParts.Contains(In_Part)) {
		
		TrackedParts.Add(In_Part);
		
	}
	
}

void AAssemblyManager::StartSequence(const FAssemblySequenceData& In_AssemblySequenceData) {
	
	CachedAssemblySequenceData = In_AssemblySequenceData;
	StartCurrentSequence();
	
}

void AAssemblyManager::StartCurrentSequence() {
	
	if (!CachedAssemblySequenceData.IsValid()) return;
	
	if (bIsRunningSequence || !BasePart || !GameState || !SilhouetteActor) return;
	
	CurrentStepIndex = 0;
	bIsRunningSequence = true;

	AdvanceStepForNextPart();
	
}

void AAssemblyManager::EndCurrentSequence() {
	
	if (SilhouetteActor) {
		
		SilhouetteActor->HideSilhouette();
		
	}

	bIsRunningSequence = false;
	CurrentStepIndex = 0;
	
}


void AAssemblyManager::AdvanceStepForNextPart() {
	
	if (!bIsRunningSequence) return;

	if (CurrentStepIndex >= CachedAssemblySequenceData.Steps.Num()) {
		
		CompleteSequence();
		return;
		
	}

	const FAssemblyStep& Step = CachedAssemblySequenceData.Steps[CurrentStepIndex];

	if (!ShowSilhouetteForStep(Step)) {
		
		EndCurrentSequence();
		return;
		
	}

	OnStepStarted.Broadcast(CurrentStepIndex, Step.PartID);
	
}

void AAssemblyManager::PartGrabbedForAssembly(APart_Base* In_Part) {

	// Stop any previously running validation timer.
	StopGlowingTimer();

	// Start the Glowing-Timer for Valid-Placement
	GetWorldTimerManager().SetTimer(TH_PartGlowing, FTimerDelegate::CreateLambda([this, In_Part] () {
		
		if (!IsValid(In_Part)) {
			
			StopGlowingTimer();
			return;
			
		}

		if (IsPlacementValid(In_Part) && (Silhouette_SuccessGlowMaterial || Silhouette_SuccessOverlayMaterial)) {
			
			// Glow the silhouette green.
			SilhouetteActor->SetSilhouetteMaterials(Silhouette_SuccessGlowMaterial, Silhouette_SuccessOverlayMaterial);
			
			
		} else {
			
			// Set the silhouette material to regular.
			SilhouetteActor->SwitchToRegularSilhouetteMaterial();
			
		}
		
	}),
	0.1f,
	true
	);
	
}

void AAssemblyManager::StopGlowingTimer() {
	
	GetWorldTimerManager().ClearTimer(TH_PartGlowing);
	
	// Set the silhouette material to regular.
	SilhouetteActor->SwitchToRegularSilhouetteMaterial();
	
}

void AAssemblyManager::OnPartReleased(APart_Base* In_ReleasedPart) {

	StopGlowingTimer();
	
	if (!bIsRunningSequence || !In_ReleasedPart) return;

	const FAssemblyStep* Step = GetCurrentStep();
	if (!Step) return;

	// Wrong part → nothing to do. Part state machine already dropped it.
	if (In_ReleasedPart->GetPartID() != Step->PartID) {
		
		OnStepFailed.Broadcast(CurrentStepIndex, In_ReleasedPart->GetPartID());
		return;
		
	}

	// Right part but wrong transform → nothing to do. Part falls naturally (D7).
	if (!IsPlacementValid(In_ReleasedPart)) {
		
		OnStepFailed.Broadcast(CurrentStepIndex, In_ReleasedPart->GetPartID());
		return;
		
	}

	// Success. Snap to silhouette transform, lock, broadcast, advance.
	if (SilhouetteActor) {
		
		In_ReleasedPart->PartAppliedToAssembly(SilhouetteActor->GetActorTransform());
		
	} else {
		
		In_ReleasedPart->PartAppliedToAssembly(In_ReleasedPart->GetActorTransform());
		
	}

	OnStepValidated.Broadcast(CurrentStepIndex, In_ReleasedPart->GetPartID(), In_ReleasedPart);

	CurrentStepIndex++;
	AdvanceStepForNextPart();
	
}

bool AAssemblyManager::IsPlacementValid(APart_Base* In_Part) const {
	
	if (!In_Part || !SilhouetteActor) return false;

	const FAssemblyStep* Step = GetCurrentStep();
	if (!Step) return false;

	// Identity — double-checked here even though the caller filters, per defensive-design rule.
	if (In_Part->GetPartID() != Step->PartID) return false;

	const FTransform SilhouetteTransform = SilhouetteActor->GetActorTransform();
	const FTransform PartTransform = In_Part->GetActorTransform();
		
	// Position tolerance (per-step, cm).
	const float PositionDelta = FVector::Dist(PartTransform.GetLocation(), SilhouetteTransform.GetLocation());
	if (PositionDelta > Step->PosTolerance) return false;
	
	// TODO : Enable when doing for VR
	/*
	// Rotation tolerance (per-step, degrees).
	const float RotationDeltaRad = PartTransform.GetRotation().AngularDistance(SilhouetteTransform.GetRotation());
	const float RotationDeltaDeg = FMath::RadiansToDegrees(RotationDeltaRad);
	if (RotationDeltaDeg > Step->RotTolerance) return false;
	*/

	return true;
	
}

const FAssemblyStep* AAssemblyManager::GetCurrentStep() const {

	if (!CachedAssemblySequenceData.Steps.IsValidIndex(CurrentStepIndex)) return nullptr;

	return &CachedAssemblySequenceData.Steps[CurrentStepIndex];
	
}

bool AAssemblyManager::ShowSilhouetteForStep(const FAssemblyStep& In_Step) {
	
	const FPartID PartID = In_Step.PartID;

	FPartVisual PartVisual;
	if (!GameState->GetPartVisual(PartID, PartVisual)) {
		
		UE_LOG(LogTemp, Warning, TEXT("No PartVisual for the SpawnPart : '%s'."), *PartID.GetTag().ToString());
		
	}
	
	// Silhouette transform is relative to the Assembly-Manager(this).
	const FTransform SilhouetteTransform = In_Step.TargetTransform * GetActorTransform();

	SilhouetteActor->ShowSilhouette(SilhouetteTransform, PartVisual);
	
	return PartVisual.IsPartVisualValid();
	
}

void AAssemblyManager::CompleteSequence() {
	
	if (!bIsRunningSequence) return;

	if (SilhouetteActor) {
		
		SilhouetteActor->HideSilhouette();
		
	}

	bIsRunningSequence = false;

	OnSequenceCompleted.Broadcast();
	
}

void AAssemblyManager::CleanupAssembly() {
	
	// Stop the glowing timer if running.
	StopGlowingTimer();

	// Hide silhouette — safe to call regardless of state.
	if (SilhouetteActor) {
		
		SilhouetteActor->HideSilhouette();
		
	}

	// Destroy every tracked part.
	for (TObjectPtr<APart_Base>& Part : TrackedParts) {
		
		if (IsValid(Part)) {
			
			Part->Destroy();
			
		}
		
	}
	
	TrackedParts.Empty();
	BasePart = nullptr;

	// Reset step state.
	CurrentStepIndex = 0;
	bIsRunningSequence = false;
	CachedAssemblySequenceData = FAssemblySequenceData();

	UE_LOG(LogTemp, Log, TEXT("AAssemblyManager: cleanup complete."));
	
}

// - Getters
int32 AAssemblyManager::GetTotalStepCount() const {
	
	return CachedAssemblySequenceData.Steps.Num();
	
}