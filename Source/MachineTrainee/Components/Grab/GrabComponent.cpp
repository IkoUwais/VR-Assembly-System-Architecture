
#include "GrabComponent.h"
#include "MotionControllerComponent.h"
#include "MachineTrainee/Interfaces/MotionControllerGrabberInterface.h"


UGrabComponent::UGrabComponent() {
	
	PrimaryComponentTick.bCanEverTick = false;

}

void UGrabComponent::BeginPlay() {
	
	Super::BeginPlay();
	
	OwnerActor = GetOwner();
	GrabTargetComponent = GetAttachParent();
	if (!IsValid(OwnerActor) || !IsValid(GrabTargetComponent) || !bStartWithGrabEnabled) {
		
		Deactivate();
		return;
		
	}

}

void UGrabComponent::Activate(bool bReset) {
	
	Super::Activate(bReset);
	
	// Side effects for grab-enabled state live here.
	// Example (later): bind input, start tick, enable hand collision.
	
	
}

void UGrabComponent::Deactivate() {
	
	Super::Deactivate();
	
	// Side effects for grab-disabled state live here.
	ForceReleaseGrab(nullptr);
	
	
}

void UGrabComponent::ToggleGrab(const bool In_GrabToggle) {

	SetActive(In_GrabToggle);
	
}
	
bool UGrabComponent::TryGrab_Implementation(UMotionControllerComponent* In_PawnMotionController) {
	
	// Called from the Motion-Controller
	
	if (!In_PawnMotionController) return false;
	
	if (GrabType == E_GrabType::Custom) {
		
		bGrabbed = true;
		
	} else if (GrabType == E_GrabType::Free || GrabType == E_GrabType::Snap) {
		
		SetPrimitiveCompPhysics(false);
		
		if (AttachParentToMotionController(In_PawnMotionController)) {
			
			bGrabbed = true;
			
			if (GrabType == E_GrabType::Snap) {
				
				//
				
			}
			
		}
		
	}
	
	if (!bGrabbed) return false;
	
	PawnMotionController = In_PawnMotionController;
	OnActorGrabbed.Broadcast(OwnerActor);
	return true;
	
}

bool UGrabComponent::TryRelease_Implementation() {
	
	// Called from the Motion-Controller
	
	if (GrabType == E_GrabType::Custom) {
		
		bGrabbed = false;
		
	} else if (GrabType == E_GrabType::Free || GrabType == E_GrabType::Snap) {
		
		PerformRelease();
		
	}
	
	if (bGrabbed) return false;
	
	OnActorReleased.Broadcast(OwnerActor, FVector::ZeroVector);
	PawnMotionController = nullptr;
	return true;
	
}

void UGrabComponent::DisableGrab_Implementation(AActor* In_ReleaseInstigator) {

	ForceReleaseGrab(In_ReleaseInstigator);
	ToggleGrab(false);
	
}

void UGrabComponent::ForceReleaseGrab(AActor* In_ReleaseInstigator) {

	// release from the hands using the Motion-Controller
	if (!bGrabbed) return;
	
	PerformRelease();
	
	OnActorReleased.Broadcast(OwnerActor, FVector::ZeroVector);
	PawnMotionController = nullptr;
	bGrabbed = false;
	
}

void UGrabComponent::PerformRelease() {
	
	if (bSimulateOnDrop) {
		
		SetPrimitiveCompPhysics(true);
		
	}
	
	if (IsValid(GrabTargetComponent)) {
		
		FDetachmentTransformRules DetachmentRule = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true);
		GrabTargetComponent->DetachFromComponent(DetachmentRule);
		
	}
	
	bGrabbed = false;
	
}

void UGrabComponent::SetShouldSimulateOnDrop() {
	
	if (!IsValid(GrabTargetComponent)) return;
	
	UPrimitiveComponent* ParentPrimitiveComponent = Cast<UPrimitiveComponent>(GrabTargetComponent);
	
	if (ParentPrimitiveComponent && ParentPrimitiveComponent->IsAnySimulatingPhysics()) {
		
		bSimulateOnDrop = true;
		
	}
	
}

void UGrabComponent::SetPrimitiveCompPhysics(bool bSimulate) {
	
	if (!IsValid(GrabTargetComponent)) return;
	
	UPrimitiveComponent* ParentPrimitiveComponent = Cast<UPrimitiveComponent>(GrabTargetComponent);
	
	if (ParentPrimitiveComponent) {
		
		ParentPrimitiveComponent->SetSimulatePhysics(bSimulate);
		
	}
	
}

EControllerHand UGrabComponent::GetHeldByHand() {
	
	EControllerHand HeldByHand = EControllerHand::Right;
	
	if (!PawnMotionController) return HeldByHand;
	
	if (PawnMotionController->Implements<UMotionControllerGrabberInterface>()) {
		
		HeldByHand = IMotionControllerGrabberInterface::Execute_GetGrabHand(PawnMotionController);
		
	}
	
	return HeldByHand;
	
}

bool UGrabComponent::AttachParentToMotionController(UMotionControllerComponent* In_MotionController) {
	
	if (!IsValid(GrabTargetComponent) || !In_MotionController) return false;

	FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
	const bool bAttached = GrabTargetComponent->AttachToComponent(In_MotionController, AttachmentRules, NAME_None);
	
	return bAttached;
	
}