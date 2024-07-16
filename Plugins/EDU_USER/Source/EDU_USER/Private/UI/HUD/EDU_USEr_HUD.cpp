// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/EDU_USER_HUD.h"

#include "Framework/Data/FlowLog.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_USER_HUD::AEDU_USER_HUD(const FObjectInitializer& ObjectInitializer) : Super (ObjectInitializer)
{ FLOW_LOG
	
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
void AEDU_USER_HUD::DrawHUD()
{ FLOW_LOG_TICK // Needed?
	// Super::DrawHUD();
	if(!bSelectionTriggered) { return; }

	// Update CurrentMousePosition.
	GetOwningPlayerController()->GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);

	DrawRect( // Draw Background
		BGColor,
		InitialMousePos.X,
		InitialMousePos.Y,
		CurrentMousePos.X - InitialMousePos.X,
		CurrentMousePos.Y - InitialMousePos.Y);

	DrawLine( // Top frame color
		InitialMousePos.X, InitialMousePos.Y,
		CurrentMousePos.X, InitialMousePos.Y,
		FrameColor,
		FrameThickness);

	DrawLine( // Bottom frame color
		InitialMousePos.X, CurrentMousePos.Y,
		CurrentMousePos.X, CurrentMousePos.Y,
		FrameColor,
		FrameThickness);

	DrawLine( // Left frame color
		InitialMousePos.X, InitialMousePos.Y,
		InitialMousePos.X, CurrentMousePos.Y,
		FrameColor,
		FrameThickness);

	DrawLine( // Right frame color
		CurrentMousePos.X, InitialMousePos.Y,
		CurrentMousePos.X, CurrentMousePos.Y,
		FrameColor,
		FrameThickness);
}

void AEDU_USER_HUD::DrawSelectionMarquee(const FVector2d& MousePosition)
{ FLOW_LOG
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->GetDeltaSeconds(), FColor::Cyan, FString::Printf(TEXT("Drawing")));
	InitialMousePos = MousePosition;
	bSelectionTriggered = true;
}

void AEDU_USER_HUD::StopDrawingSelectionMarquee()
{ FLOW_LOG
	bSelectionTriggered = false;
}

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
void AEDU_USER_HUD::SetSelectionMarqueeFrameColor(float Red, float Green, float Blue, float Opacity)
{ FLOW_LOG
	BGColor.R = Red;
	BGColor.G = Green;
	BGColor.B = Blue;
	BGColor.A = Opacity;
}

void AEDU_USER_HUD::SetSelectionMarqueeBGColor(float Red, float Green, float Blue, float Opacity)
{ FLOW_LOG
	FrameColor.R = Red;
	FrameColor.G = Green;
	FrameColor.B = Blue;
	FrameColor.A = Opacity;
}
