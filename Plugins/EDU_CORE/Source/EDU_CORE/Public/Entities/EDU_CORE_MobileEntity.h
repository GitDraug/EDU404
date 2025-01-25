// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_PhysicsEntity.h"
#include "Interfaces/EDU_CORE_CommandInterface.h"
#include "EDU_CORE_MobileEntity.generated.h"

class AEDU_CORE_SelectableEntity;
class EDU_CORE_DataTypes;
class AEDU_CORE_Waypoint;
class AEDU_CORE_Waypoint_Move;
class AEDU_CORE_PlayerController;
class AEDU_CORE_C2_Camera;

class UPhysicalMaterial;
class USphereComponent;

class UNavigationPath;
class UNavigationSystemV1;

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  An entity with self-propulsion that can navigate using a navigation system,
  and collision detection.
  
  The entity can also be interfaced with by using waypoints.
------------------------------------------------------------------------------*/

UENUM(BlueprintType)
enum class EMovementType : uint8
{
	// Can move forward and backward, and must move to turn. 
		BiDirectional      UMETA(DisplayName = "Bi-Directional"),
	// Can move forward and backward, and rotate around its center whether moving or not.
		CenterRotates		UMETA(DisplayName = "Bi-Directional with Center Rotation"),
	// Can move in all directions, and rotate around its center whether moving or not.
		OmniDirectional		UMETA(DisplayName = "Omni-Directional"),
	// Hidden max value for internal use only.
		MAX					UMETA(Hidden)
};

UENUM()
enum class EMovementOrder : uint8
{
	// Just started the game.
		None			UMETA(DisplayName = "None"),
	// Can't Move
		Disabled		UMETA(DisplayName = "Disabled"),
	// Ready for orders.	
		Idle			UMETA(DisplayName = "Ready for orders"),
	// Move to position.
		MoveTo			UMETA(DisplayName = "Moving to Waypoint"),
	// Stop and aim
		Aim				UMETA(DisplayName = "Aiming"),
	// Come to a controlled stop at the current position.
		Park			UMETA(DisplayName = "Parking"),
	// Hidden max value for internal use only.
		MAX				UMETA(Hidden)
};

UENUM()
enum class ENavigationMode : uint8
{
	// Just started the game.
		None			UMETA(DisplayName = "None"),
	// Normal movement
		Navigate		UMETA(DisplayName = "Navigating"),
	// Looking for trouble
		Roam			UMETA(DisplayName = "Roaming"),
	// Seek Cover
		SeekCover		UMETA(DisplayName = "Seeking Cover"),
	// No LOS to target
		SeekTarget		UMETA(DisplayName = "Seeking Target"),
	// Keeping minimum distance
		KeepDistance	UMETA(DisplayName = "Keeping Distance"),
	// Hidden max value for internal use only.
		MAX				UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	// Applies Angular Velocity in the local Z-axis (For normal units)
	LocalYaw     UMETA(DisplayName = "Local Yaw"),

	// Applies Angular Velocity in the World Z-axis.
	// Used for entities like hovercrafts, drones, and gunships that 
	// always turn horizontally, regardless of their pitch and rotation.
	WorldYaw     UMETA(DisplayName = "World Yaw"),
	
	// Hidden max value for internal use only.
	MAX			UMETA(Hidden)
};

UCLASS()
class EDU_CORE_API AEDU_CORE_MobileEntity : public AEDU_CORE_PhysicsEntity, public IEDU_CORE_CommandInterface
{
	GENERATED_BODY()
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
protected:
	AEDU_CORE_MobileEntity(const FObjectInitializer& ObjectInitializer);
	
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:
	// Concurrence; Running 0.75/sec
	virtual void ServerMobileBatchedCalc();

	// Concurrence; Running 1/frame
	virtual void ServerMobileCalc(float DeltaTime, int32 CurrentBatchIndex);

	// Gamethread; Running 1/frame
	virtual void ServerMobileExec(float DeltaTime, int32 CurrentBatchIndex);
	
//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------
public:
	
