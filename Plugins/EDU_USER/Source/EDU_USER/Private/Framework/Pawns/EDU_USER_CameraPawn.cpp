// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Pawns/EDU_USER_CameraPawn.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Data/EDU_USER_DataTypes.h"
#include "Framework/Data/EDU_USER_CameraPawnInputDataAsset.h"
#include "Framework/Data/FlowLog.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/HUD/EDU_USER_HUD.h"

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
	SpringArmComponent->bDoCollisionTest = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	// Initiate Controller
	if(LocalController == nullptr){	LocalController = Cast<APlayerController>(GetController()); }
}

//------------------------------------------------------------------------------
// Input Setup
//------------------------------------------------------------------------------

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
			
			EnhancedInputComponent->BindAction(InputData->FreeLookToggle, ETriggerEvent::Started, this, &ThisClass::Input_FreeLook_Pressed);
			EnhancedInputComponent->BindAction(InputData->FreeLookToggle, ETriggerEvent::Completed, this, &ThisClass::Input_FreeLook_Released);
			
			EnhancedInputComponent->BindAction(InputData->MouseDragToggle, ETriggerEvent::Started, this, &ThisClass::Input_MouseDrag_Pressed);
			EnhancedInputComponent->BindAction(InputData->MouseDragToggle, ETriggerEvent::Completed, this, &ThisClass::Input_MouseDrag_Released);

			EnhancedInputComponent->BindAction(InputData->AutoScrollToggle, ETriggerEvent::Started, this, &ThisClass::Input_AutoScroll_Pressed);
			EnhancedInputComponent->BindAction(InputData->AutoScrollToggle, ETriggerEvent::Completed, this, &ThisClass::Input_AutoScroll_Released);
			
			//------------------------------------------------------------------------------
			// Modifier Keys
			//------------------------------------------------------------------------------
			EnhancedInputComponent->BindAction(InputData->Mod_1, ETriggerEvent::Started, this, &ThisClass::Input_Mod_1_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_1, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_1_Released);
			
			EnhancedInputComponent->BindAction(InputData->Mod_2, ETriggerEvent::Started, this, &ThisClass::Input_Mod_2_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_2, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_2_Released);

			EnhancedInputComponent->BindAction(InputData->Mod_3, ETriggerEvent::Started, this, &ThisClass::Input_Mod_3_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_3, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_3_Released);
			
			EnhancedInputComponent->BindAction(InputData->Mod_4, ETriggerEvent::Started, this, &ThisClass::Input_Mod_4_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_4, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_4_Released);

			//------------------------------------------------------------------------------
			// Mouse Input functions; Multifunctional, so best to use simple names.
			//------------------------------------------------------------------------------
			EnhancedInputComponent->BindAction(InputData->Mouse_1, ETriggerEvent::Started, this, &ThisClass::Input_Mouse_1_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mouse_1, ETriggerEvent::Triggered, this, &ThisClass::Input_Mouse_1_Triggered);
			EnhancedInputComponent->BindAction(InputData->Mouse_1, ETriggerEvent::Completed, this, &ThisClass::Input_Mouse_1_Pressed);
			
			EnhancedInputComponent->BindAction(InputData->Mouse_2, ETriggerEvent::Started, this, &ThisClass::Input_Mouse_2_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mouse_1, ETriggerEvent::Triggered, this, &ThisClass::Input_Mouse_1_Triggered);
			EnhancedInputComponent->BindAction(InputData->Mouse_2, ETriggerEvent::Completed, this, &ThisClass::Input_Mouse_2_Released);

			EnhancedInputComponent->BindAction(InputData->Mouse_2, ETriggerEvent::Started, this, &ThisClass::Input_Mouse_3_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mouse_2, ETriggerEvent::Triggered, this, &ThisClass::Input_Mouse_3_Triggered);
			EnhancedInputComponent->BindAction(InputData->Mouse_2, ETriggerEvent::Completed, this, &ThisClass::Input_Mouse_3_Released);
			
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
	// Input Data DevCheck for easy debugging
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

