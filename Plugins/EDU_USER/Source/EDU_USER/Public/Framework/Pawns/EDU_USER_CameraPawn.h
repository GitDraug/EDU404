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

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  Pawn is the base class of all actors that can be possessed by players or AI.
  They are the physical representations of players and creatures in a level.
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

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// MappingContext && Mapping Priority
	virtual void SetInputDefault(const bool bEnabled = true) const;

	// Input mode is UI&Game etc.
	virtual void SetPlayerInputMode();

	// Used to initialize the variables in the DataAsset
	virtual void SetPawnControlDefaults();
	
//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------
protected:
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void AddInputMappingContext(const UInputMappingContext* InputMappingContext, const int32 MappingPriority) const;
	virtual void RemoveInputMappingContext(const UInputMappingContext* InputMappingContext) const;

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

//------------------------------------------------------------------------------
// Camera Settings
//------------------------------------------------------------------------------
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Pitch")
	bool bInvertedPitch = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Rotation")
	bool bInvertedRotation = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Control Parameters | Rotation")
	bool bInvertedZoom = false;
	
private:
	UPROPERTY()
	bool bIsInitialized { false };

	UPROPERTY()
	bool bFreeLook = false;
	
	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY()
	FVector LastValidLocation; // Last valid terrain trace

	UPROPERTY()
	FRotator TargetRotation;
	
	UPROPERTY()
	float RotationSpeed;
	
	UPROPERTY()
	float Pitch;

	UPROPERTY()
	float Yaw;

	// A YX struct to save the InputComponent axis for our Mouse
	FVector2d MouseAxis;
	
	// We need to store the Screen Size for Camera Edge Scrolling
	FVector2d ScreenSize;

	// We also need to store the Mouse Position for various functions
	FVector2d MousePos;

	//------------------------------------------------------------------------------
	// Zoom tracking Data
	//------------------------------------------------------------------------------
	UPROPERTY()
	float TargetZoom; // The length SpringArmComponent->TargetArmLength is targeting.

	UPROPERTY()
	float ZoomDistance; // Distance per key press

	UPROPERTY()
	bool bZoomIn = false;

	UPROPERTY()
	bool bZoomOut = false;

	UPROPERTY()
	bool bZoomFocus = true;
	
	//------------------------------------------------------------------------------
	// States for the modifier Keys 
	// ------------------------------------------------------------------------------
	UPROPERTY()
	bool bMod_1 = false;

	UPROPERTY()
	bool bMod_2 = false;

	UPROPERTY()
	bool bMod_3 = false;

	UPROPERTY()
	bool bMod_4 = false;

	/*--------------------------------------------------------------------------------
	  Note that the bools are for this class only. The active key, or active combo
	  is set in EEDU_USER_InputModifierKey. This means that other classes only need
	  to keep track of the enum state.
	--------------------------------------------------------------------------------*/
	UPROPERTY()
	EEDU_USER_InputModifierKey ModifierKey;
	
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
protected:
	// Uses various calculations to Move, Zoom and Rotate each frame.
	void UpdateCamera(const float DeltaTime);

	// Traces the ground to check we're inside the level
	// Note that the variables used are private; thus we pass them by ref.
	void GetTerrainPosition(FVector& TargetPos, FVector& LastValidPos) const;

	void Input_FreeLook(const FInputActionValue& InputActionValue);
	void UpdateCameraZoom(const float Deltatime);
	void CameraTraceMove();

	/*-----------------------------------------------------------------
	  These are all virtual, so they can be overwritten by children.
	-----------------------------------------------------------------*/
	// General Input functions
	virtual void Input_KeyMove			(const FInputActionValue& InputActionValue);
	virtual void Input_KeyRotate		(const FInputActionValue& InputActionValue);
	virtual void Input_Zoom				(const FInputActionValue& InputActionValue);
	virtual void Input_CameraTraceMove	(const FInputActionValue& InputActionValue);

	//---------------------------------------------------------------------------
	// Mouse Input functions
	//---------------------------------------------------------------------------
	virtual void Input_Mouse_1_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_1_Released	(const FInputActionValue& InputActionValue);
	
	virtual void Input_Mouse_2_Pressed	(const FInputActionValue& InputActionValue);
	virtual void Input_Mouse_2_Released	(const FInputActionValue& InputActionValue);
	
	//---------------------------------------------------------------------------
	// Modifier Input functions
	//---------------------------------------------------------------------------
	virtual void Input_FreeLook_Pressed		(const FInputActionValue& InputActionValue);
	virtual void Input_FreeLook_Released	(const FInputActionValue& InputActionValue);
	
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

