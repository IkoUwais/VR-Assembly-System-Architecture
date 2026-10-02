
#include "AssemblyPawnMotionControllerComponent.h"

UAssemblyPawnMotionControllerComponent::UAssemblyPawnMotionControllerComponent() {
	
	PrimaryComponentTick.bCanEverTick = false;

}

EControllerHand UAssemblyPawnMotionControllerComponent::GetGrabHand_Implementation() {

	return GrabHand;
	
}

UMotionControllerComponent* UAssemblyPawnMotionControllerComponent::GetGrabAttachComponent_Implementation() {
	
	return this;
	
}