void AEDU_USER_CameraPawn::SetPawnControlDefaults()
{ FLOW_LOG
	if(IsLocallyControlled() && SpringArmComponent != nullptr && InputData != nullptr)
	{
		// Trace Default
		TargetLocation = GetActorLocation();
		
		// Zoom Default
		TargetZoom = InputData->StartZoom;
		ZoomTraceLength = InputData->ZoomTraceLength;
		SpringArmComponent->TargetArmLength = TargetZoom;

		// Rotation Default.
		const FRotator Rotation = SpringArmComponent->GetRelativeRotation();
		TargetRotation = FRotator(Rotation.Pitch - InputData->StartPitch, Rotation.Yaw, 0.f);

		// EdgeScroll Settings
		ScreenEdgeArea = InputData->ScreenEdgeArea;
		EdgeScrollSpeed = InputData->EdgeScrollSpeed;

		// Make sure the Camera doesn't hit itself when tracing.
		CameraTraceCollisionParams.AddIgnoredActor(this); // Ignore the player
				
		// All set, start ticking!
		bIsInitialized = true;

		// Interpolation for smooth camera setup
		EnableInterpRotation();
	}
}

//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------
void AEDU_USER_CameraPawn::PossessedBy(AController* NewController)
{ FLOW_LOG
	Super::PossessedBy(NewController);
}

void AEDU_USER_CameraPawn::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();
}

void AEDU_USER_CameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug Messages
	/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Yellow, FString::Printf(TEXT("AutoScrollDirection.X %f"), ScrollDirection.X));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Yellow, FString::Printf(TEXT("AutoScrollDirection.Y %f"), ScrollDirection.Y));
	
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Red, FString::Printf(TEXT("MousePosX %f"), MousePos.X));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Red, FString::Printf(TEXT("MousePosY %f"), MousePos.Y));
	
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT("Distance to Left Edge: %f"), DistanceToLeftEdge));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT("Distance to Right Edge: %f"), DistanceToRightEdge));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT("Distance to Top Edge: %f"), DistanceToTopEdge));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT("Distance to Bottom Edge: %f"), DistanceToBottomEdge));
	
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("GetComponentRotation().SpringArm %% %f"), SpringArmComponent->TargetArmLength / InputData->MaxZoom));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("SpringArmComponent.Yaw %f"), SpringArmComponent->GetComponentRotation().Yaw));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("SpringArmComponent.Pitch %f"), SpringArmComponent->GetComponentRotation().Pitch));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("CameraAnchor.Yaw %f"), CameraAnchor->GetComponentRotation().Yaw));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("CameraAnchor.Pitch %f"), CameraAnchor->GetComponentRotation().Pitch));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("TargetPitch %f"), TargetRotation.Pitch));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("TargetYaw %f"), TargetRotation.Yaw));

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("bMouseDrag %d"), bMouseDrag));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("bFreeLook %d"), bFreeLook));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("bEdgeScroll %d"), bEdgeScroll));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("bAutoScroll %d"), bAutoScroll));

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MouseDirection.X %f"), MouseDirection.X));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MouseDirection.Y %f"), MouseDirection.Y));
	
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("SavedMousePosY %f"), SavedMousePos.Y));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("SavedMousePosX %f"), SavedMousePos.X));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MousePosY %f"), MousePos.Y));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MousePosX %f"), MousePos.X));

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, FString::Printf(TEXT("bShowMouseCursor %d"), LocalController->bShowMouseCursor));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, FString::Printf(TEXT("bEnableClickEvents %d"), LocalController->bEnableClickEvents));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, FString::Printf(TEXT("bEnableMouseOverEvents %d"), LocalController->bEnableMouseOverEvents));

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MouseDirection.X %f"), MouseDirection.X));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MouseDirection.Y %f"), MouseDirection.Y));
	-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	
	// Conditional Tick
	if(SpringArmComponent == nullptr || InputData == nullptr || !bIsInitialized) { return; }
	
	if(bZoomIn || bZoomOut)
	{
		UpdateCameraZoom(DeltaTime);
		if (bZoomFocusOn && !bZoomFocusFinished)
		{
			UpdateCameraLocation(DeltaTime);
		}
	}

	// We put Interpolation on a timer to stop it form ticking all the time.
	if(InterpTimer > 0 && bInterpMov)
	{
		UpdateCameraLocation(DeltaTime);
		InterpTimer -= DeltaTime;
	}
	else
	{
		bInterpMov = false;
	}

	// We put Interpolation on a timer to stop it form ticking all the time.
	if(InterpTimer > 0 && bInterpRot)
	{
		UpdateCameraRotation(DeltaTime);
		InterpTimer -= DeltaTime;
	}
	else
	{
		bInterpRot = false;
	}
	
	// EdgeScroll will interfere with FreeLook and MouseDrag.
	if(bFreeLook && !bMouseDrag && !bAutoScroll)
	{
		FreeLook();
		UpdateCameraRotation(DeltaTime);
		return;
	}

	// MouseDrag will interfere with FreeLook and edgeScroll.
	if(bMouseDrag && !bFreeLook&& !bAutoScroll)
	{
		MouseDrag();
		return;
	}
	
	// MouseDrag and FreeLook interferes with EdgeScroll
	if(bAutoScroll && !bFreeLook && !bMouseDrag)
	{
		AutoScroll();
		return;
	}
	
	// MouseDrag and FreeLook interferes with EdgeScroll
	if(bEdgeScroll == true)
	{
		EdgeScroll();
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
	bMouseDrag = false;
	
	Super::UnPossessed();
}

