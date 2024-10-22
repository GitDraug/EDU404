// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/EDU_CORE_MobileEntity.h"
#include "AI/WayPoints/EDU_CORE_Waypoint.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

#include "Components/SphereComponent.h"
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

void AEDU_CORE_MobileEntity::ServerMobileCalc(float DeltaTime)
{
	CurrentSpeedVector = PhysicsBodyInstance->GetUnrealWorldVelocity();
	CurrentSpeed = CurrentSpeedVector.Size();

	ActualSpeed = FVector::Dist2D(GetActorLocation(), OldPos) * 1/DeltaTime;
	OldPos = GetActorLocation();
	
	switch(MovementOrder)
	{
		case EMovementOrder::Navigate:
			if(DeltaTimer > 0.1f)
			{
			// Check distance
				//float Distance = FVector::Dist(ActorPos, TargetNavPos); // 3D version
				
				float Distance;
				if(bShouldEvade)
				{
					Distance = FVector::Dist2D(GetActorLocation(), EvadePoint);
				}
				else
				{
					Distance = FVector::Dist2D(GetActorLocation(), FormationLocation);
				}
				
				//GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
				//FString::Printf(TEXT("Distance: %f"),Distance));
				
			// Desired Speed				
				if(!bPathIsClear)
				{
					// All directions are blocked, stop!
					// return;
				}
				
				if(Distance > SlowDownThreshold) // Go as fast as possible
				{
					DesiredSpeed = MaxSpeed;
					//DesiredSpeed = MaxSpeed - (FMath::Abs(RotationDifference) * (MaxSpeed / 200.f));
				}
				else if(Distance < StopThreshold) // Check if there are other waypoints after this one, or if we should stop.
				{
					ReviewNavigationQueue();
				}
				else // Adjust speed to match distance.
				{
					DesiredSpeed = (Distance / SlowDownThreshold) * MaxSpeed;
				}

			// Should we reverse?
				if(bForceReverse || bCanReverse	&& (TurnDistance > ReverseRotationDistance && (MaxReverseDistance > Distance)))  
				{
					bShouldReverse = true;
				}
				else
				{
					bShouldReverse = false;
				}
			}
		break;
			
		case EMovementOrder::Park:
			if(MovementType == EMovementType::CanCenterRotate)
			{
				DesiredSpeed = 0;
				bShouldReverse = false;
				
				// We might be aligned with the navpoint, but not formation.
				if(!bShouldAlign)
				{
					CheckAlignment(); // Double check.
					if(!bShouldAlign)
					{
						MovementOrder = EMovementOrder::Idle;
					}
				}
			}
			else
			{
				// Max turn
				// TurnRate = (EngineOutput / MaxEngineOutput ) * ParkingTurnRate;
				
				// Wait until we are aligned
				if(bShouldAlign)
				{
					DesiredSpeed = ParkingSpeed;
				}
				else
				{
					DesiredSpeed = 0;
				}
			}
		break;
		
		default: ;
	}
}

