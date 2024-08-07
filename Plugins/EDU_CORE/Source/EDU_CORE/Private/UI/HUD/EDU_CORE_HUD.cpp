// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/EDU_CORE_HUD.h"
#include "EngineUtils.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_UI.h"
#include "Interfaces/EDU_CORE_Selectable.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_HUD::AEDU_CORE_HUD(const FObjectInitializer& ObjectInitializer) : Super (ObjectInitializer)
{ FLOW_LOG
	
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
void AEDU_CORE_HUD::DrawHUD()
{ // FLOW_LOG_TICK // Needed?
	// Super::DrawHUD();
	GEngine->AddOnScreenDebugMessage(4, GetWorld()->DeltaTimeSeconds, FColor::Blue, FString::Printf(TEXT("Elements in SelectionRectangleArray: %d "), SelectionRectangleArray.Num()));

	if(bSearchWholeScreen)
	{
		GetOwningPlayerController()->GetViewportSize(ScreenSize.X, ScreenSize.Y);
		
		DetectEntitiesInSelectionRectangle(
			SearchFilter,
			FVector2d(0.f,0.f),
			FVector2D (static_cast<float>(ScreenSize.X),static_cast<float>(ScreenSize.Y)),
			SelectionRectangleArray
		);

		// Reset
		bSearchWholeScreen = false;
	}
	
	if(!bMouseDraw) { return; }
	
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

	/*------------------------------------------------------------------------------
	  DetectEntitiesInSelectionRectangle() is a heavy process, involving getting
	  all actors in the entire world and filtering out everything not in the
	  rectangle. We don't need to do this every frame.

	  The FrameCounter measures frames passed since we last called the function.

	  TODO: It would even be better to filter out actors that don't use an interface
	  rather than basing is on selectable entity.
	------------------------------------------------------------------------------*/
	if(FrameCounter < 10) // 10 frames is still 3 times a second @ 30 FPS
	{
		FrameCounter++;
		return;
	}
	
	// We want to empty the SelectionArray an UnHighlight any actor not inside the SelectionRectangle.
	if(SelectionRectangleArray.Num() > 0)
	{
		for(AEDU_CORE_SelectableEntity* Entity : SelectionRectangleArray)
		{
			if(Entity)
			{
				Entity->UnRectangleHighlightActor();
			}
		}
	}
	
	DetectEntitiesInSelectionRectangle(
		AEDU_CORE_SelectableEntity::StaticClass(),
		InitialMousePos,
		CurrentMousePos,
		SelectionRectangleArray
	);
	
	FrameCounter = 0;
}


void AEDU_CORE_HUD::DetectEntitiesInSelectionRectangle(const TSubclassOf<class AEDU_CORE_SelectableEntity>& ClassFilter, const FVector2D& FirstPoint, const FVector2D& SecondPoint, TArray<AEDU_CORE_SelectableEntity*>& OutEntityArray) const
{ FLOW_LOG
    OutEntityArray.Reset();
	
    // Create Selection Rectangle from Points
    FBox2D SelectionRectangle(ForceInit);
    SelectionRectangle += FirstPoint;
    SelectionRectangle += SecondPoint;

    // The Actor Bounds Point Mapping
    const FVector BoundsPointMapping[8] =
    {
        FVector(1.f, 1.f, 1.f),
        FVector(1.f, 1.f, -1.f),
        FVector(1.f, -1.f, 1.f),
        FVector(1.f, -1.f, -1.f),
        FVector(-1.f, 1.f, 1.f),
        FVector(-1.f, 1.f, -1.f),
        FVector(-1.f, -1.f, 1.f),
        FVector(-1.f, -1.f, -1.f)
    };

	// For Each Pawn of the Class Filter Type
    for (TActorIterator Itr(GetWorld(), ClassFilter); Itr; ++Itr)
    {
        AEDU_CORE_SelectableEntity* EachEntity = *Itr;
    	
    	// Skip this entity and continue to the next if it is invalid
    	if(!EachEntity) { continue; }
    	if(!EachEntity->bCanBeSelected) {continue; }

        // Get Pawn Bounds
        const FBox EachEntityBounds = EachEntity->GetComponentsBoundingBox();

        // Center
        const FVector BoxCenter = EachEntityBounds.GetCenter();

        // Extents
        const FVector BoxExtents = EachEntityBounds.GetExtent();

        // Build 2D bounding box of pawn in screen space
        FBox2D ActorBox2D(ForceInit);
    	
        for (uint8 BoundsPointItr = 0; BoundsPointItr < 8; BoundsPointItr++)
        {
            // Project vert into screen space
            const FVector ProjectedWorldLocation = Project(BoxCenter + (BoundsPointMapping[BoundsPointItr] * BoxExtents), true);
            // Add to 2D bounding box if point is on the front side of the camera
            if (ProjectedWorldLocation.Z > 0.f)
            {
                ActorBox2D += FVector2D(ProjectedWorldLocation.X, ProjectedWorldLocation.Y);
            }
        }
        
        // Only consider pawn boxes that have valid points inside
        if (ActorBox2D.bIsValid && SelectionRectangle.Intersect(ActorBox2D))
        {        	
        	OutEntityArray.AddUnique(EachEntity);

        	// Highlight all Actors in the rectangle.
       		EachEntity->RectangleHighlightActor();
        }
    }
}

void AEDU_CORE_HUD::DrawSelectionMarquee(const FVector2d& MousePosition)
{ FLOW_LOG
	InitialMousePos = MousePosition;
	bMouseDraw = true;
	// DrawSelectionMarquee Uses a frame counter to lessen CPU load, reset it at mouse click.
	FrameCounter = 0;

}

void AEDU_CORE_HUD::StopDrawingSelectionMarquee()
{ FLOW_LOG
	bMouseDraw = false;
}

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
void AEDU_CORE_HUD::SetSelectionMarqueeFrameColor(float Red, float Green, float Blue, float Opacity)
{ FLOW_LOG
	BGColor.R = Red;
	BGColor.G = Green;
	BGColor.B = Blue;
	BGColor.A = Opacity;
}

void AEDU_CORE_HUD::SetSelectionMarqueeBGColor(float Red, float Green, float Blue, float Opacity)
{ FLOW_LOG
	FrameColor.R = Red;
	FrameColor.G = Green;
	FrameColor.B = Blue;
	FrameColor.A = Opacity;
}
