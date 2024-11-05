// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/EDU_CORE_MobileEntity.h"

// CORE
#include "AI/WayPoints/EDU_CORE_Waypoint.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"

// Navigation
#include "NavigationPath.h"
#include "NavigationSystem.h" // PublicDependencyModule: "NavigationSystem"

// Physics
#include "Components/SphereComponent.h" 
#include "FunctionLibrary/UtilityLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h" // PublicDependencyModule: "PhysicsCore"

/*------------------------- Running frequent booleans in tick --------------------------------------
 
  Theoretically, it is more efficient to check a boolean before calling a function,
  rather than calling the function and then evaluating a boolean inside it. Here’s why:

  1. Avoiding the Overhead of a Function Call:
  Function calls in Unreal (or C++ in general) comes with a small but real overhead.
  When you call a function, the CPU has to store the current state, move to
  the function's memory space, execute the function, and return. Even if the function ends
  up doing nothing (due to an early boolean check inside it), this overhead still occurs.
  
  If you perform the boolean check before calling the function, you can avoid this overhead
  altogether when the function does not need to run.
  
  2. Code Clarity and Maintenance:
  Checking the boolean condition before calling the function can make the code easier to read.
  It provides an early sign that the function may not be necessary under certain conditions,
  reducing confusion and improving code maintainability.
  
  If the boolean check is inside the function, someone reading the code has to look inside each
  function to understand when and why it executes, leading to potentially harder debugging or
  understanding of the code flow.
  
  3. Caching and Performance Optimization:
  Unreal Engine’s tick functions can be expensive if they involve multiple function calls
  every frame. Avoiding unnecessary function calls using boolean checks before invoking those
  functions is a typical optimization strategy, especially for tick-based operations that are
  executed very frequently (like every frame).

--------------------------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::ServerMobileCalc(float DeltaTime, int32 CurrentBatchIndex)
{
	// Caching Current Position once per tick to avoid redundant calls.
	const FVector& CurrentPos = GetActorLocation();
	
	CalculateCurrentSpeed(CurrentPos, DeltaTime); // Pass position to avoid redundant calls
	Distance = CalculateDistance(CurrentPos); // Pass position to avoid creating another FVector
	
	switch (MovementOrder)
	{
		case EMovementOrder::Navigate:
			// We don't want to runt his too often.
			if (CurrentBatchIndex == BatchIndex) HandleNavigation();
		break;
			
		case EMovementOrder::Park:
			HandleParking();
		break;
			
		default:
		break;
	}
	
}

void AEDU_CORE_MobileEntity::ServerMobilesExec(float DeltaTime, int32 CurrentBatchIndex)
{
	// Debug
	#define DEBUGMESSAGE GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds
	{		
	//	DEBUGMESSAGE, FColor::Green, FString::Printf(TEXT("NavPointArray.Num()): %d"), NavPointArray.Num()));
	
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));	

	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("MovementOrder == EMovementOrder::Navigate: %d"),MovementOrder == EMovementOrder::Navigate));
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("bCheckAlignment: %d"),bCheckAlignment));
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("bShouldAlign: %d"),bShouldAlign));
	//	DEBUGMESSAGE, FColor::Green,	FString::Printf(TEXT("RotationDifference: %f"), RotationDifference));
	//	DEBUGMESSAGE, FColor::Green,	FString::Printf(TEXT("RotationSpeed: %f"), RotationSpeed / DeltaTime));
		
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));
		
	//	DEBUGMESSAGE, FColor::Green,	FString::Printf(TEXT("TurnDistance: %f"), TurnDistance));
		
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));
		
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("Torque.Z: %f"), Torque.Z));
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("Torque.X: %f"), Torque.X));
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("Torque.Y: %f"), Torque.Y));
		
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));	
		
	//	DEBUGMESSAGE, FColor::Green,	FString::Printf(TEXT("TurnFrictionCompensation %f"),FrictionCompensation));
		
	//	DEBUGMESSAGE, FColor::Cyan,		FString::Printf(TEXT("CurrentSpeed: %f"),CurrentSpeed));
	//	DEBUGMESSAGE, FColor::Cyan,		FString::Printf(TEXT("ActualSpeed: %f"),ActualSpeed));

	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));
		
	//	DEBUGMESSAGE, FColor::Green,  FString::Printf(TEXT("EvadePoint: X%f, Y%f, Z%f"), EvadePoint.X, EvadePoint.Y, EvadePoint.Z));

	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));
		
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("bShouldReverse: %d"),bShouldReverse));

	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));
		
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("MovementOrder == EMovementOrder::Park: %d"),MovementOrder == EMovementOrder::Park));

	//	DEBUGMESSAGE, FColor::Emerald, 	FString::Printf(TEXT("-------------------------")));
	}
	
	//----------------------------------------------------------------------------------------------------
	// Update CollisionSphere
	//----------------------------------------------------------------------------------------------------
	if(CurrentBatchIndex == BatchIndex)
	{
		UpdateCollisionSphere(LastPos + (MovementVector * ActualSpeed));
	}
	
	DeltaTimer += DeltaTime;
	if (DeltaTimer >= 0.2f && CurrentBatchIndex == BatchIndex && CurrentSpeed > 0.1f)
	{
		bPathIsClear = PathIsClear();
		
		if (MovementOrder == EMovementOrder::Idle)
		{
			CheckAlignment();
			CheckPosition();
		}
		
		DeltaTimer = 0.f;
	}

	//----------------------------------------------------------------------------------------------------
	// Check if we are on a surface or free fall.
	//----------------------------------------------------------------------------------------------------
	if(CurrentBatchIndex == BatchIndex)
	{
		bIsOnSurface = OnSurface();
		if(bIsOnSurface)
		{
			LastValidLocation = GetActorLocation();
		}
		else
		{
			if(CurrentSpeedVector.Z < -1000.f && (GetActorLocation().Z < 0.f))
			{
				// Ground should never be below 0.
				SetActorLocation(LastValidLocation, false, nullptr, ETeleportType::ResetPhysics);
				PhysicsBodyInstance->SetLinearVelocity(FVector::ZeroVector, false);
			}
		}
	}

	//----------------------------------------------------------------------------------------------------
	// Surface Magnetism (Grip) for infantry, Qubits etc
	//----------------------------------------------------------------------------------------------------
	if(bCanClimb)
	{
		if(bIsOnSurface)
		{
			/*---------------------------------------------------------------------
			  GripStrength should likely be less than gravity, so we can be
			  knocked off, but if the force is less than gravity, then
			  FrictionCompensation in Align()  needs to be less too, else
			  we will over steer.	 
			---------------------------------------------------------------------*/
			PhysicsBodyInstance->SetEnableGravity(false);
			PhysicsBodyInstance->AddForce(FVector(-GetActorUpVector() * PhysicsBodyInstance->GetBodyMass() * 980.f), false);
		}
		else
		{
			PhysicsBodyInstance->SetEnableGravity(true);
		}
	}
	
	//----------------------------------------------------------------------------------------------------
	// Handle Movement
	//----------------------------------------------------------------------------------------------------
	if(MovementOrder == EMovementOrder::Navigate
	|| MovementOrder == EMovementOrder::Park)
	{
		if(MovementType == EMovementType::CenterRotates
		&& ActualSpeed < 1.f
		&& bShouldAlign)
		{
			// Turn first;
		}
		else
		{
			AdjustSpeed();
		}
	}

	//----------------------------------------------------------------------------------------------------
	// Align to the current target if needed.
	//----------------------------------------------------------------------------------------------------
	if(bShouldAlign)
	{
		if(MovementType == EMovementType::BiDirectional)
		{
			Align(DeltaTime);
			AdjustSpeed();
		}
		else
		{
			Align(DeltaTime);
		}
	}

	//----------------------------------------------------------------------------------------------------
	// We want to continuously check alignment, because we might be affected by outside forces.
	//----------------------------------------------------------------------------------------------------
	if(bShouldAlign
	|| MovementOrder == EMovementOrder::Park
	|| MovementOrder == EMovementOrder::Navigate)
	{
		CheckAlignment();
	}
}

