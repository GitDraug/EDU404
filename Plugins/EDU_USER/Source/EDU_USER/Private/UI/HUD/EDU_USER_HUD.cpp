// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/EDU_USER_HUD.h"

#include "Framework/Data/FlowLog.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_USER_HUD::AEDU_USER_HUD(const FObjectInitializer& ObjectInitializer) : Super (ObjectInitializer)
{ FLOW_LOG
	
	SetSelectionMarqueeFrameColor(FrameRed,	FrameGreen, FrameBlue, FrameOpacity);
	SetSelectionMarqueeBGColor(BG_Red, BG_Green, BG_Blue, BG_Opacity);
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
void AEDU_USER_HUD::DrawSelectionMarquee(const FVector2d& InitialPoint)
{ FLOW_LOG
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->GetDeltaSeconds(), FColor::Cyan, FString::Printf(TEXT("Drawing")));

	// TODO : InitialPoint can't be set on tick, it will be overwritten every frame.
	GetOwningPlayerController()->GetMousePosition(MousePos.X, MousePos.Y);

	DrawRect( // Draw Background
		BGColor,
		InitialPoint.X,
		InitialPoint.Y,
		CurrentPoint.X - InitialPoint.X,
		CurrentPoint.Y - InitialPoint.Y);

	DrawLine( // Top frame color
		InitialPoint.X, InitialPoint.Y,
		CurrentPoint.X, InitialPoint.Y,
		FrameColor,
		FrameThickness);

	DrawLine( // Bottom frame color
		InitialPoint.X, CurrentPoint.Y,
		CurrentPoint.X, CurrentPoint.Y,
		FrameColor,
		FrameThickness);

	DrawLine( // Left frame color
		InitialPoint.X, InitialPoint.Y,
		InitialPoint.X, CurrentPoint.Y,
		FrameColor,
		FrameThickness);

	DrawLine( // Right frame color
		CurrentPoint.X, InitialPoint.Y,
		CurrentPoint.X, CurrentPoint.Y,
		FrameColor,
		FrameThickness);
	
}

void AEDU_USER_HUD::SetSelectionMarqueeFrameColor(float Red, float Green, float Blue, float Opacity)
{

}

void AEDU_USER_HUD::SetSelectionMarqueeBGColor(float Red, float Green, float Blue, float Opacity)
{
	FrameColor.R = FrameRed;
	FrameColor.G = FrameGreen;
	FrameColor.B = FrameBlue;
	FrameColor.A = FrameOpacity;
}