//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------

void AEDU_USER_CameraPawn::ResetCamera()
{ FLOW_LOG
	EnableInterpRotation();
	EnableInterpMovement();
	
	// TargetLocation.X = 0.f;
	// TargetLocation.Y = 0.f;

	TargetRotation.Pitch = AutoPitchMin;
	// TargetRotation.Yaw = 0.f;

	if(bAutoPitch)
	{
		bAutoPitchDisengaged = false;
	}
}

void AEDU_USER_CameraPawn::EnableInterpRotation()
{ FLOW_LOG
	bInterpRot = true;
	InterpTimer = 1.f;
}

void AEDU_USER_CameraPawn::EnableInterpMovement(float Time)
{ FLOW_LOG
	bInterpMov = true;
	InterpTimer = Time;
}

void AEDU_USER_CameraPawn::CameraTrace()
{ FLOW_LOG
	// Get the mouse position
	if (LocalController && LocalController->GetMousePosition(MousePos.X, MousePos.Y))
	{
		// Deproject the mouse position to a world position and direction
		FVector WorldLocation, WorldDirection;
		if (LocalController->DeprojectScreenPositionToWorld(MousePos.X, MousePos.Y, WorldLocation, WorldDirection))
		{
			// Define the start and end points of the trace
			FVector TraceStart = WorldLocation;
			FVector TraceEnd = WorldLocation + (WorldDirection * (ZoomTraceLength+SpringArmComponent->TargetArmLength));
			
			// Perform the CameraTrace
			if(GetWorld()->LineTraceSingleByChannel(CameraTraceResult, TraceStart, TraceEnd, ECC_Visibility, CameraTraceCollisionParams))
			{
				// Check if we hit something
				if(CameraTraceResult.bBlockingHit)
				{					
					/*---------------------------------------------------------------------------
					  Due to vector math, we need to multiply the VectorSum in parts,
					  otherwise we'll end up all over the place, dividing it will make
					  it a smoother ride than jumping the entire distance immediately.
					---------------------------------------------------------------------------*/
					FVector HalfWayPoint = (CameraTraceResult.ImpactPoint + TargetLocation) * 0.5f;
					TargetLocation = HalfWayPoint; // We move to HalfWayPoint, but we know where the ground is.
					LastValidLocation = CameraTraceResult.ImpactPoint;
					
					/*///-------------------------------------------------------------------------------------
					  DrawDebugSphere(GetWorld(), CameraTraceResult.ImpactPoint, 50.0f, 12, FColor::Blue, false, 30.0f, 0, 2.0f);
					  DrawDebugSphere(GetWorld(), TargetLocation, 100.0f, 12, FColor::Green, false, 30.0f, 0, 2.0f);
					  DrawDebugSphere(GetWorld(), LastValidLocation, 150.0f, 12, FColor::Red, false, 30.0f, 0, 2.0f);
					//*///-------------------------------------------------------------------------------------
					return;
				}
			}

			//-----------------------------------------------------------------------------
			// If we don't hit the ground, we can travel in the general direction instead.
			//-----------------------------------------------------------------------------
			float Distance = SpringArmComponent->TargetArmLength > 500'00.f ? SpringArmComponent->TargetArmLength : 500'00.f;
			CameraTraceEndLocation = WorldLocation + (WorldDirection * Distance);
			TraceStart = CameraTraceEndLocation;
			TraceEnd = CameraTraceEndLocation;

			// Due to vector math we need revers direction if the CameraTraceEndLocation is below 0, else it will shoot upwards.
			if(CameraTraceEndLocation.Z <= 0.f)
			{
				TraceEnd.Z =- SpringArmComponent->TargetArmLength + Distance;
			}
			else
			{
				TraceEnd.Z =- SpringArmComponent->TargetArmLength - Distance;
			}

			/*///-------------------------------------------------------------------------------------
			DrawDebugSphere(GetWorld(), TraceStart, 1000.0f, 12, FColor::Red, false, 10.0f, 0, 25.0f);
			DrawDebugSphere(GetWorld(), TraceEnd, 1000.0f, 12, FColor::Blue, false, 10.0f, 0, 25.0f);
			
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 10.0f, 0, 25.0f);
			//*///-------------------------------------------------------------------------------------
			
			if(GetWorld()->LineTraceSingleByChannel(CameraTraceResult, TraceStart, TraceEnd, ECC_Visibility, CameraTraceCollisionParams))
			{
				if(CameraTraceResult.ImpactPoint.X && CameraTraceResult.ImpactPoint.Y)
				{
					FVector HalfWayPoint = (CameraTraceResult.ImpactPoint + TraceStart) * 0.5f;
					if(bAutoPitch && !bAutoPitchDisengaged)
					{ // If AutoPitch is on, we want to travel to the ground, pitching the camera in the air will look wierd.
						TargetLocation = CameraTraceResult.ImpactPoint;
					}
					else
					{ // If the user if FreeLooking, it's better to stop midway.
						TargetLocation = HalfWayPoint;
					}
					
					LastValidLocation = CameraTraceResult.ImpactPoint;
					
					/*///-------------------------------------------------------------------------------------
					DrawDebugSphere(GetWorld(), HalfWayPoint, 1000.0f, 12, FColor::Red, false, 10.0f, 0, 40.0f);
					DrawDebugSphere(GetWorld(), TargetLocation, 1000.0f, 12, FColor::Blue, false, 10.0f, 0, 20.0f);
					DrawDebugSphere(GetWorld(), LastValidLocation, 1000.0f, 12, FColor::Green, false, 10.0f, 0, 5.0f);
					//*///-------------------------------------------------------------------------------------
				}
			}
		}
	}
}