void AEDU_CORE_MobileEntity::AsyncPhysicsTickActor(float DeltaTime, float SimTime)
{
	/*------------------------- Fixed Physics tick ---------------------------------
	  To run the physics engine at fixed FPS, we need to make sure the most
	  important variables for physics calculations are tied to the fixed
	  DeltaTime, so even if the main thread is blocked, physics will remain stable.

	  The changes to the PhysicsBody are done in ServerMobilesExec()
	------------------------------------------------------------------------------*/
	if(bShouldAlign)
	{
		DetermineTurnSpeed();	
	}
}

//------------------------------------------------------------------------------
// Physics Movement
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::AdjustSpeed() // We Use 0.02f (50FPS) instead of DeltaTime!
{ // FLOW_LOG
	if(bMovesOnSurface && !bIsOnSurface) return;

	// We divide our movement vector between ForwardVector and our InertiaVector to control drift deterministically.
	FVector ForwardVector;
	if(MovementType == EMovementType::OmniDirectional)
	{
		ForwardVector = AlignEndRotation.Vector() * ForceOutput * (1.f - Inertia);
	}
	else
	{
		ForwardVector = GetActorForwardVector() * ForceOutput * (1.f - Inertia);
	}
	
	FVector InertiaVector = PhysicsBodyInstance->GetUnrealWorldVelocity() * Inertia;
	
	if(CurrentSpeed < DesiredSpeed)
	{
		if(bShouldReverse)
		{
			// We just need to do ForceOutput inverted.
			ForceOutput -= FMath::Clamp(Acceleration * 0.02f, 0.f, MaxSpeed);
		}
		else
		{
			ForceOutput += FMath::Clamp(Acceleration * 0.02f, 0.f, MaxSpeed);
		}
		// Draw the debug line
		// DrawDebugLine(GetWorld(),GetActorLocation(), GetActorLocation() + ForwardVector * (CurrentSpeed * 0.01f),FColor::Blue,false,0.1f, 0.f, 1.f);
		// DrawDebugLine(GetWorld(),GetActorLocation(), GetActorLocation() + InertiaVector * (CurrentSpeed * 0.01f),FColor::White,false,0.1f, 0.f, 1.f);
	}
	else // if(CurrentSpeed > DesiredSpeed)
	{
		// We want to strangle ForceOutput.
		if(ForceOutput < 0.f)
		{
			// ForceOutput is negative here, so max clamp is MaxSpeed
			ForceOutput += FMath::Clamp(Acceleration * AccelerationBrakeMult * 0.02f, 0.f, MaxSpeed);
		}
		else
		{
			// Break
			ForceOutput -= FMath::Clamp(Acceleration * AccelerationBrakeMult * 0.02f, 0.f, ForceOutput);
		}
		// Draw the debug line
		// DrawDebugLine(GetWorld(),GetActorLocation(), GetActorLocation() + ForwardVector * (CurrentSpeed * 0.01f),FColor::Red,false,0.1f, 0.f, 1.f);
		// DrawDebugLine(GetWorld(),GetActorLocation(), GetActorLocation() + InertiaVector * (CurrentSpeed * 0.01f),FColor::White,false,0.1f, 0.f, 1.f);
	}

	// Apply final calculation as force
	PhysicsBodyInstance->SetLinearVelocity(ForwardVector + InertiaVector, false);
}

