
#include "StateComponent.h"

UStateComponent::UStateComponent() {
	
	PrimaryComponentTick.bCanEverTick = false;

}

void UStateComponent::BeginPlay() {
	
	Super::BeginPlay();
	
	// Broadcast the initial state once so subscribers receive a starting signal.
	OnStateEntered.Broadcast(CurrentState);
	OnInitialEntered.Broadcast(CurrentState);
	
}

void UStateComponent::SetState(E_PartState In_NewState) {

	if (In_NewState == CurrentState) return;
	
	PreviousState = CurrentState;
	
	CurrentState = In_NewState;
	
	OnStateEntered.Broadcast(CurrentState);
	
	switch (CurrentState) {
		
		case E_PartState::Initial:
		
			OnInitialEntered.Broadcast(CurrentState);
			break;
			
			
		case E_PartState::GrabbedForAssembly:
		
			OnGrabbedForAssemblyEntered.Broadcast(CurrentState);
			break;
			
			
		case E_PartState::DroppedBeforeAssembly:
		
			OnDroppedBeforeAssemblyEntered.Broadcast(CurrentState);
			break;
			
			
		case E_PartState::AppliedToAssembly:
		
			OnAppliedToAssemblyEntered.Broadcast(CurrentState);
			break;
		
		default:
			UE_LOG(LogTemp, Warning, TEXT("UPartStateComponent: unhandled state %s"), ToString(CurrentState));
			break;
		
	}
	
}