﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/EDU_CORE_MobileEntity.h"

// CORE
#include "EDU_CORE/Public/Entities/Waypoints/EDU_CORE_Waypoint.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "FunctionLibrary/UtilityLibrary.h"

// Navigation
#include "NavigationPath.h"
#include "NavigationSystem.h" // PublicDependencyModule: "NavigationSystem"

// Physics
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
// Construction & Init
//------------------------------------------------------------------------------

AEDU_CORE_MobileEntity::AEDU_CORE_MobileEntity(const FObjectInitializer& ObjectInitializer) : Super (ObjectInitializer)
{ FLOW_LOG
	
	// Disable ticking at the start
	PrimaryActorTick.bCanEverTick = false;
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
{ FLOW_LOG
	Super::BeginPlay();

	// Server Tick
	if(GetNetMode() != NM_Client)
	{
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToMobileEntityArray(this);
		}
		
		// CreateCollisionSphere();
	}

	// Initiate PhysicsBodyInstance pointer
	PhysicsBodyInstance = PhysicsComponent->GetBodyInstance();

	// Initiate PhysicalMaterial
	PhysicalMaterial = PhysicsBodyInstance->GetSimplePhysicalMaterial();

	LastValidLocation = GetActorLocation();
	
	// Properties of representation of an 'agent' used by AI navigation/pathfinding.
	NavAgentProperties.AgentRadius = AgentRadius;			
	NavAgentProperties.AgentHeight = AgentHeight;				
	NavAgentProperties.AgentStepHeight = AgentStepHeight;		
	NavAgentProperties.NavWalkingSearchHeightScale = NavWalkingSearchHeightScale;

	BoxExtent = RootComponent->Bounds.GetBox().GetExtent();
	
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::ServerMobileBatchedCalc()
{
	if(ActualSpeed != 0.f)
	{
		bPathIsClear = PathIsClear();	
	}
}

void AEDU_CORE_MobileEntity::ServerMobileCalc(float DeltaTime, int32 CurrentBatchIndex)
{
	// Caching Current Position once per tick to avoid redundant calls.
	const FVector& CurrentPos = GetActorLocation();
	// Caching Current Rotation once per tick to avoid redundant calls.
	const FRotator& CurrentRotation = GetActorRotation();
	
	Distance = CalculateDistance(CurrentPos); // Pass position to avoid creating another FVector
	
	CalculateCurrentSpeed(CurrentPos, DeltaTime); // Pass position to avoid redundant calls
	switch (MovementOrder)
	{
		case EMovementOrder::MoveTo:
			// We don't want to runt his too often.
			if(CurrentBatchIndex == BatchIndex) HandleNavigation();
		break;

		case EMovementOrder::Aim:
			 // HandleAiming(CurrentRotation);
		break;
		
		case EMovementOrder::Park:
			HandleParking(CurrentRotation);
		break;
				
		default:
	break;
	}

	//----------------------------------------------------------------------------------------------------
	// Check if we are on a surface or free fall.
	//----------------------------------------------------------------------------------------------------
	if(CurrentBatchIndex == BatchIndex)
	{
		bIsOnSurface = OnSurface(CurrentPos);
		if(bIsOnSurface)
		{
			LastValidLocationTimer++;
			{
				if (LastValidLocationTimer > 100
					&& CurrentRotation.Pitch <= 50.f
					&& CurrentRotation.Roll <= 50.f)
				{
					LastValidLocation = CurrentPos;
					LastValidLocationTimer = 0;
				}
			}

		}
		else
		{
			if(CurrentSpeedVector.Z < -1000.f && (CurrentPos.Z < 0.f))
			{
				AsyncTask(ENamedThreads::GameThread, [this]()
				{
					// Ground should never be below 0.
					SetActorLocation(LastValidLocation, false, nullptr, ETeleportType::ResetPhysics);
					PhysicsBodyInstance->SetLinearVelocity(FVector::ZeroVector, false);
				});
			}
		}
	}

	//----------------------------------------------------------------------------------------------------
	// We want to continuously check alignment, because we might be affected by outside forces.
	//----------------------------------------------------------------------------------------------------
	if(bShouldAlign
	|| MovementOrder == EMovementOrder::Park
	|| MovementOrder == EMovementOrder::MoveTo)
	{
		CheckAlignment(CurrentRotation);
	}

	//----------------------------------------------------------------------------------------------------
	// Check if we've been bumped
	//----------------------------------------------------------------------------------------------------	
	DeltaTimer += DeltaTime;
	if(DeltaTimer > 0.5f
	&& CurrentBatchIndex == BatchIndex
	&& CurrentSpeed > 0.f
	&& Distance > StopThreshold
	&& MovementOrder == EMovementOrder::Idle)
	{
		CheckAlignment(CurrentRotation);
		CheckPosition(CurrentPos);
		
		DeltaTimer = 0.f;
	}
}

void AEDU_CORE_MobileEntity::ServerMobileExec(float DeltaTime, int32 CurrentBatchIndex)
{
	// Debug
	#define DEBUGMESSAGE GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds
	{	//*/

	//----------------------------------------------------------------------------------------------------------
	// Timers
	//----------------------------------------------------------------------------------------------------------
	//	DEBUGMESSAGE, FColor::Orange,	FString::Printf(TEXT("DebugTimer: %f"),debug_DebugTimer));
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("Distance: %f"), Distance));
	//----------------------------------------------------------------------------------------------------------
	// State
	//----------------------------------------------------------------------------------------------------------
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));
		/*/
		switch (MovementOrder)
		{
			case EMovementOrder::Navigate:
				DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("MovementOrder:Navigate")));
			break;
				
			case EMovementOrder::Park:
				DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("MovementOrder:Park")));
			break;
			
			case EMovementOrder::Idle:
				DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("MovementOrder:Idle")));
			break;
				
			default:
				DEBUGMESSAGE, FColor::White,	FString::Printf(TEXT("MovementOrder:None")));
			break;
		}//*/
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("bShouldAlign: %d"),bShouldAlign));
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("bShouldReverse: %d"),bShouldReverse));
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("bShouldEvadeLeft: %d"),bShouldEvadeLeft));
		
	//----------------------------------------------------------------------------------------------------------
	// Rotation
	//----------------------------------------------------------------------------------------------------------
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));

	//	DEBUGMESSAGE, FColor::Green,	FString::Printf(TEXT("AlignStartRotation: %f"), debug_AlignStartRotation));
	//	DEBUGMESSAGE, FColor::Green,	FString::Printf(TEXT("AlignEndRotation: %f"), debug_AlignEndRotation));
	//	DEBUGMESSAGE, FColor::Green,	FString::Printf(TEXT("Rotation Difference: %f"), debug_RotationDifference));
	//	DEBUGMESSAGE, FColor::Green,	FString::Printf(TEXT("Inverse Rotation Differenc: %f"), debug_InverseRotationDifference));

	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));

	//	DEBUGMESSAGE, FColor::Magenta,	FString::Printf(TEXT("RawAlignRotationDistance: %f"), debug_RawAlignRotationDistance));
	//	DEBUGMESSAGE, FColor::Magenta,	FString::Printf(TEXT("Inverse RawRotationDifference: %f"), debug_InverseRawRotationDifference));

	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));	

	//	DEBUGMESSAGE, FColor::Orange,	FString::Printf(TEXT("Fastest TurnRate: %f"), debug_FastestTurnRate));
	//	DEBUGMESSAGE, FColor::Orange,	FString::Printf(TEXT("RotationSpeed: %f"), debug_RotationSpeed / DeltaTime));
		
	//	DEBUGMESSAGE, FColor::Orange,	FString::Printf(TEXT("TurnRate: %f"), TurnRate));
	//	DEBUGMESSAGE, FColor::Orange,	FString::Printf(TEXT("TurnFrictionCompensation %f"), debug_TurnFrictionCompensation));

	//----------------------------------------------------------------------------------------------------------
	// Torque
	//----------------------------------------------------------------------------------------------------------
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));
		
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("Torque.Z: %f"), Torque.Z));
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("Torque.X: %f"), Torque.X));
	//	DEBUGMESSAGE, FColor::Emerald,	FString::Printf(TEXT("Torque.Y: %f"), Torque.Y));

	//----------------------------------------------------------------------------------------------------------
	// Speed
	//----------------------------------------------------------------------------------------------------------
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));	

	//	DEBUGMESSAGE, FColor::Cyan,		FString::Printf(TEXT("CurrentSpeed: %f"),	CurrentSpeed));
	//	DEBUGMESSAGE, FColor::Cyan,		FString::Printf(TEXT("ActualSpeed: %f"),	ActualSpeed));
	//	DEBUGMESSAGE, FColor::Cyan,		FString::Printf(TEXT("DesiredSpeed: %f"),	DesiredSpeed));
		
	//	DEBUGMESSAGE, FColor::Cyan,		FString::Printf(TEXT("ForceOutput: %f"),	ForceOutput));
	//	DEBUGMESSAGE, FColor::Cyan,		FString::Printf(TEXT("Acceleration: %f"),	Acceleration));

	//----------------------------------------------------------------------------------------------------------
	// Navigation && Evasion
	//----------------------------------------------------------------------------------------------------------
	//	DEBUGMESSAGE, FColor::Emerald, FString::Printf(TEXT("-------------------------")));

	//	DEBUGMESSAGE, FColor::Green, FString::Printf(TEXT("NavPointArray.Num()): %d"), NavPointArray.Num()));
	//	DEBUGMESSAGE, FColor::Green,  FString::Printf(TEXT("EvadePoint: X%f, Y%f, Z%f"), EvadePoint.X, EvadePoint.Y, EvadePoint.Z));
		
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
	// Handle Alignment
	//----------------------------------------------------------------------------------------------------

	// Alignment While Moving
	if(MovementOrder == EMovementOrder::MoveTo
	|| MovementOrder == EMovementOrder::Park)
	{
		if(MovementType == EMovementType::CenterRotates
		&& ActualSpeed == 0.f
		&& bShouldAlign)
		{
			// Turn first;
		}
		else
		{
			AdjustSpeed();
		}
	}

	// Alignment while Still
	if(bShouldAlign)
	{
		switch (MovementType)
		{
			case EMovementType::BiDirectional:
				if(ActualSpeed > 1.f)
				{
					Align();
					// AdjustSpeed();
				}
				else
				{
					AdjustSpeed();
				}
			break;

			default: ;
				Align();
			break;
		}
	}
}