void AEDU_CORE_MobileEntity::Align(float DeltaTime)
{ // FLOW_LOG
	if(bMovesOnSurface && !bIsOnSurface) return;
	
	if(bShouldAlign)
	{
		// Check how fast we are turning: this is measured between frames, so it's actual speed.
		TurnDistance = FMath::Abs(RotationDifference);

		// We need our current situation
		// Torque = PhysicsBodyInstance->GetUnrealWorldAngularVelocityInRadians();
		
		// Deprecated
		{
			// RotationSpeed was used for deprecated functions and Debug 
			RotationSpeed = FMath::Abs(LastRotationDifference - RotationDifference);
			LastRotationDifference = RotationDifference;
		}

		// If we go the lower FPS, we'll cap the delta, else movement will explode.
		// We could fix it to the same as the Async tick, more testing needed.
		float ClampedDeltaTime = FMath::Clamp(DeltaTime, 0.016f, 0.02f);
		
		/*------------------------ Friction ---------------------------------
		  Friction in PhysicalMaterial will kill all rotation if the
		  rotational velocity is below a certain threshold.

		  At a friction rate on 0.7 (UE Standard) the velocity needed to
		  overcome friction is around 37. At 1.0 it is around 40.

		  Since we apply velocity directly, we don't need to take
		  mass into account. The friction hampering movement is nullified
		  while we move, because the movement offset compensates for the
		  lack of rotation, causing us to over steer if we are moving, thus
		  we only compensate at low speeds.

		  To RESET FrictionCompensation, we set both IF and ELSE.
		--------------------------------------------------------------------*/

		float CurrentTorque = FMath::Abs(Torque.Z = (TurnRate * ClampedDeltaTime));
			
		if(bYawTurnsOnly)
		{
			// Direct rotation = less compensation needed.
			if(CurrentSpeed < 1.f && CurrentTorque < 40.f)
			{
				FrictionCompensation = 40.f - CurrentTorque;
			}
			else
			{
				FrictionCompensation = 0.f;
			}
		}
		else
		{
			// We need to apply more torque due to conversion
			if(CurrentSpeed < 1.f && CurrentTorque < 300.f)
			{
				FrictionCompensation = 300.f - CurrentTorque;
			}
			else
			{
				FrictionCompensation = 0.f;
			}
		}

		if (RotationDifference < 0) // Go left
		{
			Torque.Z = (TurnRate * ClampedDeltaTime) - FrictionCompensation;
		}
		else // Go right
		{
			Torque.Z = (TurnRate * ClampedDeltaTime) + FrictionCompensation;
		}

		/*--------------- Altering Rotation with Physics ---------------------
		  SetAngularVelocityInRadians() is a Physics equivalent to
		  AddActorRotation(). It does so by adding a velocity to our current
		  rotation, instead of setting it immediately.

		  When the parameter bool bAddToCurrent is set to false, the new
		  velocity will override the old velocity. In other words, we can
		  set the velocity to 0 once we have reached the rotation we want.

		  By measuring current rotation and velocity, we can effectively
		  use SetAngularVelocity() the same way we use AddActorRotation().
		--------------------------------------------------------------------*/
		
		// TODO: this could be an enum.
		if(bYawTurnsOnly)
		{
			/*---------------------------------------------------------------
			  <!> bYawTurnsOnly will turn the actor on the world Z axis,
			  despite pitch and rotation (certain flying entities).
			---------------------------------------------------------------*/
			if(bShouldReverse)
			{
				// Invert Torque
				PhysicsBodyInstance->SetAngularVelocityInRadians(FMath::DegreesToRadians(-Torque), false);
			}
			else
			{
				// Normal Torque
				PhysicsBodyInstance->SetAngularVelocityInRadians(FMath::DegreesToRadians(Torque), false);
			}
		}
		else
		{
			/*---------------------------------------------------------------
			  <!> This will turn the actor on its local Z axis,
			  respecting pitch and rotation (ground entities).
			---------------------------------------------------------------*/
			
			// Get the actor's world rotation
			FTransform ActorTransform = GetActorTransform();

			// Convert local angular velocity to world space using the actor's rotation
			FVector ConvertedTorque = ActorTransform.TransformVector(Torque);

			if(bShouldReverse)
			{
				// Invert Torque
				PhysicsBodyInstance->SetAngularVelocityInRadians(FMath::DegreesToRadians(-ConvertedTorque), false);
			}
			else
			{
				// Normal Torque
				PhysicsBodyInstance->SetAngularVelocityInRadians(FMath::DegreesToRadians(ConvertedTorque), false);	
			}
		}
	}
}

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

AEDU_CORE_MobileEntity::AEDU_CORE_MobileEntity()
{
	// Disable ticking at the start
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Enable Asynchronous Physics tick
	bAsyncPhysicsTickEnabled = true;
	
	// General variable replication
	bReplicates = true;

	// Always relevant for network (overrides bOnlyRelevantToOwner).
	// bAlwaysRelevant = true;

	// Whether this actor can take damage. Must be true for damage events (e. g. ReceiveDamage()) to be called.
	SetCanBeDamaged(false);
}

void AEDU_CORE_MobileEntity::BeginPlay()
{
	Super::BeginPlay();

	// Server Tick
	if(GetNetMode() != NM_Client)
	{
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToMobileEntityArray(this);
		}
		
		CreateCollisionSphere();
	}

	// Initiate PhysicsBodyInstance pointer
	PhysicsBodyInstance = PhysicsComponent->GetBodyInstance();

	// Initiate PhysicalMaterial
	PhysicalMaterial = PhysicsBodyInstance->GetSimplePhysicalMaterial();
	
	// Set TurnRate
	if(MovementType == EMovementType::CenterRotates)
	{
		TurnRate = MaxRotationRate;
	}
	else
	{
		TurnRate = 0.f;
	}

	LastValidLocation = GetActorLocation();

	// TODO: this might need to be scaled or adjusted when we add components.
	// BoxSize
	GetActorBounds(true, Origin, BoxExtent); // BoxExtent is half the actor's size in 3d space from the center, so it's the bottom.

	// Properties of representation of an 'agent' used by AI navigation/pathfinding.
	NavAgentProperties.AgentRadius = AgentRadius;			
	NavAgentProperties.AgentHeight = AgentHeight;				
	NavAgentProperties.AgentStepHeight = AgentStepHeight;		
	NavAgentProperties.NavWalkingSearchHeightScale = NavWalkingSearchHeightScale;
}

