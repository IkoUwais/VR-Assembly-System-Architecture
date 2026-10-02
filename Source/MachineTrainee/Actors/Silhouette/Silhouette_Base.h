#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MachineTrainee/Data/Structs/Assembly_StructTypes_Game.h"



#include "Silhouette_Base.generated.h"


class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;


/**
 * Dedicated silhouette actor for the silhouette preview.
 *
 * Deliberately separate from APartActor:
 *  - No grab component (never held).
 *  - No physics, no collision.
 *  - No shadow casting.
 *  - Poolable later — one actor, reused across all steps via ShowSilhouette / HideSilhouette.
 *
 * The manager owns the silhouette. It pushes mesh + transform. The silhouette resolves nothing.
 * The silhouette material is a shared material on this actor (D16 — per-part override comes later).
 */
UCLASS(BlueprintType)
class MACHINETRAINEE_API ASilhouette_Base : public AActor {
	
	
	GENERATED_BODY()
	
	
public:
	
	ASilhouette_Base();

protected:

	virtual void BeginPlay() override;

public:

	// Show the silhouette at the given world transform with the given mesh.
	// Manager calls this on every AdvanceStep.
	UFUNCTION(BlueprintCallable)
	void ShowSilhouette(const FTransform& In_TargetTransform, const FPartVisual& In_PartVisual);

	// Hide the silhouette. Manager calls this on successful placement or sequence end.
	UFUNCTION(BlueprintCallable)
	void HideSilhouette();
	
	// Swap the shared silhouette material. Reserved for later per-part override.
	UFUNCTION(BlueprintCallable)
	void SetSilhouetteMaterials(UMaterialInterface* In_GlowMaterial, UMaterialInterface* In_OverlayMaterial);
	
	void SwitchToRegularSilhouetteMaterial();
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// Shared glow material. - The Glow of the Mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> DefaultSilhouetteGlowMaterial;

	// Shared silhouette material. - The Outline of the Mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> DefaultSilhouetteOverlayMaterial;

	UPROPERTY(BlueprintReadOnly)
	FPartVisual CachedPartVisual;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bShowing = false;
	
	bool SetSilhouetteGlowMaterial(UMaterialInterface* In_GlowMaterial);
	
	bool SetSilhouetteOverlayMaterial(UMaterialInterface* In_SilhouetteOverlayMaterial);
	
private:
	
	//
	
	
public:

	UFUNCTION(BlueprintPure)
	bool IsShowing() const { return bShowing; }
	
};