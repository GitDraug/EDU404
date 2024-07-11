// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EDU_USER_HUD.generated.h"

class AEDU_USER_PlayerController;

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  Base class of the heads-up display. This has a canvas and a debug canvas on
  which primitives can be drawn.

  Note that the HUD is created by the PlayerController, it can't exist without
  one.
------------------------------------------------------------------------------*/
UCLASS(Abstract)
class EDU_USER_API AEDU_USER_HUD : public AHUD
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	AEDU_USER_HUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:	
	void SetSelectionMarqueeFrameColor(float Red, float Green, float Blue, float Opacity);
	void SetSelectionMarqueeBGColor(float Red, float Green, float Blue, float Opacity);
	
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

private:
	// Summary of Colors
	FLinearColor FrameColor {FrameRed, FrameGreen, FrameBlue, FrameThickness};
	FLinearColor BGColor {FrameRed, FrameGreen, FrameBlue, BG_Opacity};

	// To keep track of the Mouse
	FVector2d MousePos;
	FVector2d InitialMousePos;
	FVector2d CurrentMousePos;

	// Selection Marquee
	bool bSelectionTriggered = false;

	// Custom PlayerController
	UPROPERTY()
	TObjectPtr<AEDU_USER_PlayerController> LocalController;

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
public:
	virtual void DrawHUD() override; // Tick for HUDs
	void DrawSelectionMarquee(const FVector2d& MousePosition);
	void StopDrawingSelectionMarquee();
};