void AEDU_USER_CameraPawn::MoveCameraAnchor(const FVector2d& Direction, const float& Speed)
{ FLOW_LOG
	// We're rotating CameraAnchor not the SpringArm
	TargetLocation += CameraAnchor->GetRelativeRotation().RotateVector(FVector(Direction.X * Speed, Direction.Y * Speed, 0.0f)); // Z should be 0.
	GetTerrainPosition(TargetLocation, LastValidLocation);
	
	EnableInterpMovement();
}

void AEDU_USER_CameraPawn::GetTerrainPosition(FVector& TargetPos, FVector& LastValidPos) const
{ FLOW_LOG
	// New trace
	FHitResult GroundTrace;
	FCollisionQueryParams CollisionParameters;
	
	FVector TraceStart = TargetPos;
		TraceStart.Z += 100'00.f;
	
	FVector TraceEnd = TargetPos;
		TraceEnd.Z -= SpringArmComponent->TargetArmLength + 50'00.f;
	
		// Draw the debug sphere
		// DrawDebugSphere(GetWorld(), TargetPos,  50.0f, 12, FColor::Green, false, 1.0f, 0, 2.0f);

		// Draw the debug line
		// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 1.0f, 0, 2.0f);
	
	if(GetWorld())
	{ // RTS_TRACE_CHANNEL_TERRAIN is defined in EDU_USER_StaticGameData 
		if(GetWorld()->LineTraceSingleByChannel(GroundTrace, TraceStart, TraceEnd, ECC_Visibility, CollisionParameters))
		{
			TargetPos = GroundTrace.ImpactPoint;
			// Trace was valid, save it.
			LastValidPos = GroundTrace.ImpactPoint;
			
			FVector HalfWayPoint = (LastValidPos + TargetPos) * 0.5f;
			TargetPos = HalfWayPoint; // Smoother
		}
		else
		{ // If the tracing fails, because we're outside the map boundaries, we'll use the last cached position.
			FVector HalfWayPoint = (LastValidPos + TargetPos) * 0.5f;
			TargetPos = HalfWayPoint; // Smoother
		}
	}
}

