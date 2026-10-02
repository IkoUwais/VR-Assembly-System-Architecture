
#include "Silhouette_Base.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Game.h"

ASilhouette_Base::ASilhouette_Base() {

	PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // Silhouette is visual only — no physics, no collision, no shadow, no nav.
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetGenerateOverlapEvents(false);
    MeshComponent->SetCastShadow(false);
    MeshComponent->SetReceivesDecals(false);
    MeshComponent->SetCanEverAffectNavigation(false);
	
	CachedPartVisual = FPartVisual();
	
}

void ASilhouette_Base::BeginPlay() {
	
	Super::BeginPlay();
	
	SetSilhouetteGlowMaterial(DefaultSilhouetteGlowMaterial);
	SetSilhouetteOverlayMaterial(DefaultSilhouetteOverlayMaterial);
	
	HideSilhouette();
	
}

void ASilhouette_Base::ShowSilhouette(const FTransform& In_TargetTransform, const FPartVisual& In_PartVisual) {
	
	if (!In_PartVisual.IsPartVisualValid()) return;
	
	CachedPartVisual = In_PartVisual;
	
	MeshComponent->SetStaticMesh(CachedPartVisual.PartMesh);
	SetSilhouetteMaterials(CachedPartVisual.GlowMaterial, CachedPartVisual.SilhouetteOverlayMaterial);
	
	SetActorTransform(In_TargetTransform, false, nullptr, ETeleportType::TeleportPhysics);
	
	SetActorHiddenInGame(false);	
	bShowing = true;
	
}

void ASilhouette_Base::HideSilhouette() {
	
	SetActorHiddenInGame(true);
	bShowing = false;
	
}

void ASilhouette_Base::SwitchToRegularSilhouetteMaterial() {
	
	if (CachedPartVisual.IsPartVisualValid()) {
	
		SetSilhouetteMaterials(CachedPartVisual.GlowMaterial, CachedPartVisual.SilhouetteOverlayMaterial);
		return;
		
	}	
	
	SetSilhouetteMaterials(DefaultSilhouetteGlowMaterial, DefaultSilhouetteGlowMaterial);
	
}

void ASilhouette_Base::SetSilhouetteMaterials(UMaterialInterface* In_GlowMaterial, UMaterialInterface* In_OverlayMaterial) {
	
	SetSilhouetteGlowMaterial(In_GlowMaterial);
	SetSilhouetteOverlayMaterial(In_OverlayMaterial);
	/*return;
	
	
	// Apply the Cached-Silhouette-Glow-Material to the Mesh
	// if NOT then - Switch to the Default-Silhouette-Glow-Material
	if (!SetSilhouetteGlowMaterial(In_GlowMaterial)) {
		
		if (!SetSilhouetteGlowMaterial(CachedPartVisual.GlowMaterial)) {
		
			SetSilhouetteGlowMaterial(DefaultSilhouetteGlowMaterial);
		
		}
		
	}
	
	// Apply the Cached-Silhouette-Overlay-Material to the Mesh
	// if NOT then - Switch to the Default-Silhouette-Overlay-Material
	if (!SetSilhouetteOverlayMaterial(In_OverlayMaterial)) {
		
		if (!SetSilhouetteOverlayMaterial(CachedPartVisual.SilhouetteOverlayMaterial)) {
		
			SetSilhouetteOverlayMaterial(DefaultSilhouetteOverlayMaterial);
		
		}
		
	}
	*/
	
}

bool ASilhouette_Base::SetSilhouetteGlowMaterial(UMaterialInterface* In_GlowMaterial) {
	
	if (IsValid(In_GlowMaterial)) {
		
		const bool ShouldApplyMaterial = MeshComponent->GetMaterial(MeshComponent->GetNumMaterials() - 1) != In_GlowMaterial;
		
		if (ShouldApplyMaterial) {
			
			const int32 NumMats = MeshComponent->GetNumMaterials();
			
			for (int32 MaterialSlotIndex = 0; MaterialSlotIndex < NumMats; ++MaterialSlotIndex) {
		
				MeshComponent->SetMaterial(MaterialSlotIndex, In_GlowMaterial);
		
			}
			
		}
		
		return true;
		
	}
	
	return false;
	
}

bool ASilhouette_Base::SetSilhouetteOverlayMaterial(UMaterialInterface* In_SilhouetteOverlayMaterial) {

	if (IsValid(In_SilhouetteOverlayMaterial)) {
		
		const bool ShouldApplyMaterial = MeshComponent->GetOverlayMaterial() != In_SilhouetteOverlayMaterial;
		
		if (ShouldApplyMaterial) {
		
			MeshComponent->SetOverlayMaterial(In_SilhouetteOverlayMaterial);
			
		}
		
		return true;
		
	}
	
	return false;
	
}