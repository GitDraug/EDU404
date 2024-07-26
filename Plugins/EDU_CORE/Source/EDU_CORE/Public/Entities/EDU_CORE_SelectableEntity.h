// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_SelectableEntity.generated.h"

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  Base class of entities that can be detected and selected by the cursor.

  Allows the entity to be selected by the Camera UI.
------------------------------------------------------------------------------*/

UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_SelectableEntity : public AActor
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------
public:
	AEDU_CORE_SelectableEntity(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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

	virtual void MouseHighlightActor();
	virtual void MouseUnHighlightActor();
	
	virtual void RectangleHighlightActor();
	virtual void UnRectangleHighlightActor();
	
	virtual void SelectActor();
	virtual void UnSelectActor();
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
public:
	UPROPERTY(EditAnywhere,
	Category = "Selection",
	meta = (DisplayName = "Can be selected",
	ToolTip = "Allows this entity to be detected and selected by the Selection Marquee in the HUD."))
	bool bCanBeSelected = true;
	
	bool bMouseHighlighted = false;
	bool bRectangleHighlighted = false;
	bool bSelected = false;
};