//------------------------------------------------------------------------------
// Waypoint Utility
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::AddWaypoint(
	AEDU_CORE_Waypoint* Waypoint,
	const EEDU_CORE_WaypointType WaypointType,
	const FVector& WaypointLocation,
	const FRotator& WaypointRotation,
	const FVector& WaypointForwardVector,
	const FVector& WaypointRightVector,
	const int32 FormationIndex, const bool Queue)
{ FLOW_LOG
	if(!Queue && WaypointArray.Num() > 0) // Always clear the array if anything is in it and the waypoint is not queued.
	{
		ClearAllWaypoints();
	}
	else if(Queue && WaypointArray.Num() > 0) // An empty array doesn't need queueing.
	{
		// Make sure that we don't exceed the maximum number of waypoints before saving.
		if (WaypointArray.Num() < MaxWaypointCapacity)
		{
			WaypointArray.AddUnique(Waypoint);
			
			// Waypoint is queued, no need to execute it.
			return;
		}
		else // If the waypointArray is full, we'll remove the oldest one.
		{
			WaypointArray[0]->RemoveActorFromWaypoint(this);
			WaypointArray.RemoveAt(0);
		
			WaypointArray.AddUnique(Waypoint);

			// Waypoint is queued, no need to execute it.
			return;
		}
	}

	WaypointArray.AddUnique(Waypoint);
	ExecuteOrders(WaypointType, WaypointLocation, WaypointRotation, WaypointForwardVector, WaypointRightVector, FormationIndex);
}

void AEDU_CORE_MobileEntity::RemoveWaypoint(AEDU_CORE_Waypoint* Waypoint)
{ FLOW_LOG
	bool bRetrieveNewWaypointOrders = false;
	if (WaypointArray.Contains(Waypoint))
	{
		// The waypoint we removed might be the first in the array, in that case we need to retrieve new orders.
		if(Waypoint == WaypointArray[0]) bRetrieveNewWaypointOrders = true;
		
		// Unsubscribe from updates, the waypoint will destroy itself when no one longer listens.
		Waypoint->RemoveActorFromWaypoint(this);
		WaypointArray.Remove(Waypoint);
		FLOW_LOG_WARNING("Removing Waypoint")
	}

	// Now that the WaypointArray is sorted, we can retrieve new orders.
	if(bRetrieveNewWaypointOrders)
	{
		RetrieveWaypointOrders();
	}
}

void AEDU_CORE_MobileEntity::ClearAllWaypoints()
{ FLOW_LOG
	for(AEDU_CORE_Waypoint* Waypoint : WaypointArray)
	{
		if(Waypoint)
		{
			// Unsubscribe from updates, the waypoint will destroy itself when no one longer listens.
			Waypoint->RemoveActorFromWaypoint(this); 
			FLOW_LOG_WARNING("Clearing all Waypoints")
		}
	}
	WaypointArray.Reset();
}

//------------------------------------------------------------------------------
// AI Functionality
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::RetrieveWaypointOrders()
{ FLOW_LOG
	if(WaypointArray.Num() > 0)
	{
		if(WaypointArray[0])
		{
			const EEDU_CORE_WaypointType WaypointType = WaypointArray[0]->GetWaypointType();
			const FVector WaypointLocation = WaypointArray[0]->GetActorLocation();
			const FRotator WaypointRotation = WaypointArray[0]->GetActorRotation();
			const FVector WaypointForwardVector = WaypointArray[0]->GetActorForwardVector();
			const FVector WaypointRightVector = WaypointArray[0]->GetActorRightVector();
			const int32 FormationIndex = WaypointArray[0]->GetFormationIndex(this);
			
			ExecuteOrders(WaypointType, WaypointLocation, WaypointRotation, WaypointForwardVector, WaypointRightVector, FormationIndex);

			// Used for Delays
			SavedFormationIndex = FormationIndex;
		}
	}
}

void AEDU_CORE_MobileEntity::ReviewNavigationQueue()
{ // FLOW_LOG
	if(bShouldEvade)
	{
		bShouldEvade = false;
		return;
	}

	if (NavPointArray.Num() > 0)
	{
		// We have reached the first navpoint, so remove it.
		UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("NavPointArray.RemoveAt(0)"));
		NavPointArray.RemoveAt(0);
		return;
	}
	
	if(WaypointArray.Num() == 1)
	{
		// We have reached final destination, we should park.
		MovementOrder = EMovementOrder::Park;
	}

	if(WaypointArray.Num() == 0)
	{
		// We have arrived
		MovementOrder = EMovementOrder::Park;
		return;
	}

	// We shouldn't get here if this is empty (causing a crash because ReviewNavigationQueue() shouldn't be called unless we know what we are doing.), 
	RemoveWaypoint(WaypointArray[0]);

	if(WaypointArray.Num() > 0)
	{
		RequestPathAsync(GetActorLocation(), FormationLocation);
		//RequestPath(GetActorLocation(), FormationLocation);
	}

}

