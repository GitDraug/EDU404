// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Engine/DataAsset.h"
#include "EDU_USER_ControllerInputDataAsset.generated.h"

class UInputMappingContext;
class UInputAction;

/*------------------------------------------------------------------------------
  This DataAsset holds the Data and functionality for the Controller.

  This is not a PrimaryDataAsset, so it's ALWAYS loaded, but with such
  a small Data Asset it really doesn't matter.
------------------------------------------------------------------------------*/
UCLASS()
class EDU_USER_API UEDU_USER_ControllerInputDataAsset : public UDataAsset
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
	  keyboard keys. In this plugin, all mapping contexts (keybindings) are
	  set in the Controller. Pawns instead manage their own, internal
	  functionality, but not keybindings.

	  The BP Input Actions in the editor are actually stand-alone. They act as
	  extensions of the triggers in the C++ file that binds C++ functionality
	  for pointers in this file.
	  
	  The UInputAction pointers in this class are bound to BP through
	  a DataAsset in the editor inherited from this DataAsset. The DataAsset
	  is the most important, without it C++ can't connect to BP.
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
		
	//------------------------------------------------------------------------------
	// Modifier Input Actions
	//------------------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Modifier Keys")
	UInputAction* Mod_1;

	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Modifier Keys")
	UInputAction* Mod_2;
		
	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Modifier Keys")
	UInputAction* Mod_3;

	UPROPERTY(EditDefaultsOnly, Category = "Mapping Context | Modifier Keys")
	UInputAction* Mod_4;
};