
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDU_USER_PlayerController.generated.h"

enum class EEDU_USER_InputModifierKey : uint8;
enum class EEDU_USER_CurrentPawn : uint8;
struct FInputActionValue;
class APawn;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UDataAsset;

class UEDU_USER_CameraPawnInputDataAsset;
class UEDU_USER_ControllerInputDataAsset;
class AEDU_USER_HUD;
class IEDU_UNIT_Selectable;

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
class EDU_USER_API AEDU_USER_PlayerController : public APlayerController
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
public:
	AEDU_USER_PlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------  
public:
	// The GameMode in EDU_USER effect these
	FORCEINLINE virtual void SetPlayerStartLocation(const FVector Location) { PlayerStartLocation = Location; };
	FORCEINLINE	virtual FVector GetPlayerStartLocation() const { return PlayerStartLocation; }
	FORCEINLINE virtual UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() { return InputSubSystem; }

	virtual void SetMappingContext(EEDU_USER_CurrentPawn Context);
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	// Used to Spawn Entities at a certain location.
	UPROPERTY()
	FVector PlayerStartLocation;

	UPROPERTY()
	TObjectPtr<AEDU_USER_HUD> LocalHUD;
	
	/*---------------- Pointers for Selectable interface in UNIT plugin ------------
	  TScriptInterface is essentially, a fancy pointer. It's a template class that
	  provides a way to work with interface types in a manner that is compatible
	  with both C++ and the Engine reflection system (UObject system).

	  It wraps around a UObject that implements a given interface, allowing you to
	  call the interface methods directly and ensuring type safety.

	  Make sure the UNIT Plugin is included in this plugin as a private dependency.
	------------------------------------------------------------------------------*/
	TScriptInterface<IEDU_UNIT_Selectable> LastActor; // Last Actor under Cursor
	TScriptInterface<IEDU_UNIT_Selectable> CurrentActor; // Current Actor under Cursor
	
//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------
private:
	void CursorTrace();
	
//------------------------------------------------------------------------------
// Input: Components
//------------------------------------------------------------------------------
protected:
	// Holds Default Input Data with Actions and Contexts, we need to set it in the BP.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "EDU_USER Settings")
	TObjectPtr<UDataAsset> ImportedInputDataAsset;
	
private:
	// Internal pointer to Default InputMappingContext (Keymappings) and Actions.
	UPROPERTY()
	TObjectPtr<UEDU_USER_ControllerInputDataAsset> InputDataAsset;
	
	// The EnhancedInputLocalPlayerSubsystem is a global singleton to that managed all input contexts.
	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubSystem;

	// The UEnhancedInputComponent handles Action Inputs.
	UPROPERTY()
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	// Keeps track of what InputContext (keymapping) we want to use.
	UPROPERTY()
	EEDU_USER_CurrentPawn CurrentPawn;
	
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
	
	//------------------------------------------------------------------------------
	// States for the modifier Keys to manage USER_InputModifierKey
	//------------------------------------------------------------------------------
	bool bMod_1 = false;
	bool bMod_2 = false;
	bool bMod_3 = false;
	bool bMod_4 = false;
	
	/*--------------------------------------------------------------------------------
	Note that the bools are for the controller class only. The active key, or active
	combo of keys is set in the enum USER_InputModifierKey. This means that
	other classes only need to keep track of the enum state, instead if 4 bool.
	-------------------------------------------------------------------------------*/
	UPROPERTY()
	EEDU_USER_InputModifierKey ModifierKey;
	
//---------------------------------------------------------------------------
// Modifier Input functions
//---------------------------------------------------------------------------
protected:
	// Assigns the UEnhancedInputLocalPlayerSubsystem pointer and debugs if it fails.
	virtual void SetupInputSubSystem();

	virtual void SetupInputComponent() override;
	
	// Update EEDU_USER_InputModifierKey
	virtual void SetModifierKey();

	// Load Default CameraPawnInputContext from DataAsset or UserSettings from SaveFile
	virtual void LoadKeyMappings();

	virtual void AddInputMappingContext(const UInputMappingContext* InputMappingContext, const int32 MappingPriority);
	virtual void RemoveInputMappingContext(const UInputMappingContext* InputMappingContext);
	
	// input Actions
	virtual void Input_Mod_1_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_1_Released		(const FInputActionValue& InputActionValue);
	
	virtual void Input_Mod_2_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_2_Released		(const FInputActionValue& InputActionValue);
	
	virtual void Input_Mod_3_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_3_Released		(const FInputActionValue& InputActionValue);

	virtual void Input_Mod_4_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_4_Released		(const FInputActionValue& InputActionValue);
	
};