void AEDU_USER_CameraPawn::UpdateCameraRotation(const float DeltaTime) const
{ FLOW_LOG
	// Rotate the Camera to target pitch
	// Observe the SpringArmComponent; it acts like a barrel, on a turret, only going up and down. It doesn't affect the rotation or the pitch of the CameraAnchor. 
	FRotator InterpPitch = FMath::RInterpTo(SpringArmComponent->GetRelativeRotation(), TargetRotation, DeltaTime, InputData->PitchInterpSpeed);
	InterpPitch.Yaw = 0.f;
	SpringArmComponent->SetRelativeRotation(InterpPitch);

	// Rotate the Camera to target rotation
	// Observe the CameraAnchor; since the new move position is in front of the CameraAnchor, we don't want it to pitch down into the ground, only rotate.
	float RotationSpeed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ? InputData->RotationInterpSpeed * InputData->RotationSpeedMultiplier : InputData->RotationInterpSpeed;
	FRotator InterpRotation = FMath::RInterpTo(CameraAnchor->GetRelativeRotation(), TargetRotation, DeltaTime, RotationSpeed);
	InterpRotation.Pitch = 0.f;
	CameraAnchor->SetRelativeRotation(InterpRotation);	
}

void AEDU_USER_CameraPawn::UpdateCameraLocation(const float DeltaTime)
{ FLOW_LOG
	// Move Pawn to target location
	// Interpolate vector from Current to Target, scaled by distance to Target, so it has a strong start speed and ease out.
	FVector InterpLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, InputData->MoveInterSpeed);
	SetActorLocation(InterpLocation);
}

void AEDU_USER_CameraPawn::UpdateCameraZoom(const float DeltaTime)
{ FLOW_LOG
	// <!> Note that this is a tick executed function, so be careful to set values that shouldn't be updated on tick.
	if (bZoomOut)
	{
		// Zoom camera to target zoom
		// Interpolate float from Current to Target, scaled by distance to Target, so it has a strong start speed and ease out.
		const float InterpZoom = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetZoom, DeltaTime, InputData->ZoomInterpSpeed);
		SpringArmComponent->TargetArmLength = InterpZoom;
		
		// Check if we should stop, we use 95%, because Interpolation algebra will run close to forever as the value gets smaller.
		if(SpringArmComponent->TargetArmLength > TargetZoom * 0.98f)
		{
			bZoomOut = false;

			if(bAutoPitch && SpringArmComponent->TargetArmLength > InputData->MaxZoom * 0.98f)
			{
				// If AutoPitch is Disengaged due to FreeLook, then we want to reset the camera when it Zooms out fully, same as in SupCom.
				ResetCamera();
			}
		}
		
		if(bAutoPitch && !bAutoPitchDisengaged)
		{
			AutoPitch(DeltaTime);
		}
	}

	if (bZoomIn)
	{
		// Zoom camera to target zoom
		// Interpolate float from Current to Target, scaled by distance to Target, so it has a strong start speed and ease out.
		const float InterpZoom = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetZoom, DeltaTime, InputData->ZoomInterpSpeed);
		SpringArmComponent->TargetArmLength = InterpZoom;
		
		// Check if we should stop, we use 95%, because Interpolation algebra will run close to forever as the value get smaller.
		if(SpringArmComponent->TargetArmLength < TargetZoom * 1.1f)
		{
			bZoomIn = false;
			bZoomFocusFinished = true;
		}

		if(bAutoPitch && !bAutoPitchDisengaged)
		{
			AutoPitch(DeltaTime);
		}
	}
}

void AEDU_USER_CameraPawn::UpdateMouseDirection()
{ // TODO: Copy this to the playerController as a Getter.
	/*----------------------- vs GetInputMouseDelta ------------------------------------
	  This function updates the Mouse Input as a value, without the need of a viewport,
	  meaning you can keep dragging as long as you have mouse pad space on your desk.

	  The APlayerController->GetInputMouseDelta() does a similar thing, but only if
	  a mouse button is held down. this one runs on tick instead, regardless if a
	  mouse button uis held.
	---------------------------------------------------------------------------------*/
	LocalController->GetMousePosition(MousePos.X, MousePos.Y);

	//-----------------------------------------------
	MouseDirection.X = MousePos.X - SavedMousePos.X;
	MouseDirection.Y = SavedMousePos.Y - MousePos.Y;
	//-----------------------------------------------

	// If the Player prefers inverted settings
	if(bInvertedYaw){ MouseDirection.X *= -1; }
	if(bInvertedPitch){ MouseDirection.Y *= -1; }

	// We return the mouse Position to where we started, else it will run in to the end of the monitor.
	LocalController->SetMouseLocation(SavedMousePos.X, SavedMousePos.Y);
}

