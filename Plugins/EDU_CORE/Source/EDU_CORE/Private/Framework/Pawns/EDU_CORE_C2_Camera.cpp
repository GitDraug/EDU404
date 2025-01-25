// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Pawns/EDU_CORE_C2_Camera.h"

#include "Framework/Data/DataAssets/EDU_CORE_CameraPawnInputDataAsset.h"
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_PLAYER.h"

#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Framework/Player/EDU_CORE_PlayerController.h"

#include "Entities/Waypoints/EDU_CORE_Waypoint.h"
#include "Entities/EDU_CORE_SelectableEntity.h"
#include "Interfaces/EDU_CORE_CommandInterface.h"

// UE
#include "EnhancedInputComponent.h"



//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

AEDU_CORE_C2_Camera::AEDU_CORE_C2_Camera(const FObjectInitializer& ObjectInitializer) : Super (ObjectInitializer)
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
			// TODO: This is used for debugging!
			SetTeam(EEDU_CORE_Team::Team_1);
			GameMode->AddActorToTeamArray(this);
		}
	}
	else
	{
		// TODO: This is used for debugging!
		SetTeam(EEDU_CORE_Team::Team_2);
		FLOW_LOG_WARNING("SetTeam(EEDU_CORE_Team::Team_1) for debugging!")
	}
}

// Called every frame
void AEDU_CORE_C2_Camera::Tick(float DeltaTime)
{ // FLOW_LOG_TICK
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan,
	FString::Printf(TEXT("ActiveTeam: %d"), ActiveTeam));

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan,
	FString::Printf(TEXT("bNoEntityUnderCursorOnInitialClick: %d"), bNoEntityUnderCursorOnInitialClick));

	/*--------------------- Prep for Movement Waypoint -----------------
	  If we clicked ground with Mouse 2 while having units selected,
	  it means we want to place a waypoint where we initially clicked
	  and rotate it.
	//-----------------------------------------------------------------*/
	if(bNoEntityUnderCursorOnInitialClick
	&& bMouse_2
	&& CameraSelectionArray.Num() > 0)
	{
		int32 ViewportWidth, ViewportHeight;
		FVector2D MousePos;
		LocalController->GetViewportSize(ViewportWidth, ViewportHeight);
		LocalController->GetMousePosition(MousePos.X, MousePos.Y);

		// Calculate the screen percentage distances in X and Y directions
		float CursorDistX = InitialMousePos.X - MousePos.X;
		float CursorDistY = InitialMousePos.Y - MousePos.Y;

		// Note that we always measure against height, because we want the activation distance to be equal in all directions.
		float XPercentage = (CursorDistX / ViewportHeight) * 100.0f;
		float YPercentage = (CursorDistY / ViewportHeight) * 100.0f;

		// Check if either X or Y percentage distance exceeds RotationActivationDistance
		if (FMath::Abs(XPercentage) > RotationActivationDistance || FMath::Abs(YPercentage) > RotationActivationDistance)
		{
			// Rotate the vector towards the cursor world position.
			CursorRotation = (CursorWorldPos - InitialCursorWorldPos).Rotation();
			bRotateWaypoint = true;
			// GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT("XPercentage: %f | YPercentage: %f"), XPercentage, YPercentage));
			// GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT("ViewportWidth: %d | ViewportHeight: %d"), ViewportWidth, ViewportHeight));
		}
		else
		{
			bRotateWaypoint = false;
		}
	}
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
		if(CameraSelectionArray.Num() > 0)
		{
			ServerIDArray.Reset();
			for(AEDU_CORE_SelectableEntity* LocalEntity : CameraSelectionArray)
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

	// This is the order button, if no enteties are selected, we can't give orders.
	if(CameraSelectionArray.Num() == 0) return;

	/*------------------ Mouse Cursor Context ----------------
	  Context Matters here, depending on what is under
	  the cursor the moment we press the button.
	//-------------------------------------------------------*/
	switch(EntityUnderMouseCursor)
	{
		case EEntityUnderMouseCursor::None:
			// Clicking Terrain = Move Command
			bNoEntityUnderCursorOnInitialClick = true;
				
			// We want to spawn a Waypoint were we first clicked, so we can preview and rotate it.
			InitialCursorWorldPos = CursorWorldPos;

			// We use the viewport to measure distance.
			LocalController->GetMousePosition(InitialMousePos.X, InitialMousePos.Y);

			// Spawn a waypoint locally!
		break;
		
		case EEntityUnderMouseCursor::Unknown:
			bNoEntityUnderCursorOnInitialClick = false;
		break;
		
		case EEntityUnderMouseCursor::Friendly:
			bNoEntityUnderCursorOnInitialClick = false;
		break;

		case EEntityUnderMouseCursor::Hostile:
			bNoEntityUnderCursorOnInitialClick = false;
		break;
		
	default: ;
	}	
}

