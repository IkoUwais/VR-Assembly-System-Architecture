#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"



#include "PDA_Base.generated.h"

UCLASS()
class MACHINETRAINEE_API UPDA_Base : public UPrimaryDataAsset {

	
	GENERATED_BODY()


public:


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName PrimaryAssetTypeName;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override {

		return FPrimaryAssetId(FPrimaryAssetType(PrimaryAssetTypeName), GetFName());
		
	}
	
};