void AEDU_USER_CameraPawn::DisableMouseEvents()
{ FLOW_LOG
	LocalController->GetMousePosition(MousePos.X, MousePos.Y);

	// Initial Position
	SavedMousePos.X = MousePos.X;
	SavedMousePos.Y = MousePos.Y;
	
	LocalController->bShowMouseCursor = false;
	LocalController->bEnableClickEvents = false;
	LocalController->bEnableTouchEvents = false;
	LocalController->bEnableMouseOverEvents = false;
}

void AEDU_USER_CameraPawn::EnableMouseEvents() const
{ FLOW_LOG
	LocalController->SetMouseLocation(SavedMousePos.X, SavedMousePos.Y);
	
	LocalController->bShowMouseCursor = true;
	LocalController->bEnableClickEvents = true;
	LocalController->bEnableTouchEvents = true;
	LocalController->bEnableMouseOverEvents = true;
}

void AEDU_USER_CameraPawn::AutoPitch(const float DeltaTime)
{
	float PitchRange = AutoPitchMax - AutoPitchMin;
	float InvertedPercentage = 1.0f - SpringArmComponent->TargetArmLength / InputData->MaxZoom;
	float AutoPitch = AutoPitchMin + (PitchRange * InvertedPercentage);
			
	FRotator InterpPitch = FMath::RInterpTo(SpringArmComponent->GetRelativeRotation(), TargetRotation, DeltaTime, InputData->PitchInterpSpeed);
	InterpPitch.Yaw = 0.f;
	TargetRotation.Pitch = AutoPitch;
	SpringArmComponent->SetRelativeRotation(InterpPitch);
}

//------------------------------------------------------------------------------
// Functionality: Special Movement
//------------------------------------------------------------------------------

void AEDU_USER_CameraPawn::EdgeScroll()
{ // Tick Function
	// Don't EdgeScroll if the mouse is outside the window.
	if(!LocalController->GetMousePosition(MousePos.X, MousePos.Y)) {return; };
	LocalController->GetViewportSize(ScreenSize.X, ScreenSize.Y);

	DistanceToLeftEdge = MousePos.X;
	DistanceToRightEdge = ScreenSize.X - MousePos.X;
	DistanceToTopEdge = MousePos.Y;
	DistanceToBottomEdge = ScreenSize.Y - MousePos.Y;
	
	if (DistanceToTopEdge <= ScreenEdgeArea)	{ ScrollDirection.X = ScreenEdgeArea - DistanceToTopEdge; } // Up
	if (DistanceToBottomEdge <= ScreenEdgeArea)	{ ScrollDirection.X = DistanceToBottomEdge - ScreenEdgeArea; } // Down
	if (DistanceToRightEdge <= ScreenEdgeArea)	{ ScrollDirection.Y = ScreenEdgeArea - DistanceToRightEdge; } // Right
	if (DistanceToLeftEdge <= ScreenEdgeArea)	{ ScrollDirection.Y = DistanceToLeftEdge - ScreenEdgeArea; } // Left
	
	if(ScrollDirection.X != 0 || ScrollDirection.Y != 0)
	{ FLOW_LOG
		const float Speed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ?
		/*----------------------------------------------------------------------------------
		  Since some monitors are larger than others, we need to divide our ScrollDirection
		  calculation with ScreenEdgeArea to turn the distance to the edge into a percentage,
		  rather than using pixels.

		  We also multiply the calculations with SpringArmComponent->TargetArmLength
		  since we need to go faster if we are zoomed out.
		----------------------------------------------------------------------------------*/
		InputData->EdgeScrollSpeed * InputData->EdgeScrollSpeedMultiplier * SpringArmComponent->TargetArmLength * 0.02f:
		InputData->EdgeScrollSpeed * SpringArmComponent->TargetArmLength * 0.02f;

		ScrollDirection.X /= ScreenEdgeArea;
		ScrollDirection.Y /= ScreenEdgeArea;
		
		MoveCameraAnchor(ScrollDirection, Speed);
	}
}

