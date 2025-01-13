// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_SelectableEntity.generated.h"

class UStatusComponent;
class UEngagementComponent;

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

	virtual void BeginPlay() override;

//---------------------------------------------------------------
// Get/Set
//--------------------------------------------------------------
public:
	
	FORCEINLINE TObjectPtr<UStatusComponent> GetStatusComponent() const { return StatusComponent; };

	FORCEINLINE TObjectPtr<UEngagementComponent> GetEngagementComponent() const { return EngagementComponent; };
	
//------------------------------------------------------------------------------
// Networking
//------------------------------------------------------------------------------
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated)
	FGuid ServerEntityID;
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
public:
	UPROPERTY(EditAnywhere,
	Category = "Selection",
	meta = (DisplayName = "Can be selected",
	ToolTip = "Allows this entity to be detected and selected by the Selection Marquee in the HUD."))
	bool bCanBeSelected = true;

	UPROPERTY()
	bool bMouseHighlighted = false;

	UPROPERTY()
	bool bRectangleHighlighted = false;

	UPROPERTY()
	bool bSelected = false;

protected:
	// Pointer to our StatusComponent (if we have one)
	UPROPERTY()
	TObjectPtr<UStatusComponent> StatusComponent = nullptr;
	
	// Pointer to our EngagementComponent (if we have one)
	UPROPERTY()
	TObjectPtr<UEngagementComponent> EngagementComponent = nullptr;

	
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

	virtual void SetGUID(FGuid NewID) { ServerEntityID = NewID; };
	virtual FGuid GetGUID() { return ServerEntityID; };

};
