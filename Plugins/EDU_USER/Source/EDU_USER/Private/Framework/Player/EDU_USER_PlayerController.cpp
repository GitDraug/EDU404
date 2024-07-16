// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Player/EDU_USER_PlayerController.h"
#include "Framework/Data/EDU_USER_ControllerInputDataAsset.h"
#include "Framework/Data/FlowLog.h"

// External: These needs to be added in the .uplugin file too!
#include "Interfaces/EDU_UNIT_Selectable.h"
#include "UObject/ScriptInterface.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Data/EDU_USER_DataTypes.h"

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------

AEDU_USER_PlayerController::AEDU_USER_PlayerController(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{ FLOW_LOG
	bReplicates = true;
}

void AEDU_USER_PlayerController::OnPossess(APawn* InPawn)
{ FLOW_LOG
	Super::OnPossess(InPawn);
}

void AEDU_USER_PlayerController::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();

	// Make sure this pointer is initiated.
	if(!InputSubSystem) { SetupInputSubSystem(); }
	
	FInputModeGameAndUI InputModeData; // Settings container
	InputModeData.SetHideCursorDuringCapture(false); // Whether to hide the cursor during temporary mouse capture caused by a mouse down
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // Don't lock to window. It's better to put this in a menu.
	SetShowMouseCursor(true);
}

void AEDU_USER_PlayerController::PlayerTick(float DeltaTime)
{ FLOW_LOG_TICK
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
}

//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------

void AEDU_USER_PlayerController::CursorTrace()
{ FLOW_LOG_TICK
	
	// TODO: It would be best to make a custom CollisionProfile or channel for only selectables.
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	if(!CursorHit.bBlockingHit) { return; }

	/*---------------- Pointers for Selectable interface in UNIT plugin ------------
	  TScriptInterface allows us to do some abstract sugar; we don't need to cast
	  CursorHit.GetActor() to check it it can be molded into the interface class
	  we use, it's checked automagically. =)
	------------------------------------------------------------------------------*/
	LastActor = CurrentActor;
	CurrentActor = CursorHit.GetActor();

	/*------------------------------------------------------------------------------
	  Line Trace from Cursor, possible scenarios:
		  1. LastActor == null && CurrentActor && null
				Do Nothing.
		  2. LastActor == null && CurrentActor is valid
				Highlight CurrentActor
		  3. LastActor is valid && CurrentActor && null
				Unhighlight LastActor
		  4. Both actors are valid, but LastActor != CurrentActor
				Unhighlight LastActor, Highlight CurrentActor
		  5. Both actors are valid, and LastActor == CurrentActor
				Do nothing.
	------------------------------------------------------------------------------*/
	if(LastActor == nullptr)
	{
		if(CurrentActor == nullptr)	{ } // Scenario 1; Do Nothing.
		else { CurrentActor->HighlightActor(); } // Scenario 2: Highlight CurrentActor
	}
	else
	{   
		if(CurrentActor == nullptr) 
		{
			// Scenario 3: LastActor is valid && CurrentActor && null
			LastActor->UnHighlightActor();
		}
		else //  Both actors are valid...
		{
			if(LastActor != CurrentActor) // ...but LastActor != CurrentActor
			{  // Scenario 4:
				LastActor->UnHighlightActor();
				CurrentActor->HighlightActor();
			}
			// Else:Both actors are valid, and LastActor == CurrentActor
			// Scenario 5: Do nothing.
		}
	}
}
	
//------------------------------------------------------------------------------
// Input
//------------------------------------------------------------------------------