void AEDU_CORE_C2_Camera::Input_Mouse_2_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	Super::Input_Mouse_2_Released(InputActionValue);
	
	// Enteties might have disappeared (Died?) while we held the button?
	if(CameraSelectionArray.Num() == 0) return;

	FWaypointParams WaypointParams;
	
	/*------------------------------------------------------------------
	  If no entity were under the cursor on initial click, then we
	  are in the process of placing a waypoint, and we are ignoring
	  whatever is under our cursor now, because the Waypoint is
	  alreayd placed.

	  By default, we are now only tracking the Rotaiton Direction
	  with the Mouse Cursor as a tracking tool. if Mod_2 is held
	  we are attacking a position.
	------------------------------------------------------------------*/
	if(bNoEntityUnderCursorOnInitialClick)
	{
		// WaypointParams
		WaypointParams.WaypointPosition = InitialCursorWorldPos;
		
		if(TObjectPtr<AActor> SelectableEntity = CameraSelectionArray[0])
		{			
			switch(ModifierKey)
			{
				// Queue and Fall through
				case EEDU_CORE_InputModifierKey::Mod_1:
				WaypointParams.bQueue = true;
				
				// NavigateTo
				case EEDU_CORE_InputModifierKey::NoModifier:
					WaypointParams.WaypointRotation = CursorRotation;
					WaypointParams.bCursorAlignment = bRotateWaypoint;
				break;

				// Queue and Fall through
				case EEDU_CORE_InputModifierKey::Mod_12:
					WaypointParams.bQueue = true;

				// AttackPosition
				case EEDU_CORE_InputModifierKey::Mod_2:
					WaypointParams.TargetPosition = WaypointParams.WaypointPosition;
				break;

				// ObservePosition
				case EEDU_CORE_InputModifierKey::Mod_3:
					// Like attackposition but without the firing. The trace guarantees line of sight, else
					// rotation is what we actually want.
				break;

				// Patrol (Queue is mandatory)
				case EEDU_CORE_InputModifierKey::Mod_13:
					WaypointParams.bQueue = true;
					WaypointParams.bPatrolPoint = true;
				break;
					
				default: FLOW_LOG_ERROR("switch default");
			}
		}
	}
	else 
	{
		switch(EntityUnderMouseCursor)
		{
			case EEntityUnderMouseCursor::None:
				// Cancel; we might have clicked an entity and regretted the order.
			break;
			
			case EEntityUnderMouseCursor::Waypoint:
				
			break;
			
			case EEntityUnderMouseCursor::Cover:
				// Take cover if apropriate entity type
			break;

			case EEntityUnderMouseCursor::Wreckage:
				// Repair, Scavange or Recover
			break;
				
			case EEntityUnderMouseCursor::Friendly:
				// if(LastActor == CameraSelectionArray[0])
				//{
				//	WaypointParams.WaypointRotation = CursorRotation;
            	//	WaypointParams.bCursorAlignment = bRotateWaypoint;	
				//}

				// Repair or Heal
			break;

			case EEntityUnderMouseCursor::Hostile:
				switch (ModifierKey)
				{
					// Queue and Fall trough
					case EEDU_CORE_InputModifierKey::Mod_1:
						WaypointParams.bQueue = true;
					
					default:
						WaypointParams.TargetArray.AddUnique(LastActor); // TODO: MAke an addentity to array function
						WaypointParams.WaypointPosition = LastActor->GetActorLocation();
						WaypointParams.WaypointType = EEDU_CORE_WaypointType::AttackTarget;
					break;
				}
			break;
		
		default: ;
		}	
	}

	//----------------------------------------------------------------------------
	// Check Authority and create waypotint
	//----------------------------------------------------------------------------

	// If the waypoint doesn't have a valid position, something is wrong.
	if(WaypointParams.WaypointPosition == FVector::ZeroVector) return;
	
	if(HasAuthority()) // If we are on the server, we call it directly.
	{
		CreateWaypoint(WaypointParams);
	}
	else if(!HasAuthority())// We ask the server to do it.
	{
		Server_CreateWaypoint(WaypointParams);
	}
}

