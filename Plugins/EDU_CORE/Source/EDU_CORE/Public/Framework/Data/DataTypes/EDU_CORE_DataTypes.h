#pragma once

#include "EDU_CORE_DataTypes.generated.h"
/*--------------------------- Trace Channels -------------------------------------
  Trace Channels are limited to 20, so I't always best to define them as a MACRO
  and use the MACRO as a variable.
--------------------------------------------------------------------------------*/

// Used by Camera, Physics hulls and NavMesh
#define TRACE_CHANNEL_GROUND		ECC_GameTraceChannel1

// For Dynamic Collision Detection. 
#define TRACE_CHANNEL_COLLISION		ECC_GameTraceChannel2

// Used by VisualConfirmation in the Sight Component
#define TRACE_CHANNEL_EYESIGHT		ECC_GameTraceChannel3

// Used by the Field of Vision Boxtrace in the Sight Component
#define TRACE_CHANNEL_FOVBOX		TraceTypeQuery2

// For Tracing Entities with Cursor
#define TRACE_CHANNEL_ENTITIES		TraceTypeQuery1

// Used by the Field of Vision Boxtrace in the Sight Component
#define TRACE_CHANNEL_FOVBOX		TraceTypeQuery2

class AEDU_CORE_Waypoint;
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
  This enum keeps track of teams. The Enum::Max works well in for loops.

  Example:
	// Cast 'Team' to uint8 if it's an enum class
	const uint8 MyTeam = static_cast<uint8>(ActiveTeam);
	constexpr uint8 Max = static_cast<uint8>(EEDU_CORE_Team::Max);

	// 0 is neutral
	for (uint8 Enum = 1; Enum < Max; ++Enum)
	{
		if (Enum != MyTeam) // Exclude the specified team
		{
			HostileTeams.AddUnique(Enum);
		}
	}
--------------------------------------------------------------------------------*/
UENUM(BlueprintType)
enum class EEDU_CORE_Team : uint8
{
	None		UMETA(DisplayName = "No Team", ToolTip = "No specific selection, spectator, all knowing"),
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
	Spectator,
	Max			UMETA(Hidden),
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
enum class EEDU_CORE_AlertnessLevel : uint8
{
	Indifferent,
	Safe,
	Aware,
	Danger,
	Stealth,
};

UENUM()
enum class EEDU_CORE_InfantryStance : uint8
{
	None,
	Prone,
	Crouched,
	Stand,
};

UENUM()
enum class EEDU_CORE_MovementSpeed : uint8
{
	Slow,
	Medium,
	Full,
};

UENUM()
enum class EEDU_CORE_ROE : uint8
{
	HoldFire,
	SelfDefence,
	FireAtTargetOnly,
	FireAtWill,
};

UENUM()
enum class EEngagementMode : uint8
{
	HoldPosition,
	EngageTargetOnly,
	EngageAtWill,
	RoamAtWill,
};

UENUM()
enum class ETargetPriority : uint8
{
	Nearest				UMETA(DisplayName = "Nearest"),
	Farthest			UMETA(DisplayName = "Farthest"),

	LowestHealth		UMETA(DisplayName = "Lowest Health"),
	HighestHealth		UMETA(DisplayName = "Highest Health"),
	
	LowestDamage		UMETA(DisplayName = "Lowest Damage"),
	HighestDamage		UMETA(DisplayName = "Highest Damage"),

	LowestDefense		UMETA(DisplayName = "Lowest Defense"),
	HighestDefense		UMETA(DisplayName = "Highest Defense"),
	
	Max					UMETA(Hidden)
};


/*--------------------------- WayPoint Settings ---------------------------------
  The Waypoint acts as a set of instructions for the entities listening to it.
--------------------------------------------------------------------------------*/

UENUM()
enum class EEDU_CORE_WaypointType : uint8
{
	// Default
	NavigateTo,
	FollowTarget,
	AttackTarget,
	AttackPosition,
	ObservePosition,
};

class AEDU_CORE_SelectableEntity;
USTRUCT()
struct FWaypointParams
{	/*-------------------------------------------------------------------
	  Instead of passing all these variables as parameters, we pass
	  the entire struct. It keeps the code a lot cleaner and readable.
	-------------------------------------------------------------------*/
	GENERATED_BODY()
	
	// Pointer to this waypoint
	UPROPERTY(EditAnywhere)
	TObjectPtr<AEDU_CORE_Waypoint> WaypointPtr = nullptr;

	// The waypoint's world position 
	UPROPERTY(EditAnywhere)
	FVector WaypointPosition = FVector::ZeroVector;

	// Rotation for the waypoint, default to zero rotation
	UPROPERTY(EditAnywhere)
	FRotator WaypointRotation = FRotator::ZeroRotator;

