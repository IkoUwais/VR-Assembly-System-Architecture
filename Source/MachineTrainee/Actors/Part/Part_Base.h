#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Game.h"



#include "Part_Base.generated.h"


class UStaticMeshComponent;
class UStateComponent;
class UGrabComponent;
class AAssemblyManager;

/*
 * The ONE actor class for every part in the project.
 * 
 * Pattern: Component (Tier 1, #2) — behavior composed components.
 * Pattern: Data-Driven (Tier 1, #1) — self-configures from pushed FPartData.
 * 
 * Lifecycle:
 *  - Spawned by APartSpawner (or AAssemblyManager for the base part).
 *  - Immediately receives PushPartData() with its FPartData (never looks it up itself).
 *  - Self-configures mesh, mass, collision, grab tuning on the next tick.
 * 
 * Input-agnostic : OnGrabbed / OnReleased / OnThrown are plain UFUNCTION()s.
 * Controllers and (later) hand tracking both call the same functions.
 */
UCLASS(BlueprintType)
class MACHINETRAINEE_API APart_Base : public AActor {
	
	
	GENERATED_BODY()

	
public:

	APart_Base();
	
protected:

	virtual void BeginPlay() override;
	
public:
	
	// Called by the spawner/manager right after SpawnActor.
	UFUNCTION(BlueprintCallable)
	void InitializePartWithData(const FPartData& In_PartData, const FPartVisual& In_PartVisual);

	// Where this part returns if it goes out of reach.
	// Set by the spawner after spawn.
	void SetRespawnTransform(const FTransform& In_Transform);

	// Called from Assembly-System to apply to the Assembly
	// Snap this part to a target transform and lock it.
	UFUNCTION(BlueprintCallable)
	void PartAppliedToAssembly(const FTransform& In_TargetTransform);
	
	void SetPartState(E_PartState In_PartState);
	
	void UpdateAssemblyManager(AAssemblyManager* In_AssemblyManager);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Toggle_Marker(const bool In_Mark);
	
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStateComponent> StateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UGrabComponent> GrabComponent;
	

	// Identity + payload, pushed in by the spawner/manager. Never resolved by this actor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPartData PartData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPartVisual PartVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bPartInitialized = false;
	
	// Fallback transform if the part goes out of reach. Set by spawner.
	UPROPERTY(BlueprintReadOnly)
	FTransform RespawnTransform;

	// Timer handle for the out-of-reach check.
	UPROPERTY()
	FTimerHandle OutOfReachTimerHandle;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AAssemblyManager> AssemblyManager;
	
	
	// - Called by Grab-Component - //
	// Input-agnostic interaction entry points.
	// Called by the grab component (controller or hand tracking), never directly by input code.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnPartGrabbed(AActor* In_GrabbingActor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnPartReleased(AActor* In_GrabbingActor, const FVector& In_ReleaseVelocity);
	
	// - Called by Grab-Component - //
	
	
	// Self-configures mesh, mass, collision, grab tuning from CachedData.
	void InitializeMeshWithPartData();
	
	// Out-of-reach safety net. Runs on a timer, not tick.
	void CheckOutOfReach();

	
	
	UFUNCTION()
	void ApplyStateInitial(E_PartState In_NewState);
	
	UFUNCTION()
	void ApplyStateGrabbedForAssembly(E_PartState In_NewState);
	
	UFUNCTION()
	void ApplyStateDroppedBeforeAssembly(E_PartState In_NewState);
	
	UFUNCTION()
	void ApplyStateAppliedToAssembly(E_PartState In_NewState);
	
	
private:
	
	//
	
public:
	
	UFUNCTION(BlueprintPure)
	UStateComponent* GetStateComponent() const { return StateComponent; }
	
	UFUNCTION(BlueprintPure)
	UGrabComponent* GetGrabComponent() const { return GrabComponent; }
	
	// Identity accessor. Safe to call before data is pushed? No — asserts.
	UFUNCTION(BlueprintPure)
	const FPartID& GetPartID() const { return PartData.PartID; }

	// Payload accessor. Returns the cached FPartData (pass by ref downstream).
	UFUNCTION(BlueprintPure)
	const FPartData& GetPartData() const { return PartData; }

	// True once PushPartData has been called and self-config succeeded.
	UFUNCTION(BlueprintPure)
	bool IsPartInitialized() const { return bPartInitialized; }
	
	UFUNCTION(BlueprintPure)
	E_PartState GetPartState() const;

	UFUNCTION(BlueprintPure)
	bool IsAppliedToAssembly() const { return GetPartState() == E_PartState::AppliedToAssembly; }
	
	UFUNCTION(BlueprintPure)
	AAssemblyManager* GetAssemblyManager() const { return AssemblyManager; }
	
};