void AEDU_CORE_MobileEntity::ServerMobilesExec(float DeltaTime)
{
	// GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	// FString::Printf(TEXT("CurrentSpeed: %f"),CurrentSpeed));

	// GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	// FString::Printf(TEXT("ActualSpeed: %f"),ActualSpeed));
	
	/*/ Debug
	{
	  GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, 
	  FString::Printf(TEXT("DeltaTimer: %f"), DeltaTimer));
	
	  GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, 
	  FString::Printf(TEXT("EvadePoint.X: %f"), EvadePoint.X));
	
	  GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, 
  	  FString::Printf(TEXT("EvadePoint.Y: %f"), EvadePoint.Y));

	  GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, 
	  FString::Printf(TEXT("EvadePoint.Z: %f"), EvadePoint.Z));
	
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("bCheckAlignment: %d"),bCheckAlignment));
		
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("bShouldReverse: %d"),bShouldReverse));
		
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("bShouldAlign: %d"),bShouldAlign));
		
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("MovementOrder == EMovementOrder::Navigate: %d"),MovementOrder == EMovementOrder::Navigate));
	
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("MovementOrder == EMovementOrder::Park: %d"),MovementOrder == EMovementOrder::Park));
	
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("-------------------------")));

		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, 
		FString::Printf(TEXT("LastDeltaTime: %f"),LastDeltaTime));

		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, 
		FString::Printf(TEXT("RotationDifference: %f"), RotationDifference));
		
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green,
		FString::Printf(TEXT("TurnDistance: %f"), TurnDistance));
		
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("Torque.Z: %f"), Torque.Z));
		
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("Torque.X: %f"), Torque.X));

		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("Torque.Y: %f"), Torque.Y));

		// GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, 
		// FString::Printf(TEXT("Torque %f"),Torque.Z));
				
		// GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, 
		// FString::Printf(TEXT("TurnFrictionCompensation %f"),FrictionCompensation));
			
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, 
		FString::Printf(TEXT("RotationSpeed: %f"), RotationSpeed / DeltaTime));

		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, 
		FString::Printf(TEXT("-------------------------")));
	
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
		FString::Printf(TEXT("DesiredSpeed: %f"),DesiredSpeed));


		
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
		FString::Printf(TEXT("ForceOutput: %f"),ForceOutput));
	} // */

	if(CollisionTimer > 0.2f && CurrentSpeed > 0.1f)
	{
		bPathIsClear = PathIsClear();
		if(MovementOrder != EMovementOrder::Idle)
		{
			// Check if we should be somewhere.
		}
	}
	
	// Check if we are on a surface or free fall.
	if(DeltaTimer > 0.1f)
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

	// Surface Magnetism (Grip)
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
	
	if(MovementOrder == EMovementOrder::Navigate || MovementOrder == EMovementOrder::Park)
	{
		AdjustSpeed(DeltaTime);
	}

	// Align to the current target if needed.
	if(bShouldAlign)
	{
		if(MovementType == EMovementType::BiDirectional)
		{
			Align(DeltaTime);
			AdjustSpeed(DeltaTime);
		}
		else
		{
			Align(DeltaTime);
		}
	}
	
	// We want to continuously check alignment, because we might be affected by outside forces.
	if(bShouldAlign || MovementOrder == EMovementOrder::Park || MovementOrder == EMovementOrder::Navigate) // && DeltaTimer > 0.1f)
	{
		bCheckAlignment = true;
	}
	else
	{
		bCheckAlignment = false;
	}

	// Check alignment between intervals to see if we need to align.  
	if(bCheckAlignment)
	{
		CheckAlignment();
	}

	// Timers
	if(DeltaTimer > 0.11f)
	{
		DeltaTimer = 0.f;
	}
	DeltaTimer += DeltaTime;

	if(CollisionTimer > 0.21)
	{
		CollisionTimer = 0.f;
	}
	CollisionTimer += DeltaTime;
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

void AEDU_CORE_MobileEntity::AdjustSpeed(float DeltaTime) // We Use 0.02f (50FPS) instead of DeltaTime!
{ // FLOW_LOG
	if(bMovesOnSurface && !bIsOnSurface) return;

	// We divide our movement vector between ForwardVector and our InertiaVector to control drift deterministically.
	FVector ForwardVector = GetActorForwardVector() * ForceOutput * (1.f - Inertia);
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
	if(MovementType == EMovementType::CanCenterRotate)
	{
		TurnRate = MaxRotationRate;
	}
	else
	{
		TurnRate = 0.f;
	}

	LastValidLocation = GetActorLocation();
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
// Server Functionality
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
{ FLOW_LOG
	if(bShouldEvade)
	{
		bShouldEvade = false;
		return;
	}

	if(WaypointArray.Num() == 1)
	{
		// We have reached final destination, we should park.
		MovementOrder = EMovementOrder::Park;
	}

	// We shouldn't get here if this is empty (causing a crash because ReviewNavigationQueue() shouldn't be called unless we know what we are doing.), 
	RemoveWaypoint(WaypointArray[0]);
}

void AEDU_CORE_MobileEntity::UpdateFormationLocation(const FVector WaypointLocation, const FRotator WaypointRotation, const FVector WaypointForwardVector, const FVector WaypointRightVector, const int32 FormationIndex)
{ FLOW_LOG
	// Simple Line
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
		break;
				
		case EEDU_CORE_WaypointType::AttackPosition:
			UpdateFormationLocation(WaypointLocation, WaypointRotation, WaypointForwardVector, WaypointRightVector, FormationIndex);
		break;
				
		case EEDU_CORE_WaypointType::ObservePosition:
			MovementOrder = EMovementOrder::Idle;
		break;
	}
	
	bCheckAlignment = true;
}

//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------

bool AEDU_CORE_MobileEntity::OnSurface() const
{
	FVector Origin, BoxExtent;
	GetActorBounds(true, Origin, BoxExtent); // BoxExtent is half the actor's size in 3d space from the center, so it's the bottom.

	FVector TraceStartLocation = GetActorLocation();
	FVector TraceEndLocation = TraceStartLocation + -GetActorUpVector() * (BoxExtent.Z * 1.05f); // TODO Check how far this reaches
	
	// Perform the line trace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore the actor itself
	
	DrawDebugLine(GetWorld(), TraceStartLocation, TraceEndLocation, FColor::Green, false, 0.1f, 0, 1.0f);

	//TODO: Make ECC_Visibility a custom channel for that which can be moved upon.
	// Line trace downwards to se if we stand on something.
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, ECC_Visibility, QueryParams))
	{
		if(HitResult.bBlockingHit)
		{
			return true;
		}
	}
	return false;
}

