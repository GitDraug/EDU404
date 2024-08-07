// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EDU_CORE_ControllerInputDataAsset.generated.h"

class UInputMappingContext;
class UInputAction;

/*------------------------------------------------------------------------------
  This DataAsset holds the Data and functionality for the Controller.

  This is not a PrimaryDataAsset, so it's ALWAYS loaded, but with such
  a small Data Asset it really doesn't matter.
------------------------------------------------------------------------------*/
UCLASS()
class EDU_CORE_API UEDU_CORE_ControllerInputDataAsset : public UDataAsset
{
	GENERATED_BODY()
//------------------------------------------------------------------------------
// Default Player Mapping Context
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
	
	// Default Controller (Shared) Context
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Default Controller")
	TObjectPtr<UInputMappingContext> ControllerInputContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context|Default Controller")
	int32 MappingContextPriority_Controller = 0;
	
	// Default Camera Context
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Default Controller")
	TObjectPtr<UInputMappingContext> CameraInputContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context|Default Controller")
	int32 MappingContextPriority_Camera = 1;

	// Default Character Context
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Default Controller")
	TObjectPtr<UInputMappingContext> CharacterInputContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context|Default Controller")
	int32 MappingContextPriority_Character = 1;
	
};