	virtual void AddWaypoint(const FWaypointParams& Params) override;
	virtual void RemoveWaypoint(AEDU_CORE_Waypoint* Waypoint) override;
	virtual void ClearAllWaypoints();

	// Waypoint needs to update listeners about FormationLocation whenever an entity leaves the formation.
	virtual void UpdateFormationLocation(const FWaypointParams& Params);

	// Get Batch Index from GameMode
	virtual void UpdateBatchIndex(const int32 ServerBatchIndex);
	
	// In Case anyone needs our Array.
	const virtual TArray<AEDU_CORE_Waypoint*>& GetWaypointArray() { return WaypointArray; }

	// Set if this Entity should align with target, necessary with fixed weapons that needs manual aiming.
	virtual void SetAlignWithTarget(bool ShouldAlignWithTarget = false) { bShouldAlignWithTarget = ShouldAlignWithTarget; };
	
	// Set if this Entity should align with target, necessary with fixed weapons that needs manual aiming.
	virtual void SetTargetActor(AEDU_CORE_SelectableEntity* InTargetEntity = nullptr) { TargetEntity = InTargetEntity; };

	// Set if this Entity should align with target, necessary with fixed weapons that needs manual aiming.
	virtual void SetTargetPosition(const FVector& InTargetPos) { TargetPosition = InTargetPos; };

	// Sets the distance to slow down while aiming at a target.
	virtual void SetSlowDownThresholdWhileAiming(const float& SlowDistance) {  SlowDownThresholdWhileAiming = SlowDistance; }

	// Sets the distance to stop while aiming at a target.
	virtual void SetStopThresholdWhileAiming(const float& StopDistance) {  StopThresholdWhileAiming = StopDistance; }
	
//------------------------------------------------------------------------------
// Components: Waypoints & Navigation
//------------------------------------------------------------------------------
protected:
	UPROPERTY(EditAnywhere, Category = "Waypoints")
	int8 MaxWaypointCapacity = 10;

	// To queue orders, we need the ability to store waypoints.
	UPROPERTY()
	TArray<TObjectPtr<AEDU_CORE_Waypoint>> WaypointArray;

	UPROPERTY()
	TObjectPtr<UNavigationSystemV1> NavSystem;
	
	// Navigation Points retrieved from the NavSystem.
	UPROPERTY()
	TArray<FVector> NavPointArray;

	// Relocate Position in case we need to evade something
	FVector EvadePoint;

	// Relocate Position in case we need to evade something
	FVector AimPoint;
	
	// Properties of representation of an 'agent' used by AI navigation/pathfinding.
	FNavAgentProperties NavAgentProperties;

//------------------------------------------------------------------------------
// Components: Physics
//------------------------------------------------------------------------------
protected:
	// Container for a physics representation of an object
	FBodyInstance* PhysicsBodyInstance;

	// Physical Material we can alter during runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	TObjectPtr<UPhysicalMaterial>PhysicalMaterial;

	//-----------------------------------
	// Current Orders
	//-----------------------------------
	
	UPROPERTY(VisibleAnywhere, Category = "Current Orders")
	EEDU_CORE_AlertnessLevel AlertnessLevel = EEDU_CORE_AlertnessLevel::Indifferent;
	
	UPROPERTY(VisibleAnywhere, Category = "Current Orders")
    EEDU_CORE_InfantryStance InfantryStance = EEDU_CORE_InfantryStance::None;

	UPROPERTY(VisibleAnywhere, Category = "Current Orders")
	EEDU_CORE_MovementSpeed MovementSpeed = EEDU_CORE_MovementSpeed::Full;

	UPROPERTY(VisibleAnywhere, Category = "Current Orders")
	EEDU_CORE_ROE ROE = EEDU_CORE_ROE::HoldFire;

	UPROPERTY(VisibleAnywhere, Category = "Current Orders")
	EEngagementMode EngagementMode = EEngagementMode::HoldPosition;

	UPROPERTY(VisibleAnywhere, Category = "Current Orders")
	EMovementOrder MovementOrder = EMovementOrder::None;