	// Entities that this waypoint targets;
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<AEDU_CORE_SelectableEntity>> TargetArray;

	// Map Location that this waypoint targets;
	UPROPERTY(VisibleAnywhere)
	FVector TargetPosition = FVector::ZeroVector;

	// Should the waypoint be queued?
	UPROPERTY(EditAnywhere)
	bool bQueue = false;

	// Should this waypoint be recycled until removed manually?
	UPROPERTY(EditAnywhere)
	bool bPatrolPoint = false;

	// Align waypoint to cursor?
	UPROPERTY(EditAnywhere)
	bool bCursorAlignment = false;

	//------------------------------------------------------------
	// Waypoint Enum Settings
	//------------------------------------------------------------
	
	UPROPERTY(EditAnywhere)
	EEDU_CORE_WaypointType WaypointType = EEDU_CORE_WaypointType::NavigateTo;
	
	UPROPERTY(VisibleAnywhere)
	EEDU_CORE_Team WaypointTeam = EEDU_CORE_Team::None;

	UPROPERTY(VisibleAnywhere)
	EEDU_CORE_Formations WaypointFormation = EEDU_CORE_Formations::NoFormation;
	
	//------------------------------------------------------------
	// Unit Enum Settings
	//------------------------------------------------------------

	UPROPERTY(VisibleAnywhere)
	EEDU_CORE_AlertnessLevel AlertnessLevel = EEDU_CORE_AlertnessLevel::Indifferent;

	UPROPERTY(VisibleAnywhere)
	EEDU_CORE_InfantryStance InfantryStance = EEDU_CORE_InfantryStance::None;

	UPROPERTY(VisibleAnywhere)
	EEDU_CORE_MovementSpeed MovementSpeed = EEDU_CORE_MovementSpeed::Full;

	UPROPERTY(VisibleAnywhere)
	EEDU_CORE_ROE ROE = EEDU_CORE_ROE::HoldFire;

	UPROPERTY(VisibleAnywhere)
	EEngagementMode EngagementMode = EEngagementMode::HoldPosition;
	
	//------------------------------------------------------------
	// Formation info for Mobile Entities
	//------------------------------------------------------------

	// Used by MobileEnties to align with formation.
	UPROPERTY()
	FVector WaypointForwardVector = FVector::ZeroVector;

	// Used by MobileEnties to offset their position in advanced formations.
	UPROPERTY()
	FVector WaypointRightVector = FVector::ZeroVector;
	
};

/*--------------------------- Weapons & Damage -----------------------------------
  To deal and take damage.
--------------------------------------------------------------------------------*/

UENUM(BlueprintType)
enum class EWeaponStatus : uint8
{
	// Can't be used, can be repaired
		Disabled		UMETA(DisplayName = "Disabled"),
	// Functions, but is faulty
		Damaged			UMETA(DisplayName = "Damaged"),
	// Can track, but can't fire
		NoAmmo			UMETA(DisplayName = "Out of Ammo"),
	// Has ammo, is functional and is aligned.
		Ready			UMETA(DisplayName = "Ready"),
	// Target is lost, searching for it.
		Searching		UMETA(DisplayName = "Searching for target"),
	// Is engaged with a suboptimal target
		Supporting		UMETA(DisplayName = "Supporting"),
	// Is engaged with an optimal targetm should not be interupted
		Engaged			UMETA(DisplayName = "Engaged"),

	Max        UMETA(Hidden)
};

// Enum for different damage types
UENUM(BlueprintType) // Allow this enum to be used in Blueprints
enum class EDamageType : uint8
{
	EDT_None			UMETA(DisplayName = "None"),

	// All physical force is kinetic, including shockwaves.
	EDT_Kinetic			UMETA(DisplayName = "Kinetic"),

	// Hazardouly low temperature caused by exposure to cold materials/chemicals or weather.
	EDT_Cold			UMETA(DisplayName = "Cold"),

	// Hazardouly high temperature, caused by exposure to hot materials/chemicals, fire, plasma or weather.
	EDT_Heat			UMETA(DisplayName = "Heat"),

	// Transmission of energy in the form of waves or particles through space or a medium.
	EDT_Radiation		UMETA(DisplayName = "Radiation"),

	// Biological agents, such as viruses, bacteria or fungus.
	EDT_Biological		UMETA(DisplayName = "Biological"),

	// Harmful chemical reactions caused by poisons and toxins or other agents.
	EDT_Chemical		UMETA(DisplayName = "Chemical"),

	// Caused by malicious software attacks or force or energy that causes software runtimes to glitch.
	EDT_Malware			UMETA(DisplayName = "Malware"),
	
