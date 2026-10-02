#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"



#include "GrabComponent.generated.h"

class IMotionControllerGrabberInterface;
class UMotionControllerComponent;

UENUM()
enum class E_GrabType : uint8 {
	
	None,
	Free,
	Snap,
	Custom
    
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabSignature, AActor*, GrabAffectedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReleaseSignature, AActor*, GrabAffectedActor, const FVector&, In_ReleaseVelocity);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class MACHINETRAINEE_API UGrabComponent : public USceneComponent {


	GENERATED_BODY()


public:	

	UGrabComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
	
	virtual void BeginPlay() override;
	
protected:
	
	// overrides
	
public:
	
	
	// - Events
	UPROPERTY(BlueprintAssignable)
	FOnGrabSignature OnActorGrabbed;

	UPROPERTY(BlueprintAssignable)
	FOnReleaseSignature OnActorReleased;
	
	
	UFUNCTION(BlueprintCallable)
	void ToggleGrab(const bool In_GrabToggle);
	
	
	
	// - Called from the Motion-Controller - //
	
	// Attempt to grab with the given MotionController. Returns true if held.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TryGrab(UMotionControllerComponent* In_PawnMotionController);

	// Attempt to release. Returns true if the release succeeded.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TryRelease();
	
	// - Called from the Motion-Controller - //
		
		
	
	UFUNCTION(BlueprintNativeEvent)
	void DisableGrab(AActor* In_ReleaseInstigator);
	
	// called by other-actors that forcefully release this actor from it's grabbed-actor
	UFUNCTION(BlueprintCallable)
	void ForceReleaseGrab(AActor* In_ReleaseInstigator);
	
	
	
	// If the parent primitive is currently simulating, remember to restore that on drop.
	UFUNCTION(BlueprintCallable)
	void SetShouldSimulateOnDrop();

	// Enable/disable physics simulation on the attached primitive parent.
	UFUNCTION(BlueprintCallable)
	void SetPrimitiveCompPhysics(bool bSimulate);

	// Which hand currently holds this component (based on MotionControllerRef).
	UFUNCTION(BlueprintPure)
	EControllerHand GetHeldByHand();
	
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bStartWithGrabEnabled = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	E_GrabType GrabType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulateOnDrop = false;

	// Motion-Controller to which this Actor is Grabbed by
	UPROPERTY(BlueprintReadOnly)
	UMotionControllerComponent* PawnMotionController = nullptr;

	// Owner Actor to which this Grab-Component is attached to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> GrabTargetComponent;
	
	// Owner Actor to which this Grab-Component is attached to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor> OwnerActor;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bGrabbed;
	
	// Attach the component's parent to the supplied motion controller. Returns success.	
	UFUNCTION(BlueprintCallable)
	bool AttachParentToMotionController(UMotionControllerComponent* In_MotionController);
	
	// Helper to detach + physics-restore.
	void PerformRelease();
	
	
private:
	
	//
	
public:
	
	UFUNCTION(BlueprintPure)
	bool CanBeGrabbed() const { return IsActive(); }
	
};