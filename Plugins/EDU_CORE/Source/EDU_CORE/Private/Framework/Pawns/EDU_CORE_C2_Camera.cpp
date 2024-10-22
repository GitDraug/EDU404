// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Pawns/EDU_CORE_C2_Camera.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

#include "Framework/Data/DataAssets/EDU_CORE_CameraPawnInputDataAsset.h"
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_PLAYER.h"

#include "AI/WayPoints/EDU_CORE_Waypoint.h"
#include "Entities/EDU_CORE_SelectableEntity.h"
#include "Interfaces/EDU_CORE_CommandInterface.h"

#include "EnhancedInputComponent.h" // Unreal Library

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

AEDU_CORE_C2_Camera::AEDU_CORE_C2_Camera()
{ FLOW_LOG

}

//------------------------------------------------------------------------------
// Input Setup
//------------------------------------------------------------------------------
// Called to bind functionality to input

void AEDU_CORE_C2_Camera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{ FLOW_LOG
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
{ FLOW_LOG
	Super::SetPlayerInputMode();
}

void AEDU_CORE_C2_Camera::SetPawnDefaults()
{ FLOW_LOG
	Super::SetPawnDefaults();
}

//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------

void AEDU_CORE_C2_Camera::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();

	if(HasAuthority())
	{
		if(AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddActorToTeamArray(this);
		}
	}
}

// Called every frame
void AEDU_CORE_C2_Camera::Tick(float DeltaTime)
{ FLOW_LOG_TICK
	Super::Tick(DeltaTime);
}

//---------------------------------------------------------------------------
// Functionality: Mouse Button Input functions
//---------------------------------------------------------------------------

void AEDU_CORE_C2_Camera::Input_Mouse_1_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	Super::Input_Mouse_1_Released(InputActionValue);
	
	// When we release Mouse 1, drawing a selection Marque, the server can act as usual, but the client needs to work through the server.
	if(!HasAuthority())
	{
		// We need to convert our actor pointers to GUIDs, so we can get them on the server.
		if(SelectionArray.Num() > 0)
		{
			ServerIDArray.Reset();
			for(AEDU_CORE_SelectableEntity* LocalEntity : SelectionArray)
			{
				ServerIDArray.AddUnique(LocalEntity->GetGUID());
			}
		}
		// Now that we have an array of Global IDs, we can tell the server to get them on the server instance.
		GetEntitiesOnServer(ServerIDArray);	
	}
}

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

	UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("%s::%hs - !HasAuthority() 1: %s"), *GetClass()->GetName(), __FUNCTION__, *this->GetName());
	if(SelectionArray.Num() > 0)
	{
		if(TObjectPtr<AActor> SelectableEntity = SelectionArray[0])
		{
			switch (ModifierKey)
			{
			// NavigateTo
			case EEDU_CORE_InputModifierKey::NoModifier:
				if(HasAuthority()) // If we are on the server, we call it directly.
				{
					UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("%s::%hs - HasAuthority: %s"), *GetClass()->GetName(), __FUNCTION__, *this->GetName());
					CreateWaypoint(SavedCursorWorldPos, EEDU_CORE_WaypointType::NavigateTo, false);
				}
				else if(!HasAuthority())// We ask the server to do it.
				{
					UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("%s::%hs - !HasAuthority() 2: %s"), *GetClass()->GetName(), __FUNCTION__, *this->GetName());
					Server_CreateWaypoint(SavedCursorWorldPos, EEDU_CORE_WaypointType::NavigateTo, false);
				}
				break;
				
			// Queue NavigateTo
			case EEDU_CORE_InputModifierKey::Mod_1:
				if(HasAuthority()) // If we are on the server, we call it directly.
				{
					CreateWaypoint(SavedCursorWorldPos, EEDU_CORE_WaypointType::NavigateTo, true);
				}
				else if(!HasAuthority())// We ask the server to do it.
				{
					Server_CreateWaypoint(SavedCursorWorldPos, EEDU_CORE_WaypointType::NavigateTo, true);
				}
				break;
				
			// AttackPosition
			case EEDU_CORE_InputModifierKey::Mod_2:
				break;

			// Queue AttackPosition
			case EEDU_CORE_InputModifierKey::Mod_12:
				break;

			// ObservePosition
			case EEDU_CORE_InputModifierKey::Mod_3:
				break;

			// Queue ObservePosition
			case EEDU_CORE_InputModifierKey::Mod_13:
				break;
				
			default: FLOW_LOG_ERROR("switch default");
			}
		}
	}
}