void AEDU_USER_PlayerController::SetupInputComponent()
{ FLOW_LOG
	// Allows the PlayerController to set up custom input bindings.
	Super::SetupInputComponent();

	/*------------------------------------------------------------------------------
	  InputDataAsset is NOT a PrimaryDataAsset, so it's ALWAYS loaded. With such
	  a small Data Asset, it really doesn't matter, but we need to ensure it's
	  not null to avoid crashes.

	  With PrimaryDataAssets, you always need to call the Asset Manager to load
	  them manually.

	  This needs to be assigned in the BP, so it's initialized, else it will be null.
	------------------------------------------------------------------------------*/
	if(ImportedInputDataAsset == nullptr) { FLOW_LOG_ERROR("InputDataAsset is null, make sure it is set in the BluePrint Controller in the Editor.") return; }

	InputDataAsset = Cast<UEDU_USER_ControllerInputDataAsset>(ImportedInputDataAsset);
	if(InputDataAsset == nullptr) { FLOW_LOG_ERROR("Something is wrong with the InputData, check the Cast.") return; }

	
	// The InputComponent pointer lives in Actor.h, but we need the derived UEnhancedInputComponent pointer
	if(!EnhancedInputComponent)
	{
		EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

		// Test if the cast worked correctly.
		if(!EnhancedInputComponent)
		{
			FLOW_LOG_ERROR("EnhancedInputComponent failed, retrying...")
			SetupInputComponent();
			return;
		}
		
		FLOW_LOG_WARNING("EnhancedInputComponent Succesfully Cast and Stored")		
	}
	
	//------------------------------------------------------------------------------
	// Modifier Keys : 
	//------------------------------------------------------------------------------
	EnhancedInputComponent->BindAction(InputDataAsset->Mod_1, ETriggerEvent::Started, this, &ThisClass::Input_Mod_1_Pressed);
	EnhancedInputComponent->BindAction(InputDataAsset->Mod_1, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_1_Released);

	EnhancedInputComponent->BindAction(InputDataAsset->Mod_2, ETriggerEvent::Started, this, &ThisClass::Input_Mod_2_Pressed);
	EnhancedInputComponent->BindAction(InputDataAsset->Mod_2, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_2_Released);

	EnhancedInputComponent->BindAction(InputDataAsset->Mod_3, ETriggerEvent::Started, this, &ThisClass::Input_Mod_3_Pressed);
	EnhancedInputComponent->BindAction(InputDataAsset->Mod_3, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_3_Released);

	EnhancedInputComponent->BindAction(InputDataAsset->Mod_4, ETriggerEvent::Started, this, &ThisClass::Input_Mod_4_Pressed);
	EnhancedInputComponent->BindAction(InputDataAsset->Mod_4, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_4_Released);
	
	LoadKeyMappings();
}

void AEDU_USER_PlayerController::LoadKeyMappings()
{ FLOW_LOG
	if(!InputDataAsset) { SetupInputComponent(); return; }

	//TODO: Add saved option.
	// if(SavedUserIinput)
	// ControllerInputContext = SavedControllerInputContext;
	// return;
	
	// Load Default CameraPawnInputContext from DataAsset
	ControllerInputContext = InputDataAsset->ControllerInputContext;
	CameraInputContext = InputDataAsset->CameraInputContext;
	CharacterInputContext = InputDataAsset->CharacterInputContext;
}

void AEDU_USER_PlayerController::SetupInputSubSystem()
{ FLOW_LOG
	/*--------------------------------------------------------------------------------------
	  In Unreal Engine, both UEnhancedInputLocalPlayerSubsystem and UEnhancedInputComponent
	  are part of the Enhanced Input system, but they serve different purposes and are used
	  in different contexts.
	  
	  UEnhancedInputLocalPlayerSubsystem is higher up in the hierarchy and manages the
	  input contexts (KeyMappings).
	  
	  UEnhancedInputComponent works as a component on an actor to bind Input Actions to
	  functions.
	--------------------------------------------------------------------------------------*/
	if(UEnhancedInputLocalPlayerSubsystem* InputSubsystemPtr = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubSystem = InputSubsystemPtr;
		
		if(!InputSubSystem)
		{
			FLOW_LOG_WARNING("SetupEnhancedInput failed, retrying.");
			SetupInputSubSystem();
		}
		
		FLOW_LOG_WARNING("InputSubSystem is good To go");
		return;
	}

	FLOW_LOG_ERROR("UEnhancedInputLocalPlayerSubsystem is not working, is module included?");
}

void AEDU_USER_PlayerController::SetMappingContext(EEDU_USER_CurrentPawn Context)
{ FLOW_LOG
	switch (Context) {
case EEDU_USER_CurrentPawn::None:
		FLOW_LOG_WARNING("Switching to Controller")
		AddInputMappingContext(ControllerInputContext, 0);
		break;
		
	case EEDU_USER_CurrentPawn::Camera:
		FLOW_LOG_WARNING("Switching to Camera")
		RemoveInputMappingContext(CharacterInputContext);
		AddInputMappingContext(CameraInputContext, 0);

		break;
	case EEDU_USER_CurrentPawn::Character:
		FLOW_LOG_WARNING("Switching to Character")
		RemoveInputMappingContext(CameraInputContext);
		AddInputMappingContext(CharacterInputContext, 0);
		break;
	}
}

void AEDU_USER_PlayerController::AddInputMappingContext(const UInputMappingContext* InputMappingContext, const int32 MappingPriority)
{ FLOW_LOG
	if(InputMappingContext == nullptr) { FLOW_LOG_ERROR("InputMappingContext == nullptr, make sure KeyMappings are properly loaded.") return; }
	if(!InputSubSystem) { SetupInputSubSystem(); }
	
	if(!InputSubSystem->HasMappingContext(InputMappingContext))
	{
		InputSubSystem->AddMappingContext(InputMappingContext, MappingPriority);
		FLOW_LOG_WARNING("InputMappingContext Added.")
	}
}

