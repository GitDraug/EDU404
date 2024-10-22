// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "Entities/EDU_CORE_SelectableEntity.h"
#include "EDU_CORE_Waypoint.generated.h"

class EDU_CORE_SelectableEntity;

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
	AEDU_CORE_Waypoint();

	void InitiateWaypoint(const EEDU_CORE_Team Team, const FVector& WorldLocation, const FRotator& WorldRotation);

	// Override GetLifetimeReplicatedProps
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------
public:
	// Adds an actor to be notified by this waypoint.
	virtual void AddActorToWaypoint(AActor* Actor);

	// removes an actor to be notified by this waypoint.
	virtual void RemoveActorFromWaypoint(AActor* Actor);

	// Sets a global, unique ID that works across instances.
	virtual void SetWaypointID(FGuid GUID);
	
	// Notifies a listener in the ListenerArray that they need to save this waypoint in their WaypointArray.
	virtual void NotifyListeners(bool Queue);

	// What team does this waypoint belong to?
	virtual void SetTeam(EEDU_CORE_Team NewTeam);

	// Resets the waypoint, so it can be returned to an ActorPool
	virtual void ResetWaypoint();

	// Sets what kind of Waypoint this is
	virtual void SetWaypointType(EEDU_CORE_WaypointType InWaypointType);
	
	// Getters
	FORCEINLINE EEDU_CORE_CombatMode GetWaypointCombatMode() const { return CombatMode; };
	FORCEINLINE EEDU_CORE_WaypointType GetWaypointType() const { return WaypointType; };
	FORCEINLINE EEDU_CORE_Stance GetWaypointStance() const { return Stance; };
	FORCEINLINE EEDU_CORE_Speed GetWaypointSpeed() const { return Speed; };
	FORCEINLINE virtual FGuid GetWaypointID() const { return WaypointID; };

	// Return FormationIndex to a Listener
	virtual int32 GetFormationIndex(const TObjectPtr<AActor> Listener) const;
	
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

	// Unique Global ID
	UPROPERTY()
	FGuid WaypointID;

	// Entities that adhere to this waypoint;
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<AActor>> ListenerArray;

	// Entities that this waypoint targets;
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<AActor>> TargetArray;

	// Center position that members of ListenerArray can offset from.
	FVector FormationCenter;

	UPROPERTY()
	EEDU_CORE_CombatMode CombatMode;

	UPROPERTY()
	EEDU_CORE_Stance Stance;

	UPROPERTY()
	EEDU_CORE_Speed Speed;
	
	UPROPERTY()
	EEDU_CORE_Team WaypointTeam;

	UPROPERTY()
	EEDU_CORE_WaypointType WaypointType;

	UPROPERTY()
	EEDU_CORE_Formations WaypointFormation;
	
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
