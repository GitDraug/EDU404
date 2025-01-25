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

UENUM(BlueprintType)
enum class EEntitytype : uint8
{

	Unit		UMETA(DisplayName = "Unit"),

	Waypoint	UMETA(DisplayName = "Waypoint"),

	Resource	UMETA(DisplayName = "Resource"),

	Weapon		UMETA(DisplayName = "Weapon"),

	MAX				UMETA(Hidden)
};

UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_SelectableEntity : public AActor
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------
public:
	AEDU_CORE_SelectableEntity(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

//---------------------------------------------------------------
// Get/Set
//--------------------------------------------------------------
public:

	// Returns the Entety's StatusComponent.
	FORCEINLINE TObjectPtr<UStatusComponent> GetStatusComponent() const { return StatusComponent; };

	// Returns the Entety's EngagementComponent.
	FORCEINLINE TObjectPtr<UEngagementComponent> GetEngagementComponent() const { return EngagementComponent; };

	FORCEINLINE EEntitytype GetEntityType() const { return EntityType; }
	
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

	UPROPERTY()
	bool bMouseHighlighted = false;
	
	UPROPERTY()
	bool bRectangleHighlighted = false;

	UPROPERTY()
	bool bSelected = false;
	
	UPROPERTY(EditAnywhere,
	Category = "Selection",
	meta = (DisplayName = "Can Be be selected by Rectangle Marquee",
	ToolTip = "Allows this entity to be detected and selected by the Selection Marquee in the HUD."))
	bool bCanBeRectangleSelected = true;

protected:

	// What kind of Entity is this?
	UPROPERTY(EditAnywhere, Category = "Selection")
	EEntitytype EntityType = EEntitytype::Unit;
	
	// Pointer to our StatusComponent (if we have one)
	UPROPERTY()
	TObjectPtr<UStatusComponent> StatusComponent = nullptr;
	
	// Pointer to our EngagementComponent (if we have one)
	UPROPERTY()
	TObjectPtr<UEngagementComponent> EngagementComponent = nullptr;
	
//------------------------------------------------------------------------------
// Functionality: Highligtning
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
// Functionality: Server Stuff
//------------------------------------------------------------------------------
	
	virtual void SetGUID(FGuid NewID) { ServerEntityID = NewID; };
	virtual FGuid GetGUID() { return ServerEntityID; };

};
