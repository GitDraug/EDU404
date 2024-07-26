// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDU_CORE_PlayerController.generated.h"

struct FInputActionValue;
class APawn;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UDataAsset;

class UEDU_CORE_CameraPawnInputDataAsset;
class UEDU_CORE_ControllerInputDataAsset;
class AEDU_CORE_HUD;
class IEDU_CORE_Selectable;
class AEDU_CORE_SelectableEntity;
class AEDU_CORE_AbstractEntity;
class AEDU_CORE_PhysicalEntity;

/*------------------------------------------------------------------------------
  Used By SetMappingContext()
------------------------------------------------------------------------------*/
UENUM()
enum class EEDU_CORE_CurrentPawn : uint8
{
	None,
	Camera,
	Character,
};

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  PlayerControllers are used by human players to control Pawns.
 
  In networked games, PlayerControllers exist on the server for every
  player-controlled pawn, and also on the controlling client's machine.
  They do NOT exist on a client's machine for pawns controlled by remote
  players elsewhere on the network.

  The Player controller can make remote Procedure Calls, so if a client needs
  something done on the server, the PlayerController is the way to go.

  The Player controller is created on the GameMode and replicated to the
  owning client. This means that the GameMode owns all Player controllers.

  In this project, the Controller holds shared input and is able to switch
  between pawns. Pawn-specific actions and functions are typed out in each
  individual pawn.
------------------------------------------------------------------------------*/
UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_PlayerController : public APlayerController
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
public:
	AEDU_CORE_PlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------  
public:
	// The GameMode in EDU_CORE effect these
	FORCEINLINE virtual void SetPlayerStartLocation(const FVector Location) { PlayerStartLocation = Location; };
	FORCEINLINE	virtual FVector GetPlayerStartLocation() const { return PlayerStartLocation; }
	FORCEINLINE virtual UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() { return InputSubSystem; }
	
	// Adds entity to the Local PlayerController Tick, running only on a client.
	void AddToAbstractEntityArray(AEDU_CORE_AbstractEntity* TickingEntity);
	void AddToPhysicalEntityArray(AEDU_CORE_PhysicalEntity* MobileEntity);
	
	virtual void SetMappingContext(EEDU_CORE_CurrentPawn Context);
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	// Used to Spawn Entities at a certain location.
	FVector2d MousePos;
	
	UPROPERTY()
	FVector PlayerStartLocation;

	UPROPERTY()
	TObjectPtr<AEDU_CORE_HUD> LocalHUD;

	/*---------------------- Client-Side Aggregated Tick ---------------------------
	  This tick function allows us to aggregate ticks client-side. These are
	  excellent for batch executions, such as blending occasional Server updates.

	  EntityBatch: Represents the number of entities to process in each tick.
	  The loop will run EntityBatch times, allowing for controlled batch processing
	  of entities.
	  
	  Index tracks which element in the EntityArray is being processed.
	------------------------------------------------------------------------------*/
	UPROPERTY()
	TArray<AEDU_CORE_AbstractEntity*> AbstractEntityArray;

	UPROPERTY(EditDefaultsOnly)
	int32 AbstractEntityBatch = 100;
	int32 AbstractEntityIndex = 0;
	
	UPROPERTY()
	TArray<AEDU_CORE_PhysicalEntity*> PhysicalEntityArray;

	UPROPERTY(EditDefaultsOnly)
	int32 PhysicalEntityBatch = 100;
	int32 PhysicalEntityIndex = 0;

	
//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Input: Components
//------------------------------------------------------------------------------
protected:
	// Holds Default Input Data with Actions and Contexts, we need to set it in the BP.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "EDU_CORE Settings")
	TObjectPtr<UDataAsset> ImportedInputDataAsset;
	
	// Internal pointer to Default InputMappingContext (Keymappings) and Actions.
	UPROPERTY()
	TObjectPtr<UEDU_CORE_ControllerInputDataAsset> InputDataAsset;
	
	// The EnhancedInputLocalPlayerSubsystem is a global singleton to that managed all input contexts.
	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubSystem;

	// The UEnhancedInputComponent handles Action Inputs.
	UPROPERTY()
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	// Keeps track of what InputContext (keymapping) we want to use.
	UPROPERTY()
	EEDU_CORE_CurrentPawn CurrentPawn;
	
	/*------------------------------------------------------------------------------
	  Internal Pointers to InputContexts: these are either loaded with
	  UserSaved keys or Default keys from a DataAsset.
	------------------------------------------------------------------------------*/
	UPROPERTY()
	TObjectPtr<UInputMappingContext> ControllerInputContext;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> CameraInputContext;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> CharacterInputContext;
	
//---------------------------------------------------------------------------
// Modifier Input functions
//---------------------------------------------------------------------------
protected:
	// Assigns the UEnhancedInputLocalPlayerSubsystem pointer and debugs if it fails.
	virtual void SetupInputSubSystem();

	virtual void SetupInputComponent() override;

	// Load Default CameraPawnInputContext from DataAsset or UserSettings from SaveFile
	virtual void LoadKeyMappings();

	virtual void AddInputMappingContext(const UInputMappingContext* InputMappingContext, const int32 MappingPriority);
	virtual void RemoveInputMappingContext(const UInputMappingContext* InputMappingContext);
	
};
