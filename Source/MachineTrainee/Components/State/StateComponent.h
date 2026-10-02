#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MachineTrainee/Data/Types/Enum/Assembly_Types.h"



#include "StateComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartStateEntered, E_PartState, NewState);


/**
 * Reusable state machine for APart_Base (and any future stateful actor).
 *
 * Owns the current state and broadcasts one delegate per state entry.
 * Listeners (Part_Base, GameState, debug tools) subscribe to the states they care about.
 *
 * Pattern: State (Tier 1) — behavior lives behind state transitions.
 * Pattern: Observer (Tier 1) — subscribers react without the component knowing them.
 *
 * Adding a new state: add to E_PartState, add a delegate here, broadcast in SetState.
 * The switch in SetState is the single place that needs updating.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class MACHINETRAINEE_API UStateComponent : public UActorComponent {
	
	
	GENERATED_BODY()

	
public:

	UStateComponent();

	virtual void BeginPlay() override;

	
protected:
	
	// overrides
	
public:
	

	// - Events - //
	// Generic broadcast — fires on every transition. UI / debug / logging subscribe here.
	UPROPERTY(BlueprintAssignable)
	FOnPartStateEntered OnStateEntered;

	// Per-state broadcasts — Part_Base (or anyone) registers specific callbacks.
	UPROPERTY(BlueprintAssignable)
	FOnPartStateEntered OnInitialEntered;

	UPROPERTY(BlueprintAssignable)
	FOnPartStateEntered OnGrabbedForAssemblyEntered;

	UPROPERTY(BlueprintAssignable)
	FOnPartStateEntered OnDroppedBeforeAssemblyEntered;

	UPROPERTY(BlueprintAssignable)
	FOnPartStateEntered OnAppliedToAssemblyEntered;
	// - Events - //
	
	// Transition to a new state. No-op if the state is unchanged.
	// Broadcasts OnStateEntered (generic) and the state-specific delegate.
	UFUNCTION(BlueprintCallable, Category = "Part|State")
	void SetState(E_PartState In_NewState);
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	E_PartState CurrentState = E_PartState::Initial;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	E_PartState PreviousState;
	
private:
	
	//
	
public:
	
	UFUNCTION(BlueprintPure)
	E_PartState GetState() const { return CurrentState; }
	
	UFUNCTION(BlueprintPure)
	E_PartState GetPreviousState() const { return PreviousState; }
	
	
};