#pragma once

#include "CoreMinimal.h"
#include "Assembly_Struct_IDs.h"
#include "MachineTrainee/Data/Types/Enum/Assembly_Types.h"



#include "Assembly_StructTypes_Game.generated.h"


class UStaticMesh;
class UMaterialInterface;


// Per-part VR grab tuning. Embedded in FPartData.
// All values are starting points — tweak per part once the interaction loop is testable.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FGrabProfile {


    GENERATED_BODY()


	//
    // Multiplier applied to the hand's velocity when the part is thrown. 1.0 = realistic.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float ThrowVelocityMultiplier = 1.f;

    // How far the hand can be from the grab point and still snap. cm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float GrabRadius = 10.f;

    // Distance from player beyond which a held part teleports back to the table.
    // Edge case: out-of-reach recovery.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "10.0"))
    float OutOfReachThreshold = 300.f;
	
	bool IsValid() const {
		
		return ThrowVelocityMultiplier >= 0.f
				&& GrabRadius > 0.f
				&& OutOfReachThreshold > 0.f;
		
	}
    
};


// Used to Cache in the Subsystem
USTRUCT(BlueprintType)
struct FPartVisual {

    
	GENERATED_BODY()
  
    
	//
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UStaticMesh> PartMesh;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> GlowMaterial;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> SilhouetteOverlayMaterial;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> IconImage;
	
	bool IsPartVisualValid() const;
    
};