void AEDU_USER_CameraPawn::AutoScroll()
{ // Tick Function
// Don't EdgeScroll if the mouse is outside the window.
	UpdateMouseDirection();
	LocalController->GetViewportSize(ScreenSize.X, ScreenSize.Y);

	// We need to normalize the movement according to ScreenSize.
	ScrollDirection.X += MouseDirection.Y/ScreenSize.Y;
	ScrollDirection.Y += MouseDirection.X/ScreenSize.X;
	
	if(ScrollDirection.X != 0 || ScrollDirection.Y != 0)
	{ FLOW_LOG
		const float Speed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ?
		/*----------------------------------------------------------------------------------
		  Since some monitors are larger than others, we need to divide our ScrollDirection
		  calculation with ScreenEdgeArea to turn the distance to the edge into a percentage,
		  rather than using pixels.

		  We also multiply the calculations with SpringArmComponent->TargetArmLength
		  since we need to go faster if we are zoomed out.
		----------------------------------------------------------------------------------*/
		InputData->EdgeScrollSpeed * InputData->EdgeScrollSpeedMultiplier * SpringArmComponent->TargetArmLength * 0.1f:
		InputData->EdgeScrollSpeed * SpringArmComponent->TargetArmLength * 0.1f;
		
		MoveCameraAnchor(ScrollDirection, Speed);
	}
}

void AEDU_USER_CameraPawn::FreeLook()
{ // FLOW_LOG
	UpdateMouseDirection();
    
	// Modifier Keys
	float PitchSpeed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ?
		InputData->PitchSpeed * InputData->PitchSpeedMultiplier :
		InputData->PitchSpeed;
    
	float YawSpeed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ?
		InputData->RotationSpeed * InputData->RotationSpeedMultiplier :
		InputData->RotationSpeed;

	// Final Output
	float PitchInputValue = MouseDirection.Y * PitchSpeed / 10;;
	float YawInputValue = MouseDirection.X * YawSpeed / 10;
	
	TargetRotation = FRotator(
	// We need to clamp the Pitch to stay within reason
		FMath::Clamp(TargetRotation.Pitch + PitchInputValue, InputData->PitchMin, InputData->PitchMax),
	// Add the new Yaw Input to the already existing Yaw
		TargetRotation.Yaw + YawInputValue,
	// Do not roll.
		0.f
	);
}

void AEDU_USER_CameraPawn::MouseDrag()
{ // FLOW_LOG
	UpdateMouseDirection();
	
	// Scale the speed of we zoom out or in.
	MouseDragSpeed = InputData->MouseDragSpeed * SpringArmComponent->TargetArmLength * 0.001f;
	
	if(MouseDirection.X != 0 || MouseDirection.Y != 0)
	{ FLOW_LOG
		// Note that we don't interpolate, we go exactly and immediately to location. It is the same in both SupCom and CoH.
		// Also note that X and Y are inverted here, likely because of the pawns positioning (I don't know for sure though // Draug )
		TargetLocation -= CameraAnchor->GetRelativeRotation().RotateVector(FVector(MouseDirection.Y * MouseDragSpeed, MouseDirection.X * MouseDragSpeed, 0.0f)); // Z should be 0.
		GetTerrainPosition(TargetLocation, LastValidLocation);
		SetActorLocation(TargetLocation);
	}
}

//------------------------------------------------------------------------------
// Functionality: Input Functions
//------------------------------------------------------------------------------

void AEDU_USER_CameraPawn::Input_KeyMove(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	// Keyboard movement (aka WASD)
	if(SpringArmComponent != nullptr && InputData != nullptr && !bMouseDrag)
	{
		// Check Input
		const FVector2d Direction = InputActionValue.Get<FVector2d>();
		const float Speed = ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ?
			InputData->KeyMoveSpeed * InputData->KeyMoveSpeedMultiplier * SpringArmComponent->TargetArmLength * 0.0002f:
			InputData->KeyMoveSpeed * SpringArmComponent->TargetArmLength * 0.0002f;
		
		MoveCameraAnchor(Direction, Speed);
	}
}