	// Abstract or supernatural damage such as cosmic, void, distortion or unkown).
	EDT_Chaos			UMETA(DisplayName = "Chaos"),
	
	EDT_Max				UMETA(Hidden)
};

class AProjectileBase;
class UTurretWeaponComponent;
USTRUCT(BlueprintType)
struct FProjectileWeaponInformation
{
	GENERATED_BODY()
	
	// The ID this Weapon
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Characteristics")
	FGuid WeaponID = FGuid::NewGuid();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Characteristics")
	FString WeaponName = "";
	
	// The owning Actor of this Weapon
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Characteristics")
	TObjectPtr<AActor> OwningActor = nullptr;
	
	// The owning Turret of this Weapon (if attached to a Turret)
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Characteristics")
	TObjectPtr<UTurretWeaponComponent> OwningTurret = nullptr;

	// The Mount Array Index of this weapon
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Characteristics")
	int32 WeaponArrayIndex = 0;

	// Allows this weapon to fire simultaneously with others.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Characteristics")
	bool ParallelBarrel = true;

	// Allows this weapon to fire simultaneously with others.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Characteristics")
	float RealoadSpeed = 5.f;
	
	// How much offset does this weapon have from the Owner's center?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	FVector BarrelOffset = FVector::ZeroVector;
	
	// What kind of projectile does this weapon fire?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	TSubclassOf<AProjectileBase> ProjectileClass = nullptr;

	// A burst is a controlled sequence of shots before releasing the trigger.
	// A Rifleman will usually fire a single shot at a time, while a Machinegunner will first burst of 10-15 bullets before a short delay.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	float BurstSize = 1.f;
	
	// How fast does this weapon cycle between shots within a burst?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	float CycleRate = 0.2f;

	// How long before the next burst?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	float BurstDelay = 1.f;

	// How inacurate (cm) is this weapon (regardles of distance) while still?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	float Inaccuracy  = 0.f;
	
	// How inacurate (cm) is this weapon (regardles of distance) while moving?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	float MovingInaccuracy  = 0.f;
	
	// The Maximum Amount of Ammunition this Weapon can carry.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ammo")
	int32 MaxAmmo = 0.f;
	
	// The Amount of Ammunition this Weapon currently has.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ammo")
	int32 CurrentAmmo = 0.f;
	
	// The Type of Damage this weapon inflicts on Impact.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	EDamageType DamageType;

	// The Amount of Damage this weapon inflicts on Impact.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	float Damage;

	// What percentage of energy is designed to penetrate armor?
	// 90% would imply the weapon's energy penetrates solid objects with ease, similar to gamma rays.
	// 100% would imply the damage phases through the armor similar to God-Rei in Evangelion.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	float Penetration = 0.f;
	
	// Area of Effect (splash) Radius
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	float AreaOfEffectRadius = 0.f;
	
	// Does this weapon charge before firing?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	float ChargeDelay  = 0.f;

	// Minimum Distance to Target for this wepaon to fire.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	float MinDistance = 0.f;
	
	// Maximum Distance to Target for this wepaon to fire.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Characteristics")
	float MaxDistance = 0.f;

	// Define equality operator to evaluate Weapon Uniqueness
	bool operator==(const FProjectileWeaponInformation& Other) const
	{
		return WeaponID == Other.WeaponID;
	}
	
