// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"

#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "EDU_CORE_SpectatorCamera.generated.h"

struct FInputActionValue;
class USceneComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UDataAsset;

class UEDU_CORE_CameraPawnInputDataAsset;
class AEDU_CORE_PlayerController;
class AEDU_CORE_HUD;
class AEDU_CORE_SelectableEntity;

/*------------------------------------------------------------------------------
  CTRL Group struct
------------------------------------------------------------------------------*/
USTRUCT()
struct FCTRL_Group
{
	GENERATED_BODY()
	
	// Default constructor
	FCTRL_Group() : // Init list
	Leader(TEXT(""))
	{}
	
	// An array of names
	UPROPERTY()
	TArray<AEDU_CORE_SelectableEntity*> GroupArray;

	// A single name for the leader
	UPROPERTY()
	FString Leader;
};

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  The SpectatorCamera gives a player a camera that can navigate around the map,
  select and review happenings, but it can't send any commands.

  For a Camera that can Command and Control, use the C2 Camera

  <!> TODO: Make sure the Trace Channel is correct.

  <!> Make sure the camera does not do collision tests in the BP, it will end up
  below ground all the time if you do.
------------------------------------------------------------------------------*/
UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_SpectatorCamera : public APawn
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	AEDU_CORE_SpectatorCamera(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------
protected:
	/*------------------------------------------------------------------------------
	  PawnClientRestart() is called on the owning client of a player-controlled
	  Pawn when it is restarted. It will immediately set up the camera, and then
	  call CreatePlayerInputComponent() followed by SetupPlayerInputComponent.
	------------------------------------------------------------------------------*/
	virtual void PawnClientRestart() override;
	
	// virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void UnPossessed() override;
	
//------------------------------------------------------------------------------
// Input Setup
//------------------------------------------------------------------------------
	// Called upon possession by a PlayerController, to bind functionality to input.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Input mode is UI && Game etc.
	virtual void SetPlayerInputMode();

	// Used to initialize the variables in the DataAsset
	virtual void SetPawnDefaults();

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------  
public:
	FORCEINLINE virtual FVector GetCursorWorldPos() const { return CursorWorldPos; }
	FORCEINLINE virtual FVector GetSavedCursorWorldPos() const { return SavedCursorWorldPos; }

//------------------------------------------------------------------------------
// Public Camera Settings
//------------------------------------------------------------------------------
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Pitch")
	bool bInvertedPitch = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Rotation")
	bool bInvertedYaw = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Rotation")
	bool bInvertedZoom = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Rotation")
	bool bAutoPitch = true; // In the settings menu, can be turned off.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Rotation")
	float AutoPitchMin = -80.f; // In the settings menu, can be turned off.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Rotation")
	float AutoPitchMax = -35.f; // In the settings menu, can be turned off.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Movement")
	float MouseDragSpeed = 2.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Movement")
	float DoubleClickDelay = 0.25f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Movement")
	float MousePressDelay = 0.2f;
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> CameraAnchor;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;
	
	// Holds Input Data with keys and functions, we need to set it in the BP.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "EDU_CORE Settings")
	TObjectPtr<UDataAsset> ImportedInputDataAsset;
	
	UPROPERTY()
  	TObjectPtr<UEDU_CORE_CameraPawnInputDataAsset> InputData;

	UPROPERTY()
	TObjectPtr<AEDU_CORE_PlayerController> LocalController;
	
	UPROPERTY()
	TObjectPtr<AEDU_CORE_HUD> LocalHUD;
	
	/*---------------- Pointers for Selectable interface in UNIT plugin ------------
	  TScriptInterface is essentially, a fancy pointer. It's a template class that
	  provides a way to work with interface types in a manner that is compatible
	  with both C++ and the Engine reflection system (UObject system).

	  It wraps around a UObject that implements a given interface, allowing you to
	  call the interface methods directly and ensuring type safety.

	  Make sure the UNIT Plugin is included in this plugin as a private dependency.
	------------------------------------------------------------------------------*/
	//TScriptInterface<IEDU_CORE_Selectable> LastActor; // Last Actor under Cursor
	//TScriptInterface<IEDU_CORE_Selectable> CurrentActor; // Current Actor under Cursor
	
	TObjectPtr<AEDU_CORE_SelectableEntity> LastActor;
	TObjectPtr<AEDU_CORE_SelectableEntity> CurrentActor;

	//---------------------------------------------------------------------------
	// Mouse data
	//---------------------------------------------------------------------------
	// For passing the cursor 2D position in the 3D world.
	FVector2d SavedMousePos;
	
	FVector CursorWorldPos;
	FVector SavedCursorWorldPos; 
	FRotator CursorRotation; // CursorWorldPos - SavedCursorWorldPos).Rotation()

	// A YX struct to save the Mouse movement Input, using APlayerController->GetInputMouseDelta();
	FVector2d MouseDirection {0.f, 0.f};
	
	float Mouse_2_timer;
	float Mouse_1_StartTime;
	float Mouse_2_PressedTime;
	
	//---------------------------------------------------------------------------
	// Tick Managment
	//---------------------------------------------------------------------------
	// For limiting tick functions to once every X frame.
	int8 FrameCounter = 0;

	// Ready to tick?
	bool bIsInitialized = false;
	
	//---------------------------------------------------------------------------
	// Group Assignment
	//---------------------------------------------------------------------------
	UPROPERTY()
	TArray<AEDU_CORE_SelectableEntity*> SelectionArray;
	
	// CTRL Groups
	FCTRL_Group CTRL_Group_1;
	FCTRL_Group CTRL_Group_2;
	FCTRL_Group CTRL_Group_3;
	FCTRL_Group CTRL_Group_4;
	FCTRL_Group CTRL_Group_5;
	FCTRL_Group CTRL_Group_6;
	FCTRL_Group CTRL_Group_7;
	FCTRL_Group CTRL_Group_8;
	FCTRL_Group CTRL_Group_9;
	FCTRL_Group CTRL_Group_0;
	
	//---------------------------------------------------------------------------
	// Camera Tracing
	//---------------------------------------------------------------------------
	// If we can't trace the ground, we can use the end of the trace.
	FVector CameraTraceEndLocation;
	
	// Last valid terrain trace
	FVector LastValidLocation;

	// Used for interpolation
	FRotator TargetRotation;
	FVector TargetLocation;
	
	//------------------------------------------------------------------------------
	// Zoom tracking Data
	//------------------------------------------------------------------------------
	UPROPERTY()
	float ZoomTraceLength; // Distance to trace for ground.
	
	UPROPERTY()
	float ZoomDistance; // Distance traveled per key press
	
	// The length SpringArmComponent->TargetArmLength is targeting.
	float TargetZoom;

	// Default settings
	bool bZoomIn = false;
	bool bZoomOut = false;
	bool bZoomFocusOn = true;
	bool bZoomFocusFinished = true;
	
	//------------------------------------------------------------------------------
	// EdgeScroll settings
	//------------------------------------------------------------------------------
	float ScreenEdgeArea;
	float EdgeScrollSpeed;

	// EdgeScroll Calculations can be used for Other functions too
	FVector2d ScrollDirection;
	
	//------------------------------------------------------------------------------
	// Movement States
	//------------------------------------------------------------------------------
	bool bAutoPitchDisengaged = false;
	bool bFreeLook = false;
	bool bMouseDrag = false;
	bool bEdgeScroll = true;
	bool bAutoScroll = false;

	// Should we interpolate?
	bool bInterpMov;
	bool bInterpRot;
	
	// Used for interpolation while no key is pressed.
	float InterpTimer;

	//------------------------------------------------------------------------------
	// States for the modifier Keys to manage USER_InputModifierKey
	//------------------------------------------------------------------------------
	bool bMod_1 = false;
	bool bMod_2 = false;
	bool bMod_3 = false;
	bool bMod_4 = false;
	
	/*--------------------------------------------------------------------------------
	  Note that the bools are for the controller class only. The active key, or active
	  combo of keys is set in the enum USER_InputModifierKey. This means that other
	  classes only need to keep track of the enum state, instead of several bool.
	-------------------------------------------------------------------------------*/
public:
	EEDU_CORE_InputModifierKey ModifierKey;

//------------------------------------------------------------------------------
// Functionality : Selection Controls
//------------------------------------------------------------------------------
protected:
	void ResetCameraSelectionArray(); // Unselect selected entities and empty the Camera's SelectionArray.
	void ResetHUDSelectionArray() const; // Unselect selected entities and empty the HUD's SelectionArray.
	void SelectEntitiesInRectangle(); // Save selected entities and empty the HUDArray.
	void CopyEntitiesInHUDArray(); 
	void CallCtrlGroup(const TArray<AEDU_CORE_SelectableEntity*>& CtrlGroupArray);
	void ReviseSelection(); 	// Remove an Entity from selection, or adds it if not already present. 
	
//------------------------------------------------------------------------------
// Functionality : Camera Controls
//------------------------------------------------------------------------------
	// Uses various calculations to Move, Zoom and Rotate each frame.
	void UpdateCameraRotation(const float DeltaTime) const;
	void UpdateCameraLocation(const float DeltaTime);
	void UpdateCameraZoom(const float DeltaTime);
	void AutoPitch(const float DeltaTime); // Pitches the camera depending on zoom level (optional in settings)
	
	// Utility
	void MoveCameraAnchor(const FVector2d& Direction, const float& Speed);
	void CameraTrace();
	void CursorTrace();
	
	void UpdateMouseDirection();
	void ResetCamera();
	
	// Special Movement
	void EdgeScroll();
	void AutoScroll();
	void FreeLook();
	void MouseDrag();

	// Cursor interaction
	void DisableMouseEvents();
	void EnableMouseEvents() const;
	
	// This allows Interpolation for a brief time, instead of doing calculations each tick.
	void EnableInterpRotation();
	void EnableInterpMovement(float Time = 1.f);
	
	/*---------------------------------------------------------------------------
	  GetTerrainPosition Traces the ground to check we're inside the allowed area.
	  In order for it to work correctly, we need a Trace Channel defined in
	  USER_StaticGameData, that way we can restrict camera movement without
	  using blocking collision meshes.

	  Trivia: GetTerrainPosition() originally resided in the controller,
	  thus we passed arguments by ref.
	---------------------------------------------------------------------------*/
	void GetTerrainPosition(FVector& TargetPos, FVector& LastValidPos) const;
	
	/*---------------------------------------------------------------------------
	  These are all virtual, so they can be overwritten by children.
	---------------------------------------------------------------------------*/
	// General Input functions
	virtual void Input_KeyMove			(const FInputActionValue& InputActionValue);
	virtual void Input_KeyRotate		(const FInputActionValue& InputActionValue);
	virtual void Input_Zoom				(const FInputActionValue& InputActionValue);

	/*---------------------------------------------------------------------------
	  ETriggerEvent::Triggered == Fires while the key is down.
	  ETriggerEvent::Started == Fires once only, on pressed.
	  ETriggerEvent::Completed == Fires once only, on released.

	  A toggle should not call the same function on ETriggerEvent, because one
	  condition might negate another on press or release, and invert the key.
	  
	  A toggle on Started should call its own function, Triggered its own function
	  and Completed its own function. If all the states are to do the same thing,
	  then they all call the same function.

	  By having the same key call different functions on different ETriggerEvents,
	  you are keeping the different functions compartmentalized and avoid bugs.

	  The Input Action Key can all be the same though.
	---------------------------------------------------------------------------*/
	
	virtual void Input_MouseDrag_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_MouseDrag_Released	(const FInputActionValue& InputActionValue);

	virtual void Input_FreeLook_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_FreeLook_Released	(const FInputActionValue& InputActionValue);
	
	virtual void Input_AutoScroll_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_AutoScroll_Released	(const FInputActionValue& InputActionValue);

	//---------------------------------------------------------------------------
	// Mouse Input functions
	//---------------------------------------------------------------------------	
	virtual void Input_Mouse_1_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_1_Released	(const FInputActionValue& InputActionValue);
	
	virtual void Input_Mouse_2_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_2_Released	(const FInputActionValue& InputActionValue);

	virtual void Input_Mouse_3_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_3_Released	(const FInputActionValue& InputActionValue);

public:
	// Keep track of Mouse keys for combos
	bool bMouse_1 = false;
	bool bMouse_2 = false;
	bool bMouse_3 = false;

protected:
	// Mouse Specials
	virtual void Input_Mouse_1_DoubleClick();
	bool bMouse_1_DoubleClick;
	
	//---------------------------------------------------------------------------
	// CTRL Group Assignment
	//---------------------------------------------------------------------------
	virtual void Ctrl_Group_1_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Ctrl_Group_2_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Ctrl_Group_3_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Ctrl_Group_4_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Ctrl_Group_5_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Ctrl_Group_6_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Ctrl_Group_7_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Ctrl_Group_8_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Ctrl_Group_9_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Ctrl_Group_0_Pressed	(const FInputActionValue& InputActionValue);

	//---------------------------------------------------------------------------
	// Modifier Keys
	//---------------------------------------------------------------------------
	virtual void Input_Mod_1_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_1_Released		(const FInputActionValue& InputActionValue);
	
	virtual void Input_Mod_2_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_2_Released		(const FInputActionValue& InputActionValue);
	
	virtual void Input_Mod_3_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_3_Released		(const FInputActionValue& InputActionValue);

	virtual void Input_Mod_4_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_4_Released		(const FInputActionValue& InputActionValue);

	// Update Modifier keys
	virtual void SetModifierKey();

	//---------------------------------------------------------------------------
	// ShortCuts
	//---------------------------------------------------------------------------
	virtual void SelectAll(); // TODO: Needs to check owner, else it will select every entity on the map.
	
};

