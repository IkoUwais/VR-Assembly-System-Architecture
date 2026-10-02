#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "MachineTrainee/Interfaces/MotionControllerGrabberInterface.h"



#include "AssemblyPawnMotionControllerComponent.generated.h"

/**
 * UMotionControllerComponent + a strongly-typed EControllerHand field.
 * This is the grabber the GrabComponent talks to in VR.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class MACHINETRAINEE_API UAssemblyPawnMotionControllerComponent : public UMotionControllerComponent, public IMotionControllerGrabberInterface {
	
	
	GENERATED_BODY()

	
public:

	UAssemblyPawnMotionControllerComponent();
	
protected:
	
	// overrides
	
public:
	
	// Hand this controller represents. Replaces string matching on MotionSource.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EControllerHand GrabHand = EControllerHand::Right;
	
	virtual EControllerHand GetGrabHand_Implementation() override;
	virtual UMotionControllerComponent* GetGrabAttachComponent_Implementation() override;
	
protected:
	
	//
	
private:
	
	//
	
public:
	
	//
	
	
};