/**
 * Pure data payload for a single assembly part.
 * 
 * This is a Value Object / DTO. It carries no behavior and no identity beyond PartID.
 * It is wrapped by UPartDefinition (a UPrimaryDataAsset) purely so that the
 * AssetManager can cook/stream it independently (D2 chunked pak delivery).
 *
 * Rules:
 *  - Always pass as `const FPartData&`. Never by value.
 *  - Never embed FPartData inside another struct (use TSoftObjectPtr<UPartDefinition> instead).
 *  - Never add behavior here. Behavior belongs in systems/components.
 */
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FPartData {
	
	
	GENERATED_BODY()

	
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FPartID PartID;

	// Human-readable name. Used by UI later.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FText DisplayName;

	/** Soft reference to the Static Mesh asset.
	 *  Not loaded until the part is spawned (conveyor-belt spawning).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> PartMesh;

	/** Per-part glow material slot.
	 *  Reserved for future per-part glow looks. Currently unused.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UMaterialInterface> GlowMaterial;

	/** Per-part silhouette material slot.
	 *  Reserved for future per-part silhouette looks. Currently unused.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UMaterialInterface> SilhouetteOverlayMaterial;
	
	// Part Icon Image.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UTexture2D> IconImage;

	// Physics mass in kg. Drives grab/throw feel.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0.01"))
	float Mass = 1.f;

	// Collision preset applied to the StaticMeshComponent.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	FName CollisionPreset = FName("PhysicsActor");
	
	// VR interaction tuning for this part.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	FGrabProfile GrabProfile;
	
	// Structural validity. Does not check that soft refs resolve —
	// that is content validation and belongs to the data subsystem.
	bool IsValid() const {
		
		return PartID.IsValid()
				&& !PartMesh.IsNull()
				&& Mass > 0.f
				&& GrabProfile.IsValid();
		
	}
	
};


// Value Object. Referenced by FPartID only — no asset payload, no FPartData embed.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FAssemblyStep {
	
	
	GENERATED_BODY()

	
	//
	// Which part the trainee must install at this step. Resolved via UPartRegistry.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ID")
	FPartID PartID;

	// Target transform, relative to the base part.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FTransform TargetTransform;

	// Position tolerance in cm (per-step).
	// Create a bounding box trace and if the distance is less than the Tolerance then the Part could be snapped
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.0"))
	float PosTolerance = 5.f;

	// Rotation tolerance in degrees (per-step).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.0"))
	float RotTolerance = 5.f;

	// Discriminator for the manager's step handler dispatch (Strategy).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	E_AssemblyStepType StepType = E_AssemblyStepType::SimplePlacement;

	bool IsValid() const {
		
		return PartID.IsValid();
		
	}
	
};


// Pure data payload for a complete assembly sequence.
// Value Object / DTO. Wrapped by UAssemblySequenceDefinition for asset identity.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FAssemblySequenceData {
	
	
	GENERATED_BODY()

	
	//
	// Identifier for a Unique Assembly
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assembly")
	FAssemblySequenceID AssemblySequenceID;
	
	// Human-readable name. Used by UI later.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assembly")
	FText DisplayName;
	
	// Assembly Icon Image.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UTexture2D> IconImage;

	// Which part acts as the base. Pre-placed at assembly area on StartSequence.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Base")
	FPartID BasePartID;

	// World-space transform for the base part. Data-driven.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Base")
	FTransform BaseTransform;

	// Ordered steps. One-by-one. Traversed forward for assembly,
	// backward later for Undo and reverse mode.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Steps")
	TArray<FAssemblyStep> Steps;
	
	// Structural validity. Content validation is the subsystem's job.
	bool IsValid() const {
		
		if (!AssemblySequenceID.IsValid() || !BasePartID.IsValid()) return false;
		if (Steps.Num() == 0) return false;
		
		for (const FAssemblyStep& Step : Steps) {
			
			if (!Step.IsValid()) return false;
			
		}
		
		return true;
		
	}

};


// Captured when a sequence completes. Drives the summary UI.
// Value Object. Constructed by AAssemblyManager at completion, consumed by UI.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FAssemblySessionResult {
	
	
	
	GENERATED_BODY()

	
	//
	// Total elapsed time from StartSequence to final successful placement, in seconds.
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float ElapsedSeconds = 0.f;

	// Count of failed placement attempts (wrong part or out of tolerance on release).
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 FailedAttempts = 0;

	// Total steps in the sequence (for reference).
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 TotalSteps = 0;

	// Percentage of steps completed correctly on first attempt. 0..100.
	// Derived: (TotalSteps - FailedAttempts) / TotalSteps * 100. Clamped to [0, 100].
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float CleanRunPercent = 0.f;
	
	void SetCalculatedCleanRunPercent();
	
};


// Describes what the Spawner must create for one assembly session.
// Built by GameMode (from GameState data) and handed to AAssemblySpawner.
// Value Object — pure data, no behavior, pass by const ref.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FSpawnData {
	
	
	GENERATED_BODY()

	
	// Base part that is pre-placed at BaseTransform (D3).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FPartID BasePartID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FTransform BaseTransform;

	// Ordered list of table parts. First K spawn immediately (D13).
	// The rest trickle in on success — conveyor logic added later.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TArray<FPartID> TablePartIDs;

	// Number of parts to spawn at sequence start. The rest are spawned on demand (D13).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0"))
	int32 InitialSpawnCount = 3;
	
};


// - Session Progress Track
// A single attempt to place a part at a given step.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FPartAttemptCount {
	
	
	GENERATED_BODY()
	
	
	//
	// The part the trainee tried to place.
	UPROPERTY(BlueprintReadOnly)
	FPartID AttemptedPartID;

	// How many times they tried this specific part at this step.
	UPROPERTY(BlueprintReadOnly)
	int32 Count = 0;
	
};


// Failure record for a single step.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FStepFailureRecord {
	
	
	GENERATED_BODY()

	
	//
	// Which step index was active when the failure occurred.
	UPROPERTY(BlueprintReadOnly)
	int32 StepIndex = 0;

	// The part the player tried to place.
	UPROPERTY(BlueprintReadOnly)
	FPartID ExpectedPartID;

	// Total failed attempts at this step (sum of all Attempts.Count).
	UPROPERTY(BlueprintReadOnly)
	int32 TotalFailures = 0;

	// Every wrong part tried at this step, and how many times each.
	UPROPERTY(BlueprintReadOnly)
	TArray<FPartAttemptCount> Attempts;
	
};

// Full failure summary for a session. Passed through the delegate — no templated type on the wire.
USTRUCT(BlueprintType)
struct MACHINETRAINEE_API FAssemblyFailureSummary {
	
	
	GENERATED_BODY()

	
	//
	// One record per step that had at least one failure.
	UPROPERTY(BlueprintReadOnly)
	TArray<FStepFailureRecord> StepFailures;

	// Total failed attempts across the whole session.
	UPROPERTY(BlueprintReadOnly)
	int32 TotalFailures = 0;
	
};