#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"



#include "Assembly_StructTypes_Lookup.generated.h"

// One row in DT_PartLookup.
// RowName = FGameplayTag name (e.g. "Part.Engine.Piston"). Payload = primary asset id.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FPartLookupRow : public FTableRowBase {
	
	
	GENERATED_BODY()

	
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPartID PartID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPrimaryAssetId PartPrimaryAsset;
	
};

// One row in DT_AssemblyLookup.
// RowName = FGameplayTag name (e.g. "Assembly.Engine"). Payload = primary asset id.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FAssemblyLookupRow : public FTableRowBase {
	
	
	GENERATED_BODY()

	
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAssemblySequenceID AssemblySequenceID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPrimaryAssetId AssemblyPrimaryAsset;
	
};