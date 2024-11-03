// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_PhysicsEntity.h"
#include "Interfaces/EDU_CORE_CommandInterface.h"
#include "EDU_CORE_MobileEntity.generated.h"

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
		OmniDirectional    UMETA(DisplayName = "Omni-Directional")	
};

UENUM()
enum class EMovementOrder : uint8
{
	// Just started the game.
		Disabled		UMETA(DisplayName = "Disabled"),
	// Ready for orders.	
		Idle			UMETA(DisplayName = "Ready for orders"),
	// Navigate to position.
		Navigate		UMETA(DisplayName = "Move To"),
	// Come to a controlled stop at the current position.
		Park			UMETA(DisplayName = "Park"),
	// Never Move, not for anything.
		HoldPosition	UMETA(DisplayName = "Hold Position"),
};

UCLASS()
class EDU_CORE_API AEDU_CORE_MobileEntity : public AEDU_CORE_PhysicsEntity, public IEDU_CORE_CommandInterface
{
	GENERATED_BODY()
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
protected:
	AEDU_CORE_MobileEntity();
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:
	virtual void ServerMobileCalc(float DeltaTime, int32 CurrentBatchIndex);
	virtual void ServerMobilesExec(float DeltaTime, int32 CurrentBatchIndex);

	// Fixed Physics Tick for calculations dependent on the physics thread
	virtual void AsyncPhysicsTickActor(float DeltaTime, float SimTime) override;
	
//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------
public:
	
	virtual void AddWaypoint(AEDU_CORE_Waypoint* Waypoint, const EEDU_CORE_WaypointType WaypointType, const FVector& WaypointLocation, const FRotator& WaypointRotation, const FVector& WaypointForwardVector, const FVector& WaypointRightVector, const int32 FormationIndex, const bool Queue) override;
	virtual void RemoveWaypoint(AEDU_CORE_Waypoint* Waypoint) override;
	virtual void ClearAllWaypoints();

	// Waypoint need to update listeners about FormationLocation whenever an entity leaves the formation.
	virtual void UpdateFormationLocation(const FVector WaypointLocation, FRotator WaypointRotation, FVector WaypointForwardVector, FVector WaypointRightVector, int32 FormationIndex);

	// Waypoint need to update listeners about FormationLocation whenever an entity leaves the formation.
	virtual void UpdateBatchIndex(const int32 ServerBatchIndex);
	
	// In Case anyone needs our Array.
	const virtual TArray<AEDU_CORE_Waypoint*>& GetWaypointArray() { return WaypointArray; }
	
//------------------------------------------------------------------------------
// Components: Waypoints & Navigation
//------------------------------------------------------------------------------
protected:
	UPROPERTY(EditAnywhere, Category = "Waypoints")
	int8 MaxWaypointCapacity = 10;

	// To queue orders, we need the ability to store waypoints.
	UPROPERTY()
	TArray<TObjectPtr<AEDU_CORE_Waypoint>> WaypointArray;

	// If we save this, weäll need to reset it manually all the time.
	// UPROPERTY()
	// TObjectPtr<UNavigationPath> NavPath;
	
	UPROPERTY()
	TObjectPtr<UNavigationSystemV1> NavSystem;
	
	// Navigation Points retrieved from the NavSystem.
	UPROPERTY()
	TArray<FVector> NavPointArray;

	// Relocate Position in case we need to evade something
	UPROPERTY()
	FVector EvadePoint;

	// Place in formation
	UPROPERTY()
	int32 SavedFormationIndex;

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

	// Collision Sphere used for Dynamic Collision Avoidance
	TObjectPtr<USphereComponent>DynamicCollisionSphere = nullptr;

	// Current Order
	EMovementOrder MovementOrder;

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
	
	// Do wee need a surface to move on?
	UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
	bool bMovesOnSurface = true;
	
	// Can this entity go backwards?.
	UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
	bool bCanReverse = true;
	
	// Can this entity climb surfaces more steep than 40 degrees?
    UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
    bool bCanClimb = false;

	// Do we only want to do Yaw turns?
	// Relevant for hovercrafts, drones, gunships and other flying
	// objects that always turn horizontal despite their pitch and rotation.
	UPROPERTY(EditAnywhere, Category = "Movement | Characteristics")
	bool bYawTurnsOnly = false;
	
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

	// How much drifting do we want?
	UPROPERTY(EditAnywhere, Category = "Movement | Locomotion", meta = (ClampMin = "0.0", ClampMax = "0.999"))
	float Inertia = 0.f;

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

	// At what distance should we turn rather than reverse?
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
	UPROPERTY(EditAnywhere, Category = "Movement | Collision Detection")
	TEnumAsByte<ECollisionChannel> CollisionDetectionVolumeChannel;
	
	// What surface does this entity move on?
	UPROPERTY(EditAnywhere, Category = "Movement | Collision Detection")
	TEnumAsByte<ECollisionChannel> GroundChannel;
	
	// We want to trace a bit forward or back depending on which way we are going.
	// Recommended is half of the actors' length.
	UPROPERTY(EditAnywhere, Category = "Movement | Collision Detection")
	float CollisionDetectionStartOffset  = 35.f;

	// If the collision detection is oversensitive, or not enough, adjust it here.
	UPROPERTY(EditAnywhere, Category = "Movement | Collision Detection")
	float CollisionTraceMult  = 1.f;

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
	// Time sliced tick
	//---------------------------------------------
	// Issued by GameMode
	int32 BatchIndex;

