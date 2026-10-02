
#include "AssemblyGameState_Base.h"
#include "MachineTrainee/Actors/AssemblyManager/AssemblyManager.h"
#include "MachineTrainee/System/Subsystem/AssemblyDataSubsystem.h"


AAssemblyGameState_Base::AAssemblyGameState_Base() {
	
	//
	
}

void AAssemblyGameState_Base::BeginPlay() {
	
	Super::BeginPlay();
	
	CachedAssemblyDataSubsystem = GetGameInstance()->GetSubsystem<UAssemblyDataSubsystem>();
	
}

void AAssemblyGameState_Base::StartGameSession(const FAssemblySequenceID In_AssemblyID) {

	ShowLoadingUI();
	
	PendingSessionID = In_AssemblyID;

	if (!CachedAssemblyDataSubsystem) {
		
		UE_LOG(LogTemp, Error, TEXT("AAssemblyGameState: data subsystem unavailable."));
		BroadcastOnSessionReady(false);
		return;
		
	}

	CachedAssemblyDataSubsystem->RequestLoadSession(In_AssemblyID, FOnAssemblySessionLoaded::CreateUObject(this, &AAssemblyGameState_Base::HandleSessionDataLoaded));
	
}

void AAssemblyGameState_Base::RegisterAssemblyManager(AAssemblyManager* In_Manager) {
	
	if (AssemblyManager == In_Manager) return;
	
	// Unbind from a previous manager if one was already registered.
	if (AssemblyManager) {
		
		AssemblyManager->OnStepStarted.RemoveDynamic(this, &AAssemblyGameState_Base::HandleStepStarted);
		AssemblyManager->OnStepValidated.RemoveDynamic(this, &AAssemblyGameState_Base::HandleStepValidated);
		AssemblyManager->OnStepFailed.RemoveDynamic(this, &AAssemblyGameState_Base::HandleStepFailed);
		AssemblyManager->OnSequenceCompleted.RemoveDynamic(this, &AAssemblyGameState_Base::HandleSequenceCompleted);
		
	}
	
	if (!In_Manager) return;

	AssemblyManager = In_Manager;

	AssemblyManager->OnStepStarted.AddDynamic(this, &AAssemblyGameState_Base::HandleStepStarted);
	AssemblyManager->OnStepValidated.AddDynamic(this, &AAssemblyGameState_Base::HandleStepValidated);
	AssemblyManager->OnStepFailed.AddDynamic(this, &AAssemblyGameState_Base::HandleStepFailed);
	AssemblyManager->OnSequenceCompleted.AddDynamic(this, &AAssemblyGameState_Base::HandleSequenceCompleted);

}

void AAssemblyGameState_Base::BeginAssemblySequence() {
	
	if (!AssemblyManager) return;
	
	if (CachedSessionData.Steps.Num() == 0) return;

	// sequence already active
	if (bSequenceActive) return;

	// Reset outcome state for the new run.
	FailedAttempts = 0;
	LastResult = FAssemblySessionResult();
	SequenceStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	bSequenceActive = true;

	// Hand the sequence to the manager and let it drive.
	AssemblyManager->StartSequence(CachedSessionData);
	
}

void AAssemblyGameState_Base::AbortAssemblySequence() {
	
	if (!bSequenceActive) return;

	if (AssemblyManager) {
		
		AssemblyManager->EndCurrentSequence();
		
	}

	bSequenceActive = false;
	
}

void AAssemblyGameState_Base::CleanupSession() {
	
	bSequenceActive = false;
	FailedAttempts = 0;
	SequenceStartTime = 0.f;
	LastResult = FAssemblySessionResult();
	CachedSessionData = FAssemblySequenceData();
	PendingSessionID = FAssemblySequenceID();
	AssemblyFailureSummary = FAssemblyFailureSummary();

	UE_LOG(LogTemp, Log, TEXT("AAssemblyGameState: session state cleared."));
	
}

const FPartData* AAssemblyGameState_Base::GetResolvedPartData(const FPartID& In_PartID) const {

	return CachedAssemblyDataSubsystem->GetPartData(In_PartID);
	
}

bool AAssemblyGameState_Base::GetPartVisual(const FPartID& In_PartID, FPartVisual& Out_PartVisual) {

	const TMap<FPartID, FPartVisual>& PartVisuals = CachedAssemblyDataSubsystem->GetPartVisuals();
	
	const FPartVisual* FoundPartVisual = PartVisuals.Find(In_PartID);

	if (!FoundPartVisual || !FoundPartVisual->IsPartVisualValid()) {
		
		Out_PartVisual = FPartVisual();
		return false;
		
	}

	Out_PartVisual = *FoundPartVisual;
	return true;
	
}

void AAssemblyGameState_Base::HandleSessionDataLoaded(bool bSuccess) {

	if (!bSuccess) {

		BroadcastOnSessionReady(false);
		return;
		
	}

	const FAssemblySequenceData* Data = CachedAssemblyDataSubsystem->GetAssemblyData(PendingSessionID);
	
	if (!Data) {

		BroadcastOnSessionReady(false);
		return;
		
	}

	CachedSessionData = *Data;
	BroadcastOnSessionReady(true);
	
}

// - Events - All called from the Delegate of the Assembly-Manager
void AAssemblyGameState_Base::HandleStepStarted(int32 In_StepIndex, FPartID In_ExpectedPartID) {
	
	UE_LOG(LogTemp, Verbose, TEXT("AAssemblyGameState: step %d started — expect '%s'."), In_StepIndex, *In_ExpectedPartID.GetTag().ToString());
	
	OnStepStarted.Broadcast(In_StepIndex, In_ExpectedPartID);
	
}

