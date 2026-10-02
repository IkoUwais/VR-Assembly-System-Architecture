
#include "AssemblyDataSubsystem.h"

#include "Engine/AssetManager.h"
#include "MachineTrainee/Data/Storage/Assembly_DataAsset_Types.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Lookup.h"
#include "MachineTrainee/System/Settings/AssemblyDataSettings.h"

void UAssemblyDataSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	
	Super::Initialize(Collection);
	
	// Get the Table-Reference from the Developer-Settings
	const UAssemblyDataSettings* Settings = UAssemblyDataSettings::Get();
	if (!Settings) return;
	
	if (!Settings->PartLookupTable.IsNull()) {
		
		PartLookupTableRef = Settings->PartLookupTable.LoadSynchronous();
		
	}
	
	if (!Settings->AssemblyLookupTable.IsNull()) {
		
		AssemblyLookupTableRef = Settings->AssemblyLookupTable.LoadSynchronous();
		
	}
	
	if (!BuildPartLookup()) {
		
		UE_LOG(LogTemp, Error, TEXT("UAssemblyDataSubsystem: failed to build part lookup."));
		
	}
	
	if (!BuildAssemblyLookup()) {
		
		UE_LOG(LogTemp, Error, TEXT("UAssemblyDataSubsystem: failed to build assembly lookup."));
		
	}
	
	UE_LOG(LogTemp, Log, TEXT("UAssemblyDataSubsystem: initialized. Parts=%d Assemblies=%d"), PartLookup.Num(), AssemblyLookup.Num());
	
}

bool UAssemblyDataSubsystem::BuildPartLookup() {

	PartLookup.Empty();

	if (!PartLookupTableRef) return false;

	static const FString ContextString(TEXT("UAssemblyDataSubsystem::BuildPartLookup"));
	
	TArray<FPartLookupRow*> PartLookupRows;
	PartLookupTableRef->GetAllRows<FPartLookupRow>(ContextString, PartLookupRows);

	for (const FPartLookupRow* PartLookupRow : PartLookupRows) {
		
		if (!PartLookupRow) continue;
		
		// Row name IS the tag name.
		FPartID PartID = PartLookupRow->PartID;
		
		if (!PartID.IsValid() || !PartLookupRow->PartPrimaryAsset.IsValid()) continue;
		
		if (PartLookup.Contains(PartID)) continue;
		
		PartLookup.Add(PartID, PartLookupRow->PartPrimaryAsset);
		
	}

	return PartLookup.Num() > 0;

}

bool UAssemblyDataSubsystem::BuildAssemblyLookup() {

	AssemblyLookup.Empty();

	if (!AssemblyLookupTableRef) return false;

	static const FString ContextString(TEXT("UAssemblyDataSubsystem::BuildAssemblyLookup"));
	
	TArray<FAssemblyLookupRow*> AssemblyLookupRows;
	AssemblyLookupTableRef->GetAllRows<FAssemblyLookupRow>(ContextString, AssemblyLookupRows);

	for (const FAssemblyLookupRow* AssemblyLookupRow : AssemblyLookupRows) {
	
		if (!AssemblyLookupRow) continue;
		
		// Row name IS the tag name.
		FAssemblySequenceID AssemblySequenceID = AssemblyLookupRow->AssemblySequenceID;
		
		if (!AssemblySequenceID.IsValid() || !AssemblyLookupRow->AssemblyPrimaryAsset.IsValid()) continue;

		if (AssemblyLookup.Contains(AssemblySequenceID)) continue;
		
		AssemblyLookup.Add(AssemblySequenceID, AssemblyLookupRow->AssemblyPrimaryAsset);
		
	}

	return AssemblyLookup.Num() > 0;

}

void UAssemblyDataSubsystem::RequestLoadSession(const FAssemblySequenceID In_ID, const FOnAssemblySessionLoaded& In_OnComplete) {

	if (!In_ID.IsValid()) {
		
		In_OnComplete.ExecuteIfBound(false);
		return;
		
	}

	const FPrimaryAssetId* AssemblyAssetId = AssemblyLookup.Find(In_ID);
	if (!AssemblyAssetId) {
		
		In_OnComplete.ExecuteIfBound(false);
		return;
		
	}

	// Step 1: async load the assembly definition.
	FStreamableDelegate OnAssemblyLoaded = FStreamableDelegate::CreateLambda([this, In_ID, In_OnComplete] () {
		
		OnAssemblyDefinitionLoaded(In_ID, In_OnComplete);
		
	});

	TArray<FName> Bundles;
	UAssetManager::Get().LoadPrimaryAsset(*AssemblyAssetId, Bundles, OnAssemblyLoaded);

}