void AEDU_CORE_MobileEntity::UpdateFormationLocation(const FVector WaypointLocation, const FRotator WaypointRotation, const FVector WaypointForwardVector, const FVector WaypointRightVector, const int32 FormationIndex)
{ FLOW_LOG
	//------------------------------------------------------------------------------------
	// Line Formation
	//------------------------------------------------------------------------------------
	if(FormationIndex == 0) // Alone, leader or the Index is faulty.
	{
		FormationLocation = WaypointLocation;
	}
	else if(FormationIndex % 2 != 0) // Odd number, go right.
	{
		FormationLocation = WaypointLocation + (-WaypointRightVector * FormationSpacing) * FormationIndex;
	}
	else if(FormationIndex % 2 == 0) // Even number, go right.
	{
		FormationLocation = WaypointLocation + (WaypointRightVector * FormationSpacing) * FormationIndex;
	}
	
	/*
	//------------------------------------------------------------------------------------
	// Circular Formation with Multiple Rows
	//------------------------------------------------------------------------------------
	const float InitialCircleRadius = 200.0f; // Radius for the first row
	const float RowSpacing = 200.0f; // Space between rows
	const int EntitiesPerRow = 10; // Number of entities in each row

	// Calculate the current row based on the FormationIndex
	int CurrentRow = FormationIndex / EntitiesPerRow;

	// Calculate the angle for the current entity based on its index within the current row
	int IndexInRow = FormationIndex % EntitiesPerRow; // Get the index within the current row
	float Angle = (IndexInRow / static_cast<float>(EntitiesPerRow)) * 2.0f * PI; // Full circle in radians

	// Calculate the radius for the current row
	float CircleRadius = InitialCircleRadius + (CurrentRow * RowSpacing);

	// Calculate the x and y offsets based on the angle and current row radius
	float XOffset = CircleRadius * cos(Angle); // X offset
	float YOffset = CircleRadius * sin(Angle); // Y offset

	// Set the formation location relative to the waypoint
	FormationLocation = WaypointLocation + FVector(XOffset, YOffset, 0); // Assuming Z is up
	
	//------------------------------------------------------------------------------------
	// Staggered Column
	//------------------------------------------------------------------------------------
	if(FormationIndex == 0) // Alone, leader or the Index is faulty.
	{
		FormationLocation = WaypointLocation;
	}
	else if(FormationIndex % 2 != 0) // Odd number, go back.
	{
		FormationLocation = WaypointLocation + (-WaypointForwardVector * FormationSpacing) * FormationIndex;
		FormationRotation.Yaw =- 90.f;
	}
	else if(FormationIndex % 2 == 0) // Even number, go right.
	{
		FormationLocation = WaypointLocation + (-WaypointForwardVector * FormationSpacing * FormationIndex) + (WaypointRightVector * FormationSpacing);
		FormationRotation.Yaw =+ 90.f;
	}
	*/
}

void AEDU_CORE_MobileEntity::UpdateBatchIndex(const int32 ServerBatchIndex)
{ FLOW_LOG
	BatchIndex = ServerBatchIndex;
}

void AEDU_CORE_MobileEntity::ExecuteOrders(
	const EEDU_CORE_WaypointType WaypointType,
	const FVector& WaypointLocation,
	const FRotator& WaypointRotation,
	const FVector& WaypointForwardVector,
	const FVector& WaypointRightVector,
	int32 FormationIndex)
{ FLOW_LOG
	// The Following information depends on the type of waypoint we read.
	switch (WaypointType)
	{
		case EEDU_CORE_WaypointType::NavigateTo:
		// Activate navigation
			UpdateFormationLocation(WaypointLocation, WaypointRotation, WaypointForwardVector, WaypointRightVector, FormationIndex);

			MovementOrder = EMovementOrder::Navigate;
			FormationRotation = WaypointRotation;

		// Get a path to waypoint
			//RequestPath(GetActorLocation(), FormationLocation);
			RequestPathAsync(GetActorLocation(), FormationLocation);
		break;
				
		case EEDU_CORE_WaypointType::AttackPosition:
			UpdateFormationLocation(WaypointLocation, WaypointRotation, WaypointForwardVector, WaypointRightVector, FormationIndex);
		break;
				
		case EEDU_CORE_WaypointType::ObservePosition:
			MovementOrder = EMovementOrder::Idle;
		break;
	}
}

//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------

bool AEDU_CORE_MobileEntity::OnSurface() const
{ // FLOW_LOG
	FVector TraceStartLocation = GetActorLocation();
	FVector TraceEndLocation = TraceStartLocation + -GetActorUpVector() * BoxExtent.Z;
	
	// Perform the line trace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore the actor itself
	
	// DrawDebugLine(GetWorld(), TraceStartLocation, TraceEndLocation, FColor::Green, false, 0.1f, 0, 1.0f);
	
	// Line trace downwards to se if we stand on something.
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, GroundChannel, QueryParams))
	{
		if(HitResult.bBlockingHit)
		{
			return true;
		}
	}
	return false;
}

void AEDU_CORE_MobileEntity::CheckAlignment()
{ // FLOW_LOG	
	// Check Start Rotation.
	AlignStartRotation = GetActorRotation();
		
	// Check End Rotation depending on what we are doing.
	if(MovementOrder == EMovementOrder::Navigate)
	{
		if(bShouldEvade)
		{
			// UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Aligning with EvadePoint."));
			AlignEndRotation.Yaw = UtilityLibrary::GetRotationToTargetPos(this, EvadePoint).Yaw;
		}
		else if (NavPointArray.Num() > 0)
		{
			// UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Aligning with NavPointArray[0]."));
			AlignEndRotation.Yaw = UtilityLibrary::GetRotationToTargetPos(this,NavPointArray[0]).Yaw;

			// Draw a debug sphere at the location of each path point
			DrawDebugSphere(
				this->GetWorld(),				// World context
				NavPointArray[0],               // Location of the sphere
				30,								// Radius of the sphere
				3,                              // Segments for smoother sphere edges
				FColor::Orange,                 // Color of the sphere
				false,                          // Persistent (will disappear after duration)
				1.f								// Duration in seconds
			);
		}
		else
		{
			// UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Aligning with FormationLocation."));
			AlignEndRotation.Yaw = UtilityLibrary::GetRotationToTargetPos(this,FormationLocation).Yaw;
		}
	}
	else
	{
		// TODO: there might be case where we shouldn't align, simple set bShouldAlign = false in that case.
		
		// We have Reached our destination, now align with waypoint's instruction.
		AlignEndRotation.Yaw = FormationRotation.Yaw;
	}

	// Adjust negative Yaw to the range [0, 360)
	if(AlignStartRotation.Yaw < 0)
	{
		AlignStartRotation.Yaw += 360.0f;  
	}
	if(AlignEndRotation.Yaw < 0)
	{
		AlignEndRotation.Yaw += 360.0f;
	}

	if(MovementType == EMovementType::OmniDirectional
	&& MovementOrder == EMovementOrder::Navigate
	&& Distance < OmniDirectionalAlignDistance)
	{
		bShouldAlign = false;
		return;
	}
	
	// Measure difference to see if we need ot Align, we don't need to be perfect.
	float Difference = FMath::Abs(AlignStartRotation.Yaw - AlignEndRotation.Yaw);
	
	// The Difference is not normalized to 180, it uses 360 as max.
	if(Difference < 1.f || bShouldReverse && Difference < 180.f && Difference > 179.f)
	{
		bShouldAlign = false;
	}
	else
	{
		bShouldAlign = true;
	}
}