void AAssemblyGameState_Base::HandleStepValidated(int32 In_StepIndex, FPartID In_PlacedPartID, APart_Base* PlacedPart) {
	
	UE_LOG(LogTemp, Log, TEXT("AAssemblyGameState: step %d validated — placed '%s'."), In_StepIndex, *In_PlacedPartID.GetTag().ToString());
	
	OnStepAdvanced.Broadcast(In_StepIndex, In_PlacedPartID, PlacedPart);
	
}

void AAssemblyGameState_Base::HandleStepFailed(int32 In_StepIndex, FPartID In_AttemptedPartID) {
	
	FailedAttempts++;
	
	// - Find or create the record for this step - //
	FStepFailureRecord* StepFailureRecord = nullptr;

	for (FStepFailureRecord& ExistingStepFailureRecord : AssemblyFailureSummary.StepFailures) {
		
		if (ExistingStepFailureRecord.StepIndex == In_StepIndex) {
			
			StepFailureRecord = &ExistingStepFailureRecord;
			break;
			
		}
		
	}
	
	if (!StepFailureRecord) {
		
		FStepFailureRecord NewFailureRecord;
		NewFailureRecord.StepIndex = In_StepIndex;

		// Pull the expected part from the cached sequence for context.
		if (CachedSessionData.Steps.IsValidIndex(In_StepIndex)) {
			
			NewFailureRecord.ExpectedPartID = CachedSessionData.Steps[In_StepIndex].PartID;
			
		}

		AssemblyFailureSummary.StepFailures.Add(NewFailureRecord);
		StepFailureRecord = &AssemblyFailureSummary.StepFailures.Last();
		
	}

	StepFailureRecord->TotalFailures++;
	
	// - Log + broadcast - //
	UE_LOG(LogTemp, Log, TEXT("AAssemblyGameState: step %d failed with '%s' (session total: %d, this step: %d)."), In_StepIndex, *In_AttemptedPartID.GetTag().ToString(), FailedAttempts, StepFailureRecord->TotalFailures);

	OnStepFailed.Broadcast(In_StepIndex, In_AttemptedPartID);
		
}

void AAssemblyGameState_Base::HandleSequenceCompleted() {
	
	bSequenceActive = false;
	
	const int32 TotalSteps = CachedSessionData.Steps.Num();
	
	FAssemblySessionResult Result;
	Result.TotalSteps = TotalSteps;
	Result.FailedAttempts = FailedAttempts;
	Result.ElapsedSeconds = GetSequenceElapsedSeconds();
	Result.SetCalculatedCleanRunPercent();
	
	LastResult = Result;
	
	// - Finalize the failure summary - //
	// Per-step records are already populated by HandleStepFailed.
	// Sync the top-level total in case it drifted.
	AssemblyFailureSummary.TotalFailures = FailedAttempts;
	
	/*
	// - Log the session - //
	UE_LOG(LogTemp, Log, TEXT("AAssemblyGameState: sequence complete. Steps=%d Failures=%d Time=%.1fs Clean=%.1f%%"), Result.TotalSteps, Result.FailedAttempts, Result.ElapsedSeconds, Result.CleanRunPercent);
	
	// Detailed failure breakdown for debugging / future JSON export.
	for (const FStepFailureRecord& Record : AssemblyFailureSummary.StepFailures) {
		
		UE_LOG(LogTemp, Log, TEXT("  Step %d: expected '%s', %d failure(s)."), Record.StepIndex, *Record.ExpectedPartID.GetTag().ToString(), Record.TotalFailures);

		for (const FPartAttemptCount& Attempt : Record.Attempts) {
			
			UE_LOG(LogTemp, Log, TEXT("    Attempted '%s' x%d."), *Attempt.AttemptedPartID.GetTag().ToString(), Attempt.Count);
		}
		
	}
	*/
	
	OnSequenceCompleted.Broadcast(LastResult);
	
}

void AAssemblyGameState_Base::BroadcastOnSessionReady(const bool In_Success) {

	OnSessionReady.Broadcast(In_Success);
	HideLoadingUI();
	
}

// - Getters
float AAssemblyGameState_Base::GetSequenceElapsedSeconds() const {
	
	if (!GetWorld()) return 0.f;
	
	return GetWorld()->GetTimeSeconds() - SequenceStartTime;
	
}

const FStepFailureRecord* AAssemblyGameState_Base::FindFailureRecordByExpectedPart(const FPartID In_PartID) const {
	
	for (const FStepFailureRecord& Record : AssemblyFailureSummary.StepFailures) {
		
		if (Record.ExpectedPartID == In_PartID) return &Record;
		
	}

	return nullptr;
	
}

bool AAssemblyGameState_Base::GetFailureRecordByExpectedPart(const FPartID In_PartID, FStepFailureRecord& Out_Record) const {
	
	const FStepFailureRecord* Found = FindFailureRecordByExpectedPart(In_PartID);

	if (!Found) {
		
		Out_Record = FStepFailureRecord();
		return false;
		
	}

	Out_Record = *Found;
	return true;
	
}

bool AAssemblyGameState_Base::GetFailureRecordByStepIndex(const int32 In_StepIndex, FStepFailureRecord& Out_Record) const {
	
	for (const FStepFailureRecord& Record : AssemblyFailureSummary.StepFailures) {
		
		if (Record.StepIndex == In_StepIndex) {
			
			Out_Record = Record;
			return true;
			
		}
		
	}

	Out_Record = FStepFailureRecord();
	return false;
	
}