void UAssemblyDataSubsystem::OnAssemblyDefinitionLoaded(const FAssemblySequenceID In_ID, const FOnAssemblySessionLoaded& In_OnComplete) {
	
	const FPrimaryAssetId* AssemblyAssetId = AssemblyLookup.Find(In_ID);
	
	if (!AssemblyAssetId) {
		
		In_OnComplete.ExecuteIfBound(false);
		return;
		
	}

	UAssemblySequenceDefinition* AssemblyDef = Cast<UAssemblySequenceDefinition>(UAssetManager::Get().GetPrimaryAssetObject(*AssemblyAssetId));

	if (!AssemblyDef || !AssemblyDef->IsValid()) {
		
		In_OnComplete.ExecuteIfBound(false);
		return;
		
	}

	// Collect all unique PartIDs referenced by this sequence.
	TArray<FPartID> RequiredParts;
	RequiredParts.AddUnique(AssemblyDef->AssemblySequenceData.BasePartID);
	
	for (const FAssemblyStep& Step : AssemblyDef->AssemblySequenceData.Steps) {
		
		RequiredParts.AddUnique(Step.PartID);
		
	}

	// Map to asset ids.
	TArray<FPrimaryAssetId> PartPrimaryAssets;
	PartPrimaryAssets.Reserve(RequiredParts.Num());
	
	for (const FPartID& PartId : RequiredParts) {
		
		const FPrimaryAssetId* FoundId = PartLookup.Find(PartId);
		if (!FoundId) {

			In_OnComplete.ExecuteIfBound(false);
			return;
			
		}
		
		PartPrimaryAssets.AddUnique(*FoundId);
		
	}

	// Step 2: async load all parts.
	FStreamableDelegate OnPartsLoadedDelegate = FStreamableDelegate::CreateLambda([this, In_ID, PartPrimaryAssets, In_OnComplete] () {
	
		OnPartsLoaded(In_ID, PartPrimaryAssets, In_OnComplete);
		
	});

	TArray<FName> Bundles;
	UAssetManager::Get().LoadPrimaryAssets(PartPrimaryAssets, Bundles, OnPartsLoadedDelegate);
	
}

void UAssemblyDataSubsystem::OnPartsLoaded(const FAssemblySequenceID In_ID, const TArray<FPrimaryAssetId>& In_PartAssetIds, const FOnAssemblySessionLoaded& In_OnComplete) {
	
	if (!CacheAndValidateParts(In_PartAssetIds)) {
		
		In_OnComplete.ExecuteIfBound(false);
		return;
		
	}

	const FPrimaryAssetId* AssemblyPrimaryAsset = AssemblyLookup.Find(In_ID);
	if (!AssemblyPrimaryAsset) {
		
		In_OnComplete.ExecuteIfBound(false);
		return;
		
	}

	UAssemblySequenceDefinition* AssemblyDef = Cast<UAssemblySequenceDefinition>(UAssetManager::Get().GetPrimaryAssetObject(*AssemblyPrimaryAsset));

	if (!AssemblyDef) {
		
		In_OnComplete.ExecuteIfBound(false);
		return;
		
	}

	CachedAssemblyData.Add(In_ID, AssemblyDef->AssemblySequenceData);

	In_OnComplete.ExecuteIfBound(true);
	
}

bool UAssemblyDataSubsystem::CacheAndValidateParts(const TArray<FPrimaryAssetId>& In_PartPrimaryAssets) {
	
	const UAssetManager& AssetManager = UAssetManager::Get();
	
	// Build into locals — commit only if every part validates.
	TMap<FPartID, FPartVisual> StagedVisuals;
	TMap<FPartID, FPartData> StagedData;

	StagedVisuals.Reserve(In_PartPrimaryAssets.Num());
	StagedData.Reserve(In_PartPrimaryAssets.Num());
	
	for (const FPrimaryAssetId& PartPrimaryAsset : In_PartPrimaryAssets) {
		
		FPartVisual LoadedPartVisual;

		UPartDefinition* PartDefinition = Cast<UPartDefinition>(AssetManager.GetPrimaryAssetObject(PartPrimaryAsset));

		if (!IsValid(PartDefinition)) return false;

		// Content validation: Resolve the Soft-References.
		LoadedPartVisual.PartMesh = PartDefinition->PartData.PartMesh.LoadSynchronous();
		LoadedPartVisual.GlowMaterial = PartDefinition->PartData.GlowMaterial.LoadSynchronous();
		LoadedPartVisual.SilhouetteOverlayMaterial = PartDefinition->PartData.SilhouetteOverlayMaterial.LoadSynchronous();
		LoadedPartVisual.IconImage = PartDefinition->PartData.IconImage.LoadSynchronous();
		
		if (!LoadedPartVisual.IsPartVisualValid()) return false;

		StagedVisuals.Add(PartDefinition->PartData.PartID, LoadedPartVisual);
		StagedData.Add(PartDefinition->PartData.PartID, PartDefinition->PartData);
		
	}
	
	// All validated — commit atomically. Both maps are replaced together.
	PartVisuals = MoveTemp(StagedVisuals);
	CachedPartData = MoveTemp(StagedData);
	
	return true;
	
}

const FAssemblySequenceData* UAssemblyDataSubsystem::GetAssemblyData(const FAssemblySequenceID& In_ID) const {

	return CachedAssemblyData.Find(In_ID);

}

const FPartData* UAssemblyDataSubsystem::GetPartData(const FPartID& In_ID) const {

	return CachedPartData.Find(In_ID);

}

bool UAssemblyDataSubsystem::GetPartPrimaryAsset(const FPartID& In_ID, FPrimaryAssetId& Out_AssetId) const {

	if (const FPrimaryAssetId* Found = PartLookup.Find(In_ID)) {
		
		Out_AssetId = *Found;
		return true;
		
	}
	
	return false;

}

bool UAssemblyDataSubsystem::GetAssemblyPrimaryAsset(const FAssemblySequenceID& In_ID, FPrimaryAssetId& Out_AssetId) const {

	if (const FPrimaryAssetId* Found = AssemblyLookup.Find(In_ID)) {
		
		Out_AssetId = *Found;
		return true;
		
	}
	
	return false;

}

void UAssemblyDataSubsystem::Deinitialize() {
	
	PartVisuals.Empty();
	PartLookup.Empty();
	AssemblyLookup.Empty();
	CachedPartData.Empty();
	CachedAssemblyData.Empty();
	
	Super::Deinitialize();
	
}