#pragma once

#include "CoreMinimal.h"
#include "Base/PDA_Base.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Game.h"



#include "Assembly_DataAsset_Types.generated.h"

/**
 * Thin DataAsset wrapper around FPartData.
 * 
 * Its ONLY responsibilities:
 *  1. Give the struct an identity in the AssetManager (GetPrimaryAssetId).
 *  2. Enable chunked .pak cooking per part (D2).
 *  3. Provide a designer-editable asset in the Content Browser.
 *
 * It owns no logic. All data lives in the wrapped FPartData.
 *
 * Access pattern at runtime:
 *     const UPartDefinition* Def = Registry->ResolvePart(Tag);
 *     const FPartData& Data = Def->Data;      // <-- pass this around by ref
 *     
 * "PartDefinition"
 */
UCLASS(BlueprintType)
class MACHINETRAINEE_API UPartDefinition : public UPDA_Base {
    
    
	GENERATED_BODY()

    
public:
    
	UPartDefinition() {
		
		PrimaryAssetTypeName = FName("AssemblyPart");
		
	}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FPartData PartData;
	
	bool IsValid() const {
		
		return PartData.IsValid();
		
	}
	
};

/*
 * Thin DataAsset wrapper around FAssemblySequenceData.
 * Same pattern as UPartDefinition: identity + chunking + designer-editable asset.
 *
 * Runtime access:
 *     const FAssemblySequenceData& Seq = SequenceDef->Data;
 *     const FAssemblyStep& Step = Seq.Steps[Index];
 *     
 * "AssemblySequenceDefinition"
 */
UCLASS(BlueprintType)
class MACHINETRAINEE_API UAssemblySequenceDefinition : public UPDA_Base {
	
	
	GENERATED_BODY()

	
public:
	
	UAssemblySequenceDefinition() {
		
		PrimaryAssetTypeName = FName("AssemblySequence");
		
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAssemblySequenceData AssemblySequenceData;
	
	bool IsValid() const {
		
		return AssemblySequenceData.IsValid();
		
	}
	
};