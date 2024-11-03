#pragma once

#include "EDU_CORE_DataTypes.generated.h"
/*--------------------------- Trace Channels -------------------------------------
--------------------------------------------------------------------------------*/
#define TRACE_CHANNEL_TERRAIN   ECC_GameTraceChannel1
#define TRACE_CHANNEL_ENTITIES  ECC_GameTraceChannel2

/*------------------------- Input: Modifier Keys ---------------------------------
  This Enums keeps track of the active mod key, or active combo of mod keys
  in the CameraPawn. This means that any class can review this enum to se what
  mod state the controller is in.
--------------------------------------------------------------------------------*/
UENUM()
enum class EEDU_CORE_InputModifierKey : uint8
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

/*------------------------------- Teams ------------------------------------------
--------------------------------------------------------------------------------*/
UENUM()
enum class EEDU_CORE_Team : uint8
{
	None,
	Team_1,
	Team_2,
	Team_3,
	Team_4,
	Team_5,
	Team_6,
	Team_7,
	Team_8,
	Team_9,
	Team_10,
};


/*----------------------------- Formations----------------------------------------
  This Enum is used by WayPoints and CTRL Groups to allocate a formation
  position to listeners.
--------------------------------------------------------------------------------*/

UENUM()
enum class EEDU_CORE_Formations : uint8
{
	// Default
	NoFormation,

	Line,
	Wedge,
	File,
	StaggeredColumn
};

/*---------------------------- Unit settings -------------------------------------
--------------------------------------------------------------------------------*/

UENUM()
enum class EEDU_CORE_CombatMode : uint8
{
	Indifferent,
	Safe,
	Aware,
	Danger,
	Stealth,
};

UENUM()
enum class EEDU_CORE_Stance : uint8
{
	Prone,
	Crouched,
	Stand,
};

UENUM()
enum class EEDU_CORE_Speed : uint8
{
	Slow,
	Medium,
	Fast,
};

UENUM()
enum class EEDU_CORE_ROE : uint8
{
	HoldFire,
	SelfDefence,
	FireAtTarget,
	FireAtPosition,
	FireAtWill,
	EngageAtWill
};

/*--------------------------- WayPoint Settings ---------------------------------
  The Waypoint acts as a set of instructions for the entities listening to it.
--------------------------------------------------------------------------------*/

UENUM()
enum class EEDU_CORE_WaypointType : uint8
{
	// Default
	NavigateTo,
	AttackPosition,
	ObservePosition,
};

USTRUCT()
struct FWaypointParams
{	/*-------------------------------------------------------------------
	  Instead of passing all these variables as parameters, we pass
	  the entire struct. It keeps the cose a lot cleaner and readable.
	-------------------------------------------------------------------*/
	GENERATED_BODY()

	// World position for the waypoint
	UPROPERTY(EditAnywhere)
	FVector WorldPosition = FVector::ZeroVector;

	// Rotation for the waypoint, default to zero rotation
	UPROPERTY(EditAnywhere)
	FRotator WaypointRotation = FRotator::ZeroRotator;

	// Type of waypoint, default to NavigateTo
	UPROPERTY(EditAnywhere)
	EEDU_CORE_WaypointType WaypointType = EEDU_CORE_WaypointType::NavigateTo;

	// Should the waypoint be queued?
	UPROPERTY(EditAnywhere)
	bool Queue = false;

	// Align waypoint to cursor?
	UPROPERTY(EditAnywhere)
	bool CursorAlignment = false;
};

