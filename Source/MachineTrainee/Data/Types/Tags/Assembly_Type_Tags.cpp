
#include "Assembly_Type_Tags.h"


// Assembly-Sequence
namespace AssemblySequence {

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(JIB_HEAD, "AssemblySequence.JIB_HEAD", "JIB_HEAD Assembly Sequence");
	// UE_DEFINE_GAMEPLAY_TAG(JIB_HEAD, "AssemblySequence.JIB_HEAD");		// example, can also be defined "Without Comment"
	
}

// Part
namespace Part {

	namespace General {
		
		namespace Screw {
		
			UE_DEFINE_GAMEPLAY_TAG(Screw_1, "Part.General.Screw.Screw_1");
		
		}
		
		namespace Tools {

			UE_DEFINE_GAMEPLAY_TAG(DrillMachine, "Part.General.Tools.DrillMachine");

		}

	}
	
	namespace JIB_HEAD {

		namespace Parts {

			UE_DEFINE_GAMEPLAY_TAG(BossExtrude_1, "Part.JIB_HEAD.Parts.BossExtrude_1");
			UE_DEFINE_GAMEPLAY_TAG(BossExtrude_2, "Part.JIB_HEAD.Parts.BossExtrude_2");
			UE_DEFINE_GAMEPLAY_TAG(BossExtrude_1_2, "Part.JIB_HEAD.Parts.BossExtrude_1_2");
			UE_DEFINE_GAMEPLAY_TAG(BossExtrude_1_3, "Part.JIB_HEAD.Parts.BossExtrude_1_3");
			UE_DEFINE_GAMEPLAY_TAG(Chamfer_1, "Part.JIB_HEAD.Parts.Chamfer_1");
			UE_DEFINE_GAMEPLAY_TAG(Chamfer_1_2, "Part.JIB_HEAD.Parts.Chamfer_1_2");
			UE_DEFINE_GAMEPLAY_TAG(CutExtrude_1, "Part.JIB_HEAD.Parts.CutExtrude_1");
			UE_DEFINE_GAMEPLAY_TAG(PlateBody_1, "Part.JIB_HEAD.Parts.PlateBody_1");
			UE_DEFINE_GAMEPLAY_TAG(PullyBody_1, "Part.JIB_HEAD.Parts.PullyBody_1");
			UE_DEFINE_GAMEPLAY_TAG(TopStripBody_1, "Part.JIB_HEAD.Parts.TopStripBody_1");

		}

	}
	
}