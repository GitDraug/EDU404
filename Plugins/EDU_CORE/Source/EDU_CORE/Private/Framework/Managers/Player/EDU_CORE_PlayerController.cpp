// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Player/EDU_CORE_PlayerController.h"
#include "Framework/Data/DataAssets/EDU_CORE_ControllerInputDataAsset.h"
#include "Framework/Data/FLOWLOG/FLOWLOG_PLAYER.h"

#include "UObject/ScriptInterface.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Entities/EDU_CORE_SelectableEntity.h"

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------

AEDU_CORE_PlayerController::AEDU_CORE_PlayerController(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{ FLOW_LOG
	bReplicates = true;
	
}

void AEDU_CORE_PlayerController::OnPossess(APawn* InPawn)
{ FLOW_LOG
	Super::OnPossess(InPawn);
}

void AEDU_CORE_PlayerController::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();

	// Make sure this pointer is initiated.
	if(!InputSubSystem) { SetupInputSubSystem(); }
	
	FInputModeGameAndUI InputModeData; // Settings container
	InputModeData.SetHideCursorDuringCapture(false); // Whether to hide the cursor during temporary mouse capture caused by a mouse down
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // Don't lock to window. It's better to put this in a menu.
	SetShowMouseCursor(true);
}

void AEDU_CORE_PlayerController::PlayerTick(float DeltaTime)
{ FLOW_LOG_TICK
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
}

//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------

void AEDU_CORE_PlayerController::CursorTrace()
{ FLOW_LOG_TICK
	
	// TODO: It would be best to make a custom CollisionProfile or channel for only selectables.
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	
	if(!CursorHit.bBlockingHit) { return; }
	
	/*---------------- Pointers for Selectable interface in UNIT plugin ------------
	  TScriptInterface allows us to do some abstract sugar; we don't need to cast
	  CursorHit.GetActor() to check it it can be molded into the interface class
	  we use, it's checked automagically. =)
	  
		TScriptInterface<IEDU_CORE_Selectable> LastActor;
		CurrentActor = TScriptInterface<IEDU_CORE_Selectable>(CursorHit.GetActor());

	  <!> Draug from the future here: I opted for a ASelectableEntity pointer
	  instead because using it as a base class streamlined the functions, rather
	  than using an interface,
	------------------------------------------------------------------------------*/
	LastActor = CurrentActor;
	CurrentActor = Cast<AEDU_CORE_SelectableEntity>(CursorHit.GetActor());
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
		if(CurrentActor == nullptr)
		{
			// Scenario 1; Do Nothing.
		}
		else
		{
			CurrentActor->MouseHighlightActor(); // Scenario 2: Highlight CurrentActor
		}
	}
	else // LastActor is valid...
	{   
		if(CurrentActor == nullptr) 
		{
			LastActor->MouseUnHighlightActor(); // Scenario 3: LastActor is valid && CurrentActor && null
		}
		else //  Both actors are valid...
		{
			if(LastActor != CurrentActor) // ...but LastActor != CurrentActor
			{  // Scenario 4:
				LastActor->MouseUnHighlightActor();
				CurrentActor->MouseHighlightActor();
			}
			// Else:Both actors are valid, and LastActor == CurrentActor
			// Scenario 5: Do nothing.
		}
	}
}
	
//------------------------------------------------------------------------------
// Input
//------------------------------------------------------------------------------

void AEDU_CORE_PlayerController::SetupInputComponent()
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

	InputDataAsset = Cast<UEDU_CORE_ControllerInputDataAsset>(ImportedInputDataAsset);
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
	
	LoadKeyMappings();
}

void AEDU_CORE_PlayerController::LoadKeyMappings()
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

void AEDU_CORE_PlayerController::SetupInputSubSystem()
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

void AEDU_CORE_PlayerController::SetMappingContext(EEDU_CORE_CurrentPawn Context)
{ FLOW_LOG
	switch (Context) {
	case EEDU_CORE_CurrentPawn::None:
		FLOW_LOG_WARNING("Switching to Controller")
		AddInputMappingContext(ControllerInputContext, 0);
		break;
		
	case EEDU_CORE_CurrentPawn::Camera:
		FLOW_LOG_WARNING("Switching to Camera")
		RemoveInputMappingContext(CharacterInputContext);
		AddInputMappingContext(CameraInputContext, 0);

		break;
	case EEDU_CORE_CurrentPawn::Character:
		FLOW_LOG_WARNING("Switching to Character")
		RemoveInputMappingContext(CameraInputContext);
		AddInputMappingContext(CharacterInputContext, 0);
		break;
	}
}

void AEDU_CORE_PlayerController::AddInputMappingContext(const UInputMappingContext* InputMappingContext, const int32 MappingPriority)
{ FLOW_LOG
	if(InputMappingContext == nullptr) { FLOW_LOG_ERROR("InputMappingContext == nullptr, make sure KeyMappings are properly loaded.") return; }
	if(!InputSubSystem) { SetupInputSubSystem(); }
	
	if(!InputSubSystem->HasMappingContext(InputMappingContext))
	{
		InputSubSystem->AddMappingContext(InputMappingContext, MappingPriority);
		FLOW_LOG_WARNING("InputMappingContext Added.")
	}
}

void AEDU_CORE_PlayerController::RemoveInputMappingContext(const UInputMappingContext* InputMappingContext)
{ FLOW_LOG
	if(!InputSubSystem) { SetupInputSubSystem(); }
	
	if(InputSubSystem->HasMappingContext(InputMappingContext))
	{
		InputSubSystem->RemoveMappingContext(InputMappingContext);
		FLOW_LOG_WARNING("InputMappingContext Removed.")
	}
}