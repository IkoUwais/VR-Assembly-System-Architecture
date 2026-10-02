#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Game.h"
#include "MachineTrainee/Data/Structs/Assembly_Struct_IDs.h"



#include "AssemblyDataSubsystem.generated.h"

class UDataTable;

// Called when an async load of a session completes. bSuccess = all data loaded + validated.
DECLARE_DELEGATE_OneParam(FOnAssemblySessionLoaded, bool /*bSuccess*/);

/**
 * Isolated data source for the assembly game.
 *
 * The ONLY place that knows about the DataTables and the mapping from
 * FPartID / FAssemblySequenceID to UPrimaryDataAsset. Nothing else touches them.
 *
 * Lifecycle:
 *  - Initialize: loads the two lookup DataTables, builds in-memory maps.
 *  - On request: async-loads the assembly + all its parts, validates once, caches.
 *  - Provides const accessors for cached data.
 *
 * Contract:
 *  - Once RequestLoadSession calls back with true, every part referenced by the
 *    assembly is loaded, validated, and cached. No caller ever needs to validate again.
 *  - All data returned is const — callers must not mutate.
 *
 * Pattern: Data Source + Registry + Async Loader.
 * Pattern: Value-typed Cache — returns const refs, no ownership transfer.
 */
UCLASS()
class MACHINETRAINEE_API UAssemblyDataSubsystem : public UGameInstanceSubsystem {
	
	
	GENERATED_BODY()
	
	
public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
protected:
	
	// overrides
	
public:
	
	// Asynchronously load and validate the assembly + all its parts.
	// OnComplete is called on the game thread with true/false.
	// If true, every subsequent accessor for this session is populated.
	void RequestLoadSession(const FAssemblySequenceID In_ID, const FOnAssemblySessionLoaded& In_OnComplete);

	// - Lookups (valid only after RequestLoadSession has completed with success) - //

	const FPartData* GetPartData(const FPartID& In_ID) const;
	const FAssemblySequenceData* GetAssemblyData(const FAssemblySequenceID& In_ID) const;

	// - Direct lookup-table access (before loading — resolves tags to asset ids) - //
	bool GetPartPrimaryAsset(const FPartID& In_ID, FPrimaryAssetId& Out_AssetId) const;
	bool GetAssemblyPrimaryAsset(const FAssemblySequenceID& In_ID, FPrimaryAssetId& Out_AssetId) const;
	
	
protected:
	
	// Keep strong refs to the DataTables so the GC doesn't collect them mid-session.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTable> PartLookupTableRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTable> AssemblyLookupTableRef;
	
	
	// - Lookup maps (tag -> asset id), built at Initialize - //
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FPartID, FPrimaryAssetId> PartLookup;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FAssemblySequenceID, FPrimaryAssetId> AssemblyLookup;


	// - Cached loaded data (session-scoped) - //
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FPartID, FPartData> CachedPartData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FAssemblySequenceID, FAssemblySequenceData> CachedAssemblyData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FPartID, FPartVisual> PartVisuals;

	
private:
	
	// Build TMap<FPartID, FPrimaryAssetId> from the PartLookupTable.
	bool BuildPartLookup();
	bool BuildAssemblyLookup();

	// Async load chain.
	void OnAssemblyDefinitionLoaded(const FAssemblySequenceID In_ID, const FOnAssemblySessionLoaded& In_OnComplete);
	void OnPartsLoaded(const FAssemblySequenceID In_ID, const TArray<FPrimaryAssetId>& In_PartAssetIds, const FOnAssemblySessionLoaded& In_OnComplete);

	// Called after every part def is loaded. Loads meshes, validates, caches.
	bool CacheAndValidateParts(const TArray<FPrimaryAssetId>& In_PartPrimaryAssets);	
	
public:
	
	// UFUNCTION(BlueprintPure)
	const TMap<FPartID, FPartVisual>& GetPartVisuals() const { return PartVisuals; }
	
	// N O T   U S E D
	int32 GetPartLookupCount() const { return PartLookup.Num(); }
	int32 GetAssemblyLookupCount() const { return AssemblyLookup.Num(); }
	int32 GetCachedPartCount() const { return CachedPartData.Num(); }
	
};