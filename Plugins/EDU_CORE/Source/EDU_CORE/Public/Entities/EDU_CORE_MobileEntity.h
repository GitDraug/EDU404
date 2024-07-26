// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_PhysicalEntity.h"
#include "Interfaces/EDU_CORE_CommandInterface.h"
#include "EDU_CORE_MobileEntity.generated.h"

class AEDU_CORE_Waypoint;
class AEDU_CORE_Waypoint_Move;
class AEDU_CORE_PlayerController;
class AEDU_CORE_C2_Camera;

/**
 * 
 */
UCLASS()
class EDU_CORE_API AEDU_CORE_MobileEntity : public AEDU_CORE_PhysicalEntity, public IEDU_CORE_CommandInterface
{
	
	GENERATED_BODY()
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
protected:
	virtual void BeginPlay() override;

	//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:
	virtual void ServerMobileCalc(float DeltaTime); 
	virtual void ServerMobilesExec(float DeltaTime);
	
//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------
public:

	//------------------------------------------------------------------------------
	// Interface Commands
	//------------------------------------------------------------------------------
	UFUNCTION()
	virtual void Command_NavigateTo(AActor* Commander, const FVector& Position, const FRotator& Rotation, bool Que, float Delay) override;

	//------------------------------------------------------------------------------
	// Waypoint Utility
	//------------------------------------------------------------------------------
	UFUNCTION()
	virtual void SaveWaypoint(AEDU_CORE_Waypoint* Waypoint);
	virtual void DeleteWaypoint(AEDU_CORE_Waypoint* Waypoint);
	virtual void ClearAllWaypoints();
	const virtual TArray<AEDU_CORE_Waypoint*>& GetWaypointArray() { return WaypointArray; }

	//---------------------------------------------------------------------------
	// Server executables
	//---------------------------------------------------------------------------
	// Function that will be called from the client and executed on the server
	//UFUNCTION(Server, Reliable)
	//void ServerCommandMove(const FVector& Location);

	// Client-side function to call the server RPC
	//void CallServerFunction(const FVector& Location);
	void Accelerate();

	// Align the actor to a target position over time.
	void Align(float DeltaTime);
	
	UPROPERTY(EditAnywhere, Category = "Force")
	float ForwardForce = 0.f;
	
//------------------------------------------------------------------------------
// Components Waypoints
//------------------------------------------------------------------------------
protected:
	UPROPERTY(EditAnywhere, Category = "Waypoints")
	TArray<TObjectPtr<AEDU_CORE_Waypoint>> WaypointArray;

	int8 MaxWaypointCapacity = 20;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoints")
	TSubclassOf<AEDU_CORE_Waypoint> WaypointClass;
	
	TObjectPtr<AEDU_CORE_Waypoint_Move> Waypoint_Move ;

//------------------------------------------------------------------------------
// Simplified Physics Movement (Not using the chaos physics engine)
//------------------------------------------------------------------------------
	FVector2d Location;
	
	float Inertia;
	
	float Acceleration;
	float AccelerationReverse;

	float MaxSpeed;
	float CurrentSpeed;

	float MaxBrake;
	float CurrentBreak;
	
	bool bRotateOnSpot = false;
	float MaxTurnRate = 0.5f;
	float SlowDownAtDegrees = 2;

	
//------------------------------------------------------------------------------
// Pure Calculations for Multithreading
//------------------------------------------------------------------------------
protected:
	int32 FrameCounter;

	
	//------------------------------------------------------------------------------
	// Command: Align
	//------------------------------------------------------------------------------
	bool bAlign = false;
	FVector NavigationLocation;
	FVector CachedWaypointLocation;

	FRotator AlignEndRotation;
	FRotator AlignStartRotation;
	FRotator AlignInterpRotation;

	FVector TargetLocation;
//------------------------------------------------------------------------------
// Utility
//------------------------------------------------------------------------------
protected:
	UFUNCTION()
	virtual AEDU_CORE_Waypoint_Move* SpawnWaypoint(AActor* Camera, const FVector& Position, const FRotator& Rotation);

	// Get the Angle to a target position, can be used to filter out Yaw etc.
	FRotator GetRotationToTargetPos(const FVector& Target) const;
	FRotator GetRotationToTargetActor(const AActor* TargetActor) const;

	virtual void UpdateNavigation();
};
