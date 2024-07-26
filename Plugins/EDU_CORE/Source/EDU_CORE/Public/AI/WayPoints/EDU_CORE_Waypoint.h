// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Entities/EDU_CORE_SelectableEntity.h"
#include "EDU_CORE_Waypoint.generated.h"

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  Base class for a Waypoint that can be selected and moved. It only exists
  on the client.
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

	//------------------------------------------------------------------------------
	// Functionality
	//------------------------------------------------------------------------------
public:
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
};
