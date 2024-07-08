// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EDU_USER_HUD.generated.h"

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  Base class of the heads-up display. This has a canvas and a debug canvas on
  which primitives can be drawn.

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
	FVector2d InitialPoint;
	FVector2d CurrentPoint;

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
public:
	// Get/Set
	FORCEINLINE TObjectPtr<AEDU_USER_HUD> GetHUD() { return this; }
	
	void SetSelectionMarqueeFrameColor(float Red = 1.f, float Green = 1.f, float Blue = 1.f, float Opacity = 1.f);
	void SetSelectionMarqueeBGColor(float Red = 1.f, float Green = 1.f, float Blue = 1.f, float Opacity = 1.f);

	void DrawSelectionMarquee(const FVector2d& InitialPoint);
};
