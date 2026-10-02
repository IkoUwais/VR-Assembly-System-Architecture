#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"



#include "Assembly_Struct_IDs.generated.h"

USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FPartID {
	
	
	GENERATED_BODY()

	
	//
	// Example: Part.Engine.Piston
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Categories = "Part"))
	FGameplayTag Part_ID;
	
	FGameplayTag GetTag() const {
		
		return Part_ID;
		
	}
	
	bool IsValid() const {
		
		return Part_ID.IsValid();
		
	}
	
	bool operator==(const FPartID& Other) const {
		
		return Part_ID == Other.Part_ID;
		
	}
	
	friend uint32 GetTypeHash(const FPartID& ID) {
		
		return GetTypeHash(ID.Part_ID);
		
	}
	
};

USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FAssemblySequenceID {
	
	
	GENERATED_BODY()

	
	//
	// Example: Assembly.Engine
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Categories = "AssemblySequence"))
	FGameplayTag AssemblySequence_ID;
	
	FGameplayTag GetTag() const {
		
		return AssemblySequence_ID;
		
	}
	
	bool IsValid() const {
		
		return AssemblySequence_ID.IsValid();
		
	}
	
	bool operator==(const FAssemblySequenceID& Other) const {
		
		return AssemblySequence_ID == Other.AssemblySequence_ID;
		
	}
	
	friend uint32 GetTypeHash(const FAssemblySequenceID& ID) {
		
		return GetTypeHash(ID.AssemblySequence_ID);
		
	}
	
};