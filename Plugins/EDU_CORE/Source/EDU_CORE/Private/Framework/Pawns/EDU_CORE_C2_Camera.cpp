// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Pawns/EDU_CORE_C2_Camera.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "AI/WayPoints/EDU_CORE_Waypoint.h"
#include "Framework/Data/DataAssets/EDU_CORE_CameraPawnInputDataAsset.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_PLAYER.h"
#include "Entities/EDU_CORE_SelectableEntity.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Interfaces/EDU_CORE_CommandInterface.h"

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
void AEDU_CORE_C2_Camera::Input_Mouse_1_Released(const FInputActionValue& InputActionValue)
{
	Super::Input_Mouse_1_Released(InputActionValue);
	
	// The server can act as usual, but the client needs to work through the server.
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

	if(SelectionArray.Num() > 0)
	{
		if(AActor* SelectableEntity = SelectionArray[0])
		{
			if(ModifierKey == EEDU_CORE_InputModifierKey::NoModifier)
			{
				if(HasAuthority()) // If we are on the server, we call it directly.
				{
					Command_LookAt();
				}
				else if(!HasAuthority())// We ask the server to do it.
				{
					ServerCommand_LookAt();
				}
				
			}
			if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_3 || ModifierKey == EEDU_CORE_InputModifierKey::Mod_13)
			{
				Command_LookAt();
			}
		}
	}
}

//---------------------------------------------------------------------------
// Server logic
//---------------------------------------------------------------------------

void AEDU_CORE_C2_Camera::ServerCommand_LookAt_Implementation()
{ FLOW_LOG
	Command_LookAt();
}

//---------------------------------------------------------------------------
// Functionality: Commands
//---------------------------------------------------------------------------

void AEDU_CORE_C2_Camera::Command_NavigateTo()
{ FLOW_LOG
	
}

void AEDU_CORE_C2_Camera::GetEntitiesOnServer_Implementation(const TArray<FGuid>& IDArray)
{ FLOW_LOG
	if(HasAuthority())
	{
		// We need pointers on the serverInstance to fill the SelectionArray on the Server version of our Player Controller. 
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			// Process the IDArray on the server
			for (const FGuid& Guid : IDArray)
			{
				// Find the actor by GUID using the GameMode method
				AActor** ActorPtr = GameMode->GuidActorMap.Find(Guid);
				if(AActor* Actor = *ActorPtr) //  GameMode->FindActorInMap(Guid))
				{
					if (AEDU_CORE_SelectableEntity* SelectableEntity = Cast<AEDU_CORE_SelectableEntity>(Actor))
					{
						SelectionArray.AddUnique(SelectableEntity);
						UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("Server: %s::%hs - Added %s to SelectionArray"), *GetClass()->GetName(), __FUNCTION__, *SelectableEntity->GetClass()->GetName());
					}
				}
				else
				{
					UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Server: %s::%hs - No actor found with GUID: %s"), *GetClass()->GetName(), __FUNCTION__, *Guid.ToString());
				}
			}
		}
		/*
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			// Process the IDArray on the server
			for (const FGuid& Guid : IDArray)
			{
				// Find the actor in the level by GUID
				for (TActorIterator<AActor> It(GetWorld()); It; ++It)
				{
					if(AActor* ActorPtr = *GameMode->FindActorInMap(Guid))
					{
						if(AEDU_CORE_SelectableEntity* SelectableEntity = Cast<AEDU_CORE_SelectableEntity>(ActorPtr))
						{
							SelectionArray.AddUnique(SelectableEntity);
							UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("Server: %s::%hs - Added %s to SelectionArray"), *GetClass()->GetName(), __FUNCTION__, *SelectableEntity->GetClass()->GetName());
						}
					}
				}
			}
		}*/
	}
}


void AEDU_CORE_C2_Camera::Command_LookAt()
{ FLOW_LOG
	// We need an ID for the Waypoint
	FGuid WaypointID = FGuid::NewGuid(); 
	
	// We'll spawn a waypoint to do work for us.
	
	// Declare the Waypoint we will issue.
	AEDU_CORE_Waypoint* Waypoint = nullptr;

	if(SelectionArray.Num() == 0)
	{
		FLOW_LOG_WARNING("SelectionArray has no elements.")
	}
	
	for(AActor* SelectableEntity : SelectionArray)
	{
		// Check that it's alive.
		if(SelectableEntity)
		{	// Check if the entity can receive commands.
			if(IEDU_CORE_CommandInterface* Entity = Cast<IEDU_CORE_CommandInterface>(SelectableEntity))
			{
				// There might be entities that can't take orders in the array, so we will only create the waypoint when we know for sure that someone can adhere to it.
				if(!Waypoint)
				{
					FActorSpawnParameters SpawnParams;
					Waypoint = GetWorld()->SpawnActor<AEDU_CORE_Waypoint>(WaypointClass, SavedCursorWorldPos, CursorRotation, SpawnParams);
					UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, *Waypoint->GetName())
				}

				// The waypoint should be created for the first entity in the array now, and skipped for everyone else.
				if(Waypoint)
				{
					Waypoint->AddActorToWaypoint(SelectableEntity);
				}
				else
				{
					FLOW_LOG_ERROR("Waypoint is null - check the BP_Camera in the Editor, also make sure it's the BP version, not .cpp")
				}
			}
		}
		else
		{
			FLOW_LOG_WARNING("Entity don't exist.")
		}
	}

	// Now that the Waypoint is loaded, we want to tell it to Notify everyone it has saved.

	if(Waypoint)
	{
		if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_13) // Queue the waypoint
		{
			Waypoint->NotifyListeners(true);
		}
		else
		{
			Waypoint->NotifyListeners(false);
		}
	}
	else
	{
		FLOW_LOG_WARNING("Waypoint don't exist.")
	}
}
