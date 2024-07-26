// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Pawns/EDU_CORE_C2_Camera.h"

#include "EnhancedInputComponent.h"
#include "Framework/Data/DataAssets/EDU_CORE_CameraPawnInputDataAsset.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_PLAYER.h"
#include "Entities/EDU_CORE_MobileEntity.h"
#include "Entities/EDU_CORE_SelectableEntity.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_C2_Camera::AEDU_CORE_C2_Camera()
{ FLOW_LOG
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//------------------------------------------------------------------------------
// Input Setup
//------------------------------------------------------------------------------
// Called to bind functionality to input
void AEDU_CORE_C2_Camera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if(ImportedInputDataAsset == nullptr) { FLOW_LOG_ERROR("InputDataAsset is null, make sure it is set on the BluePrint CameraPawn.") return; }

	InputData = Cast<UEDU_CORE_CameraPawnInputDataAsset>(ImportedInputDataAsset);
	if(InputData == nullptr) { FLOW_LOG_ERROR("Something is wrong with the InputData, check the Cast.") return; }

	// Make sure the pointer is of the correct type, because why not.
		if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
		{
			/*------------------------------------------------------------------------------
				<!> Input Action Names are in InputData!
				Here Bind a BP Input Action (with Trigger condition),
				to C++ class and its C++ function.

				ETriggerEvent::Triggered == Fires while the key is down.
				ETriggerEvent::Started == Fires once only, on pressed.
				ETriggerEvent::Completed == Fires once only, on released.
			------------------------------------------------------------------------------*/
			
			//------------------------------------------------------------------------------
			// Mouse Input functions; Multifunctional, so best to use simple names.
			//------------------------------------------------------------------------------

			//------------------------------------------------------------------------------
			// Modifier Keys : 
			//------------------------------------------------------------------------------

			//---------------------------------------------------------------------------
			// CTRL Group Assignment
			//---------------------------------------------------------------------------

		}
}

void AEDU_CORE_C2_Camera::SetPlayerInputMode()
{
	Super::SetPlayerInputMode();
}

void AEDU_CORE_C2_Camera::SetPawnDefaults()
{
	Super::SetPawnDefaults();
}

//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------
// Called when the game starts or when spawned
void AEDU_CORE_C2_Camera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEDU_CORE_C2_Camera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}


//---------------------------------------------------------------------------
// Functionality: Mouse Button Input functions
//---------------------------------------------------------------------------
void AEDU_CORE_C2_Camera::Input_Mouse_2_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	Super::Input_Mouse_2_Pressed(InputActionValue);
	// We want to spawn the waypoint were we first clicked, so we can preview and rotate it.
	SavedCursorWorldPos = CursorWorldPos;
	
}

void AEDU_CORE_C2_Camera::Input_Mouse_2_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	Super::Input_Mouse_2_Released(InputActionValue);
	// Mouse 2 is no longer pressed.
	Mouse_2_PressedTime = 0;

	Send_Command();
}

void AEDU_CORE_C2_Camera::Command_LookAt()
{ FLOW_LOG
}

void AEDU_CORE_C2_Camera::Send_Command()
{ FLOW_LOG
	// TODO: It would be best to make a custom CollisionProfile or channel for only selectables and ground.
	for (int32 Index = 0; Index < SelectionArray.Num(); ++Index)
	{
		// Get the entity from the SelectionArray
		if(AActor* SelectableEntity = SelectionArray[Index])
		{
			// Check if the entity is a MobileEntity.
			if(AEDU_CORE_MobileEntity* MobileEntity = Cast<AEDU_CORE_MobileEntity>(SelectableEntity))
			{
				CommandDelay++;
				// Que up if shift is held, else delete old waypoints.
				if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_1)
				{
					MobileEntity->Command_NavigateTo(this, SavedCursorWorldPos, CursorRotation, true, CommandDelay/SelectionArray.Num());
				}
				else
				{
					MobileEntity->Command_NavigateTo(this, SavedCursorWorldPos, CursorRotation, false, CommandDelay/SelectionArray.Num());
				}
			}
		}
	}
	CommandDelay = 0;
}

