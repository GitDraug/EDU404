// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Entities/EDU_CORE_SelectableEntity.h"
#include "GameFramework/HUD.h"
#include "EDU_CORE_HUD.generated.h"

class AEDU_CORE_PlayerController;

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  Base class of the heads-up display. This has a canvas and a debug canvas on
  which primitives can be drawn.

  Note that the HUD is created by the PlayerController, it can't exist without
  one.
------------------------------------------------------------------------------*/
UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_HUD : public AHUD
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	AEDU_CORE_HUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

//------------------------------------------------------------------------------
// Get/Set & Public API
//------------------------------------------------------------------------------
public:	
	void SetSelectionMarqueeFrameColor(float Red, float Green, float Blue, float Opacity);
	void SetSelectionMarqueeBGColor(float Red, float Green, float Blue, float Opacity);
	
	void DrawSelectionMarquee(const FVector2d& MousePosition);
	void StopDrawingSelectionMarquee();
	void DetectEntitiesInSelectionRectangle(const TSubclassOf<class AEDU_CORE_SelectableEntity>& ClassFilter, const FVector2D& FirstPoint, const FVector2D& SecondPoint, TArray<AEDU_CORE_SelectableEntity*>& OutEntityArray) const;
	
	void ResetSelectionRectangleArray() { SelectionRectangleArray.Reset(); }
	void EmptySelectionRectangleArray() { SelectionRectangleArray.Empty(); }
	
	/*------------------------------------------------------------------------------
	  The use of & in TArray<ASelectableEntity*>& means that the function returns
	  a reference, which avoids making a copy of the array and allows for
	  modifications directly on the original array. 
	------------------------------------------------------------------------------*/
	FORCEINLINE TArray<AEDU_CORE_SelectableEntity*>& GetSelectionRectangleArray() { return SelectionRectangleArray; } 
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marquee Selection | Frame color (Red)", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FrameRed = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marquee Selection | Frame color (Green)", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FrameGreen = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marquee Selection | Frame color (Blue)", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FrameBlue = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marquee Selection | Frame Thickness", meta=(ClampMin="0.1", ClampMax="10.0"))
	float FrameOpacity = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marquee Selection | Frame Thickness", meta=(ClampMin="0.1", ClampMax="10.0"))
	float FrameThickness = 1.f;

	//--------------------------------------------------------------------------------------------------
	// Background color and thickness
	//--------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marquee Selection | Background color (Red)", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BG_Red = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marquee Selection | Background color (Green)", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BG_Green = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marquee Selection | Background color (Blue)", meta=(ClampMin="0.0", ClampMax="1.0"))
	float  BG_Blue = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marquee Selection | Background Opacity", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BG_Opacity = 0.2f;

	// To search for certain classes
	bool bSearchWholeScreen;
	TSubclassOf<class AEDU_CORE_SelectableEntity> SearchFilter = AEDU_CORE_SelectableEntity::StaticClass();
	
private:
	// Summary of Colors
	FLinearColor FrameColor {FrameRed, FrameGreen, FrameBlue, FrameThickness};
	FLinearColor BGColor {FrameRed, FrameGreen, FrameBlue, BG_Opacity};

	// To keep track of the Mouse
	FVector2d MousePos;
	FVector2d InitialMousePos;
	FVector2d CurrentMousePos;

	// ScreenSize
	FIntVector2 ScreenSize;

	// Selection Marquee
	bool bMouseDraw = false;
	int8 FrameCounter = 0;

	// Custom PlayerController
	UPROPERTY()
	TObjectPtr<AEDU_CORE_PlayerController> LocalController;

	// Temp Selection Array
	UPROPERTY()
	TArray<AEDU_CORE_SelectableEntity*> SelectionRectangleArray;
	
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
private:
	/*------------------------------------------------------------------------------
	  This is a bastard of to GetActorsInSelectionRectangle() in HUD.h.

	  The original function gets all actors in the entire map and filters them
	  out afterward. This function only gets the Selectable Entities, our
	  derived version of actors.
	------------------------------------------------------------------------------*/
	virtual void DrawHUD() override; // Tick for HUDs

};