	// We use a DeltaTimer for functions that need a Realtime Delay.
	float DeltaTimer;

	//---------------------------------------------
	// BoxSize
	//---------------------------------------------
	FVector Origin;
	FVector BoxExtent;
	float DistanceToGround;
	
	//---------------------------------------------
	// Navigation Data
	//---------------------------------------------
	// Location of the next NavPoint, retrieved from the navigation system.
	FVector NavPointLocation;

	// checks if we have ground friction
	bool bIsOnSurface;

	// Saved location, in case we clip through the landscape
	FVector LastValidLocation;

	// Retrieved from the UnrealWorld(Level)
	FVector CurrentSpeedVector;

	// Actor position saved from the last frame
	FVector LastPos;

	// Used if the entity is moved against its will
	FVector DesignatedPos;

	// Measured by measuring the distance between CurrentPos and LastPos
	float ActualSpeed;
	
	//---------------------------------------------
	// Formation Data
	//---------------------------------------------
	// Location in formation, in offset to the waypoint. The offset depends on formation.
	FVector FormationLocation;
	
	// Rotation in formation.
	FRotator FormationRotation;
	
	//---------------------------------------------
	// NavigateTo
	//---------------------------------------------
	bool bShouldReverse = false;
	bool bForceReverse = false;

	// Distance to Position
	float Distance;

	// Accumulated force over time.
	float ForceOutput;
	
	// 3D Vector caused by the entities own force.
	FVector EntityForwardVector;
	
	// Desired sum of X + Y + Z velocity.
	float DesiredSpeed = 0.f;

	// Current Magnitude of our VelocityVectors(XYZ) combined. 
	float CurrentSpeed;
	
	//---------------------------------------------
	// Align
	//---------------------------------------------
	float TurnRate = 0.f;
	
	float FrictionCompensation;
	
	float RotationSpeed;
	
	bool bCheckAlignment = false;
	bool bShouldAlign = false;
	
	FVector Torque;

	FRotator AlignEndRotation;
	FRotator AlignStartRotation;
	FRotator AlignRotationDistance;

	// How far we need to turn (Always positive)
	float TurnDistance;

	// The difference in rotation to target (Can be Negative)
	float RotationDifference;
	float LastRotationDifference;
	
	//---------------------------------------------
	// CollisionAvoidance
	//---------------------------------------------
	bool bPathIsClear;
	bool bShouldEvade;
	bool bShouldEvadeLeft;

	// Predicts future (0.5 second) position.
	FVector MovementVector;
	
//------------------------------------------------------------------------------
// Physics Movement
//------------------------------------------------------------------------------
protected:
	void AdjustSpeed();

	// Align the actor to a target position over time.
	void Align(float DeltaTime);
	
	
	
//------------------------------------------------------------------------------
// AI Functionality
//------------------------------------------------------------------------------
protected:
	// Get the first waypoint in the store WaypointArray and retrieve its orders.
	virtual void RetrieveWaypointOrders();

	// Checks if the entity should rest or continue to next waypoint.
	virtual void ReviewNavigationQueue();
	
	// Carry out waypoint orders
	virtual void ExecuteOrders(const EEDU_CORE_WaypointType WaypointType, const FVector& WaypointLocation, const FRotator& WaypointRotation, const FVector& WaypointForwardVector, const FVector& WaypointRightVector, int32 FormationIndex);
	
//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------
protected:
	// See if there is anything under us to drive on.
	bool OnSurface() const;
	
	// Check our Alignment to target
	void CheckAlignment();

	// Check if we are where we're supposed to be, especially when parked.
	void CheckPosition();
	
	// TurnRate calculation for Align().
	void DetermineTurnSpeed();

	// Fetch velocity vector once per tick and cache it
	void CalculateCurrentSpeed(const FVector& Vector, float DeltaTime);

	// Calculates distance to target
	float CalculateDistance(const FVector& CurrentPos) const;

	// Checks if we should reverse
	void CheckReversalConditions();

	// Checks speed and position
	void HandleNavigation();

	// Aligns and comes to controlled stop
	void HandleParking();
	
//------------------------------------------------------------------------------
// Functionality: Collision avoidance
//------------------------------------------------------------------------------

	// Crates a Collision Sphere that other entities check against
	void CreateCollisionSphere();

	// Updates the Collision Sphere with future position
	void UpdateCollisionSphere(const FVector& CollisionSpherePosition) const;
	
	// Checks all directions in order: Front, Right, Left, Back, and returns an EvadePoint if it is found
	bool PathIsClear();

	// Helper function: Traces in certain direction
	bool IsPathClear(const FVector& TraceStartLocation, const FVector& TraceEndLocation, FHitResult& HitResult, const FCollisionQueryParams& QueryParams) const;


//------------------------------------------------------------------------------
// Functionality: Navigation
//------------------------------------------------------------------------------

	// Request a NavPath from UNavigationSystemV1
	void RequestPath(const FVector& Start, const FVector& End);

	// Request a NavPath Async
	void RequestPathAsync(const FVector& Start, const FVector& End);

	// Execute AsyncPath
	void OnRequestPathAsyncComplete(uint32 RequestID, ENavigationQueryResult::Type Result, FNavPathSharedPtr Path);
	
};
