// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Pawns/EDU_USER_CameraPawn.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Framework/Data/EDU_USER_DataTypes.h"
#include "Framework/Data/EDU_USER_CameraPawnInputDataAsset.h"
#include "Framework/Data/FlowLog.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_USER_CameraPawn::AEDU_USER_CameraPawn(const FObjectInitializer& ObjectInitializer) : Super (ObjectInitializer)
{ FLOW_LOG
	// Tick
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Components
	CameraAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("CameraAnchor"));
	RootComponent = CameraAnchor;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 2000.f;
	SpringArmComponent->bDoCollisionTest = false;
	// SpringArmComponent->bInheritPitch = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
}

void AEDU_USER_CameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{ FLOW_LOG
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if(IsLocallyControlled())
	{
		/*------------------------------------------------------------------------------
		  InputDataAsset is NOT a PrimaryDataAsset, so it's ALWAYS loaded. With such
		  a small Data Asset, it really doesn't matter, but we need to ensure it's
		  not null to avoid crashes.

		  With PrimaryDataAssets, you always need to call the Asset Manager to load
		  them manually.

		  This needs to be assigned in the BP, so it's initialized, else it will be null.
		------------------------------------------------------------------------------*/
		if(InputDataAsset == nullptr) { FLOW_LOG_ERROR("InputDataAsset is null, make sure it is set on the CameraPawn.") return; }

		InputData = Cast<UEDU_USER_CameraPawnInputDataAsset>(InputDataAsset);
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
			
			EnhancedInputComponent->BindAction(InputData->KeyMove, ETriggerEvent::Triggered, this, &ThisClass::Input_KeyMove);
			EnhancedInputComponent->BindAction(InputData->KeyRotate, ETriggerEvent::Triggered, this, &ThisClass::Input_KeyRotate);
		
			EnhancedInputComponent->BindAction(InputData->Zoom, ETriggerEvent::Triggered, this, &ThisClass::Input_Zoom);

			EnhancedInputComponent->BindAction(InputData->FreeLook, ETriggerEvent::Triggered, this, &ThisClass::Input_FreeLook);
			EnhancedInputComponent->BindAction(InputData->FreeLook_Pressed, ETriggerEvent::Started, this, &ThisClass::Input_FreeLook_Pressed);
			EnhancedInputComponent->BindAction(InputData->FreeLook_Released, ETriggerEvent::Completed, this, &ThisClass::Input_FreeLook_Released);
			
			//------------------------------------------------------------------------------
			// Modifier Keys
			//------------------------------------------------------------------------------
			EnhancedInputComponent->BindAction(InputData->Mod_1_Pressed, ETriggerEvent::Started, this, &ThisClass::Input_Mod_1_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_1_Released, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_1_Released);
			
			EnhancedInputComponent->BindAction(InputData->Mod_2_Pressed, ETriggerEvent::Started, this, &ThisClass::Input_Mod_2_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_2_Released, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_2_Released);

			EnhancedInputComponent->BindAction(InputData->Mod_3_Pressed, ETriggerEvent::Started, this, &ThisClass::Input_Mod_3_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_3_Released, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_3_Released);
			
			EnhancedInputComponent->BindAction(InputData->Mod_4_Pressed, ETriggerEvent::Started, this, &ThisClass::Input_Mod_4_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_4_Released, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_4_Released);
			
			// TODO
			// EnhancedInputComponent->BindAction(InputData->MouseMove_Pressed, ETriggerEvent::Triggered, this, &ThisClass::);
			// EnhancedInputComponent->BindAction(InputData->MouseMove_Released, ETriggerEvent::Completed, this, &ThisClass::);
			
			// Finish Setup
			SetPlayerInputMode();
			SetInputDefault();
			SetPawnControlDefaults();
		}
	}		
}

