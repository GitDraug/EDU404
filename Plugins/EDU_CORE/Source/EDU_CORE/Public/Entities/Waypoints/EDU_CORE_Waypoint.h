// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "Entities/EDU_CORE_SelectableEntity.h"
#include "EDU_CORE_Waypoint.generated.h"

class AEDU_CORE_SelectableEntity;

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  Base class for a Waypoint that can be selected and moved.
------------------------------------------------------------------------------*/


UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_Waypoint : public AEDU_CORE_SelectableEntity
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	AEDU_CORE_Waypoint(const FObjectInitializer& ObjectInitializer);

	void InitiateWaypoint(const EEDU_CORE_Team Team, const FVector& WorldLocation, const FRotator& WorldRotation);

	// Override GetLifetimeReplicatedProps
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------
public:
	// Adds an actor to be notified by this waypoint.
	void AddActorToWaypoint(const TObjectPtr<AEDU_CORE_SelectableEntity>& Entity);

	// removes an actor to be notified by this waypoint.
	void RemoveActorFromWaypoint(const TObjectPtr<AEDU_CORE_SelectableEntity>& Entity);

	// Sets a global, unique ID that works across instances.
	void SetWaypointID(FGuid GUID);
	
	// Notifies a listener in the ListenerArray that they need to save this waypoint in their WaypointArray.
	void NotifyListeners();

	// What team does this waypoint belong to?
	void SetTeam(const EEDU_CORE_Team NewTeam) { Params.WaypointTeam = NewTeam; };

	// Resets the waypoint, so it can be returned to an ActorPool
	void ResetWaypoint();

	// Sets what kind of Waypoint this is
	FORCEINLINE void SetWaypointType(const EEDU_CORE_WaypointType Type) { Params.WaypointType = Type; };

	// Sets this Waypint to a Patrolpoint or not
	FORCEINLINE void SetPatrolPoint(const bool bIsPatrolPoint) { Params.bPatrolPoint = bIsPatrolPoint; } ;
	
	// Sets what kind of Waypoint this is
	FORCEINLINE void SetTargetLocation(const FVector& Location) { Params.TargetPosition = Location; };

	// Sets what kind of Waypoint this is
	FORCEINLINE void SetWaypointParams(const FWaypointParams& NewParams) { Params = NewParams; };
	
	// Getters
	FORCEINLINE FWaypointParams GetWaypointParams()					 const	{ return Params; };
	FORCEINLINE FGuid GetWaypointID()								 const	{ return WaypointID; };
	
	FORCEINLINE bool IsPatrolPoint()								 const	{ return Params.bPatrolPoint; } ;
	FORCEINLINE FVector GetTargetPosition()							 const	{ return Params.TargetPosition; }
	FORCEINLINE EEDU_CORE_MovementSpeed GetWaypointSpeed()			 const	{ return Params.MovementSpeed; };
	FORCEINLINE EEDU_CORE_InfantryStance GetWaypointStance()		 const	{ return Params.InfantryStance; };
	FORCEINLINE EEDU_CORE_WaypointType GetWaypointType()	 		 const	{ return Params.WaypointType; };
	FORCEINLINE EEDU_CORE_AlertnessLevel GetWaypointAlertnessLevel() const	{ return Params.AlertnessLevel; };

	// Return FormationIndex to a Listener
	int32 GetFormationIndex(const TObjectPtr<AEDU_CORE_SelectableEntity>& Listener) const;
	
	/*-------------------------------------------------------------------------------------
	  Highlighting and Selection are triggered by
	  - PlayerController::CursorTrace()
	    The playerController will trigger MouseHighlightActor on MouseHover.
	  - HUD::DrawHUD()
	    The playerController will trigger Highlight on all entities in Drawn Rectangle.
	  - CameraPawn::Input_Mouse_1_Released
	    The CameraPawn will select all entities in Drawn Rectangle on Mouse_1_Released.
	-------------------------------------------------------------------------------------*/

	virtual void RectangleHighlightActor() override;
	virtual void UnRectangleHighlightActor() override;
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:

	// Parameters
	UPROPERTY(EditAnywhere, Category = "Parameters", DisplayName= "Waypoint Parameters")
	FWaypointParams Params;

	// Unique Global ID
	UPROPERTY(VisibleAnywhere)
	FGuid WaypointID;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<AEDU_CORE_SelectableEntity>> ListenerArray;

//------------------------------------------------------------------------------
// Replication
//------------------------------------------------------------------------------
public:
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedLocation)
	FVector_NetQuantize10 Rep_Location;
	
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedRotation)
	FRotator Rep_Rotation;

protected:	
	UFUNCTION()
	void OnRep_ReplicatedLocation();

	UFUNCTION()
	void OnRep_ReplicatedRotation();
};