void AEDU_CORE_MobileEntity::CheckPosition()
{ // FLOW_LOG
	if(GetActorLocation() != FormationLocation
	&& WaypointArray.Num() == 0)
	{
		MovementOrder = EMovementOrder::Navigate;
	}
}

void AEDU_CORE_MobileEntity::DetermineTurnSpeed()
{
	// We use 0.02f (50FPS) instead of DeltaTime, then scale if because Delta is so small.
	constexpr float FixedDeltaTimeWithScaling = 0.02f * 100.f;
	
	// Pre-compute the scaled rotation rate to avoid repetitive calculations
	float ScaledMaxRotationRate = MaxRotationRate * FixedDeltaTimeWithScaling;

	// Determine TurnRate based on MovementType and bShouldReverse
	if (bShouldReverse)
	{
		// As TurnDistance approaches 180, TurnRate should decrease
		TurnRate = (1.f - (TurnDistance / 180.f)) * ScaledMaxRotationRate;
	}
	else
	{
		// How fast should we turn?
		if (TurnDistance > SlowRotationDistance)
		{
			TurnRate = ScaledMaxRotationRate; // Max rotation rate
		}
		else
		{
			// When within the slow rotation distance, scale down the turn rate based on the distance
			TurnRate = (TurnDistance / SlowRotationDistance) * ScaledMaxRotationRate;
		}
	}
	
	/*-------------------------- FMath::Abs --------------------------------------
	  Absolute Value: The absolute value of a number is its distance from zero
	  on the number line, regardless of direction. In other words, it converts
	  any negative number to its positive counterpart while leaving positive
	  numbers unchanged.
	----------------------------------------------------------------------------*/

	// Calculate how far we need to turn, and which side is fastest.
	RotationDifference = AlignEndRotation.Yaw - AlignStartRotation.Yaw;

	// Normalize DeltaYaw to the range (-180, 180]
	if (RotationDifference > 180.0f)
	{
		RotationDifference -= 360.0f;
	}
	else if (RotationDifference < -180.0f)
	{
		RotationDifference += 360.0f;
	}

	// DeltaTime inflates numbers in the editor, so we scale it.
	if (RotationDifference < 0)
	{
		// Go Left
		TurnRate = -FMath::Abs(TurnRate) * 1000.f;
		if(bShouldEvade)
		{
			bShouldEvadeLeft = true;
		}
	}
	else
	{
		TurnRate = FMath::Abs(TurnRate) * 1000.f;
		if(bShouldEvade)
		{
			bShouldEvadeLeft = false;
		}
	}
}

void AEDU_CORE_MobileEntity::CalculateCurrentSpeed(const FVector& CurrentPos, float DeltaTime)
{
	// Fetch velocity vector once per tick and cache it.
	CurrentSpeedVector = PhysicsBodyInstance->GetUnrealWorldVelocity();
	CurrentSpeed = CurrentSpeedVector.Size();

	// Calculate ActualSpeed on the XY plane without additional allocations.
	ActualSpeed = FVector::Dist2D(CurrentPos, LastPos) / DeltaTime;
	MovementVector = (CurrentPos - LastPos) * 0.5f;

	// Update Last Position without reallocation.
	LastPos = CurrentPos;
}

float AEDU_CORE_MobileEntity::CalculateDistance(const FVector& CurrentPos) const
{
	if (bShouldEvade)
	{
		return FVector::Dist2D(CurrentPos, EvadePoint);
	}
	else if (NavPointArray.Num() > 0)
	{
		return FVector::Dist2D(CurrentPos, NavPointArray[0]);
	}
	else
	{
		return FVector::Dist2D(CurrentPos, FormationLocation);
	}
}

void AEDU_CORE_MobileEntity::CheckReversalConditions()
{
	if (bForceReverse)
	{
		bShouldReverse = true;
	}
	else if (bCanReverse && TurnDistance > ReverseRotationDistance && MaxReverseDistance > Distance)
	{
		bShouldReverse = true;
	}
	else
	{
		bShouldReverse = false;
	}
}

void AEDU_CORE_MobileEntity::HandleNavigation()
{
	// Setting DesiredSpeed based on Distance thresholds.
	if (Distance > SlowDownThreshold)
	{
		DesiredSpeed = MaxSpeed;
	}
	else if (Distance < StopThreshold)
	{
		ReviewNavigationQueue();
	}
	else
	{
		DesiredSpeed = (Distance / SlowDownThreshold) * MaxSpeed;
	}

	// Check if reversal conditions apply
	if(MovementType != EMovementType::OmniDirectional)
	{
		CheckReversalConditions();
	}
}

