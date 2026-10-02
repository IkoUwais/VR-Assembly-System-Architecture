
#include "Part_Base.h"
#include "Kismet/GameplayStatics.h"
#include "MachineTrainee/Actors/AssemblyManager/AssemblyManager.h"
#include "MachineTrainee/Components/State/StateComponent.h"
#include "MachineTrainee/Components/Grab/GrabComponent.h"


APart_Base::APart_Base() {
	
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionProfileName(FName("PhysicsActor"));
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetNotifyRigidBodyCollision(false);

	// StateComponent
	StateComponent = CreateDefaultSubobject<UStateComponent>(TEXT("StateComponent"));
	
	// GrabComponent
	GrabComponent = CreateDefaultSubobject<UGrabComponent>(TEXT("GrabComponent"));
	
}

void APart_Base::BeginPlay() {
	
	Super::BeginPlay();

	if (StateComponent) {
		
		StateComponent->OnInitialEntered.AddDynamic(this, &APart_Base::ApplyStateInitial);
		StateComponent->OnGrabbedForAssemblyEntered.AddDynamic(this, &APart_Base::ApplyStateGrabbedForAssembly);
		StateComponent->OnDroppedBeforeAssemblyEntered.AddDynamic(this, &APart_Base::ApplyStateDroppedBeforeAssembly);
		StateComponent->OnAppliedToAssemblyEntered.AddDynamic(this, &APart_Base::ApplyStateAppliedToAssembly);
		
	}
	
	if (GrabComponent) {
		
		GrabComponent->OnActorGrabbed.AddDynamic(this, &APart_Base::OnPartGrabbed);
		GrabComponent->OnActorReleased.AddDynamic(this, &APart_Base::OnPartReleased);
		
	}
	
}

void APart_Base::InitializePartWithData(const FPartData& In_PartData, const FPartVisual& In_PartVisual) {
	
	PartData = In_PartData;
	PartVisual = In_PartVisual;
	InitializeMeshWithPartData();
	
	SetPartState(E_PartState::Initial);
	
	bPartInitialized = true;
	
}


void APart_Base::InitializeMeshWithPartData() {

	if (PartVisual.IsPartVisualValid()) {
		
		MeshComponent->SetStaticMesh(PartVisual.PartMesh);
		
	}

	// Mass. SetSimulatePhysics must be on before mass sticks.
	MeshComponent->SetMassOverrideInKg(NAME_None, PartData.Mass, true);

	// Collision profile.
	MeshComponent->SetCollisionProfileName(PartData.CollisionPreset);

}

void APart_Base::SetRespawnTransform(const FTransform& In_Transform) {
	
	RespawnTransform = In_Transform;
	
}

// Called from Assembly-System to apply to the Assembly
// Snap this part to a target transform and lock it.
void APart_Base::PartAppliedToAssembly(const FTransform& In_TargetTransform) {
	
	SetActorTransform(In_TargetTransform, false, nullptr, ETeleportType::TeleportPhysics);
	SetPartState(E_PartState::AppliedToAssembly);

}


// - Called by Grab-Component - //
void APart_Base::OnPartGrabbed_Implementation(AActor* In_GrabbingActor) {

	SetPartState(E_PartState::GrabbedForAssembly);
	
	// Update grab to Manager for glowing the silhouette.
	if (AssemblyManager) {
		
		AssemblyManager->PartGrabbedForAssembly(this);
		
	}
	
	// Toggle_Marker(false);
	
}
	
void APart_Base::OnPartReleased_Implementation(AActor* In_GrabbingActor, const FVector& In_ReleaseVelocity) {

	// when it's released manually AND NOT applied to the assembly
	if (IsAppliedToAssembly()) return;
	
	SetPartState(E_PartState::DroppedBeforeAssembly);
	
	// Manager decides snap vs drop. Called after state settle, before physics takes over.
	if (AssemblyManager) {
		
		AssemblyManager->OnPartReleased(this);
		
	}
	
	if (GetPartState() == E_PartState::DroppedBeforeAssembly && MeshComponent) {
		
		const FVector Scaled = In_ReleaseVelocity * PartData.GrabProfile.ThrowVelocityMultiplier;
		MeshComponent->SetPhysicsLinearVelocity(Scaled);
		
	}
	
}
// - Called by Grab-Component - //


// Called by State-Component - //
void APart_Base::ApplyStateInitial(E_PartState In_NewState) {

	// properties for state = Initial
	MeshComponent->SetCollisionProfileName(FName("PhysicsActor"));
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}

void APart_Base::ApplyStateGrabbedForAssembly(E_PartState In_NewState) {

	// properties for state = GrabbedForAssembly
	GetWorldTimerManager().ClearTimer(OutOfReachTimerHandle);
	
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// TODO : do physics stuff for grabbing - check VR template for correct properties setup
		
	
}

void APart_Base::ApplyStateDroppedBeforeAssembly(E_PartState In_NewState) {

	// start checking if out of reach here
	GetWorldTimerManager().SetTimer(OutOfReachTimerHandle, this, &APart_Base::CheckOutOfReach, 0.5f, true);
	
	if (!MeshComponent) return;
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

}

void APart_Base::ApplyStateAppliedToAssembly(E_PartState In_NewState) {
	
	if (!MeshComponent) return;
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	Toggle_Marker(false);
	
	// Locks the part as Assembled — no grabbing, no physics. Used for base part and after successful snap.
	GetWorldTimerManager().ClearTimer(OutOfReachTimerHandle);	
	
	if (GrabComponent) {
		
		GrabComponent->DisableGrab(AssemblyManager);
		
	}
	
}
// Called by State-Component - //

void APart_Base::CheckOutOfReach() {

	if (GetPartState() != E_PartState::DroppedBeforeAssembly) return;

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player) return;

	const float GrabActorDistanceFromPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	
	if (GrabActorDistanceFromPlayer > PartData.GrabProfile.OutOfReachThreshold) {
		
		UE_LOG(LogTemp, Verbose, TEXT("APartActor: '%s' out of reach (%.1f cm), returning to table"), *PartData.PartID.GetTag().ToString(), GrabActorDistanceFromPlayer);

		SetPartState(E_PartState::Initial);
		SetActorTransform(RespawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		
	}

}

void APart_Base::UpdateAssemblyManager(AAssemblyManager* In_AssemblyManager) {

	AssemblyManager = In_AssemblyManager;
	
}

void APart_Base::Toggle_Marker_Implementation(const bool In_Mark) {

	// highlight / un-highlight the part based on "In_Mark"
	
}

void APart_Base::SetPartState(E_PartState In_PartState) {

	if (StateComponent) {
		
		StateComponent->SetState(In_PartState);
		
	}
	
}

E_PartState APart_Base::GetPartState() const {

	return StateComponent? StateComponent->GetState() : E_PartState::Initial;
	
}