//---------------------------------------------------------------------------
// Command logic
//---------------------------------------------------------------------------

void AEDU_CORE_C2_Camera::CreateWaypoint(const FVector WorldPosition, const EEDU_CORE_WaypointType WaypointType, const bool Queue)
{ FLOW_LOG
	// Declare the client-side Waypoint we will issue.
	AEDU_CORE_Waypoint* ServerWaypoint = nullptr;

	if(SelectionArray.Num() == 0)
	{
		FLOW_LOG_WARNING("SelectionArray has no elements.")
		return;
	}
	
	for(AActor* SelectableEntity : SelectionArray)
	{
		// Check that it's alive.
		if(SelectableEntity)
		{	// There might be entities that can't take orders in the array, so we will only create the waypoint when we know for sure that someone can adhere to it.
			if(IEDU_CORE_CommandInterface* Entity = Cast<IEDU_CORE_CommandInterface>(SelectableEntity))
			{
				// The first entity in the array will create the waypoint.
				if(!ServerWaypoint)
				{
					if(HasAuthority())
					{
						if(TObjectPtr<AEDU_CORE_GameMode> GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
						{
							ServerWaypoint = GameMode->GetFreshWaypointFromPool(EEDU_CORE_Team::None, WorldPosition);

							if(ServerWaypoint)
							{
								ServerWaypoint->SetOwner(this);
								ServerWaypoint->SetWaypointType(WaypointType);
								ServerWaypoint->SetActorRotation((CursorWorldPos - SavedCursorWorldPos).Rotation());

								UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - ID: %s"), *GetClass()->GetName(), __FUNCTION__, *ServerWaypoint->GetWaypointID().ToString());
								UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - Owner: %s"), *GetClass()->GetName(), __FUNCTION__, *ServerWaypoint->GetOwner()->GetName());
								UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - Waypoint: %s"), *GetClass()->GetName(), __FUNCTION__, *ServerWaypoint->GetName());
							}
						}
					}
				}

				// The waypoint should be created for the first entity in the array now, and skipped for everyone else.
				if(ServerWaypoint)
				{
					UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - SelectableEntity: %s"), *GetClass()->GetName(), __FUNCTION__, *SelectableEntity->GetName());
					ServerWaypoint->AddActorToWaypoint(SelectableEntity);
				}
				else
				{
					FLOW_LOG_ERROR("Waypoint is null - make sure WaypointType is set in the gameMode in the Editor, also make sure it's a BP version, not .cpp")
				}
			}
		}
		else
		{
			FLOW_LOG_WARNING("Entity don't exist.")
		}
	}
	
	/*-------------------------------------------------------------------------------
	 We need to notify all listeners outside the for-loop, else we will notify
	 them several times, and they'll end up removing duplicates, causing bugs.

	 Also, since we are outside the loop, the ServerWaypoint may not actually exist!
	--------------------------------------------------------------------------------*/
	if(ServerWaypoint)
	{
		ServerWaypoint->NotifyListeners(Queue);
	}
}

//---------------------------------------------------------------------------
// RPC logic
//---------------------------------------------------------------------------

void AEDU_CORE_C2_Camera::GetEntitiesOnServer_Implementation(const TArray<FGuid>& IDArray)
{ FLOW_LOG
	if(HasAuthority())
	{
		// We need pointers on the serverInstance to fill the SelectionArray on the Server version of our Player Controller. 
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			SelectionArray.Reset();
			// Process the IDArray sent from the client on the server
			for (const FGuid& Guid : IDArray)
			{
				// Find the actor by GUID using the GameMode method
				if(AActor* Actor = GameMode->FindActorInMap(Guid)) // 
				{
					if (AEDU_CORE_SelectableEntity* SelectableEntity = Cast<AEDU_CORE_SelectableEntity>(Actor))
					{
						SelectionArray.AddUnique(SelectableEntity);
						UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Server: %s::%hs - Added %s to SelectionArray"), *GetClass()->GetName(), __FUNCTION__, *SelectableEntity->GetClass()->GetName());
					}
				}
			}
		}
	}
}

void AEDU_CORE_C2_Camera::Server_CreateWaypoint_Implementation(const FVector WorldPosition, const EEDU_CORE_WaypointType WaypointType, const bool Queue)
{	FLOW_LOG
	CreateWaypoint(WorldPosition, WaypointType, Queue);
}