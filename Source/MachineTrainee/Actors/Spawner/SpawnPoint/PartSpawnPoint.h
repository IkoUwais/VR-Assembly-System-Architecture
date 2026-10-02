#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"



#include "PartSpawnPoint.generated.h"


/*
 * Empty marker actor. Designers place these on the table to define where parts spawn (D13).
 * The spawner consumes them in world-iteration order; the first free marker gets the next part.
 * The marker's own transform is the spawn transform.
 */
UCLASS(BlueprintType)
class MACHINETRAINEE_API APartSpawnPoint : public AActor {
	
	
	GENERATED_BODY()

	
public:

	APartSpawnPoint();
	
protected:
	
	virtual void BeginPlay() override;

public:
	
	void SetOccupied(const bool In_Occupied) { bOccupied = In_Occupied; }
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOccupied = false;
	
private:
	
	
public:
	
	UFUNCTION(BlueprintPure)
	bool IsOccupied() const { return bOccupied; }
	
};