void AEDU_USER_CameraPawn::SetInputDefault(const bool bEnabled) const
{ FLOW_LOG
	if (InputData != nullptr && InputData->MappingContext_Default)
	{
		if(bEnabled)
		{
			AddInputMappingContext(InputData->MappingContext_Default, InputData->MapPriority_Default);
		}
		else
		{
			RemoveInputMappingContext(InputData->MappingContext_Default);
		}
	}

	//------------------------------------------------------------------------------
	// Input Data DevCheck for easy bebugging
	//------------------------------------------------------------------------------
	// KeyMove
	FLOW_LOG_IF( (InputData->KeyMoveSpeed ==0 ),			FLOW_LOG_ERROR("InputData->MoveSpeed is 0, the pawn might not be able to Move"))
	FLOW_LOG_IF( (InputData->KeyMoveSpeedMultiplier ==0 ),	FLOW_LOG_ERROR("InputData->KeyMoveSpeedMultiplier is 0, the pawn might not be able to Move"))

	// Mouse Move Scroll
	FLOW_LOG_IF( (InputData->MouseMoveSpeed ==0 ),			FLOW_LOG_ERROR("InputData->MouseMoveSpeed is 0, the pawn might not be able to Move"))
	FLOW_LOG_IF( (InputData->MouseMoveSpeedMultiplier ==0 ),FLOW_LOG_ERROR("InputData->MouseMoveSpeedMultiplier is 0, the pawn might not be able to Move"))

	// Edge Scroll
	FLOW_LOG_IF( (InputData->EdgeScrollSpeed ==0 ),			FLOW_LOG_ERROR("InputData->StartPitch is 0, the pawn might not be able to Move"))
	FLOW_LOG_IF( (InputData->EdgeScrollSpeedMultiplier ==0 ),FLOW_LOG_ERROR("InputData->EdgeScrollSpeedMultiplier is 0, the pawn might not be able to Move"))

	// Rotation
	FLOW_LOG_IF( (InputData->RotationSpeed ==0 ),				FLOW_LOG_ERROR("InputData->RotateSpeed is 0, the pawn might not be able to Move"))
	FLOW_LOG_IF( (InputData->RotationSpeedMultiplier ==0 ),	FLOW_LOG_ERROR("InputData->KeyMoveSpeedMultiplier is 0, the pawn might not be able to Move"))
}

void AEDU_USER_CameraPawn::SetPlayerInputMode()
{ FLOW_LOG
	if(LocalController == nullptr){	LocalController = Cast<APlayerController>(GetController()); }
	
	if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalController->GetLocalPlayer()))
	{ // Reset InputMappings, in case anything remains from the previous level or menu or whatnot.
		InputSubsystem->ClearAllMappings();

		FInputModeGameAndUI InputMode; // Settings container
		InputMode.SetHideCursorDuringCapture(false); // Whether to hide the cursor during temporary mouse capture caused by a mouse down
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // Don't lock to window. It's better to put this in a menu.

		LocalController->SetInputMode(InputMode);
		LocalController->SetShowMouseCursor(true);
	}
}

void AEDU_USER_CameraPawn::SetPawnControlDefaults()
{ FLOW_LOG
	if(IsLocallyControlled() && SpringArmComponent != nullptr && InputData != nullptr)
	{
		// Trace Default
		TargetLocation = GetActorLocation();
		
		// Zoom Default
		TargetZoom = InputData->StartZoom;

		// Rotation Default.
		const FRotator Rotation = SpringArmComponent->GetRelativeRotation();
		TargetRotation = FRotator(Rotation.Pitch - InputData->StartPitch, Rotation.Yaw, 0.f);
		Pitch = TargetRotation.Pitch;
		Yaw = TargetRotation.Yaw;
				
		// All set, start ticking!
		bIsInitialized = true;
	}
}

//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------
void AEDU_USER_CameraPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AEDU_USER_CameraPawn::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();
	
}

void AEDU_USER_CameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Conditional Tick
	if(SpringArmComponent == nullptr || !bIsInitialized) return;

	// Actual movement
	UpdateCamera(DeltaTime);

	if (bZoomIn || bZoomOut)
	{
		UpdateCameraZoom(DeltaTime);
	}
}

void AEDU_USER_CameraPawn::AddInputMappingContext(const UInputMappingContext* InputMappingContext, const int32 MappingPriority) const
{ FLOW_LOG
	if(InputMappingContext == nullptr) { FLOW_LOG_ERROR("InputMappingContext == nullptr") return; }
	
	// To use functions on the APlayerController, we need to guarantee the pointer is of the right type, that's why we cast.
	if(const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if(!InputSubsystem->HasMappingContext(InputMappingContext))
			{
				InputSubsystem->AddMappingContext(InputMappingContext, MappingPriority);
			}
		}
	}
}

void AEDU_USER_CameraPawn::RemoveInputMappingContext(const UInputMappingContext* InputMappingContext) const
{ FLOW_LOG
	if(const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			InputSubsystem->RemoveMappingContext(InputMappingContext);
		}
	}
}

