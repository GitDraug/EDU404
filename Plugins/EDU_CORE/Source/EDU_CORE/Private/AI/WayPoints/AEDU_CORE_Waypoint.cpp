// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/WayPoints/EDU_CORE_Waypoint.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_Waypoint::AEDU_CORE_Waypoint()
{
	// We don't do normal Selection with Waypoints.
	bCanBeSelected = false;
}

void AEDU_CORE_Waypoint::RectangleHighlightActor()
{
	//Super::RectangleHighlightActor();
}

void AEDU_CORE_Waypoint::UnRectangleHighlightActor()
{
	//Super::UnRectangleHighlightActor();
}

//------------------------------------------------------------------------------
// Network Functionality
//------------------------------------------------------------------------------