#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/*
Part                                  (atomic installable parts)
├── Part.Structural                   Frame, Bracket, Plate, Beam, Rail, Chassis, Mount
├── Part.Fastener                     Bolt, Nut, Screw, Washer, Pin, Rivet, Clip, Key
├── Part.Mechanical                   Gear, Shaft, Bearing, Belt, Chain, Pulley, Coupling, Spring
├── Part.Electrical                   Wire, Connector, Sensor, Board, Switch, Fuse, Relay
├── Part.Fluid                        Pipe, Hose, Valve, Fitting, Pump, Nozzle
├── Part.Seal                         Gasket, Oring, Packing, Bushing
├── Part.Thermal					  HeatSink, Insulation, Duct
├── Part.Safety                       Guard, Cover, Shield, Barrier
└── Part.Surface                      Panel, Skin, Trim, Decal

Assembly                              (composite sub-assemblies — later)
├── Assembly.Sub                      smaller nested assembly
├── Assembly.Module                   medium scope
└── Assembly.Unit                     larger scope, top-level

Tool                                  (tools — different gameplay, later)
├── Tool.Hand
│   ├── Tool.Hand.Wrench              Combination, Adjustable, Socket, Ratchet, Torque,
│   │                                 Impact, Crowfoot, Monkey, Spud, Cone, Allen, Pipe
│   ├── Tool.Hand.Hammer              Claw, Ball, Sledge, Mallet
│   ├── Tool.Hand.Plier               Standard, Needle, Locking, Cutting
│   ├── Tool.Hand.Screwdriver         Flat, Phillips, Torx, Hex
│   └── Tool.Hand.Cutter              Knife, Snip, Shear
├── Tool.Power
│   ├── Tool.Power.Drill
│   ├── Tool.Power.Saw
│   ├── Tool.Power.Grinder
│   └── Tool.Power.Impact
└── Tool.Measuring
	├── Tool.Measuring.Caliper
	├── Tool.Measuring.Gauge
	└── Tool.Measuring.Meter
*/

/*
 * to access a tag...
 * for instance a Screw_1, which is a "Screw", "General" and a "Part"
 * Part::General::Screw::Screw_1
*/

// Assembly-Sequence
namespace AssemblySequence {

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(JIB_HEAD);

}


// Part
namespace Part {

	namespace General {
		
		namespace Screw {
		
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Screw_1);
		
		}
		
		
		namespace Tools {

			UE_DECLARE_GAMEPLAY_TAG_EXTERN(DrillMachine);

		}

	}
	
	namespace JIB_HEAD {

		namespace Parts {

			UE_DECLARE_GAMEPLAY_TAG_EXTERN(BossExtrude_1);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(BossExtrude_2);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(BossExtrude_1_2);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(BossExtrude_1_3);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamfer_1);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamfer_1_2);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(CutExtrude_1);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(PlateBody_1);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(PullyBody_1);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(TopStripBody_1);

		}

	}
	
}