void AEDU_CORE_MobileEntity::HandleParking()
{
	if (MovementType >= EMovementType::CenterRotates)
	{
		DesiredSpeed = 0;
		bShouldReverse = false;

		if (!bShouldAlign)
		{
			CheckAlignment();
			if (!bShouldAlign)
			{
				MovementOrder = EMovementOrder::Idle;
			}
		}
	}
	else
	{
		if (bShouldAlign)
		{
			DesiredSpeed = ParkingSpeed;
			bShouldReverse = false;
		}
		else
		{
			DesiredSpeed = 0;
			MovementOrder = EMovementOrder::Idle;
		}
	}
}

//------------------------------------------------------------------------------
// Functionality: Collision avoidance
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::CreateCollisionSphere()
{ FLOW_LOG
	// Create a new sphere component
	DynamicCollisionSphere = NewObject<USphereComponent>(this);

	if (DynamicCollisionSphere)
	{
		DynamicCollisionSphere->InitSphereRadius(CollisionDetectionVolumeRadius); // Set sphere radius based on the bounds
		DynamicCollisionSphere->SetWorldLocation(GetActorLocation()); // Set the sphere's location at the actor's origin
		DynamicCollisionSphere->RegisterComponent(); // Register component to make it active in the game world

		// Attach the sphere to the root component of the actor
		DynamicCollisionSphere->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

		// Set the collision object type to Custom for trace channels
		DynamicCollisionSphere->SetCollisionObjectType(CollisionDetectionVolumeChannel);

		// Set specific collision responses for trace channels (Example: Custom Trace Channel)
		DynamicCollisionSphere->SetCollisionResponseToChannel(CollisionDetectionVolumeChannel, ECR_Block);
		
		this->AddInstanceComponent(DynamicCollisionSphere); // Add the sphere as an instance component to the actor, tied to its lifecycle.
	}
}

void AEDU_CORE_MobileEntity::UpdateCollisionSphere(const FVector& CollisionSpherePosition) const
{ 
	if(DynamicCollisionSphere == nullptr) return;
	
	DynamicCollisionSphere->SetWorldLocation(CollisionSpherePosition);
	
	// Draw the debug sphere in the world at the actor's bounds origin
	DrawDebugSphere(GetWorld(), CollisionSpherePosition, CollisionDetectionVolumeRadius, 4, FColor::Blue, false, 0.1f, 0, 1.0f);
}

bool AEDU_CORE_MobileEntity::PathIsClear()
{
	FVector ForwardVector;
	if(MovementType == EMovementType::OmniDirectional)
	{
		ForwardVector = AlignEndRotation.Vector();
	}
	else
	{
		ForwardVector = GetActorForwardVector();
	}
	
	FVector SideVector = bShouldEvadeLeft ? GetActorRightVector() : -GetActorRightVector();
	FVector TraceDirection = bShouldReverse ? -ForwardVector : ForwardVector;
	FVector TraceStartLocation = GetActorLocation() + TraceDirection * CollisionDetectionStartOffset;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// First check: Trace directly forward (or backward if reversing)
	if (IsPathClear(TraceStartLocation, TraceStartLocation + TraceDirection * ActualSpeed * CollisionTraceMult, HitResult, QueryParams))
	{
		return true; // If clear, no need for evasion
	}

	// Helper lambda for side traces
	auto CheckSide = [&](const FVector& Direction, float Distance) -> bool
	{
		FVector TraceEndLocation = TraceStartLocation + Direction * Distance * CollisionTraceMult;
		if (IsPathClear(TraceStartLocation, TraceEndLocation, HitResult, QueryParams))
		{
			EvadePoint = TraceEndLocation;
			bShouldEvade = true;
			return true; // Path is clear
		}
		return false; // Path is blocked
	};

	// Array of directions for both sides (priority and non-priority)
	TArray<FVector> Directions = {
		TraceDirection + SideVector * 0.5f,   // 22.5 degrees to priority side
		TraceDirection + SideVector,          // 45 degrees to priority side
		SideVector,                           // 90 degrees to priority side
		TraceDirection - SideVector * 0.5f,   // 22.5 degrees to non-priority side
		TraceDirection - SideVector,          // 45 degrees to non-priority side
		-SideVector                           // 90 degrees to non-priority side
	};

	// Loop over all side directions
	for (const FVector& Direction : Directions)
	{
		if (CheckSide(Direction, ActualSpeed))
		{
			return true; // Early exit if any side is clear
		}
	}

	// Start Checking Backwards
	TraceDirection = -ForwardVector; // Reverse the direction
	TraceStartLocation = GetActorLocation() + TraceDirection * CollisionDetectionStartOffset;

	// Array of backward directions
	TArray<FVector> BackwardDirections = {
		TraceDirection - SideVector, // 45 degrees left
		TraceDirection + SideVector, // 45 degrees right
		TraceDirection               // Directly backward
	};

	// Check backward directions
	for (const FVector& Direction : BackwardDirections)
	{
		if (CheckSide(Direction, StopThreshold))
		{
			return true; // Early exit if any backward direction is clear
		}
	}

	// No path is clear, return false
	FLOW_LOG_WARNING("All directions false, need navigation.")
	bShouldEvade = false;
	return false;
}

bool AEDU_CORE_MobileEntity::IsPathClear(const FVector& TraceStartLocation, const FVector& TraceEndLocation, FHitResult& HitResult, const FCollisionQueryParams& QueryParams) const
{
	// Perform line trace and check for hits
	if (GetWorld()->SweepSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, FQuat::Identity, CollisionDetectionVolumeChannel, FCollisionShape::MakeSphere(CollisionDetectionVolumeRadius), QueryParams))
	{
		if(HitResult.bBlockingHit)
		{
			// Perform the line trace
			// DrawDebugSphere(GetWorld(), TraceEndLocation, CollisionDetectionVolumeRadius, 12, FColor::Red, false, 0.1f);    // End position
			return false;
		}
	}

	// Visualize the trace start, end, and the path between them
	// DrawDebugLine(GetWorld(), TraceStartLocation, TraceEndLocation, FColor::Blue, false, 0.05f, 0, 0.1f); // Path of the trace

	// Perform the line trace
	DrawDebugSphere(GetWorld(), TraceEndLocation, CollisionDetectionVolumeRadius, 12, FColor::Green, false, 0.05f); // End position
	return true;
}