	UPROPERTY(VisibleAnywhere, Category = "Current Orders")
	ENavigationMode NavigationMode = ENavigationMode::None;

//------------------------------------------------------------------------------
// Physics Movement
//------------------------------------------------------------------------------
protected:
	
	//------------------------------------------------------------
   	// Movement: Characteristics
   	//------------------------------------------------------------
	
	// How does this entity move?
	UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
	EMovementType MovementType;

	// How does this entity Rotate?
	UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
	ERotationMode RotationMode;
	
	// Can this entity go backwards?.
	UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
	bool bCanReverse = true;

	// Do wee need a surface to move on?
	UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
	bool bMovesOnSurface = true;

	// Should this Entity align with target? Attached FixedWeaponComponents that needs manual aiming will set this to true.
	UPROPERTY(VisibleAnywhere, Category = "Movement | Characteristics")
	bool bShouldAlignWithTarget = false;

	// Can this entity climb surfaces more steep than 40 degrees?
	UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
	bool bCanClimb = false;
	
	// What surface does this entity move on?
	UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
	TEnumAsByte<ECollisionChannel> SurfaceChannel;
	
	//------------------------------------------------------------
	// Movement: Locomotion
	//------------------------------------------------------------
	
	// The force this entity can output by its own accord per frame.
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion")
	float Acceleration = 5.f;

	// Max allowed speed before breaking.
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion")
	float MaxSpeed = 100.f;

	// The amount of speed we can strangle per second.
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion")
	float AccelerationBrakeMult = 0.3f;
	
	// Multiplier when parking with wheeled vehicles.
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion")
	float ParkingSpeed = 2.f;

	// How far from the target position should we slow down? (Centimeters)
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion")
	float SlowDownThreshold = 1000.f;

	// How far from the Waypoint are we allowed to park? (Centimeters)
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion")
	float StopThreshold = 100.f;

	// How far from the target position should we slow down if we are aiming at a target?
	// This variable is set by our EngagementComponent
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion| Combat")
	float SlowDownThresholdWhileAiming = 1500.f;

	// How far from the Waypoint are we allowed to park if are aiming at a target? (Centimeters)
	// This variable is set by our EngagementComponent
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion| Combat")
	float StopThresholdWhileAiming = 1000.f;

	// How much drifting do we want?
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion", meta = (ClampMin = "0.0", ClampMax = "0.999"),  meta = (EditCondition = "!bDynamicInertia"))
	float FixedInertia = 0.f;

	// Inertia increase and decrease with speed.
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion")
	bool bDynamicInertia;

	// Show the Show Velocity Markers for debugging.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement | Locomotion")
	bool bShowVelocityDebug = false;

	//------------------------------------------------------------
	// Movement: Rotation
	//------------------------------------------------------------
	
	// How fast can we rotate?
	UPROPERTY(EditAnywhere, Category = "Movement | Rotation")
	float MaxRotationRate = 4.f;
	
	// At what angle should we start to slow down?
	// 180 will make the rotation smooth all the way.
	UPROPERTY(EditAnywhere, Category = "Movement | Rotation", meta = (ClampMin = "1.0", ClampMax = "180.0"))
	float SlowRotationDistance = 180.f;