	// Define GetTypeHash for Uniqueness
	friend uint32 GetTypeHash(const FProjectileWeaponInformation& Info)
	{
		return GetTypeHash(Info.WeaponID);
	}
	
};

/*------------------------- Conditions & Traits ---------------------------------
  Organic Status
--------------------------------------------------------------------------------*/

UENUM()
enum class EOrganicStatus_Physique : uint8
{
	Unknown				UMETA(DisplayName = "Unknown"),
	Unconscious			UMETA(DisplayName = "Unconscious"),
	Wounded				UMETA(DisplayName = "Wounded"),
	Injured				UMETA(DisplayName = "Injured"),
	InPain				UMETA(DisplayName = "InPain"),
	Grazed				UMETA(DisplayName = "Grazed"),
	Solid				UMETA(DisplayName = "Solid"),
	
};

UENUM()
enum class EOrganicStatus_Condition : uint8
{
	Unknown				UMETA(DisplayName = "Unknown"),
	Unconscious			UMETA(DisplayName = "Unconscious"),
	Sick				UMETA(DisplayName = "Sick"),
	Feverish			UMETA(DisplayName = "Feverish"),
	Nauseous			UMETA(DisplayName = "Nauseous"),
	Discomforted		UMETA(DisplayName = "Discomforted"),
	Alert				UMETA(DisplayName = "Alert"),
	
};

UENUM()
enum class EOrganicStatus_Nutrition : uint8
{
	Unknown				UMETA(DisplayName = "Unknown"),
	Unconscious			UMETA(DisplayName = "Unconscious"),
	Starving			UMETA(DisplayName = "Starving"),
	Famished			UMETA(DisplayName = "Famished"),
	VeryHungry			UMETA(DisplayName = "Very Hungry"),
	Hungry				UMETA(DisplayName = "Hungry"),
	Satiated			UMETA(DisplayName = "Satiated"),
	
};

UENUM()
enum class EOrganicStatus_Hydration : uint8
{
	Unknown				UMETA(DisplayName = "Unknown"),
	Unconscious			UMETA(DisplayName = "Unconscious"),
	DesperateForWater	UMETA(DisplayName = "Desperate for Water"),
	Dehydrated			UMETA(DisplayName = "Dehydrated"),
	VeryThirsty			UMETA(DisplayName = "Very Thirsty"),
	Thirsty				UMETA(DisplayName = "Thirsty"),
	Satiated			UMETA(DisplayName = "Satiated"),
	
};

UENUM()
enum class EOrganicStatus_Endurance : uint8
{
	Unknown				UMETA(DisplayName = "Unknown"),
	Unconscious			UMETA(DisplayName = "Unconscious"),
	Exhausted			UMETA(DisplayName = "Exhausted"),
	Tired				UMETA(DisplayName = "Tired"),
	Winded				UMETA(DisplayName = "Winded"),
	Warmedup			UMETA(DisplayName = "Warmed up"),
	Rested				UMETA(DisplayName = "Rested"),
	
};

UENUM()
enum class EOrganicStatus_Mental : uint8
{
	Unknown				UMETA(DisplayName = "Unknown"),
	Unconscious			UMETA(DisplayName = "Unconscious"),
	Panicked			UMETA(DisplayName = "Panicked"),
	Scared				UMETA(DisplayName = "Scared"),
	Shaken				UMETA(DisplayName = "Shaken"),
	Stressed			UMETA(DisplayName = "Stressed"),
	Steady				UMETA(DisplayName = "Steady"),
	
};

/*------------------------- Conditions & Traits ---------------------------------
  Sickness and Health
--------------------------------------------------------------------------------*/

USTRUCT(BlueprintType)
struct FOrganicCondition
{
	GENERATED_BODY()
	//--------------------------------------------------------------------------------------
	// Description
	//--------------------------------------------------------------------------------------
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Severity = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Treatment = "";
	
	//--------------------------------------------------------------------------------------
	// Regen / Degen
	//--------------------------------------------------------------------------------------

	// Physical
	UPROPERTY()
	float PhysDegen = 0.f;

	UPROPERTY()
	float CondDegen = 0.f;

	// Sustinance
	UPROPERTY()
	float HydroDegen = 0.f;

	UPROPERTY()
	float NutrDegen = 0.f;

		
	//--------------------------------------------------------------------------------------
	// Flat Damage
	//--------------------------------------------------------------------------------------
	
	UPROPERTY()
	float PhysDamage = 0.f;

	UPROPERTY()
	float CondDamage = 0.f;
	
	UPROPERTY()
	float EnduDamage = 0.f;

	UPROPERTY()
	float SustDamage = 0.f;

	// Define the custom hash function for TSet
	friend uint32 GetTypeHash(const FOrganicCondition& Condition)
	{
		uint32 Hash = 0;

		// Combine the hashes of the relevant fields of the struct
		Hash = HashCombine(Hash, GetTypeHash(Condition.Name));
		Hash = HashCombine(Hash, GetTypeHash(Condition.Description));
		Hash = HashCombine(Hash, GetTypeHash(Condition.Severity));
		Hash = HashCombine(Hash, GetTypeHash(Condition.Treatment));
		
		Hash = HashCombine(Hash, GetTypeHash(Condition.PhysDegen));
		Hash = HashCombine(Hash, GetTypeHash(Condition.CondDegen));
		Hash = HashCombine(Hash, GetTypeHash(Condition.HydroDegen));
		Hash = HashCombine(Hash, GetTypeHash(Condition.NutrDegen));

		Hash = HashCombine(Hash, GetTypeHash(Condition.PhysDamage));
		Hash = HashCombine(Hash, GetTypeHash(Condition.CondDamage));
		Hash = HashCombine(Hash, GetTypeHash(Condition.EnduDamage));
		Hash = HashCombine(Hash, GetTypeHash(Condition.SustDamage));

		return Hash;
	}
	
	// Overloaded == operator to compare each field for equality.
	bool operator==(const FOrganicCondition& Other) const = default;
	bool operator!=(const FOrganicCondition& Other) const
	{
		return !(*this == Other);  // Calls the == operator
	}
};
