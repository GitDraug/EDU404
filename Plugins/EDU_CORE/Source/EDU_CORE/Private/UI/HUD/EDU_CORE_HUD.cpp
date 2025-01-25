// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/EDU_CORE_HUD.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_UI.h"

#include "EngineUtils.h"

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
	GEngine->AddOnScreenDebugMessage(4, GetWorld()->DeltaTimeSeconds, FColor::Blue, FString::Printf(TEXT("Elements in SelectionRectangleArray: %d "), HUDSelectionArray.Num()));
	
	/*--------------------------------------------------------------------------
	  This might seem silly, but FBox2D SelectionRectangle(ForceInit), which
	  is used for drawing a rectangle, can only be called from the DrawHUD()
	  function. So it has to be done here.
	--------------------------------------------------------------------------*/
	if(bSearchWholeScreen)
	{
		GetOwningPlayerController()->GetViewportSize(ScreenSize.X, ScreenSize.Y);
		
		DetectEntitiesInSelectionRectangle(
			SearchFilter,
			FVector2d(0.f,0.f),
			FVector2D (static_cast<float>(ScreenSize.X),static_cast<float>(ScreenSize.Y)),
			HUDSelectionArray
		);

		// Reset
		bSearchWholeScreen = false;
		return;
	}
	
	if(!bDrawRectangle) { return; }
	// Update CurrentMousePosition.
	GetOwningPlayerController()->GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);

	// Only start drawing if we're actually moving the mouse.
	if(InitialMousePos == CurrentMousePos) { return; }

	DrawRect( // Draw Background
		BGColor,
		InitialMousePos.X,
		InitialMousePos.Y,
		CurrentMousePos.X - InitialMousePos.X,	// Width
		CurrentMousePos.Y - InitialMousePos.Y);	// Height

	// Top and bottom frame lines
	DrawLine(InitialMousePos.X, InitialMousePos.Y, CurrentMousePos.X, InitialMousePos.Y, FrameColor, FrameThickness);  // Top
	DrawLine(InitialMousePos.X, CurrentMousePos.Y, CurrentMousePos.X, CurrentMousePos.Y, FrameColor, FrameThickness);  // Bottom

	// Left and right frame lines
	DrawLine(InitialMousePos.X, InitialMousePos.Y, InitialMousePos.X, CurrentMousePos.Y, FrameColor, FrameThickness);  // Left
	DrawLine(CurrentMousePos.X, InitialMousePos.Y, CurrentMousePos.X, CurrentMousePos.Y, FrameColor, FrameThickness);  // Right
	
	DetectEntitiesInSelectionRectangle(
		AEDU_CORE_SelectableEntity::StaticClass(),
		InitialMousePos,
		CurrentMousePos,
		HUDSelectionArray
	);
}

void AEDU_CORE_HUD::DetectEntitiesInSelectionRectangle(const TSubclassOf<class AEDU_CORE_SelectableEntity>& ClassFilter, const FVector2D& FirstPoint, const FVector2D& SecondPoint, TArray<AEDU_CORE_SelectableEntity*>& OutEntityArray) const
{
	// Define the selection rectangle
	FBox2D SelectionRectangle(ForceInit);
	SelectionRectangle += FirstPoint;
	SelectionRectangle += SecondPoint;

	for (TActorIterator<AEDU_CORE_SelectableEntity> Itr(GetWorld(), ClassFilter); Itr; ++Itr)
	{
		AEDU_CORE_SelectableEntity* Entity = *Itr;

		// Skip if the entity is invalid or cannot be selected
		if (!Entity || !Entity->bCanBeRectangleSelected)
		{
			continue;
		}

		// Get the actor's origin (location in world space)
		const FVector ActorLocation = Entity->GetActorLocation();

		// Project the 3D location to 2D screen space
		const FVector2D ScreenPosition = FVector2D(Project(ActorLocation));

		// Check if the projected point is inside the selection rectangle
		if (SelectionRectangle.IsInside(ScreenPosition))
		{
			if(!Entity->bRectangleHighlighted)
			{
				OutEntityArray.AddUnique(Entity);
				Entity->RectangleHighlightActor();
			}
		}
		else // If Outside
		{
			if(Entity->bRectangleHighlighted)
			{
				Entity->UnRectangleHighlightActor();
				OutEntityArray.RemoveSingleSwap(Entity);
			}
		}
	}
}