void AEDU_USER_CameraPawn::Input_KeyRotate(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(InputData != nullptr && ensure(InputActionValue.GetValueType() == EInputActionValueType::Axis1D))
	{
		// Interpolation for smooth movement
		EnableInterpRotation();
		
		float YawInputValue = InputActionValue.Get<float>();
		if(bInvertedYaw){ YawInputValue *= -1; }
		
		TargetRotation = FRotator(
		// Don't Change the already existing Pitch
			TargetRotation.Pitch,
		// Add the new Yaw Input to the already existing Yaw, 360 is no issue.
			TargetRotation.Yaw + YawInputValue * InputData->RotationSpeed,
		// Do not roll.
			0.f
		);
	}
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
	if(!bInvertedZoom) { Direction *= -1; }
	
	// Modify speed depending on button combo.
	ModifierKey == EEDU_USER_InputModifierKey::Mod_1 ?
		ZoomDistance = (SpringArmComponent->TargetArmLength * 0.1) * InputData->ZoomDistance * InputData->ZoomDistanceMultiplier :
		ZoomDistance = (SpringArmComponent->TargetArmLength * 0.1) * InputData->ZoomDistance;
	
	// Calculate how far we should zoom.
	TargetZoom = FMath::Clamp(TargetZoom + (ZoomDistance * Direction), InputData->MinZoom, InputData->MaxZoom);
	
	if(Direction < 0.f)
	{ // FLOW_LOG_WARNING("Zooming in")
		bZoomIn = true;
		bZoomOut = false;

		// Check if we should focus on Mouse while zooming in
		if (bZoomFocusOn)
		{
			CameraTrace();
			InterpTimer = 0.f;
			bZoomFocusFinished = false;

			// If we are fully Zoomed in, and the player still tries to zoom in, it's likely that the
			// player has a low pitch and is actually trying to move the CameraAnchor to another location.
			if(SpringArmComponent->TargetArmLength < TargetZoom * 1.1f)
			{
				EnableInterpMovement(0.5f);
			}
		}
	}
	else
	{ // FLOW_LOG_WARNING("Zooming out")
		bZoomIn = false;
		bZoomOut = true;
	}
}

void AEDU_USER_CameraPawn::Input_MouseDrag_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bFreeLook || bAutoScroll) {return; }
	
	bMouseDrag = true;
	DisableMouseEvents();
}

void AEDU_USER_CameraPawn::Input_MouseDrag_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bFreeLook || bAutoScroll) {return; }
	
	bMouseDrag = false;
	EnableMouseEvents();
}

void AEDU_USER_CameraPawn::Input_FreeLook_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bMouseDrag || bAutoScroll) {return; }
	
	bFreeLook = true;
	bAutoPitchDisengaged = true;

	DisableMouseEvents();
}

void AEDU_USER_CameraPawn::Input_FreeLook_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bMouseDrag || bAutoScroll) {return; }
	
	bFreeLook = false;

	EnableMouseEvents();
	EnableInterpRotation();
}

void AEDU_USER_CameraPawn::Input_AutoScroll_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bFreeLook || bMouseDrag) {return; }
	
	bAutoScroll = true;
	DisableMouseEvents();
}

void AEDU_USER_CameraPawn::Input_AutoScroll_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bFreeLook || bMouseDrag) {return; }

	// Reset Direction
	ScrollDirection.X = 0;
	ScrollDirection.Y = 0;
	
	bAutoScroll = false;
	EnableMouseEvents();
}

//---------------------------------------------------------------------------
// Functionality: Mouse Button Input functions
//---------------------------------------------------------------------------

void AEDU_USER_CameraPawn::Input_Mouse_1_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	LocalController->GetMousePosition(MousePos.X, MousePos.Y);
	LocalHUD->DrawSelectionMarquee(MousePos);
}

void AEDU_USER_CameraPawn::Input_Mouse_1_Triggered(const FInputActionValue& InputActionValue)
{// Called on Triggered, so it's a tick.
	
}

void AEDU_USER_CameraPawn::Input_Mouse_1_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_Mouse_2_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_Mouse_2_Triggered(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_Mouse_2_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_Mouse_3_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_Mouse_3_Triggered(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

void AEDU_USER_CameraPawn::Input_Mouse_3_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
}

//------------------------------------------------------------------------------
// Functionality: Modifier Keys
//------------------------------------------------------------------------------

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