void AEDU_CORE_MobileEntity::CheckAlignment()
{
	// Check Start Rotation.
	AlignStartRotation = GetActorRotation();
		
	// Check End Rotation depending on what we are doing.
	if(MovementOrder == EMovementOrder::Navigate)
	{
		if(bShouldEvade)
		{
			AlignEndRotation.Yaw = GetRotationToTargetPos(EvadePoint).Yaw;
		}
		else
		{
			AlignEndRotation.Yaw = GetRotationToTargetPos(FormationLocation).Yaw;
		}
	}
	else
	{
		// We have Reached our destination, now align with waypoint's instruction.
		AlignEndRotation.Yaw = FormationRotation.Yaw;
	}

	// Adjust negative Yaw to the range [0, 360)
	if (AlignStartRotation.Yaw < 0)
	{
		AlignStartRotation.Yaw += 360.0f;  
	}
	if(AlignEndRotation.Yaw < 0)
	{
		AlignEndRotation.Yaw += 360.0f;
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

void AEDU_CORE_MobileEntity::DetermineTurnSpeed()
{
	// We use 0.02f (50FPS) instead of DeltaTime, then scale if because Delta is so small.
	float FixedDeltaTimeWithScaling = 0.02f * 100.f;
	
	if(MovementType == EMovementType::BiDirectional)
	{
		// TurnRate is dependent on speed for entities that can't CenterRotate.
		TurnRate = (CurrentSpeed / MaxSpeed) * MaxRotationRate;
	}
	else
	{
		if(bShouldReverse)
		{
			// As TurnDistance approaches 180, TurnRate should decrease
			TurnRate = (1.f - (TurnDistance / 180.f)) * MaxRotationRate * FixedDeltaTimeWithScaling;
		}
		else
		{
			// How fast should we turn?
			if(TurnDistance > SlowRotationDistance)
			{
			
				TurnRate = MaxRotationRate * FixedDeltaTimeWithScaling;
			}
			else
			{
				// When within the slow rotation distance, scale down the turn rate based on the distance.
				TurnRate = ((TurnDistance/SlowRotationDistance) * MaxRotationRate * FixedDeltaTimeWithScaling);	
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
}

FRotator AEDU_CORE_MobileEntity::GetRotationToTargetPos(const FVector& TargetPos) const
{ //  FLOW_LOG
	// This is crazy fast, but will not return the shortest route.
	FVector LocalDirectionToTarget = TargetPos - this->GetActorLocation();
	return LocalDirectionToTarget.Rotation();
}

FRotator AEDU_CORE_MobileEntity::GetRotationFromTargetPos(const FVector& TargetPos) const
{ //  FLOW_LOG
	// This is crazy fast, but will not return the shortest route.
	FVector LocalDirectionToTarget = this->GetActorLocation() - TargetPos;
	return LocalDirectionToTarget.Rotation();
}

FRotator AEDU_CORE_MobileEntity::GetRotationToTargetActor(const AActor* TargetActor) const
{ FLOW_LOG
	// This is crazy fast, but will not return the shortest route.
	FVector LocalDirectionToTarget = TargetActor->GetActorLocation() - this->GetActorLocation();
	return LocalDirectionToTarget.Rotation();
}

FRotator AEDU_CORE_MobileEntity::GetRotationFromTargetActor(const AActor* TargetActor) const
{ FLOW_LOG
	// This is crazy fast, but will not return the shortest route.
	FVector LocalDirectionToTarget = this->GetActorLocation() - TargetActor->GetActorLocation();
	return LocalDirectionToTarget.Rotation();
}

//------------------------------------------------------------------------------
// Functionality: Collision avoidance
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::CreateCollisionSphere()
{ FLOW_LOG
	// Get the bounds of the actor
	FVector Origin;
	FVector BoxExtent;
	GetActorBounds(false, Origin, BoxExtent);

	// Find the largest extent to use as the radius of the sphere
	float SphereRadius = FMath::Max3(BoxExtent.X, BoxExtent.Y, BoxExtent.Z);
	
	// Create a new sphere component
	DynamicCollisionSphere = NewObject<USphereComponent>(this);

	if (DynamicCollisionSphere)
	{
		DynamicCollisionSphere->InitSphereRadius(SphereRadius); // Set sphere radius based on the bounds
		DynamicCollisionSphere->SetWorldLocation(Origin);       // Set the sphere's location at the actor's origin
		DynamicCollisionSphere->SetCollisionProfileName(TEXT("OverlapAll")); // Set appropriate collision profile
		DynamicCollisionSphere->RegisterComponent();            // Register component to make it active in the game world

		// Attach the sphere to the root component of the actor
		DynamicCollisionSphere->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

		// Set the collision object type to Custom for trace channels
		DynamicCollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);

		// Set specific collision responses for trace channels (Example: Custom Trace Channel)
		DynamicCollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		
		this->AddInstanceComponent(DynamicCollisionSphere); // Add the sphere as an instance component to the actor, tied to its lifecycle.
	}
}

void AEDU_CORE_MobileEntity::UpdateCollisionSphere(const FVector& CollisionSpherePosition) const
{
	DynamicCollisionSphere->SetWorldLocation(CollisionSpherePosition);
	
	// Draw the debug sphere in the world at the actor's bounds origin
	DrawDebugSphere(GetWorld(), CollisionSpherePosition, CollisionDetectionVolumeRadius, 4, FColor::Blue, false, 0.1f, 0, 1.0f);
}

bool AEDU_CORE_MobileEntity::PathIsClear()
{
	FVector ForwardVector = GetActorForwardVector();
	
    // Determine which side to evade based on the flag bShouldEvadeLeft
    FVector SideVector = bShouldEvadeLeft ? GetActorRightVector() : -GetActorRightVector();

    // If reversing, trace in the opposite of the forward direction
    FVector TraceDirection = bShouldReverse ? -ForwardVector : ForwardVector;

    // Set the start location of the trace slightly ahead (or behind if reversing) of the actor
    FVector TraceStartLocation = GetActorLocation() + TraceDirection * 35.f;

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    // First check: Trace directly forward (or backward if reversing)
    if (IsPathClear(TraceStartLocation, TraceStartLocation + TraceDirection * ActualSpeed, HitResult, QueryParams))
    {
    	if(bShouldReverse)
    	{
    		// FLOW_LOG_WARNING("Checking direction backward");
    	}
	    else
	    {
	    	// FLOW_LOG_WARNING("Checking direction forward");
	    }
    	UpdateCollisionSphere(TraceStartLocation + TraceDirection * ActualSpeed);
    	return true; // If clear, no need for evasion
    }


    /*------------------------------------- Check Sides ---------------------------------------------------
      Helper lambda function to reduce code duplication when performing traces.
      Checks if the path in the given direction is clear, and sets an EvadePoint if successful.
    -----------------------------------------------------------------------------------------------------*/
    auto CheckSide = [&](const FVector& Direction, float Distance)
    {
        // Calculate the final end location for the trace
        FVector TraceEndLocation = TraceStartLocation + Direction * Distance;

        // Perform a line trace in the calculated direction
        if(IsPathClear(TraceStartLocation, TraceEndLocation, HitResult, QueryParams))
        {
            // Set evade point based on the direction that's clear
            EvadePoint = TraceEndLocation;
        	UpdateCollisionSphere(EvadePoint);
            bShouldEvade = true;
            return true; // Path is clear
        }
        return false; // Path is blocked
    };

    // Check at multiple angles and distances for evasion, starting with the priority side.

    // Check 22.5 degrees to the priority side
    if (CheckSide(TraceDirection + (SideVector * 0.5f), ActualSpeed))
    {
    	// FLOW_LOG_WARNING("Check 22.5 degrees to the priority side")
    	return true;
    }

    // Check 45 degrees to the priority side
    if (CheckSide(TraceDirection + SideVector, ActualSpeed))
    {
    	// FLOW_LOG_WARNING("Check 45 degrees to the priority side")
		return true;
    }
	
    // Check 90 degrees (completely to the side) on the priority side
    if (CheckSide(SideVector, ActualSpeed))
    {
    	// FLOW_LOG_WARNING("Check 90 degrees to the priority side")
		return true;
    }

    // Check 22.5 degrees to the non-priority side
    if (CheckSide(TraceDirection + (-SideVector * 0.5f), ActualSpeed))
    {
	    // FLOW_LOG_WARNING("Check 22.5 degrees to the non-priority side")
		return true;
    }

    // Check 45 degrees to the non-priority side
    if (CheckSide(TraceDirection + -SideVector, ActualSpeed))
	{
		// FLOW_LOG_WARNING("Check 45 degrees to the non-priority side")
		return true;
	}

    // Check 90 degrees (completely to the side) on the non-priority side
    if (CheckSide(-SideVector, ActualSpeed))
    {
		// FLOW_LOG_WARNING("Check 90 degrees to the non-priority side")
		return true;
	}

    /*----------------------------------- End Lambda ----------------------------------------------------*/

    // Final check: Trace directly backward (for a retreat or reverse evasion)
    TraceDirection = -ForwardVector;
	TraceStartLocation = GetActorLocation() + TraceDirection * 35.f;
	if (IsPathClear(TraceStartLocation, TraceStartLocation + TraceDirection * ActualSpeed, HitResult, QueryParams))
    {
        // If clear, set the EvadePoint directly behind the actor
		// FLOW_LOG_WARNING("Check directly backward")
        EvadePoint = TraceStartLocation + TraceDirection * StopThreshold;
		UpdateCollisionSphere(EvadePoint);
        bShouldEvade = true;
        return true;
    }

    // If no path was clear, return false (no evasion possible)
	bShouldEvade = false;
	// FLOW_LOG_WARNING("No path found")
    return false;
}

bool AEDU_CORE_MobileEntity::IsPathClear(const FVector& TraceStartLocation, const FVector& TraceEndLocation, FHitResult& HitResult, const FCollisionQueryParams& QueryParams) const
{
	float Size = 35.f;

	// Visualize the trace start, end, and the path between them
	DrawDebugLine(GetWorld(), TraceStartLocation, TraceEndLocation, FColor::Blue, false, 0.05f, 0, 0.1f);     // Path of the trace

	// Perform line trace and check for hits
	if (GetWorld()->SweepSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, FQuat::Identity, ECC_WorldDynamic, FCollisionShape::MakeSphere(Size), QueryParams))
	{
		if(HitResult.bBlockingHit)
		{
			// Perform the line trace
			DrawDebugSphere(GetWorld(), TraceEndLocation, Size, 12, FColor::Red, false, 0.1f);    // End position
			return false;
		}
	}

	// Perform the line trace
	DrawDebugSphere(GetWorld(), TraceEndLocation, Size, 12, FColor::Green, false, 0.05f);    // End position
	return true;
}
