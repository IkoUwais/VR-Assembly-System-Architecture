
#include "Assembly_StructTypes_Game.h"

// - Part-Visual - //
bool FPartVisual::IsPartVisualValid() const {

	return IsValid(PartMesh) && (IsValid(GlowMaterial) || IsValid(SilhouetteOverlayMaterial));
	
}
// - Part-Visual - //



// - Assembly-Session-Result - //
void FAssemblySessionResult::SetCalculatedCleanRunPercent() {

	CleanRunPercent = (TotalSteps > 0) ? FMath::Clamp(100.f * (1.f - static_cast<float>(FailedAttempts) / static_cast<float>(TotalSteps)), 0.f, 100.f) : 0.f;
	
}
// - Assembly-Session-Result - //