void AEDU_USER_CameraPawn::UnPossessed()
{ FLOW_LOG

	// This might be an overkill, but if another controller posses this pawn, then we want to make sure it is reset.
	bMod_1 = false;
	bMod_2 = false;
	bMod_3 = false;
	bMod_4 = false;
	bFreeLook = false;
	
	Super::UnPossessed();
}

//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------
void AEDU_USER_CameraPawn::GetTerrainPosition(FVector& TargetPos, FVector& LastValidPos) const
{ FLOW_LOG
	// New trace
	FHitResult GroundTrace;
	FCollisionQueryParams CollisionParameters;
	
	FVector TraceStart = TargetPos;
		TraceStart.Z += 10'000.f;
	
	FVector TraceEnd = TargetPos;
		TraceEnd.Z -= 10'000.f;
	
		// Draw the debug sphere
		DrawDebugSphere(GetWorld(), TargetPos,  50.0f, 12, FColor::Green, false, 1.0f, 0, 2.0f);

		// Draw the debug line
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 1.0f, 0, 2.0f);
	
	if(GetWorld())
	{ // RTS_TRACE_CHANNEL_TERRAIN is defined in EDU_USER_StaticGameData 
		if(GetWorld()->LineTraceSingleByChannel(GroundTrace, TraceStart, TraceEnd, ECC_Visibility, CollisionParameters))
		{
			TargetPos = GroundTrace.ImpactPoint;
			// Trace was valid, save it.
			LastValidPos = GroundTrace.ImpactPoint;
		}
		else
		{ // If the tracing fails, because we're outside the map boundaries, we'll use the last cached position.
			TargetPos = LastValidPos;
		}
	}
}

void AEDU_USER_CameraPawn::UpdateCamera(const float Deltatime)
{ FLOW_LOG_TICK
	// TODO: These need their own separate functions with bCheck in tick
	
	if(SpringArmComponent != nullptr && InputData != nullptr)
	{
		// Move Pawn to target location
		// Interpolate vector from Current to Target, scaled by distance to Target, so it has a strong start speed and ease out.
		FVector InterpLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, Deltatime, InputData->MoveInterSpeed);
		SetActorLocation(InterpLocation);


		// Rotate the Camera to target pitch
		// Observer the SpringArmComponent; it acts like a barrel, on a turret, only going up and down. It doesn't affect the rotation or the pitch of the CameraAnchor. 
		FRotator InterpPitch = FMath::RInterpTo(SpringArmComponent->GetRelativeRotation(), TargetRotation, Deltatime, InputData->PitchInterpSpeed);
		InterpPitch.Yaw = 0.f;
		SpringArmComponent->SetRelativeRotation(InterpPitch);

		// Rotate the Camera to target rotation
		// Observer the camera Anchor; since the new move position is in front of the CameraAchor, we don't want it to pitch down into the ground, only rotate.
		RotationSpeed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ? InputData->RotationInterpSpeed * InputData->RotationSpeedMultiplier : InputData->RotationInterpSpeed;
		FRotator InterpRotation = FMath::RInterpTo(CameraAnchor->GetRelativeRotation(), TargetRotation, Deltatime, RotationSpeed);
		InterpRotation.Pitch = 0.f;
		CameraAnchor->SetRelativeRotation(InterpRotation);	

	}
}

void AEDU_USER_CameraPawn::UpdateCameraZoom(const float Deltatime)
{ FLOW_LOG

	TODO: This functions uses tracks the lengt of the cameraarm to know when to stop, but it becopmes near to impossible to
	sync it with the hrizontal movement without a timer. This funciton should be refactored so it tracks trough a timer instead.
	Put the timer in tick and link it to horizontal movement, or link the horizontal movement to this spring arms countdown.
	
	// <!> Note that this is a tick executed function, so be careful to set values that shouldn't be updated on tick.
	if (bZoomOut)
	{
		// Zoom camera to target zoom
		// Interpolate float from Current to Target, scaled by distance to Target, so it has a strong start speed and ease out.
		const float InterpZoom = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetZoom, Deltatime, InputData->ZoomInterpSpeed);
		SpringArmComponent->TargetArmLength = InterpZoom;

		// Check if we should stop
		if(SpringArmComponent->TargetArmLength > TargetZoom * 0.95f)
		{
			bZoomOut = false;
		}
	}

	if (bZoomIn)
	{
		// Zoom camera to target zoom
		// Interpolate float from Current to Target, scaled by distance to Target, so it has a strong start speed and ease out.
		const float InterpZoom = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetZoom, Deltatime, InputData->ZoomInterpSpeed);
		SpringArmComponent->TargetArmLength = InterpZoom;
		
		// Check if we should stop
		if(SpringArmComponent->TargetArmLength < TargetZoom * 1.1f)
		{
			bZoomIn = false;
		}
	}
}