void AEDU_CORE_HUD::DrawSelectionMarquee(const FVector2d& MousePosition)
{ FLOW_LOG
	InitialMousePos = MousePosition;
	bDrawRectangle = true;
}

void AEDU_CORE_HUD::StopDrawingSelectionMarquee()
{ FLOW_LOG
	bDrawRectangle = false;
}

void AEDU_CORE_HUD::DetectEntityUnderCursor(const FVector2d& MousePosition, const float& Distance)
{ FLOW_LOG
	// We don't want to select the entity under the cursor if we are already selecting units in a rectangle.
	if(HUDSelectionArray.Num() > 0) return;
	
	// CurrentMousePos is updated on tick while LMB is down.
	FVector WorldLocation, WorldDirection;
	if (!GetOwningPlayerController()->DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldLocation, WorldDirection))
	{
		return;
	}

	// Define the start and end points of the trace
	FVector TraceStart = WorldLocation;
	FVector TraceEnd = TraceStart + (WorldDirection * Distance);
	
	// Perform line trace from TraceStart to TraceEnd
	FHitResult CameraTraceResult;
	FCollisionQueryParams CameraTraceCollisionParams;

	// TODO: Make sure the ECC channel we're colliding with is set to query and block!
	if (GetWorld()->LineTraceSingleByChannel(CameraTraceResult, TraceStart, TraceEnd, ECC_Visibility, CameraTraceCollisionParams))
	{
		if (CameraTraceResult.bBlockingHit)
		{
			if (AEDU_CORE_SelectableEntity* EntityUnderCursor = Cast<AEDU_CORE_SelectableEntity>(CameraTraceResult.GetActor()))
			{
				if(EntityUnderCursor->bCanBeRectangleSelected)
				{
					HUDSelectionArray.AddUnique(EntityUnderCursor);
				}
			}
		}
	}
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

//------------------------------------------------------------------------------
// Deprecated
//------------------------------------------------------------------------------

/*/ Old Implementation of DetectEntitiesInSelectionRectangle That uses box detection
void AEDU_CORE_HUD::DetectEntitiesInSelectionRectangle(const TSubclassOf<class AEDU_CORE_SelectableEntity>& ClassFilter, const FVector2D& FirstPoint, const FVector2D& SecondPoint, TArray<AEDU_CORE_SelectableEntity*>& OutEntityArray) const
{ FLOW_LOG
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
        AEDU_CORE_SelectableEntity* Entity = *Itr;
    	
    	// Skip invalid or non-selectable entities
    	if (!Entity || !Entity->bCanBeSelected)
    	{
    		continue;
    	}

        // Get Pawn Bounds
		// const FBox EachEntityBounds = EachEntity->GetComponentsBoundingBox();
    	const FBox EachEntityBounds = Entity->GetComponentsBoundingBox();
    	
        // Center
        const FVector BoxCenter = EachEntityBounds.GetCenter();

        // Extents
        const FVector BoxExtents = EachEntityBounds.GetExtent() * 0.5f;

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
			if(!Entity->bRectangleHighlighted)
			{
				OutEntityArray.AddUnique(Entity);
				Entity->RectangleHighlightActor();
			}
		}
		else // If Outside
		{
			if(Entity->bRectangleHighlighted)
			{
				Entity->UnRectangleHighlightActor();
				OutEntityArray.Remove(Entity);
			}
		}
	}
}//*/

	