	// At what angle should we reverse rather than turning?
	UPROPERTY(EditAnywhere, Category = "Movement | Rotation", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float ReverseRotationDistance = 100.f;

	// At what rotational distance should we turn rather than reverse?
	// Note that Omni-Directional enteties don't reverse, they use AlignDistance instead. 
	UPROPERTY(EditAnywhere, Category = "Movement | Rotation")
	float MaxReverseDistance = 300.f;

	// How far before we start aligning if we are Omni-Directional?
	UPROPERTY(EditAnywhere, Category = "Movement | Rotation")
	float OmniDirectionalAlignDistance = 100.f;

	//------------------------------------------------------------
	// Movement: Formation
	//------------------------------------------------------------

	UPROPERTY(EditAnywhere, Category = "Movement | Formation")
	float FormationSpacing = 150.f;

	//------------------------------------------------------------
	// Movement: Collision Detection
	//------------------------------------------------------------

	// The Collision Detection system creates a dynamic sphere at BeginPlay that calculates where the actor will be in the near future.
	// How large should the sphere radius be?
	UPROPERTY(EditAnywhere, Category = "Movement | Collision Detection")
	float CollisionDetectionVolumeRadius  = 50.f;

	// What collision channel should this prediction volume occupy?
	// <!> This should not be the same as the root component because it
	//     will cause the prediction volume to collide with it.
	UPROPERTY(EditAnywhere, Category = "Movement | Collision Detection")
	TEnumAsByte<ECollisionChannel> CollisionDetectionVolumeChannel;
	
	// We want to trace a bit forward or back depending on which way we are going.
	// Recommended is half of the actors' length.
	UPROPERTY(EditAnywhere, Category = "Movement | Collision Detection")
	float CollisionDetectionStartOffset  = 35.f;

	// If the collision detection is oversensitive, or not enough, adjust it here.
	UPROPERTY(EditAnywhere, Category = "Movement | Collision Detection")
	float CollisionTraceMult  = 1.f;

	// Show the CollisionDetectionVolume for debugging.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement | Collision Detection")
	bool bShowCollisionDebug = false;

	//------------------------------------------------------------
	// Movement: Navigation
	//------------------------------------------------------------

	// Radius of the hull used for navigation/pathfinding.
	UPROPERTY(EditAnywhere, Category = "Movement | Navigation")
	float AgentRadius = -1.f;

	// Total height of the hull used for navigation/pathfinding.
	UPROPERTY(EditAnywhere, Category = "Movement | Navigation")
	float AgentHeight = -1.f;

	// Step height to use, or -1 for default value from navdata's config.
	UPROPERTY(EditAnywhere, Category = "Movement | Navigation")
	float AgentStepHeight = -1.f;

	// Scale factor to apply to height of bounds when searching for navmesh to project to when nav walking.
	UPROPERTY(EditAnywhere, Category = "Movement | Navigation")
	float NavWalkingSearchHeightScale = 0.5f;

	//---------------------------------------------
	// TargetData
	//---------------------------------------------

	UPROPERTY(VisibleAnywhere, Category = "Target")
	TObjectPtr<AEDU_CORE_SelectableEntity> TargetEntity = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Target")
	FVector TargetPosition = FVector::ZeroVector;
	
	//---------------------------------------------
	// Time sliced tick
	//---------------------------------------------
	// Issued by GameMode
	int32 BatchIndex;

	// We use a DeltaTimer for functions that need a Realtime Delay.
	float DeltaTimer;
	
	//---------------------------------------------
	// Navigation Data
	//---------------------------------------------
	// Location of the next NavPoint, retrieved from the navigation system.
	FVector NavPointLocation;

	// Omni-Directional beings always move directly to target, they don't need to align.
	FVector OmniDirectionalVector;

	// checks if we have ground friction
	bool bIsOnSurface;

	// Saved location, in case we clip through the landscape
	FVector LastValidLocation;

	// For saving location within intervals.
	uint8 LastValidLocationTimer;
	
	// Retrieved from the UnrealWorld(Level)
	FVector CurrentSpeedVector;

	// Actor position saved from the last frame
	FVector LastPos;

	// Measured by measuring the distance between CurrentPos and LastPos
	float ActualSpeed;
	
	//---------------------------------------------
	// Formation Data
	//---------------------------------------------
	
	// Location in formation, in offset to the waypoint. The offset depends on formation.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Current Orders")
	FVector FormationLocation;
	
	// Rotation in formation.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Current Orders")
	FRotator FormationRotation;
	
	//---------------------------------------------
	// NavigateTo
	//---------------------------------------------
	bool bShouldReverse = false;
	bool bForceReverse = false;

	bool bShouldAim = false;

	// Distance to Position
	float Distance;

	// Accumulated force over time.
	float ForceOutput;
	
	// Desired sum of X + Y + Z velocity.
	float DesiredSpeed = 0.f;

	// Current Magnitude of our VelocityVectors(XYZ) combined. 
	float CurrentSpeed;
	
	//---------------------------------------------
	// Align
	//---------------------------------------------
	
	float TurnRate = 0.f;
	
	bool bCheckAlignment = false;
	bool bShouldAlign = false;

	FVector Torque = FVector::ZeroVector;
	
	// Debug
	float debug_RotationSpeed;
	float debug_LastRotationDifference;

	float debug_AlignStartRotation;
	float debug_AlignEndRotation;
	
	float debug_RotationDifference;
	float debug_InverseRotationDifference;
	
	float debug_RawAlignRotationDistance;
	float debug_InverseRawRotationDifference;

	float debug_FastestTurnRate;
	float debug_TurnFrictionCompensation;

	bool debug_bRuntimer;
	float debug_DebugTimer;
	
	//---------------------------------------------
	// CollisionAvoidance
	//---------------------------------------------
	bool bPathIsClear;
	bool bShouldEvade;
	bool bShouldEvadeLeft;

	//---------------------------------------------
	// GroundCheck
	//---------------------------------------------

	// the sice of our collisionbox
	FVector BoxExtent;

	// Alows us to alternate our groundcheck to systematically cover the entire surface area
	bool bGroundAlter;
	
//------------------------------------------------------------------------------
// Physics Movement
//------------------------------------------------------------------------------
protected:

	// Adjust speed to desired speed
	void AdjustSpeed();

	// Align the actor to a target position over time.
	void Align();
	
//------------------------------------------------------------------------------
// AI Functionality
//------------------------------------------------------------------------------
protected:
	
	// Get the first waypoint in the store WaypointArray and retrieve its orders.
	virtual void RetrieveWaypointOrders();

	// Checks if the entity should rest or continue to next waypoint.
	virtual void ReviewNavigationQueue();
	
	// Carry out waypoint orders
	virtual void ExecuteOrders(const FWaypointParams& Params);
	
//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------
protected:
	
	// See if there is anything under us to move on.
	bool OnSurface(const FVector& CurrentPos);
	
	// Check our Alignment to target
	void CheckAlignment(const FRotator& CurrentPos);

	// Helper function for CheckAlignment() - checks if we should reverse.
	void CheckReversalConditions(const float RotationDistance);

	// Check if we are where we're supposed to be, especially when parked.
	void CheckPosition(const FVector& CurrentPos);
	
	// Fetch velocity vector once per tick and cache it
	void CalculateCurrentSpeed(const FVector& Vector, float DeltaTime);

	// Calculates distance to target
	float CalculateDistance(const FVector& CurrentPos) const;
	
	// Checks speed and position
	void HandleNavigation();

	// Aligns and comes to controlled stop
	void HandleParking(const FRotator& CurrentPos);
	
//------------------------------------------------------------------------------
// Functionality: Collision avoidance
//------------------------------------------------------------------------------
	
	// Checks all directions in order: Front, Right, Left, Back, and returns an EvadePoint if it is found
	bool PathIsClear();

	// Helper function: Traces in certain direction
	bool IsPathClear(const FVector& TraceStartLocation, const FVector& TraceEndLocation, const FCollisionQueryParams& QueryParams) const;


//------------------------------------------------------------------------------
// Functionality: Navigation
//------------------------------------------------------------------------------

	// Request a NavPath from UNavigationSystemV1
	void RequestPath(const FVector& Start, const FVector& End);

	// Request a NavPath Async
	void RequestPathAsync(const FVector& Start, const FVector& End);

	// Execute AsyncPath
	void OnRequestPathAsyncComplete(uint32 RequestID, ENavigationQueryResult::Type Result, FNavPathSharedPtr Path);

//------------------------------------------------------------------------------
// Legacy stuff (Deprecated)
//------------------------------------------------------------------------------

	// Crates a Collision Sphere that other entities check against (Deprecated)
	void CreateCollisionSphere();

	// Updates the Collision Sphere with future position (Deprecated)
	void UpdateCollisionSphere(const FVector& CollisionSpherePosition) const;
	
	// Collision Sphere used for Dynamic Collision Avoidance (Deprecated)
	TObjectPtr<USphereComponent>DynamicCollisionSphere = nullptr;
	
};
