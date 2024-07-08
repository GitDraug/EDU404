#pragma once

#include "EDU_USER_DataTypes.generated.h"
//-----------------------------------------------------------------------------------
// Trace Channels
//-----------------------------------------------------------------------------------
#define TRACE_CHANNEL_TERRAIN   ECC_GameTraceChannel1
#define TRACE_CHANNEL_ENTITIES  ECC_GameTraceChannel2

/*--------------------------------------------------------------------------------
  EEDU_USER_InputModifierKey: Enum for Selection
--------------------------------------------------------------------------------
  This Enums keeps track of the active mod key, or active combo of mod keys
  in the CameraPawn. This means that any class can review this enum to se what
  mod state the cntroller is in.
--------------------------------------------------------------------------------*/
UENUM()
enum class EEDU_USER_InputModifierKey : uint8
{
	// Default
	NoModifier,

	// Single keys
	Mod_1,
	Mod_2,
	Mod_3,
	Mod_4,

	// 2-Key combo
	Mod_12,
	Mod_13,
	Mod_14,
	
	Mod_23,
	Mod_24,

	Mod_34,

	// 3-key combo
	Mod_123,
	Mod_124,
	Mod_134,

	Mod_234,
};

UENUM()
enum class EEDU_USER_AutoScroll : uint8
{
	Toggle,
	Hold
};