void AEDU_USER_CameraPawn::CameraTraceMove()
{
	// Get the mouse position
	if (LocalController && LocalController->GetMousePosition(MousePos.X, MousePos.Y))
	{
		// Deproject the mouse position to a world position and direction
		FVector WorldLocation, WorldDirection;
		if (LocalController->DeprojectScreenPositionToWorld(MousePos.X, MousePos.Y, WorldLocation, WorldDirection))
		{
			// Define the start and end points of the trace
			FVector TraceStart = WorldLocation;
			FVector TraceEnd = WorldLocation + (WorldDirection * 10000'00.0f); // Trace 10k meters in the direction of the world direction

			// Perform the line trace
			FHitResult CameraTrace;
			FCollisionQueryParams CollisionParameters;
			CollisionParameters.AddIgnoredActor(this); // Ignore the player

			// Perform the trace
			if(GetWorld()->LineTraceSingleByChannel(CameraTrace, TraceStart, TraceEnd, ECC_Visibility, CollisionParameters))
			{
				// Check if we hit something
				if(CameraTrace.ImpactPoint.X != 0 && CameraTrace.ImpactPoint.Y != 0)
				{
					/*---------------------------------------------------------------------------
					  Due to vector math, we need to multiply the VectorSum in parts,
					  otherwise we'll end up all over the place. Dividing it twice will give
					  us 25% of the distance between, making it a smooth ride.
					---------------------------------------------------------------------------*/
					FVector HalfWayPoint = (CameraTrace.ImpactPoint + TargetLocation) * 0.5f;
					TargetLocation = (TargetLocation + HalfWayPoint) * 0.5f;

					// Level the new position with the ground.
					GetTerrainPosition(TargetLocation, LastValidLocation);
					
					// Draw the debug sphere
					DrawDebugSphere(GetWorld(), CameraTrace.ImpactPoint, 50.0f, 12, FColor::Blue, false, 1.0f, 0, 2.0f);
					
					DrawDebugSphere(GetWorld(), TargetLocation, 100.0f, 12, FColor::Green, false, 1.0f, 0, 2.0f);
					
					DrawDebugSphere(GetWorld(), LastValidLocation, 150.0f, 12, FColor::Red, false, 1.0f, 0, 2.0f);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
// Functionality: Input Functions
//------------------------------------------------------------------------------

void AEDU_USER_CameraPawn::Input_KeyMove(const FInputActionValue& InputActionValue)
{ FLOW_LOG	
	// WASD movement
	if(SpringArmComponent != nullptr && InputData != nullptr && ensure(InputActionValue.GetValueType() == EInputActionValueType::Axis2D))
	{
		const FVector2d Value = InputActionValue.Get<FVector2d>();
		const float Speed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ? InputData->KeyMoveSpeed * InputData->KeyMoveSpeedMultiplier : InputData->KeyMoveSpeed;

		GetTerrainPosition(TargetLocation,LastValidLocation);

		// We're not rotating the SpringArm, but the CameraAnchor.
		TargetLocation += CameraAnchor->GetRelativeRotation().RotateVector(FVector(Value.X * Speed, Value.Y * Speed, 0.0f)); // Z should be 0.
	}
}

void AEDU_USER_CameraPawn::Input_KeyRotate(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	// if(bFreeLook) { return; }
	if(InputData != nullptr && ensure(InputActionValue.GetValueType() == EInputActionValueType::Axis1D))
	{
		float YawInputValue = InputActionValue.Get<float>();
		if(bInvertedRotation){ YawInputValue *= -1; }
		
		TargetRotation = FRotator(
		// Don't Change the already existing Pitch
			TargetRotation.Pitch,
		// Add the new Yaw Input to the already existing Yaw, 360 is no issue. // TODO: Do the same with Pitch
			TargetRotation.Yaw + YawInputValue * InputData->RotationSpeed,
		// Do not roll.
			0.f
		);
	}
	UE_LOG(LogTemp, Display, TEXT("%f"),TargetRotation.Yaw);
}

void AEDU_USER_CameraPawn::Input_FreeLook(const FInputActionValue& InputActionValue)
{
	if (!bFreeLook) { return; }
	if (InputData == nullptr || InputActionValue.GetValueType() != EInputActionValueType::Axis2D) {	FLOW_LOG_ERROR("InputActionValue is not Axis2D, check the InputAction."); return; }

	// Extract X and Y from the InputActionValue
	FVector2D AxisValue = InputActionValue.Get<FVector2D>();

	// If the Player prefers inverted settings
	if(bInvertedRotation){ AxisValue.X *= -1; }
	if(bInvertedPitch){ AxisValue.Y *= -1; }
    
	// Modifier Keys
	float PitchSpeed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ? InputData->PitchSpeed * InputData->PitchSpeedMultiplier : InputData->PitchSpeed;
	float PitchInputValue = AxisValue.Y * PitchSpeed;
    
	float YawSpeed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ? InputData->RotationSpeed * InputData->RotationSpeedMultiplier : InputData->RotationSpeed;
	float YawInputValue = AxisValue.X * YawSpeed;
	
	TargetRotation = FRotator(
	// We need to clamp the Pitch to stay within reason
		FMath::Clamp(TargetRotation.Pitch + PitchInputValue, InputData->PitchMin, InputData->PitchMax),
	// Add the new Yaw Input to the already existing Yaw
		TargetRotation.Yaw + YawInputValue,
	// Do not roll.
		0.f
	);
}

void AEDU_USER_CameraPawn::Input_Zoom(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	/*------------------------------------------------------------------------------
	  Since we track the mouse even after the player stops scrolling, as the
	  camera interpolates, we need the mouse scroll to act as a trigger with
	  instruction of how far to move.
	 ------------------------------------------------------------------------------*/
	// Break FInputActionValue& into float.
	float Direction = InputActionValue.Get<float>();

	// Invert direction if player prefers it.
	if(!bInvertedZoom){ Direction *= -1; }
	
	// Modify speed depending on button combo.
	ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ?
		ZoomDistance = (SpringArmComponent->TargetArmLength * 0.1) * InputData->ZoomDistance * InputData->ZoomDistanceMultiplier :
		ZoomDistance = (SpringArmComponent->TargetArmLength * 0.1) * InputData->ZoomDistance;
	
	// Calculate how far we should zoom.
	TargetZoom = FMath::Clamp(TargetZoom + (ZoomDistance * Direction), InputData->MinZoom, InputData->MaxZoom);
	
	if(Direction < 0.f)
	{ // Zoom in
		bZoomIn = true;
		bZoomOut = false;
		FLOW_LOG_WARNING("Zooming in")

		// Check if we should focus on Mouse while zooming in
		if (bZoomFocus)
		{
			CameraTraceMove();
		}
	}
	else
	{ // Zoom out
		bZoomIn = false;
		bZoomOut = true;
		FLOW_LOG_WARNING("Zooming out")
	}
	
	/*----------------------------------------------- Deprecated -------------------------------------------------------
	// This version only Zooms in and out, it doesn't track.
	 
	if(InputData != nullptr && ensure(InputActionValue.GetValueType() == EInputActionValueType::Axis1D))
	{
		// Modifier Key
		float Speed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ? InputData->ZoomSpeed * InputData->ZoomSpeedMultiplier : InputData->ZoomSpeed;
		if(bInvertedZoom){ Speed *= -1; }

		TargetZoom = FMath::Clamp(TargetZoom + InputActionValue.Get<float>() * Speed, InputData->MinZoom, InputData->MaxZoom);
	}
	------------------------------------------------------------------------------------------------------------------*/
}

//---------------------------------------------------------------------------
// Functionality: Mouse Input functions
//---------------------------------------------------------------------------

void AEDU_USER_CameraPawn::Input_Mouse_1_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_Mouse_1_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_Mouse_2_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_Mouse_2_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_CameraTraceMove(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	CameraTraceMove();
}

//------------------------------------------------------------------------------
// Functionality: Modifier Keys
//------------------------------------------------------------------------------

void AEDU_USER_CameraPawn::Input_FreeLook_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bFreeLook = true;
	
	if(APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// In FInputModeGameOnly, the Enhanced input system will always track the mouse.
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->bShowMouseCursor = false; 
			PlayerController->bEnableClickEvents = false; 
			PlayerController->bEnableMouseOverEvents = false;
		}
	}

}

void AEDU_USER_CameraPawn::Input_FreeLook_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bFreeLook = false;
	if(APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// In FInputModeGameAndUI, the Enhanced input system will only track the mouse while a mouse button is clicked.
			FInputModeGameAndUI InputMode;
			LocalController->SetInputMode(InputMode);
			LocalController->bShowMouseCursor = true; 
			LocalController->bEnableClickEvents = true; 
			LocalController->bEnableMouseOverEvents = true;
		}
	}
}

void AEDU_USER_CameraPawn::Input_Mod_1_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_1 = true;
	SetModifierKey();
}

void AEDU_USER_CameraPawn::Input_Mod_1_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_1 = false;
	SetModifierKey();
}