//------------------------------------------------------------------------------
// Physics Movement
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::AdjustSpeed() // We Use 0.02f (50FPS) instead of DeltaTime!
{ // FLOW_LOG
	if(bMovesOnSurface && !bIsOnSurface) return;

	// We divide our movement vector between ForwardVector and our InertiaVector to control drift deterministically.
	float Inertia = FixedInertia;
	FVector ForwardVector;
	FVector InertiaVector = PhysicsBodyInstance->GetUnrealWorldVelocity() * Inertia;
	
	if(bDynamicInertia)
	{
		Inertia = (CurrentSpeed / MaxSpeed);
	}
	
	if(MovementType == EMovementType::OmniDirectional)
	{
		// Vector towards target
		ForwardVector = OmniDirectionalVector;
		
		// Vector Perpendicular with the Actors Upvector, towards target.
		const FVector Upvector = GetActorUpVector();
		FVector PerpendicularVector = ForwardVector - FVector::DotProduct(ForwardVector, Upvector) * Upvector;
		PerpendicularVector.Normalize();

		// Final Vector
		ForwardVector = PerpendicularVector * (ForceOutput * (1.f - Inertia));
	}
	else
	{
		ForwardVector = GetActorForwardVector() * ForceOutput * (1.f - Inertia);
	}
	
	if(ActualSpeed < DesiredSpeed)
	{
		if(bShouldReverse)
		{
			// We just need to do ForceOutput inverted.
			ForceOutput = FMath::Clamp(ForceOutput - Acceleration * 0.02f, -MaxSpeed, MaxSpeed);
		}
		else
		{
			ForceOutput = FMath::Clamp(ForceOutput + Acceleration * 0.02f, -MaxSpeed, MaxSpeed);
		}

		// Draw the debug line
		#if WITH_EDITOR
			if(bShowVelocityDebug)
			{
				DrawDebugLine(GetWorld(),GetActorLocation(), GetActorLocation() + ForwardVector * (CurrentSpeed * 0.01f),FColor::Green,false,0.1f, 0.f, 1.f);
				DrawDebugLine(GetWorld(),GetActorLocation(), GetActorLocation() + InertiaVector * (CurrentSpeed * 0.01f),FColor::White,false,0.1f, 0.f, 1.f);
			}
		#endif
	}
	else // if(ActualSpeed > DesiredSpeed)
	{
		// We want to strangle ForceOutput.
		if(ForceOutput < 0.f)
		{
			// ForceOutput is negative here, so max clamp is MaxSpeed
			ForceOutput = FMath::Clamp(ForceOutput + (Acceleration * AccelerationBrakeMult * 0.02f), -MaxSpeed, MaxSpeed);
		}
		else
		{
			// Break
			ForceOutput = FMath::Clamp(ForceOutput - (Acceleration * AccelerationBrakeMult * 0.02f), -MaxSpeed, MaxSpeed);
		}

		// Draw the debug line
		#if WITH_EDITOR
			if(bShowVelocityDebug)
			{
				DrawDebugLine(GetWorld(),GetActorLocation(), GetActorLocation() + ForwardVector * (CurrentSpeed * 0.01f),FColor::Red,false,0.1f, 0.f, 1.f);
				DrawDebugLine(GetWorld(),GetActorLocation(), GetActorLocation() + InertiaVector * (CurrentSpeed * 0.01f),FColor::White,false,0.1f, 0.f, 1.f);
			}
		#endif
	}

	// Apply final calculation as Linear Velocity
	PhysicsBodyInstance->SetLinearVelocity(ForwardVector + (InertiaVector), false);
	// PhysicsBodyInstance->AddForce(FVector(-GetActorUpVector() * PhysicsBodyInstance->GetBodyMass() * 980.f), false);
}

