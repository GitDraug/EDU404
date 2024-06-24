// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Engine/DataAsset.h"
#include "EDU_USER_CameraPawnInputDataAsset.generated.h"

class UInputMappingContext;
class UInputAction;

/*------------------------------------------------------------------------------
  This DataAsset holds the Data and functionality for the CameraPawn.

  This is nat a PrimaryDataAsset, so it's ALWAYS loaded, but with such
  a small Data Asset it really doesn't matter.
------------------------------------------------------------------------------*/
UCLASS()
class EDU_USER_API UEDU_USER_CameraPawnInputDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Input Control Parameters
//------------------------------------------------------------------------------
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Move")
	float KeyMoveSpeed = 100.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Move")
	float KeyMoveSpeedMultiplier = 2.f;

	//------------------------------------------------------------------------------
	// Input Control Parameters: Mouse Move Scroll
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Edge Scroll")
	float MouseMoveSpeed = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Edge Scroll")
	float MouseMoveSpeedMultiplier = 2.f;
	
	//------------------------------------------------------------------------------
	// Input Control Parameters: Edge Scroll
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Edge Scroll")
	float EdgeScrollSpeed = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Edge Scroll")
	float EdgeScrollSpeedMultiplier = 2.f;
	
	//------------------------------------------------------------------------------
	// Input Control Parameters: Rotation
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Rotation")
	float RotationSpeed = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Rotation")
	float RotationSpeedMultiplier = 2.f;
	
	//------------------------------------------------------------------------------
	// Input Control Parameters: Pitch
	//------------------------------------------------------------------------------
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Pitch")
	float StartPitch = 35.f; // We are looking down on the units, so make sure this is negative
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Pitch")
	float PitchMin = -90.f; // We are looking down on the units, so make sure this is negative

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Pitch")
	float PitchMax = -15.f; // We are looking down on the units, so make sure this is negative

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Pitch")
	float PitchSpeed = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Pitch")
	float PitchSpeedMultiplier = 2.f;
	
	//------------------------------------------------------------------------------
	// Input Control Parameters: Zoom
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Zoom")
	float StartZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Zoom")
	float MinZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Zoom")
	float MaxZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Zoom")
	float ZoomDistance = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Zoom")
	float ZoomDistanceMultiplier = 2.f;

	//------------------------------------------------------------------------------
	// Input Control Parameters: Interpolation Speed
	//------------------------------------------------------------------------------
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Interpolation Speed")
	float MoveInterSpeed = 10.f ;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Interpolation Speed")
	float ZoomInterpSpeed = 10.f ;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Interpolation Speed")
	float RotationInterpSpeed = 10.f ;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Interpolation Speed")
	float PitchInterpSpeed = 10.f ;
	
//------------------------------------------------------------------------------
// Default Player Mapping Context
//------------------------------------------------------------------------------
public:
	/*--------------------------- Enhanced Input -----------------------------
	  UInputAction pointers are bound to C++ functions in the pawn using
	  EnhancedInputComponent->BindAction.
	  
	  The Mapping Context (in the Editor) binds BP Input Actions to
	  keyboard keys.

	  The BP Input Actions in the editor are actually stand-alone. They act as
	  extensions of the triggers in the C++ file that binds C++ functionality
	  for pointers in this file.
	  
	  The UInputAction pointers in this class are bound to BP through
	  a DataAsset in the editor inherited from this DataAsset. The DataAsset
	  is the most important, without it C++ can't connect to BP.
	--------------------------------------------------------------------------*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Default")
	UInputMappingContext* MappingContext_Default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Default")
	int32 MapPriority_Default;

	//------------------------------------------------------------------------------
	// Movement
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Movement")
	UInputAction* KeyMove; // Moving the camera with keyboard.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Movement")
	UInputAction* CameraTraceMove; // Moving the camera with Mouse.
		
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Movement")
	UInputAction* MouseMove_Pressed; // Moving the camera with Mouse.
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Movement")
	UInputAction* MouseMove_Released; // Moving the camera with Mouse.
	
	//------------------------------------------------------------------------------
	// FreeLook
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|FreeLook")
	UInputAction* FreeLook_Pressed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|FreeLook")
	UInputAction* FreeLook_Released;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|FreeLook")
	UInputAction* FreeLook;

	//------------------------------------------------------------------------------
	// Zoom
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Zoom")
	UInputAction* Zoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Rotation")
	UInputAction* KeyRotate; // Rotating the camera with keyboard.

	//------------------------------------------------------------------------------
	// Mouse Input
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Left Mouse button")
	UInputAction* Mouse_1_Pressed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Left Mouse button")
	UInputAction* Mouse_1_Released;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Right Mouse button")
	UInputAction* Mouse_2_Pressed;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Right Mouse button")
	UInputAction* Mouse_2_Released;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Middle Mouse button")
	UInputAction* Mouse_3_Pressed;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Middle Mouse button")
	UInputAction* Mouse_3_Released;

	//------------------------------------------------------------------------------
	// Modifier Keys
	//------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Modifier Keys")
	UInputAction* Mod_1_Pressed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Modifier Keys")
	UInputAction* Mod_1_Released;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Modifier Keys")
	UInputAction* Mod_2_Pressed;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Modifier Keys")
	UInputAction* Mod_2_Released;
		
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Modifier Keys")
	UInputAction* Mod_3_Pressed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Modifier Keys")
	UInputAction* Mod_3_Released;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Modifier Keys")
	UInputAction* Mod_4_Pressed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mapping Context|Modifier Keys")
	UInputAction* Mod_4_Released;
};