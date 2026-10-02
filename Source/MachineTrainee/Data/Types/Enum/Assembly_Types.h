#pragma once

#include "CoreMinimal.h"



#include "Assembly_Types.generated.h"

UENUM(BlueprintType)
enum class E_PartState : uint8 {
	
	Initial						UMETA(DisplayName = "Initial"),
	GrabbedForAssembly			UMETA(DisplayName = "GrabbedForAssembly"),
	DroppedBeforeAssembly		UMETA(DisplayName = "DroppedBeforeAssembly"),
	AppliedToAssembly			UMETA(DisplayName = "AppliedToAssembly")
	
};

inline const TCHAR* ToString(const E_PartState In_State) {
	
	switch (In_State) {
		case E_PartState::Initial:						return TEXT("Initial");
		case E_PartState::GrabbedForAssembly:			return TEXT("GrabbedForAssembly");
		case E_PartState::DroppedBeforeAssembly:		return TEXT("DroppedBeforeAssembly");
		case E_PartState::AppliedToAssembly:			return TEXT("AppliedToAssembly");
		default:										return TEXT("Unknown");
	}
	
}


UENUM(BlueprintType)
enum class E_AssemblyStepType : uint8 {
	
	SimplePlacement		UMETA(DisplayName = "Simple Placement"),
	SubAssembly			UMETA(DisplayName = "Sub Assembly (reserved)"),
	ToolUsage			UMETA(DisplayName = "Tool Usage (reserved)")
	
};