//---------------------------------------------------------------------------
// Command logic
//---------------------------------------------------------------------------

void AEDU_CORE_C2_Camera::CreateWaypoint(const FWaypointParams& WaypointParams)
{ FLOW_LOG
	// Declare the client-side Waypoint we will issue.
	TObjectPtr<AEDU_CORE_Waypoint> ServerWaypoint = nullptr;

	if(CameraSelectionArray.Num() == 0)
	{
		FLOW_LOG_WARNING("SelectionArray has no elements.")
		return;
	}
	
	for(TObjectPtr<AEDU_CORE_SelectableEntity> SelectableEntity : CameraSelectionArray)
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
							ServerWaypoint = GameMode->GetFreshWaypointFromPool(EEDU_CORE_Team::None, WaypointParams.WaypointPosition);

							if(ServerWaypoint)
							{
								ServerWaypoint->SetOwner(this);
								ServerWaypoint->SetWaypointParams(WaypointParams);
								
								if(WaypointParams.bCursorAlignment)
								{
									// Align waypoint to Cursor World Position
									ServerWaypoint->SetActorRotation(WaypointParams.WaypointRotation);
								}
								else
								{
									// Align Waypoint with the Entity's direction
									FVector WaypointDirection = ServerWaypoint->GetActorLocation() - SelectableEntity->GetActorLocation();
									FRotator NewWaypointRotation = WaypointDirection.Rotation();
									ServerWaypoint->SetActorRotation(NewWaypointRotation);
								}
								
								UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - ID: %s"), *GetClass()->GetName(), __FUNCTION__, *ServerWaypoint->GetWaypointID().ToString());
								UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - Owner: %s"), *GetClass()->GetName(), __FUNCTION__, *ServerWaypoint->GetOwner()->GetName());
								UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - Waypoint: %s"), *GetClass()->GetName(), __FUNCTION__, *ServerWaypoint->GetName());
							}
						}
					}
				}

				// The waypoint should be created for the first entity in the array now (once only), and skipped for everyone else.
				if(ServerWaypoint)
				{
					UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - SelectableEntity: %s"), *GetClass()->GetName(), __FUNCTION__, *SelectableEntity->GetName());
					ServerWaypoint->AddActorToWaypoint(SelectableEntity);
				}
				else
				{
					FLOW_LOG_ERROR("Waypoint is null - make sure WaypointType is set in the GameMode in the Editor, also make sure it's a BP version, not .cpp")
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
		ServerWaypoint->NotifyListeners();
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
			CameraSelectionArray.Reset();
			// Process the IDArray sent from the client on the server
			for (const FGuid& Guid : IDArray)
			{
				// Find the actor by GUID using the GameMode method
				if(AActor* Actor = GameMode->FindActorInMap(Guid)) // 
				{
					if (AEDU_CORE_SelectableEntity* SelectableEntity = Cast<AEDU_CORE_SelectableEntity>(Actor))
					{
						CameraSelectionArray.AddUnique(SelectableEntity);
						UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Server: %s::%hs - Added %s to SelectionArray"), *GetClass()->GetName(), __FUNCTION__, *SelectableEntity->GetClass()->GetName());
					}
				}
			}
		}
	}
}

void AEDU_CORE_C2_Camera::Server_CreateWaypoint_Implementation(const FWaypointParams& WaypointParams)
{	FLOW_LOG
	// We'll just pass along all parameters from the client.
	CreateWaypoint(WaypointParams);
}