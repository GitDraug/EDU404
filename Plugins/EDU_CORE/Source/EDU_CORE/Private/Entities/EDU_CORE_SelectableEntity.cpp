// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/EDU_CORE_SelectableEntity.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"

//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------

AEDU_CORE_SelectableEntity::AEDU_CORE_SelectableEntity(const FObjectInitializer& ObjectInitializer)
{ FLOW_LOG
	PrimaryActorTick.bCanEverTick = false;
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
void AEDU_CORE_SelectableEntity::MouseHighlightActor()
{ FLOW_LOG
	bMouseHighlighted = true;
}

void AEDU_CORE_SelectableEntity::MouseUnHighlightActor()
{ FLOW_LOG
	bMouseHighlighted = false;
}

void AEDU_CORE_SelectableEntity::RectangleHighlightActor()
{ FLOW_LOG
	bRectangleHighlighted = true;
}

void AEDU_CORE_SelectableEntity::UnRectangleHighlightActor()
{ FLOW_LOG
	bRectangleHighlighted = false;
}

void AEDU_CORE_SelectableEntity::SelectActor()
{ FLOW_LOG
	bSelected = true;
}

void AEDU_CORE_SelectableEntity::UnSelectActor()
{ FLOW_LOG
	bSelected = false;
}