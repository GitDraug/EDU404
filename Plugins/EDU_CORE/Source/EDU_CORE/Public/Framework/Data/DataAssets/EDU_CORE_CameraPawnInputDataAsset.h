// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Engine/DataAsset.h"
#include "EDU_CORE_CameraPawnInputDataAsset.generated.h"

class UInputMappingContext;
class UInputAction;

/*------------------------------------------------------------------------------
  This DataAsset holds the Data and functionality for the CameraPawn.

  This is not a PrimaryDataAsset, so it's ALWAYS loaded, but with such
  a small Data Asset it really doesn't matter.
------------------------------------------------------------------------------*/
UCLASS()
class EDU_CORE_API UEDU_CORE_CameraPawnInputDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Input Control Parameters
//------------------------------------------------------------------------------
public:
	//------------------------------------------------------------------------------
	// Input Control Parameters: Mouse Click
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Mouse Click")
	float DoubleClickDelay = 0.25f;
	
	//------------------------------------------------------------------------------
	// Input Control Parameters: Keyboard Movement
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Move")
	float KeyMoveSpeed = 100.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Move")
	float KeyMoveSpeedMultiplier = 2.f;

	//------------------------------------------------------------------------------
	// Input Control Parameters: Mouse Move Scrolling
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Edge Scroll")
	float MouseMoveSpeed = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Edge Scroll")
	float MouseMoveSpeedMultiplier = 2.f;
	
	//------------------------------------------------------------------------------
	// Input Control Parameters: Edge Scrolling
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Edge Scroll")
	float ScreenEdgeArea = 5.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Edge Scroll")
	float EdgeScrollSpeed = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Edge Scroll")
	float EdgeScrollSpeedMultiplier = 2.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Edge Scroll")
	float MouseDragSpeed = 1.f;
	
	//------------------------------------------------------------------------------
	// Input Control Parameters: Rotation
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Rotation")
	float RotationSpeed = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Rotation")
	float RotationSpeedMultiplier = 2.f;
	
	//------------------------------------------------------------------------------
	// Input Control Parameters: Pitch
	//------------------------------------------------------------------------------
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Pitch")
	float StartPitch = 35.f; // We are looking down on the units, so make sure this is negative
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Pitch")
	float PitchMin = -90.f; // We are looking down on the units, so make sure this is negative

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Pitch")
	float PitchMax = -15.f; // We are looking down on the units, so make sure this is negative

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Pitch")
	float PitchSpeed = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Pitch")
	float PitchSpeedMultiplier = 2.f;
	
	//------------------------------------------------------------------------------
	// Input Control Parameters: Zoom
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Zoom")
	float StartZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Zoom")
	float MinZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Zoom")
	float MaxZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Zoom")
	float ZoomDistance = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Zoom")
	float ZoomDistanceMultiplier = 2.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Zoom")
	float ZoomTraceLength = 100000.f;

	//------------------------------------------------------------------------------
	// Input Control Parameters: Interpolation Speed
	//------------------------------------------------------------------------------
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Interpolation Speed")
	float MoveInterSpeed = 5.f ;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Interpolation Speed")
	float ZoomInterpSpeed = 5.f ;
	
	// Mouse input can accelerate speed, so rotation and pitch interp should always be higher than the speed, preferably x2 or more.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Interpolation Speed")
	float RotationInterpSpeed = 10.f ;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters  |  Interpolation Speed")
	float PitchInterpSpeed = 10.f ;
	
//------------------------------------------------------------------------------
// Mapping Actions
//------------------------------------------------------------------------------
public:
	/*--------------------------- Enhanced Input -----------------------------
	  UInputAction pointers are bound to C++ functions in the pawn using
	  EnhancedInputComponent->BindAction.
	  
	  The Mapping Context (in the Editor) binds BP Input Actions to
	  keyboard keys. In this project, all mapping contexts (keybindings) are
	  set in the Controller. Pawns instead manage their own, internal
	  functionality, but not keybindings.

	  It is possible to have overlapping and shared contexts, but from my [Draug]
	  experience, it's a bad idea. It's extra work to jump around and make
	  sure overlapping contexts work well together instead of separation
	  of concerns, even if it involves some code duplication.

	  The BP Input Actions in the editor are actually stand-alone. They act as
	  extensions of the triggers in the C++ file that binds C++ functionality
	  for pointers in this file.
	  
	  The UInputAction pointers in this class are bound to BP through
	  a DataAsset in the editor inherited from this DataAsset. The derived DataAsset
	  in the Editor is the most important, without it C++ can't connect to BP.
	--------------------------------------------------------------------------*/
	
	//-----------------------------------------------------------------------------
	// Movement
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Movement")
	TObjectPtr<UInputAction> KeyMove; // Moving the camera with keyboard.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Movement")
	TObjectPtr<UInputAction> MouseDragToggle; // Enables Mouse Drag
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Movement")
	TObjectPtr<UInputAction> AutoScrollToggle; // Enables Mouse Drag
	
	//------------------------------------------------------------------------------
	// FreeLook
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | FreeLook")
	TObjectPtr<UInputAction> FreeLookToggle;

	//------------------------------------------------------------------------------
	// Zoom
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Zoom")
	TObjectPtr<UInputAction> Zoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Rotation")
	TObjectPtr<UInputAction> KeyRotate; // Rotating the camera with keyboard.

	//------------------------------------------------------------------------------
	// Mouse Input
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Left Mouse button")
	TObjectPtr<UInputAction> Mouse_1;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Right Mouse button")
	TObjectPtr<UInputAction> Mouse_2;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Middle Mouse button")
	TObjectPtr<UInputAction> Mouse_3;

	//------------------------------------------------------------------------------
	// Modifier Input Actions
	//------------------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Modifier Keys")
	TObjectPtr<UInputAction> Mod_1;

	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Modifier Keys")
	TObjectPtr<UInputAction> Mod_2;
		
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Modifier Keys")
	TObjectPtr<UInputAction> Mod_3;

	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Modifier Keys")
	TObjectPtr<UInputAction> Mod_4;
	
	//---------------------------------------------------------------------------
	// CTRL Group Assignment
	//---------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_4;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_6;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_7;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_8;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_9;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Numeric Keys")
	TObjectPtr<UInputAction> Numeric_0;

	//---------------------------------------------------------------------------
	// Shortcuts
	//---------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context | Shortcuts")
	TObjectPtr<UInputAction> SelectAll;
};