void AEDU_CORE_MobileEntity::Align()
{ // FLOW_LOG
	if(bMovesOnSurface && !bIsOnSurface) return;

	if(bShouldAlign)
	{
		/*------------------ No need to DeltaTime ---------------------------
		  Because our physicscalculations already make use of steady Tick
		  there is no need to scale TurnRate with DeltaTime.

		  Physics require rather high numbers to get anything done though,
		  so we scale turnrate in order not to inflate the editor settings.
		--------------------------------------------------------------------*/

		Torque.Z = TurnRate * 10.f;
		
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

		  To reset FrictionCompensation, we always start at 0.
		--------------------------------------------------------------------*/

		float CurrentTorque = FMath::Abs(Torque.Z);
		float FrictionCompensation = 0.f;
		
		switch(RotationMode)
		{
			case ERotationMode::LocalYaw:
			// We need to apply more torque due to conversion
				if(ActualSpeed == 0.f && CurrentTorque < 300.f)
				{
					FrictionCompensation = 300.f - CurrentTorque;
				}
			break;
			
			case ERotationMode::WorldYaw:
				// Direct rotation = less compensation needed.
				if(ActualSpeed == 0.f && CurrentTorque < 40.f)
				{
					FrictionCompensation = 40.f - CurrentTorque;
				}
			break;
			default: ;
		}

		if (Torque.Z < 0)
		{
			Torque.Z -= FrictionCompensation;  // Go left
		}
		else 
		{
			Torque.Z += FrictionCompensation; // Go right
		}
		
		/*--------------- Altering Rotation with Physics ---------------------
		  SetAngularVelocityInRadians() is a Physics equivalent to
		  AddActorRotation(). It does so by adding a velocity to our
		  current rotation, instead of setting it immediately.

		  When the parameter bool bAddToCurrent is set to false, the new
		  velocity will override the old velocity. In other words, we can
		  set the velocity to 0 once we have reached the rotation we want.

		  By scaling our turnrate depending on rotation, we can effectively
		  use SetAngularVelocity() the same way we use AddActorRotation().
		--------------------------------------------------------------------*/

		FTransform ActorTransform;
		FVector LocalTorque;
		
		switch (RotationMode) {
			case ERotationMode::LocalYaw:
				/*---------------------------------------------------------------
				  <!> This will turn the actor on its local Z axis,
				  respecting pitch and rotation (ground entities).
				---------------------------------------------------------------*/
				
				// Get the actor's world rotation
				ActorTransform = GetActorTransform();

				// Convert local angular velocity to world space using the actor's rotation
				LocalTorque = ActorTransform.TransformVector(Torque);

				PhysicsBodyInstance->SetAngularVelocityInRadians(FMath::DegreesToRadians(LocalTorque), false);
			break;
				
			case ERotationMode::WorldYaw:
				/*---------------------------------------------------------------
				  <!> WorldYaw will turn the actor on the world Z axis,
				  despite pitch and rotation (certain flying entities).
				---------------------------------------------------------------*/
					
				PhysicsBodyInstance->SetAngularVelocityInRadians(FMath::DegreesToRadians(Torque), false);
			break;

			default:
				UE_LOG(LogTemp, Warning, TEXT("Invalid RotationMode in switch statement."));
			break;
		}

		// Debug
		// Check how fast we are turning: this is measured between frames, so it's actual speed.
		debug_RotationSpeed = FMath::Abs(debug_LastRotationDifference - debug_RotationDifference);
		debug_LastRotationDifference = debug_RotationSpeed;
		debug_TurnFrictionCompensation = FrictionCompensation;
	}
}

