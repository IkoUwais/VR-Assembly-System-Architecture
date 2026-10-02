# VR-Assembly-System-Architecture
VR Assembly Training System - Data-Driven | Multiplayer | DLC | XR

# Technologies Used
Unreal Engine 5 (C++ & Blueprints)
Gameplay Tags & Primary Data Assets
VR Motion Controllers & Physics
Async Asset Loading (Streamable Manager)

|Build-Link| -> https://drive.google.com/file/d/1o7ktLmsBT23WI0cvKCI88fqZe1JYhXEN/view?usp=sharing


# VR Assembly Training System - Architectural Showcase

**Author:** Aaves Shaikh (Senior Unreal Engine C++ Programmer)
**Status:** Core logic is private (IP Protection). This repository serves as a technical breakdown of the architecture, patterns, and C++ implementation strategies used in the project.

## 📖 Overview
This project is a highly modular, data-driven VR assembly simulation built in Unreal Engine 5 using C++. The goal was to create a robust framework where designers can add new assemblies, parts, and step-by-step sequences without writing any new code. 

It features a strict separation of concerns, asynchronous asset loading, and a custom VR grab/interaction system.

## 🏗️ Architecture & Design Patterns
The system was architected to avoid the common "Spaghetti Blueprint" trap. It relies heavily on C++ interfaces, data assets, and the Observer/Mediator patterns.

*   **Data-Driven Design:** Uses `UPrimaryDataAsset` (e.g., `UPartDefinition`, `UAssemblySequenceDefinition`) and Data Tables to define parts, meshes, and assembly steps. The game logic knows nothing about specific parts; it only reads data.
*   **Mediator Pattern:** `AAssemblyManager` acts as the central coordinator. It does not know about spawning or UI; it only manages the `CurrentStepIndex` and validates placements, broadcasting events to subscribers.
*   **Component-Based Architecture:** Parts (`APart_Base`) are composed of reusable components (`UStateComponent`, `UGrabComponent`), making them highly modular and easy to extend.
*   **State Machine:** A custom `UStateComponent` handles the lifecycle of a part (Initial -> Grabbed -> Dropped -> Applied), broadcasting events to update physics, collision, and visuals.
*   **Async Asset Management:** A `UAssemblyDataSubsystem` handles the async loading of all required assets (meshes, materials) for a specific assembly sequence, ensuring no frame-rate hitches during gameplay.

## 🔍 Key Subsystems Breakdown

### 1. Assembly Manager (Mediator / Facade)
*   **Responsibility:** Drives the step-by-step assembly sequence. Hides the silhouette, validates if the correct part is placed in the correct position (with position/rotation tolerance), and advances the sequence.
*   **Implementation:** Decoupled from spawning and UI. Uses delegates (`OnStepStarted`, `OnStepValidated`, `OnStepFailed`) to communicate with the rest of the game.

### 2. Assembly Spawner (Builder)
*   **Responsibility:** Constructs the assembly from a data payload (`FSpawnData`). Spawns the base part and initial table parts (conveyor-belt style).
*   **Implementation:** Uses a spawn-point marker system (`APartSpawnPoint`) to dynamically place parts on tables, freeing markers when parts are successfully assembled.

### 3. Part Base & Grab Component (Component Pattern)
*   **Responsibility:** Handles VR interaction. `APart_Base` is input-agnostic. `UGrabComponent` interfaces with `UMotionControllerComponent`.
*   **Implementation:** Handles physics attachment, velocity-based throwing, and out-of-reach recovery (teleporting the part back to its spawn point if the player drops it too far away).

### 4. Assembly Data Subsystem (Registry / Data Source)
*   **Responsibility:** The single source of truth for all data. It reads from Data Tables, resolves `FGameplayTags` to `FPrimaryAssetIds`, and async-loads the required assets.
*   **Implementation:** Strictly validates data before committing it to the cache, ensuring the game never starts with missing assets.

## 💻 Code Snippet Example (Validation Logic)
*This snippet from `AAssemblyManager::IsPlacementValid` demonstrates the defensive programming and per-step tolerance checking used to validate a player's placement.*

```cpp
bool AAssemblyManager::IsPlacementValid(APart_Base* In_Part) const {
 
    if (!In_Part || !SilhouetteActor) return false;

    const FAssemblyStep* Step = GetCurrentStep();
    if (!Step) return false;

    // Identity check
    if (In_Part->GetPartID() != Step->PartID) return false;

    const FTransform SilhouetteTransform = SilhouetteActor->GetActorTransform();
    const FTransform PartTransform = In_Part->GetActorTransform();
        
    // Position tolerance (per-step, cm)
    const float PositionDelta = FVector::Dist(PartTransform.GetLocation(), SilhouetteTransform.GetLocation());
    if (PositionDelta > Step->PosTolerance) return false;
    
    return true;
}