void AEDU_USER_CameraPawn::Input_Mod_2_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_2 = true;
	SetModifierKey();
}

void AEDU_USER_CameraPawn::Input_Mod_2_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_2 = false;
	SetModifierKey();
}

void AEDU_USER_CameraPawn::Input_Mod_3_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_3 = true;
	SetModifierKey();
}

void AEDU_USER_CameraPawn::Input_Mod_3_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_3 = false;
	SetModifierKey();
}

void AEDU_USER_CameraPawn::Input_Mod_4_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_4 = true;
	SetModifierKey();
}

void AEDU_USER_CameraPawn::Input_Mod_4_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_4 = false;
	SetModifierKey();
}

void AEDU_USER_CameraPawn::SetModifierKey()
{ FLOW_LOG
	/*--------------------------------------------------------------------------------
	  Note that this evaluation will start over every time a new key is pressed,
	  I tried using a switch, but switches can't be used with structs unless
	  overloaded, and it's not worth the headache.

	  I don't like nested ifs, but since this is only called on key press,
	  it's really not that performance heavy.  // Draug
	--------------------------------------------------------------------------------*/

	using enum EEDU_USER_InputModifierKey; //  for readability
	
	//----------------------------------------------------------
	// Shift
	//-----------------------------------------------------------
	if (bMod_1)
	{
		ModifierKey = Mod_1;
		FLOW_LOG_WARNING("Mod Key = 1")
	
		if (bMod_1 && bMod_2){ ModifierKey = Mod_12; FLOW_LOG_WARNING("Mod Key = 12")	}
		if (bMod_1 && bMod_3){ ModifierKey = Mod_13; FLOW_LOG_WARNING("Mod Key = 13") }
		if (bMod_1 && bMod_4){ ModifierKey = Mod_14; FLOW_LOG_WARNING("Mod Key = 14") }

		if (bMod_1 && bMod_2 && bMod_3){ ModifierKey = Mod_123; FLOW_LOG_WARNING("Mod Key = 123") }
		if (bMod_1 && bMod_2 && bMod_4){ ModifierKey = Mod_124; FLOW_LOG_WARNING("Mod Key = 124") }
		if (bMod_1 && bMod_3 && bMod_4){ ModifierKey = Mod_134;	FLOW_LOG_WARNING("Mod Key = 134") }

		return;
	}

	//-----------------------------------------------------------
	// Ctrl
	//-----------------------------------------------------------
	if (bMod_2)
	{
		ModifierKey = Mod_2; FLOW_LOG_WARNING("ModifierKey = 2")
		
		if (bMod_2 && bMod_3){ ModifierKey = Mod_23; FLOW_LOG_WARNING("Mod Key = 23") }
		if (bMod_2 && bMod_4){ ModifierKey = Mod_24; FLOW_LOG_WARNING("Mod Key = 24") }
		
		if (bMod_2 && bMod_3 && bMod_4){ ModifierKey = Mod_234; FLOW_LOG_WARNING("Mod Key = 234") }
		return;
	}

	//-----------------------------------------------------------
	// Alt
	//-----------------------------------------------------------
	if (bMod_3)
	{
		ModifierKey = Mod_3; FLOW_LOG_WARNING("Mod Key = 3")
		
		if (bMod_3 && bMod_4){ ModifierKey = Mod_34; FLOW_LOG_WARNING("Mod Key = 34")	}
		return;
	}
	
	//-----------------------------------------------------------
	// Space
	//-----------------------------------------------------------
	if (bMod_4)
	{
		ModifierKey = Mod_4; FLOW_LOG_WARNING("Mod Key = 4")
		return;
	}

	//-----------------------------------------------------------
	// No Modifier
	//-----------------------------------------------------------
	ModifierKey = NoModifier;
	FLOW_LOG_WARNING("Mod Key = NoModifier")
	
	// end using enum EEDU_USER_InputModifierKey;
}