//------------------------------------------------------------------------------
// Waypoint Utility
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::AddWaypoint(const FWaypointParams& Params)
{ FLOW_LOG
	if(!Params.bQueue && WaypointArray.Num() > 0) // Always clear the array if anything is in it and the waypoint is not queued.
	{
		ClearAllWaypoints();
	}
	
	else if(Params.bQueue && WaypointArray.Num() > 0) // An empty array doesn't need queueing.
	{
		// Make sure that we don't exceed the maximum number of waypoints before saving.
		if (WaypointArray.Num() < MaxWaypointCapacity)
		{
			WaypointArray.AddUnique(Params.WaypointPtr);
			
			// Waypoint is queued, no need to execute it.
			return;
		}
		else // If the waypointArray is full, we'll remove the oldest one.
		{
			WaypointArray[0]->RemoveActorFromWaypoint(this);
			WaypointArray.RemoveAt(0);
		
			WaypointArray.AddUnique(Params.WaypointPtr);

			// Waypoint is queued, no need to execute it.
			return;
		}
	}

	// Clear the NavPointArray, else the unit will try to complete it before moving on.
	NavPointArray.Reset();
	bShouldEvade = false;
	
	WaypointArray.AddUnique(Params.WaypointPtr);
	ExecuteOrders(Params);
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
			ExecuteOrders(WaypointArray[0]->GetWaypointParams());
		}
	}
}

void AEDU_CORE_MobileEntity::ReviewNavigationQueue()
{ FLOW_LOG	
	if(bShouldEvade)
	{
		// We have reached evasionpoint, return and check distance to the next point.
		bShouldEvade = false;
		return;
	}

	if(NavPointArray.Num() > 0)
	{
		// We have reached the first navpoint, remove it and check distance to the next point.
		UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("NavPointArray.RemoveAt(0)"));
		NavPointArray.RemoveAt(0);
		return;
	}

	if(WaypointArray.Num() > 1 && WaypointArray[0]->IsPatrolPoint()) // Infinite loop unless there are more than 1.
	{
		// Remove the first element and add it to the end of the array
		TObjectPtr<AEDU_CORE_Waypoint> Patrolpoint = WaypointArray[0];
		WaypointArray.RemoveAt(0);
		WaypointArray.Add(Patrolpoint);

		// Continue
		RetrieveWaypointOrders();
		return;
	}

	switch (NavigationMode)
	{
		case ENavigationMode::None:
		case ENavigationMode::Navigate:
		case ENavigationMode::Roam: // Remove waypoint
			{
				if(WaypointArray.Num() > 0&& NavPointArray.Num() == 0)
				{
					RemoveWaypoint(WaypointArray[0]);
		
					if(WaypointArray.Num() > 0 && NavPointArray.Num() == 0) // Find a way to the next waypoint.
					{
						RequestPathAsync(GetActorLocation(), FormationLocation);
					}
					else
					{
						// We have reached final destination, we should park.
						MovementOrder = EMovementOrder::Park;
						return;
					}
				}
			}
		break;
		
		case ENavigationMode::SeekCover:
		break;
		
		case ENavigationMode::SeekTarget: // We want to save the waypoint until the target is gone.
			// If LOS stop, else get closer.
			// If fixed weapon; align
			DesiredSpeed = 0.f;
			MovementOrder = EMovementOrder::Aim;
			return;
		break;
		
		case ENavigationMode::KeepDistance:
		break;
		
		case ENavigationMode::MAX:
		break;
	default: ;
	}

	if(WaypointArray.Num() == 0)
	{
		// We have arrived
		MovementOrder = EMovementOrder::Park;
	}
}

