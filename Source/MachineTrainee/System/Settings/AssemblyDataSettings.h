#pragma once


#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"



#include "AssemblyDataSettings.generated.h"


class UDataTable;

/*
 * Project-wide configuration for the assembly data subsystem.
 * Editable via Project Settings -> Game -> Assembly Data.
 *
 * Holds soft refs to the two lookup DataTables so nothing is hardcoded.
*/
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Assembly Data"))
class MACHINETRAINEE_API UAssemblyDataSettings : public UDeveloperSettings {
	
	
	GENERATED_BODY()
	
	
public:
	
	// DT_PartLookup. RowName = tag name, payload = FPrimaryAssetId of a UPartDefinition.
	UPROPERTY(Config, EditAnywhere, Category = "Lookup Tables", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> PartLookupTable;

	// DT_AssemblyLookup. RowName = tag name, payload = FPrimaryAssetId of a UAssemblySequenceDefinition.
	UPROPERTY(Config, EditAnywhere, Category = "Lookup Tables", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> AssemblyLookupTable;

	static const UAssemblyDataSettings* Get() {
		
		return GetDefault<UAssemblyDataSettings>();
		
	}
	
};