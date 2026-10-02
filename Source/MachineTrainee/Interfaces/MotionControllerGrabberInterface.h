#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"



#include "MotionControllerGrabberInterface.generated.h"


UINTERFACE(BlueprintType)
class UMotionControllerGrabberInterface : public UInterface {
	
	
	GENERATED_BODY()

	
};

/**
 * Anything that can grab a UGrabComponent.
 * Implemented by UVRMotionControllerComponent, but any UObject can implement it.
 * GrabComponent never references a concrete grabber class.
 */
class MACHINETRAINEE_API IMotionControllerGrabberInterface {
	
	
	GENERATED_BODY()


public:
	
	// Which hand / controller side this grabber represents.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	EControllerHand GetGrabHand();

	// Scene component that a grabbed actor should attach to.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UMotionControllerComponent* GetGrabAttachComponent();

	
};