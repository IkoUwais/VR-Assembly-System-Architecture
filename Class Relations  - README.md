# VR-Assembly-System-Architecture
VR Assembly Training System - Data-Driven | Multiplayer | DLC | XR



# VR Assembly Training System - Class Relations (In Brief Description)

"Author": Aaves Shaikh (Senior Unreal Engine C++ Programmer)
"Status": Core logic is private (IP Protection). This repository serves as a technical breakdown of the architecture, patterns, and C++ implementation strategies used in the project.



Subsystem Breakdown :
1. Orchestration Layer (The Directors)
AAssemblyGameMode_Base: The entry point. It handles the bootstrapping of the level. It does not run gameplay; it spawns the required actors (Manager, Spawner, Silhouette) and coordinates the start/end of the session.
AAssemblyGameState_Base: The central hub for runtime session data. It holds the CurrentStepIndex, tracks failure attempts, and holds the FAssemblySessionResult. It listens to the Manager and rebroadcasts events for the UI.


2. Data & Asset Layer (The Source of Truth)
UAssemblyDataSubsystem: A GameInstance Subsystem. It is the only system that knows about Data Tables and Asset Managers. It asynchronously loads UPartDefinition and UAssemblySequenceDefinition assets, validates them, and caches them in memory.
UPartDefinition / UAssemblySequenceDefinition: Primary Data Assets. These wrap pure data structs (FPartData, FAssemblySequenceData). Designers can edit these in the editor to create new parts or assembly sequences without writing code.


3. Assembly Logic Layer (The Brains)
AAssemblyManager (Mediator / Facade): The core step-machine. It holds the CachedAssemblySequenceData and drives the CurrentStepIndex. It validates if a part is placed correctly (position/rotation tolerance) and broadcasts the result. It never spawns actors or touches UI.
AAssemblySpawner (Builder): Responsible for constructing the physical assembly. It reads FSpawnData, spawns the base part, and manages the "conveyor belt" spawning of table parts using APartSpawnPoint markers. It listens to the Manager to know when to spawn the next part.


4. Actor & Component Layer (The Physical World)
APart_Base (Actor): The physical representation of a part. It is "dumb" and input-agnostic. It receives its data (FPartData) from the Spawner.

UStateComponent (Component): A reusable state machine. It manages the lifecycle of a part (Initial -> GrabbedForAssembly -> DroppedBeforeAssembly -> AppliedToAssembly) and broadcasts state transitions to update physics and collision.

UGrabComponent (Component): Handles VR interaction. It interfaces with UMotionControllerComponent to attach/detach the part, handle throwing velocity, and manage physics simulation.

ASilhouette_Base (Actor): A purely visual actor with no physics or collision. It is controlled entirely by the Manager to show the player where the next part goes.



Typical Data Flow (The "Happy Path") :
Initialization: AAssemblyGameMode_Base calls StartGameSession().

Loading: AAssemblyGameState_Base asks UAssemblyDataSubsystem to async-load the sequence and all required part assets.

Spawning: Once loaded, AAssemblyGameMode_Base builds an FSpawnData payload and passes it to AAssemblySpawner.

Setup: The Spawner creates the base part and the first 3 table parts. It registers them with AAssemblyManager.

Step Start: The Manager calls ShowSilhouette() on the ASilhouette_Base and broadcasts OnStepStarted.

Interaction: The player grabs a part. UGrabComponent broadcasts OnActorGrabbed. The part transitions to GrabbedForAssembly.

Validation: The player releases the part. UGrabComponent broadcasts OnActorReleased. The Manager checks IsPlacementValid().

Success: If valid, the Manager snaps the part to the silhouette, broadcasts OnStepValidated, and advances to the next step.

Failure: If invalid, the Manager broadcasts OnStepFailed, and the part falls naturally via physics. The AAssemblyGameState_Base records the failure.

Completion: Once all steps are validated, the Manager broadcasts OnSequenceCompleted, and the GameState calculates the final FAssemblySessionResult.


Key Design Patterns Used :
Data-Driven Design: Separating data (Data Assets) from logic (C++ systems) allows for rapid content creation without recompiling code.

Mediator Pattern: AAssemblyManager centralizes complex step logic, preventing actors from directly communicating with each other.

Observer Pattern: Systems broadcast events (OnStepStarted, OnStepValidated) without knowing who is listening, allowing UI, Spawner, and GameState to react independently.

Component-Based Architecture: Behavior like grabbing and state management is encapsulated in reusable components (UGrabComponent, UStateComponent), keeping APart_Base clean and focused.

Dependency Injection: The Spawner and Manager receive their dependencies (GameState, PartClass) from the GameMode rather than looking them up, making the code highly testable.