void AEDU_CORE_MobileEntity::UpdateFormationLocation(const FWaypointParams& Params)
{ // FLOW_LOG

	int32 FormationIndex = WaypointArray[0]->GetFormationIndex(this);
	
	// Default
	FVector NextPosition = Params.WaypointPosition;
	NavigationMode = ENavigationMode::Navigate;

	//If we have a target, we use that instead.
	if(Params.TargetPosition != FVector::ZeroVector)
	{
		NavigationMode = ENavigationMode::SeekTarget;
		NextPosition = Params.TargetPosition;
	}
	else if(Params.TargetArray.Num() > 0)
	{
		NavigationMode = ENavigationMode::SeekTarget;
		NextPosition = Params.TargetArray[0]->GetActorLocation();
	}
	
	//------------------------------------------------------------------------------------
	// Line Formation
	//------------------------------------------------------------------------------------
	if(FormationIndex == 0) // Alone, leader or the Index is faulty.
	{
		FormationLocation = NextPosition;
	}
	else if(FormationIndex % 2 != 0) // Odd number, go right.
	{
		FormationLocation = NextPosition + (-Params.WaypointRightVector * FormationSpacing) * FormationIndex;
	}
	else if(FormationIndex % 2 == 0) // Even number, go right.
	{
		FormationLocation = NextPosition + (Params.WaypointRightVector * FormationSpacing) * FormationIndex;
	}

	// Draw a debug sphere at the location of each path point
	DrawDebugSphere(
		this->GetWorld(),				 // World context
		FormationLocation,               // Location of the sphere
		25,								 // Radius of the sphere
		4,                               // Segments for smoother sphere edges
		FColor::Magenta,                 // Color of the sphere
		false,                           // Persistent (will disappear after duration)
		5.f								 // Duration in seconds
	);
	
	
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
-
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
{ // FLOW_LOG
	if(!HasAuthority()) return;	
	BatchIndex = ServerBatchIndex;
}

void AEDU_CORE_MobileEntity::ExecuteOrders(const FWaypointParams& Params)
{ // FLOW_LOG

	// Default until change
	MovementOrder = EMovementOrder::MoveTo;

	TargetPosition = Params.TargetPosition;
	FormationRotation = Params.WaypointRotation;
	
	UpdateFormationLocation(Params);
	RequestPathAsync(GetActorLocation(), FormationLocation);
	
}

//------------------------------------------------------------------------------
// Functionality: AI Utility
//------------------------------------------------------------------------------

bool AEDU_CORE_MobileEntity::OnSurface(const FVector& CurrentPos)
{  // FLOW_LOG
	FVector ForwardVector = GetActorForwardVector();
	FVector RightVector = GetActorRightVector();
	FVector DownVector = -GetActorUpVector();
	FVector TraceStartLocation;
	FVector TraceEndLocation;
	
	if(bGroundAlter)
	{
		bGroundAlter = false;
		TraceStartLocation = CurrentPos + ((DownVector * 2.f) * BoxExtent.Z) + (ForwardVector * BoxExtent.X) + (RightVector * BoxExtent.Y);
		TraceEndLocation = CurrentPos + (DownVector * BoxExtent.Z) + (-ForwardVector * BoxExtent.X) + (-RightVector * BoxExtent.Y);
	}
	else
	{
		bGroundAlter = true;
		TraceStartLocation = CurrentPos + ((DownVector * 2.f) * BoxExtent.Z) + (ForwardVector * BoxExtent.X) + (-RightVector * BoxExtent.Y);
		TraceEndLocation = CurrentPos + (DownVector * BoxExtent.Z) + (-ForwardVector * BoxExtent.X) + (RightVector * BoxExtent.Y);
	}

	// Perform the line trace
	// FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore the actor itself

	/*// Always do debug on the main thread.
	#if WITH_EDITOR
		AsyncTask(ENamedThreads::GameThread, [this, TraceStartLocation, TraceEndLocation]()
		{
			DrawDebugLine(GetWorld(), TraceStartLocation, TraceEndLocation, FColor::Green, false, 0.1f, 0, 1.0f);
		});
	#endif //*/
	
	// Line trace downwards to se if we stand on something.
	if(GetWorld()->LineTraceTestByChannel(TraceStartLocation, TraceEndLocation, SurfaceChannel, QueryParams))
	{
		return true;
	}
	/*if(GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, SurfaceChannel, QueryParams))
	{
		if(HitResult.bBlockingHit)
		{
			return true;
		}
	}*/
	return false;
}

void AEDU_CORE_MobileEntity::CheckAlignment(const FRotator& CurrentPos)
{ // FLOW_LOG
	// Check if we are aligned already
	FRotator AlignStartRotation = CurrentPos;

	//---------------------------------------------------------------------------------
	// End Rotation evaluation // This one depends on what we are doing
	//---------------------------------------------------------------------------------
	FRotator AlignEndRotation;							
	
	if(MovementOrder == EMovementOrder::MoveTo)
	{
		if(bShouldEvade)
		{
			// UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Aligning with EvadePoint."));
			AlignEndRotation.Yaw = UtilityLibrary::GetRotationToTargetPos(this, EvadePoint).Yaw;
		}
		else if(NavPointArray.Num() > 0)
		{
			// UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Aligning with NavPointArray[0]."));
			AlignEndRotation.Yaw = UtilityLibrary::GetRotationToTargetPos(this, NavPointArray[0]).Yaw;

			#if WITH_EDITOR
				if(bShowCollisionDebug)
				{
					FVector NavPointPos = NavPointArray[0]; // Making sure it's threadsafe.
					// Always do this on the main thread.
					AsyncTask(ENamedThreads::GameThread, [this, NavPointPos]()
					{
						// Draw a debug square at the location of each path point
						DrawDebugSphere(
							GetWorld(),				// World context
							NavPointPos,       // Location of the sphere
							30,						// Radius of the sphere
							3,                      // Segments for smoother sphere edges
							FColor::Orange,         // Color of the sphere
							false,                  // Persistent (will disappear after duration)
							1.f						// Duration in seconds
						);
					});
				}
			#endif
		}
		else if(bShouldAim)
		{
			// UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Aligning with NavPointArray[0]."));
			AlignEndRotation.Yaw = UtilityLibrary::GetRotationToTargetPos(this, AimPoint).Yaw;
		}
		else
		{
			// UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Aligning with FormationLocation."));
			AlignEndRotation.Yaw = UtilityLibrary::GetRotationToTargetPos(this, FormationLocation).Yaw;
		}
	}
	else
	{
		// TODO: there might be case where we shouldn't align, simple set bShouldAlign = false in that case.
		
		// We have Reached our destination, now align with waypoint's instruction.
		AlignEndRotation.Yaw = FormationRotation.Yaw;
	}

	//---------------------------------------------------------------------------------
	// Omni-Directional Enteties (ODE)
	//---------------------------------------------------------------------------------

	// TODO: this need to be refactored to work with all types depending on movementorder, (attackmove etc.)
	
	if(MovementType == EMovementType::OmniDirectional)
	{
		// (ODE) Normally ignore Alignment and go directly towards target...
		OmniDirectionalVector = AlignEndRotation.Vector();

		// ... but they do align if the distance is very far... 
		if(MovementOrder == EMovementOrder::MoveTo
		&& Distance < OmniDirectionalAlignDistance)
		{		
			bShouldAlign = false;
			return;
		}

		// ... or if they have a target + fixed weapons.
		if(bShouldAlignWithTarget)
		{
			if(TargetEntity != nullptr)
			{
				AlignEndRotation.Yaw = UtilityLibrary::GetRotationToTargetActor(this,TargetEntity).Yaw;
			}
			if(TargetPosition != FVector::ZeroVector)
			{
				AlignEndRotation.Yaw = UtilityLibrary::GetRotationToTargetPos(this, TargetPosition).Yaw;
			}
		}
	}

	//---------------------------------------------------------------------------------
	// Rotation calculation
	//---------------------------------------------------------------------------------
	
	float RawAlignRotationDistance = UtilityLibrary::CalculateRotationDistance(AlignStartRotation.Yaw, AlignEndRotation.Yaw, 0.f);
	float AlignRotationDistance = FMath::Abs(RawAlignRotationDistance);

	//---------------------------------------------------------------------------------
	// Now that we know the rotation difference, we can see if we should reverse.
	//---------------------------------------------------------------------------------
	CheckReversalConditions(AlignRotationDistance);

	if(bShouldReverse)
	{
		/*-------------------------- FMath::Abs --------------------------------------
		  Absolute Value: The absolute value of a number is its distance from zero
		  on the number line, regardless of direction. In other words, it converts
		  any negative number to its positive counterpart while leaving positive
		  numbers unchanged.

		  We want to reverse the Rotation Distance while keeping the number
		  positive, and since the Rotation Distance is already positive,
		  subtracting 180 will work in both directions
		----------------------------------------------------------------------------*/
		AlignRotationDistance = FMath::Abs(AlignRotationDistance-180.f);

		// Flips front and back (Branchless)
		RawAlignRotationDistance += RawAlignRotationDistance < 0 ? 180.f : -180.f;
		// Legacy version for readability
		// if (RawAlignRotationDistance < 0) RawAlignRotationDistance += 180.f; else RawAlignRotationDistance -= 180.f;
	}

	//---------------------------------------------------------------------------------
	// Now we can evaluate the difference to see if we should align based of tolerance.
	//---------------------------------------------------------------------------------
	// Branchless Execution (Better than if)
	bShouldAlign = AlignRotationDistance > 1.f;
	if (MovementOrder == EMovementOrder::Park && MovementType == EMovementType::BiDirectional)
	{
		bShouldAlign = AlignRotationDistance > 2.f;
	}
	
	//---------------------------------------------------------------------------------
	// Calculate Turnrate based on distance
	//---------------------------------------------------------------------------------	
	TurnRate = UtilityLibrary::CalculateFastestTurnRate(MaxRotationRate, RawAlignRotationDistance, SlowRotationDistance);
	
	//---------------------------------------------------------------------------------
	// Alignment effects evasion
	//---------------------------------------------------------------------------------

	bShouldEvadeLeft = TurnRate < 0.f;

	// Legacy version for readability
	//if(FastestTurnRate < 0.f) {	bShouldEvadeLeft = true; } else	{ bShouldEvadeLeft = false; }
	
	//---------------------------------------------------------------------------------
	// Debug variables for the main thread
	//---------------------------------------------------------------------------------
	#if WITH_EDITOR
		debug_RawAlignRotationDistance = RawAlignRotationDistance;
		debug_InverseRawRotationDifference = -RawAlignRotationDistance;
		debug_InverseRotationDifference = FMath::Abs(AlignRotationDistance-180.f);
		debug_RotationDifference = AlignRotationDistance;
		debug_AlignStartRotation = AlignStartRotation.Yaw;
		debug_AlignEndRotation = AlignEndRotation.Yaw;
		debug_FastestTurnRate = TurnRate;
	#endif

}

void AEDU_CORE_MobileEntity::CheckPosition(const FVector& CurrentPos)
{ // FLOW_LOG
	if(CurrentPos != FormationLocation
	&& WaypointArray.Num() == 0)
	{
		MovementOrder = EMovementOrder::MoveTo;
	}
}

void AEDU_CORE_MobileEntity::CalculateCurrentSpeed(const FVector& CurrentPos, float DeltaTime)
{
	// Fetch velocity vector once per tick and cache it.
	CurrentSpeedVector = PhysicsBodyInstance->GetUnrealWorldVelocity();
	CurrentSpeed = CurrentSpeedVector.Size();

	// Calculate ActualSpeed on the XY plane without additional allocations.
	ActualSpeed = FVector::Dist2D(CurrentPos, LastPos) / DeltaTime;
	// MovementVector = (CurrentPos - LastPos) * 0.5f; // Deprecated

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

void AEDU_CORE_MobileEntity::CheckReversalConditions(const float RotationDistance)
{
	switch (MovementType)
	{
		// OmniDirectional enteties never explicitly reverse, they go whatever direction they deem fit.
		case EMovementType::OmniDirectional:
			bShouldReverse = false;
		break;

		// Rest move by default
		default:
			if (bForceReverse)
			{
				bShouldReverse = true;
			}
			else if(bCanReverse 
				 && Distance > StopThreshold					// Else we want to rotate on the spot
				 && RotationDistance > ReverseRotationDistance	// Else we want to face the turn
				 && MaxReverseDistance > Distance)				// Else we want to go forward
			{
				bShouldReverse = true;
			}
			else
			{
				bShouldReverse = false;
			}
		break;
	}
}

void AEDU_CORE_MobileEntity::HandleNavigation()
{
	float Stop = 0.f;
	float SlowDown = 0.f;
	
	using enum ENavigationMode;
	switch(NavigationMode)
	{
		case None:
		case Navigate:
		case Roam:
			SlowDown = SlowDownThreshold;
			Stop = StopThreshold; 
		break;
		
		case SeekTarget:
			SlowDown = SlowDownThresholdWhileAiming;
			Stop = StopThresholdWhileAiming;
		break;
		
	default: FLOW_LOG_ERROR("HandleNavigation :: switch(NavigationMode) = default");
	}
	
	// Setting DesiredSpeed based on Distance thresholds.
	if(Distance > SlowDown)
	{
		DesiredSpeed = MaxSpeed;
	}
	else if(Distance < Stop)
	{
		ReviewNavigationQueue();
	}
	else
	{
		DesiredSpeed = (Distance / SlowDownThreshold) * MaxSpeed;
	}
}

void AEDU_CORE_MobileEntity::HandleParking(const FRotator& CurrentPos)
{
	if(MovementType >= EMovementType::CenterRotates)
	{
		DesiredSpeed = 0;
		bShouldReverse = false;

		if(!bShouldAlign)
		{
			CheckAlignment(CurrentPos);
			if(!bShouldAlign)
			{
				MovementOrder = EMovementOrder::Idle;
			}
		}
	}
	else
	{
		if(bShouldAlign)
		{
			DesiredSpeed = ParkingSpeed;
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
		DynamicCollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		DynamicCollisionSphere->SetCollisionResponseToChannel(CollisionDetectionVolumeChannel, ECR_Block);
		
		this->AddInstanceComponent(DynamicCollisionSphere); // Add the sphere as an instance component to the actor, tied to its lifecycle.

		if(bShowCollisionDebug)
		{
			DynamicCollisionSphere->SetHiddenInGame(false);         // Ensure it's visible during gameplay
			DynamicCollisionSphere->SetVisibility(true);            // Set visibility property for both runtime and editor
			DynamicCollisionSphere->ShapeColor = FColor::Blue;    // Set the shape color (this will affect how it appears in the editor or as a debug shape)
		}
		else
		{
			DynamicCollisionSphere->SetHiddenInGame(true);
			DynamicCollisionSphere->SetVisibility(false); 
		}
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
	FVector CurrentPos = GetActorLocation();
	FVector ForwardVector;
	
	if(MovementType == EMovementType::OmniDirectional)
	{
		ForwardVector = OmniDirectionalVector;
	}
	else
	{
		ForwardVector = GetActorForwardVector();
	}
	
	FVector SideVector = bShouldEvadeLeft ? -GetActorRightVector() : GetActorRightVector();
	FVector TraceDirection = bShouldReverse ? -ForwardVector : ForwardVector;
	FVector TraceStartLocation = CurrentPos + TraceDirection * CollisionDetectionStartOffset;

	// FHitResult HitResult; // (We use SweepTestByChannel, it needs no HitResult)
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	//------------------------------------------------------------------
	// First check: Trace directly forward (or backward if reversing)
	//------------------------------------------------------------------
	float Speed = ActualSpeed;
	ActualSpeed < 1 ? Speed *= 3.f : ActualSpeed *= 1.f;

	if(IsPathClear(TraceStartLocation, TraceStartLocation + TraceDirection * Speed * CollisionTraceMult, QueryParams))
	{
		return true; // If clear, no need for evasion
	}

	//------------------------------------------------------------------
	// Helper lambda for side traces
	//------------------------------------------------------------------
	auto CheckDirections = [&](const FVector& Direction, float Distance) -> bool
	{
		FVector TraceEndLocation = TraceStartLocation + Direction * Distance * CollisionTraceMult;
		if (IsPathClear(TraceStartLocation, TraceEndLocation, QueryParams))
		{
			EvadePoint = TraceEndLocation;
			bShouldEvade = true;
			return true; // Path is clear
		}
		return false; // Path is blocked
	};

	//------------------------------------------------------------------
	// Check sides with offset
	//------------------------------------------------------------------
	TArray<FVector> DiagonalDirections = {
		TraceDirection + SideVector * 0.5f,   // 22.5 degrees to priority side
		TraceDirection + SideVector,          // 45 degrees to priority side
		SideVector,                           // 90 degrees to priority side
		TraceDirection - SideVector * 0.5f,   // 22.5 degrees to non-priority side
		TraceDirection - SideVector,          // 45 degrees to non-priority side
		-SideVector,                          // 90 degrees to non-priority side
	};

	// Loop over all Diagonal directions
	for (const FVector& Direction : DiagonalDirections)
	{
		if (CheckDirections(Direction, ActualSpeed))
		{
			return true; // Early exit if any side is clear
		}
	}

	//------------------------------------------------------------------
	// Start Checking Backwards
	//------------------------------------------------------------------
	TraceDirection = -ForwardVector; // Reverse the direction
	TraceStartLocation = CurrentPos + TraceDirection * CollisionDetectionStartOffset;

	// Array of backward directions
	TArray<FVector> BackwardDirections = {
		TraceDirection,						  // Directly backward
		TraceDirection + SideVector * 0.5f,   // 22.5 degrees to priority side
		TraceDirection + SideVector,          // 45 degrees to priority side
		TraceDirection - SideVector * 0.5f,   // 22.5 degrees to non-priority side
		TraceDirection - SideVector,          // 45 degrees to non-priority side

	};

	// Check backward directions
	for (const FVector& Direction : BackwardDirections)
	{
		// Check Close
		if (CheckDirections(Direction, StopThreshold * 1.1f))
		{
			return true; // Early exit if any backward direction is clear
		}
	}

	//------------------------------------------------------------------
	// Check sides without offset, good if we are stuck fron and back.
	//------------------------------------------------------------------
	TraceStartLocation = CurrentPos + TraceDirection;
	
	// Array of side directions
	TArray<FVector> SideDirections = {
		SideVector,                           // 90 degrees to priority side
		-SideVector,                          // 90 degrees to non-priority side
	};

	// Check side directions
	for (const FVector& Direction : SideDirections)
	{
		// Check Close
		if (CheckDirections(Direction, StopThreshold * 1.1f))
		{
			return true; // Early exit if any sideward direction is clear
		}
	}

	// No path is clear, return false
	if(WaypointArray.Num() > 0 && NavPointArray.Num() == 0)
	{
		RequestPathAsync(GetActorLocation(), FormationLocation);
	}
	bShouldEvade = false;
	return false;
}

bool AEDU_CORE_MobileEntity::IsPathClear(const FVector& TraceStartLocation, const FVector& TraceEndLocation, const FCollisionQueryParams& QueryParams) const
{
	// Check for hits, note that test is faster, since it doesn't require a HitResult
	if (GetWorld()->SweepTestByChannel(TraceStartLocation, TraceEndLocation, FQuat::Identity, CollisionDetectionVolumeChannel, FCollisionShape::MakeSphere(CollisionDetectionVolumeRadius), QueryParams))
	{
		// Perform the line trace
		#if WITH_EDITOR
		if(bShowCollisionDebug)
		{
			// Always do this on the main thread.
			AsyncTask(ENamedThreads::GameThread, [this, TraceEndLocation]()
			{
				DrawDebugSphere(GetWorld(), TraceEndLocation, CollisionDetectionVolumeRadius, 12, FColor::Red, false, 0.05f); // End position			
			});
		}
		#endif
		return false;
	}

	/* Old Version
	if (GetWorld()->SweepSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, FQuat::Identity, CollisionDetectionVolumeChannel, FCollisionShape::MakeSphere(CollisionDetectionVolumeRadius), QueryParams))
	{
		if(HitResult.bBlockingHit)
		{
			// Perform the line trace
			#if WITH_EDITOR
				if(bShowDynamicCollisionSphere)
				{
					// Always do this on the main thread.
					AsyncTask(ENamedThreads::GameThread, [this, TraceEndLocation]()
					{
						DrawDebugSphere(GetWorld(), TraceEndLocation, CollisionDetectionVolumeRadius, 12, FColor::Red, false, 0.05f); // End position			
					});
				}
			#endif
			
			return false;
		}
	}*/

	#if WITH_EDITOR
		if(bShowCollisionDebug)
		{
			// Always do this on the main thread.
			AsyncTask(ENamedThreads::GameThread, [this, TraceStartLocation, TraceEndLocation]()
			{
				// Visualize the trace start, end, and the path between them
				DrawDebugLine(GetWorld(), TraceStartLocation, TraceEndLocation, FColor::Blue, false, 0.05f, 0, 0.1f); // Path of the trace

				// Perform the line trace
				DrawDebugSphere(GetWorld(), TraceEndLocation, CollisionDetectionVolumeRadius, 12, FColor::Green, false, 0.05f); // End position			
			});
		}
	#endif
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
		if(!NavSystem)
		{
			UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("No NavSystem, aborting."));
			return;
		}
	}
	
	if(const ANavigationData* NavData = Cast<ANavigationData>(NavSystem->GetMainNavData()))
	{
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
	else
	{
		UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("No NavData, aborting."));
		return;
	}
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
			/*
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
			#endif*/
		}
	}
	else
	{
		UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Pathfinding failed with RequestID: %d"), RequestID);
	}
}
