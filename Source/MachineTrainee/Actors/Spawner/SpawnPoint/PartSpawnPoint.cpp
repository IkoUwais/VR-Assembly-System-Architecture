
#include "PartSpawnPoint.h"

APartSpawnPoint::APartSpawnPoint() {
	
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;
	
}

void APartSpawnPoint::BeginPlay() {
	
	Super::BeginPlay();
	
}