//------------------------------------------------------------------------------
// Functionality: Navigation
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::RequestPath(const FVector& StartPos, const FVector& EndPos)
{ FLOW_LOG

	/*------------------------------------------------------------------------------
	  FindPathToLocationSynchronously is not thread safe, and will wreak havoc on
	  shared pointers if it s called on a worker thread, so make sure we are
	  running on the main thread, and restart otherwise.
	------------------------------------------------------------------------------*/
	if (!IsInGameThread())
	{
		// If not, queue the function to run on the main thread
		AsyncTask(ENamedThreads::GameThread, [this, StartPos, EndPos]()
		{
			RequestPath(StartPos, EndPos);
			UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("RequestPath restarted on Main thread."));
		});
		return;
	}
	
	// Get a reference to the navigation system
	if(!NavSystem)
	{
		NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	}

	if(TObjectPtr<UNavigationPath> NavPath = NavSystem->FindPathToLocationSynchronously(GetWorld(), StartPos, EndPos))
	{
		if(NavPath->PathPoints.Num() < 3)
		{
			/*---------------------------------------------------------------------
			  The first NavPoint always our current position, and the last
			  NavPoint is always at the Waypoint or FormationPosition.

			  We only need the NavPoints in the middle, EI: the third or more.
			---------------------------------------------------------------------*/
			return;
		}

		// Reset and fill the NavPointArray
		NavPointArray.Reset();
		NavPointArray = NavPath->PathPoints;
		
		// Clean up the array
		NavPointArray.RemoveAt(0);
		NavPointArray.RemoveAt(NavPointArray.Num() - 1);

		// Loop through each point in the path and draw a debug sphere
		#if WITH_EDITOR
		for (const FVector& Point : NavPointArray)
		{
			// Draw a debug sphere at the location of each path point
			DrawDebugSphere(
				this->GetWorld(),				 // World context
				Point,                           // Location of the sphere
				25,								 // Radius of the sphere
				4,                               // Segments for smoother sphere edges
				FColor::White,                   // Color of the sphere
				false,                           // Persistent (will disappear after duration)
				3.f								 // Duration in seconds
			);
		}
		#endif
	}
	else
	{
		UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("Failed to find a valid path."));
	}
}

void AEDU_CORE_MobileEntity::RequestPathAsync(const FVector& StartPos, const FVector& EndPos)
{ FLOW_LOG
	
	if (!IsInGameThread())
	{
		// If not, queue the function to run on the main thread
		AsyncTask(ENamedThreads::GameThread, [this, StartPos, EndPos]()
		{
			RequestPathAsync(StartPos, EndPos);
			UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("RequestPath restarted on Main thread."));
		});
		return;
	}
	
	// Get a reference to the navigation system
	if(!NavSystem)
	{
		NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	}

	const ANavigationData* NavData = Cast<ANavigationData>(NavSystem->GetMainNavData());

	// Initialize the pathfinding query with required parameters
	FPathFindingQuery PathQuery(
		this,									// Owner (typically 'this' if within an actor or controller class)
		*NavData,								// Reference to valid ANavigationData
		StartPos,								// Start position
		EndPos,									// End position
		nullptr,								// Optional query filter (nullptr means default)
		nullptr,								// Optional path instance to fill (nullptr means new path)
		TNumericLimits<FVector::FReal>::Max(),	// Cost limit, using default max
		true									// Require navigable end location
	);

	FNavAgentProperties NavAgentPropertiess;
	
	// Use the PathQuery for pathfinding, e.g., async pathfinding:
	NavSystem->FindPathAsync(
		NavAgentPropertiess,
		PathQuery,
		FNavPathQueryDelegate::CreateUObject(this, &ThisClass::OnRequestPathAsyncComplete));

}

void AEDU_CORE_MobileEntity::OnRequestPathAsyncComplete(uint32 RequestID, ENavigationQueryResult::Type Result, FNavPathSharedPtr Path)
{ FLOW_LOG
	if (Result == ENavigationQueryResult::Success && Path.IsValid())
	{
		UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Pathfinding succeeded with RequestID: %d"), RequestID);

		// Retrieve the path points
		const TArray<FNavPathPoint>& PathPoints = Path->GetPathPoints();

		if(PathPoints.Num() > 2)
		{
			/*---------------------------------------------------------------------
			  The first NavPoint always our current position, and the last
			  NavPoint is always at the Waypoint or FormationPosition.

			  We only need the NavPoints in the middle, EI: the third or more.
			---------------------------------------------------------------------*/
			NavPointArray.Reset();
			
			for (int32 Point = 1; Point < PathPoints.Num() - 1; ++Point)
			{
				FVector PointLocation = PathPoints[Point].Location;
				NavPointArray.Add(PointLocation);
			}
			
			// Loop through each point in the path and draw a debug sphere
			#if WITH_EDITOR
			for (const FVector& Point : NavPointArray)
			{
				// Draw a debug sphere at the location of each path point
				DrawDebugSphere(
					this->GetWorld(),				 // World context
					Point,                           // Location of the sphere
					25,								 // Radius of the sphere
					4,                               // Segments for smoother sphere edges
					FColor::White,                   // Color of the sphere
					false,                           // Persistent (will disappear after duration)
					3.f								 // Duration in seconds
				);
			}
			#endif
		}
	}
	else
	{
		UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Pathfinding failed with RequestID: %d"), RequestID);
	}
}