void AEDU_USER_PlayerController::RemoveInputMappingContext(const UInputMappingContext* InputMappingContext)
{ FLOW_LOG
	if(!InputSubSystem) { SetupInputSubSystem(); }
	
	if(InputSubSystem->HasMappingContext(InputMappingContext))
	{
		InputSubSystem->RemoveMappingContext(InputMappingContext);
		FLOW_LOG_WARNING("InputMappingContext Removed.")
	}
}

//------------------------------------------------------------------------------
// Functionality: Modifier Keys
//------------------------------------------------------------------------------

void AEDU_USER_PlayerController::Input_Mod_1_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_1 = true;
	SetModifierKey();
}

void AEDU_USER_PlayerController::Input_Mod_1_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_1 = false;
	SetModifierKey();
}

void AEDU_USER_PlayerController::Input_Mod_2_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_2 = true;
	SetModifierKey();
}

void AEDU_USER_PlayerController::Input_Mod_2_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_2 = false;
	SetModifierKey();
}

void AEDU_USER_PlayerController::Input_Mod_3_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_3 = true;
	SetModifierKey();
}

void AEDU_USER_PlayerController::Input_Mod_3_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_3 = false;
	SetModifierKey();
}

void AEDU_USER_PlayerController::Input_Mod_4_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_4 = true;
	SetModifierKey();
}

void AEDU_USER_PlayerController::Input_Mod_4_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_4 = false;
	SetModifierKey();
}

void AEDU_USER_PlayerController::SetModifierKey()
{ FLOW_LOG
	/*--------------------------------------------------------------------------------
	  Note that this evaluation will start over every time a new key is pressed,
	  I tried using a switch, but switches can't be used with structs unless
	  overloaded, and it's not worth the headache.

	  I don't like nested ifs, but since this is only called on key press,
	  it's really not that performance heavy.  // Draug
	--------------------------------------------------------------------------------*/

	using enum EEDU_USER_InputModifierKey; //  for readability
	
	//----------------------------------------------------------
	// Shift
	//-----------------------------------------------------------
	if (bMod_1)
	{
		ModifierKey = Mod_1;
		FLOW_LOG_WARNING("Mod Key = 1")
	
		if (bMod_1 && bMod_2){ ModifierKey = Mod_12; FLOW_LOG_WARNING("Mod Key = 12")	}
		if (bMod_1 && bMod_3){ ModifierKey = Mod_13; FLOW_LOG_WARNING("Mod Key = 13") }
		if (bMod_1 && bMod_4){ ModifierKey = Mod_14; FLOW_LOG_WARNING("Mod Key = 14") }

		if (bMod_1 && bMod_2 && bMod_3){ ModifierKey = Mod_123; FLOW_LOG_WARNING("Mod Key = 123") }
		if (bMod_1 && bMod_2 && bMod_4){ ModifierKey = Mod_124; FLOW_LOG_WARNING("Mod Key = 124") }
		if (bMod_1 && bMod_3 && bMod_4){ ModifierKey = Mod_134;	FLOW_LOG_WARNING("Mod Key = 134") }

		return;
	}

	//-----------------------------------------------------------
	// Ctrl
	//-----------------------------------------------------------
	if (bMod_2)
	{
		ModifierKey = Mod_2; FLOW_LOG_WARNING("ModifierKey = 2")
		
		if (bMod_2 && bMod_3){ ModifierKey = Mod_23; FLOW_LOG_WARNING("Mod Key = 23") }
		if (bMod_2 && bMod_4){ ModifierKey = Mod_24; FLOW_LOG_WARNING("Mod Key = 24") }
		
		if (bMod_2 && bMod_3 && bMod_4){ ModifierKey = Mod_234; FLOW_LOG_WARNING("Mod Key = 234") }
		return;
	}

	//-----------------------------------------------------------
	// Alt
	//-----------------------------------------------------------
	if (bMod_3)
	{
		ModifierKey = Mod_3; FLOW_LOG_WARNING("Mod Key = 3")
		
		if (bMod_3 && bMod_4){ ModifierKey = Mod_34; FLOW_LOG_WARNING("Mod Key = 34")	}
		return;
	}
	
	//-----------------------------------------------------------
	// Space
	//-----------------------------------------------------------
	if (bMod_4)
	{
		ModifierKey = Mod_4; FLOW_LOG_WARNING("Mod Key = 4")
		return;
	}

	//-----------------------------------------------------------
	// No Modifier
	//-----------------------------------------------------------
	ModifierKey = NoModifier;
	FLOW_LOG_WARNING("Mod Key = NoModifier")
	
	// end using enum EEDU_USER_InputModifierKey;
}

