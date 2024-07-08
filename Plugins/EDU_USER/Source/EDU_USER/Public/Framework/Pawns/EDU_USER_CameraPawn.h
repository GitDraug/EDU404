// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Framework/Data/FlowLog.h"
#include "Framework/Data/EDU_USER_DataTypes.h"
#include "GameFramework/Pawn.h"
#include "EDU_USER_CameraPawn.generated.h"

struct FInputActionValue;
class UInputMappingContext;

class USceneComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UDataAsset;

class UEDU_USER_CameraPawnInputDataAsset;
class AEDU_USER_PlayerController;
class AEDU_USER_HUD;

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  Pawn is the base class of all actors that can be possessed by players or AI.
  They are the physical representations of players and creatures in a level.

  // TODO: Make sure the Trace Channel is correct.

------------------------------------------------------------------------------*/
UCLASS(Abstract)
class EDU_USER_API AEDU_USER_CameraPawn : public APawn
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	AEDU_USER_CameraPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

//------------------------------------------------------------------------------
// Input Setup
//------------------------------------------------------------------------------
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// MappingContext && Mapping Priority
	virtual void SetInputDefault(const bool bEnabled = true) const;

	// Input mode is UI&Game etc.
	virtual void SetPlayerInputMode();

	// Used to initialize the variables in the DataAsset
	virtual void SetPawnControlDefaults();

protected:
	// We can Add and remove various contexts depending on game
	virtual void AddInputMappingContext(const UInputMappingContext* InputMappingContext, const int32 MappingPriority) const;
	virtual void RemoveInputMappingContext(const UInputMappingContext* InputMappingContext) const;
	
//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------
protected:
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void UnPossessed() override;
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
private:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	USceneComponent* CameraAnchor;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	FLOW_LOG_TICK_VARIABLE_FOR_DEBUG

protected:
	// Holds Input Data with keys and functions, we need to set it in the BP.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "EDU_USER Settings")
	UDataAsset* InputDataAsset;

	UPROPERTY()
  	TObjectPtr<UEDU_USER_CameraPawnInputDataAsset> InputData;

	UPROPERTY()
	TObjectPtr<APlayerController> LocalController;
	
	UPROPERTY()
	TObjectPtr<AEDU_USER_HUD> LocalHUD;

//------------------------------------------------------------------------------
// Camera Settings
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
	
	
private:
	// Ready to tick?
	bool bIsInitialized = false;

	// If we can't trace the ground, we can use the end of the trace.
	FVector CameraTraceEndLocation;
	
	// Last valid terrain trace
	FVector LastValidLocation;

	// Used for interpolation
	FRotator TargetRotation;
	FVector TargetLocation;

	// A YX struct to save the Mouse movement Input, using APlayerController->GetInputMouseDelta();
	FVector2d MouseDirection {0.f, 0.f};
	
	// A YX struct to save the InputComponent axis for our Mouse
	FVector2d MouseAxis;
	
	// We need to store the Screen Size for Camera Edge Scrolling
	FIntVector2 ScreenSize;

	// We also need to store the Mouse Position for various functions cursor-based functions
	FVector2d MousePos;
	FVector2d SavedMousePos;

	//------------------------------------------------------------------------------
	// EdgeScroll settings
	//------------------------------------------------------------------------------
	float ScreenEdgeArea;
	float EdgeScrollSpeed;

	// EdgeScroll Calculations can be used for Other functions too
	FVector2d ScrollDirection;
	
	float DistanceToLeftEdge;
	float DistanceToRightEdge;
	float DistanceToTopEdge;
	float DistanceToBottomEdge;
	
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
	// Zoom tracking Data
	//------------------------------------------------------------------------------
	UPROPERTY()
	float ZoomDistance; // Distance traveled per key press

	UPROPERTY()
	float ZoomTraceLength; // Distance to trace for ground.
	
	UPROPERTY()
	float MouseDragSpeed = 2.f; // Distance to trace for ground.

	// The length SpringArmComponent->TargetArmLength is targeting.
	float TargetZoom;
	
	FHitResult CameraTraceResult;
	FCollisionQueryParams CameraTraceCollisionParams;

	// Default settings
	bool bZoomIn = false;
	bool bZoomOut = false;
	bool bZoomFocusOn = true;
	bool bZoomFocusFinished = true;
	
	//------------------------------------------------------------------------------
	// States for the modifier Keys 
	// ------------------------------------------------------------------------------
	bool bMod_1 = false;
	bool bMod_2 = false;
	bool bMod_3 = false;
	bool bMod_4 = false;

	/*--------------------------------------------------------------------------------
	  Note that the bools are for this class only. The active key, or active combo
	  is set in EEDU_USER_InputModifierKey. This means that other classes only need
	  to keep track of the enum state, instead if 4 bool.
	--------------------------------------------------------------------------------*/
	UPROPERTY()
	EEDU_USER_InputModifierKey ModifierKey;
	
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
protected:
	// Uses various calculations to Move, Zoom and Rotate each frame.
	void UpdateCameraRotation(const float DeltaTime) const;
	void UpdateCameraLocation(const float DeltaTime);
	void UpdateCameraZoom(const float DeltaTime);
	
	// Utility
	void MoveCameraAnchor(const FVector2d& Direction, const float& Speed);
	void AutoPitch(const float DeltaTime); // Pitches the camera depending on zoom level.
	void CameraTrace();
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
	
	// Traces the ground to check we're inside the allowed area.
	// This function originally resided in the controller, thus we passed them by ref.
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
	// Mouse Input functions // TODO: check if _Triggered is superfluous
	//---------------------------------------------------------------------------
	virtual void Input_Mouse_1_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_1_Triggered(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_1_Released	(const FInputActionValue& InputActionValue);
	
	virtual void Input_Mouse_2_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_2_Triggered(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_2_Released	(const FInputActionValue& InputActionValue);

	virtual void Input_Mouse_3_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_3_Triggered(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_3_Released	(const FInputActionValue& InputActionValue);
	
	//---------------------------------------------------------------------------
	// Modifier Input functions
	//---------------------------------------------------------------------------	
	virtual void Input_Mod_1_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_1_Released		(const FInputActionValue& InputActionValue);
	
	virtual void Input_Mod_2_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_2_Released		(const FInputActionValue& InputActionValue);
	
	virtual void Input_Mod_3_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_3_Released		(const FInputActionValue& InputActionValue);

	virtual void Input_Mod_4_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_Mod_4_Released		(const FInputActionValue& InputActionValue);

	// Update EEDU_USER_InputModifierKey
	virtual void SetModifierKey();
	
};

