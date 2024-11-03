// Fill out your copyright notice in the Description page of Project Settings.

//------------------------------------------------------------------------------
// This Plugin
//------------------------------------------------------------------------------
#include "Framework/Pawns/EDU_CORE_SpectatorCamera.h"

#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "Framework/Data/DataAssets/EDU_CORE_CameraPawnInputDataAsset.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_PLAYER.h"

#include "Framework/Player/EDU_CORE_PlayerController.h"

#include "UI/HUD/EDU_CORE_HUD.h"
//------------------------------------------------------------------------------
// Unreal Modules
//------------------------------------------------------------------------------
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "AI/WayPoints/EDU_CORE_Waypoint.h"
#include "GameFramework/SpringArmComponent.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_SpectatorCamera::AEDU_CORE_SpectatorCamera(const FObjectInitializer& ObjectInitializer) : Super (ObjectInitializer)
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

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);	
}

//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------
void AEDU_CORE_SpectatorCamera::PawnClientRestart()
{ FLOW_LOG // Called on the owning client of a player-controlled Pawn when it is restarted.
	Super::PawnClientRestart();

	/*-------------------------- Initiate HUD -------------------------------------
	  We need to initiate the HUD to call functions on it, but it's not enough to
	  get any HUD, we need ot make sure it is of the right type, so we can
	  guarantee that the functions we call actually exist on the HUD.

	  The HUD is owned by the PlayerController, so we get the controller and Cast
	  it to the type of Controller we want,to save in the pointer we have prepared.
	  After that, we use the custom controller to do the same thing with the HUD.

	  It's best to this in PawnClientRestart(), because we know that a controller
	  exists by then, and the Pawn might be Spawned mid-game.
	------------------------------------------------------------------------------*/
	if(!LocalController) { LocalController = GetController<AEDU_CORE_PlayerController>(); }
	
	// Restart if the cast failed.
	if(!LocalController) { PawnClientRestart(); return; }
	if(LocalController)
	// FLOW_LOG_ONSCREEN_MESSAGE("PlayerController Saved Successfully.")

	// Now that we have the right type of the controller, we can use it to get the right HUD.
	LocalHUD = LocalController->GetHUD<AEDU_CORE_HUD>();
	// FLOW_LOG_IF(LocalHUD, FLOW_LOG_ONSCREEN_MESSAGE("LocalHUD Cast and Saved Successfully."))
	
	// We've already set the InputComponent using Super::PawnClientRestart(), now we need to switch the InputContext.
	LocalController->SetMappingContext(EEDU_CORE_MappingContext::Camera);

	// Finish Setup
	SetPawnDefaults();
}

void AEDU_CORE_SpectatorCamera::Tick(float DeltaTime)
{ // FLOW_LOG_TICK
	Super::Tick(DeltaTime);
	
	// GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT("Elements in CameraSelectionArray: %d "), CameraSelectionArray.Num()));
	
	// Debug Messages
	/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MouseDirection.X %f"), MouseDirection.X));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MouseDirection.Y %f"), MouseDirection.Y));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 1: %d "), CTRL_Group_1.GroupArray.Num()));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 2: %d "), CTRL_Group_2.GroupArray.Num()));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 3: %d "), CTRL_Group_3.GroupArray.Num()));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 4: %d "), CTRL_Group_4.GroupArray.Num()));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 5: %d "), CTRL_Group_5.GroupArray.Num()));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 6: %d "), CTRL_Group_6.GroupArray.Num()));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 7: %d "), CTRL_Group_7.GroupArray.Num()));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 8: %d "), CTRL_Group_8.GroupArray.Num()));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 9: %d "), CTRL_Group_9.GroupArray.Num()));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Emerald, FString::Printf(TEXT("Elements in Ctrl Group 0: %d "), CTRL_Group_0.GroupArray.Num()));
	
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
	
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("SavedMousePosY %f"), SavedMousePos.Y));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("SavedMousePosX %f"), SavedMousePos.X));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MousePosY %f"), MousePos.Y));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MousePosX %f"), MousePos.X));

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, FString::Printf(TEXT("bShowMouseCursor %d"), PlayerController->bShowMouseCursor));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, FString::Printf(TEXT("bEnableClickEvents %d"), PlayerController->bEnableClickEvents));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Green, FString::Printf(TEXT("bEnableMouseOverEvents %d"), PlayerController->bEnableMouseOverEvents));

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MouseDirection.X %f"), MouseDirection.X));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Orange, FString::Printf(TEXT("MouseDirection.Y %f"), MouseDirection.Y));
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	
	// Conditional Tick
	if(SpringArmComponent == nullptr || InputData == nullptr || !bIsInitialized) { return; }

	/*--------------------- Frame limited tick functions ---------------------------
	  Most tick functions don't need to run 30+ times a second. A function running
	  every third tick still runs 10 times a second at 30 FPS.

	  The FrameCounter measures frames passed since we last called the function,
	  we then reset the counter.
	------------------------------------------------------------------------------*/
	if (FrameCounter++ < 3)
	{
		CursorTrace();
	}
	else
	{
		FrameCounter = 0;
	}

	//-------------------------------------------------------------------------------------------
	// Camera zoom and Location
	//-------------------------------------------------------------------------------------------
	if(bZoomIn || bZoomOut)
	{
		UpdateCameraZoom(DeltaTime);
		if (bZoomFocusOn && !bZoomFocusFinished)
		{
			UpdateCameraLocation(DeltaTime);
		}
	}

	//-------------------------------------------------------------------------------------------
	// We put Interpolation on a timer to stop it from ticking all the time.
	//-------------------------------------------------------------------------------------------
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

	//-------------------------------------------------------------------------------------------
	// EdgeScroll will interfere with FreeLook and MouseDrag.
	//-------------------------------------------------------------------------------------------
	if(bFreeLook && !bMouseDrag && !bAutoScroll)
	{
		FreeLook();
		UpdateCameraRotation(DeltaTime);
		return;
	}

	//-------------------------------------------------------------------------------------------
	// MouseDrag will interfere with FreeLook and edgeScroll.
	//-------------------------------------------------------------------------------------------
	if(bMouseDrag && !bFreeLook&& !bAutoScroll)
	{
		MouseDrag();
		return;
	}

	//-------------------------------------------------------------------------------------------
	// MouseDrag and FreeLook interferes with EdgeScroll
	//-------------------------------------------------------------------------------------------
	if(bAutoScroll && !bFreeLook && !bMouseDrag)
	{
		AutoScroll();
		return;
	}

	//-------------------------------------------------------------------------------------------
	// MouseDrag and FreeLook interferes with EdgeScroll
	//-------------------------------------------------------------------------------------------
	if(bEdgeScroll == true)
	{
		EdgeScroll();
	}
	
}

void AEDU_CORE_SpectatorCamera::UnPossessed()
{ FLOW_LOG

	// This might be an overkill, but if another controller posses this pawn, then we want to make sure it is reset.
	bFreeLook = false;
	bMouseDrag = false;

	bMouse_1 = false;
	bMouse_2 = false;
	bMouse_3 = false;
	
	bMod_1 = false;
	bMod_2 = false;
	bMod_3 = false;
	bMod_4 = false;
	
	bZoomIn = false;
	bZoomOut = false;

	bIsInitialized = false;
	
	Super::UnPossessed();
}

//------------------------------------------------------------------------------
// Input Setup
//------------------------------------------------------------------------------

void AEDU_CORE_SpectatorCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{ FLOW_LOG
	/*------------------------------------------------------------------------------
	PlayerInputComponent is responsible for binding InputMappings to function,
	the InputMappingContext is set in the PlayerController.
	------------------------------------------------------------------------------*/
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
			// Mouse Input functions; Multifunctional, so best to use simple names.
			//------------------------------------------------------------------------------
			EnhancedInputComponent->BindAction(InputData->Mouse_1, ETriggerEvent::Started, this, &ThisClass::Input_Mouse_1_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mouse_1, ETriggerEvent::Completed, this, &ThisClass::Input_Mouse_1_Released);
			
			EnhancedInputComponent->BindAction(InputData->Mouse_2, ETriggerEvent::Started, this, &ThisClass::Input_Mouse_2_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mouse_2, ETriggerEvent::Completed, this, &ThisClass::Input_Mouse_2_Released);

			EnhancedInputComponent->BindAction(InputData->Mouse_3, ETriggerEvent::Started, this, &ThisClass::Input_Mouse_3_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mouse_3, ETriggerEvent::Completed, this, &ThisClass::Input_Mouse_3_Released);

			//------------------------------------------------------------------------------
			// Modifier Keys : 
			//------------------------------------------------------------------------------
			EnhancedInputComponent->BindAction(InputData->Mod_1, ETriggerEvent::Started, this, &ThisClass::Input_Mod_1_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_1, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_1_Released);

			EnhancedInputComponent->BindAction(InputData->Mod_2, ETriggerEvent::Started, this, &ThisClass::Input_Mod_2_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_2, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_2_Released);

			EnhancedInputComponent->BindAction(InputData->Mod_3, ETriggerEvent::Started, this, &ThisClass::Input_Mod_3_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_3, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_3_Released);

			EnhancedInputComponent->BindAction(InputData->Mod_4, ETriggerEvent::Started, this, &ThisClass::Input_Mod_4_Pressed);
			EnhancedInputComponent->BindAction(InputData->Mod_4, ETriggerEvent::Completed, this, &ThisClass::Input_Mod_4_Released);
			
			//---------------------------------------------------------------------------
			// CTRL Group Assignment
			//---------------------------------------------------------------------------
			EnhancedInputComponent->BindAction(InputData->SelectAll, ETriggerEvent::Started, this, &ThisClass::SelectAll);
			
			EnhancedInputComponent->BindAction(InputData->Numeric_1, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_1_Pressed);
			EnhancedInputComponent->BindAction(InputData->Numeric_2, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_2_Pressed);
			EnhancedInputComponent->BindAction(InputData->Numeric_3, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_3_Pressed);
			EnhancedInputComponent->BindAction(InputData->Numeric_4, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_4_Pressed);
			EnhancedInputComponent->BindAction(InputData->Numeric_5, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_5_Pressed);
			EnhancedInputComponent->BindAction(InputData->Numeric_6, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_6_Pressed);
			EnhancedInputComponent->BindAction(InputData->Numeric_7, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_7_Pressed);
			EnhancedInputComponent->BindAction(InputData->Numeric_8, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_8_Pressed);
			EnhancedInputComponent->BindAction(InputData->Numeric_9, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_9_Pressed);
			EnhancedInputComponent->BindAction(InputData->Numeric_0, ETriggerEvent::Started, this, &ThisClass::Ctrl_Group_0_Pressed);
		}
	}		
}

void AEDU_CORE_SpectatorCamera::SetPlayerInputMode()
{ FLOW_LOG
	//if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = PlayerController->GetInputSubsystem())
	//{
		FInputModeGameAndUI InputMode; // Settings container
		InputMode.SetHideCursorDuringCapture(false); // Whether to hide the cursor during temporary mouse capture caused by a mouse down
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // Don't lock to window. It's better to put this in a menu.

		LocalController->SetInputMode(InputMode);
		LocalController->SetShowMouseCursor(true);
	//}
}

void AEDU_CORE_SpectatorCamera::SetPawnDefaults()
{ FLOW_LOG
	if(IsLocallyControlled() && SpringArmComponent != nullptr && InputData != nullptr)
	{
		DoubleClickDelay = InputData->DoubleClickDelay;
		
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
				
		// All set, start ticking!
		bIsInitialized = true;

		// Interpolation for smooth camera setup
		EnableInterpRotation();

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
		FLOW_LOG_IF( (InputData->RotationSpeed ==0 ),			FLOW_LOG_ERROR("InputData->RotateSpeed is 0, the pawn might not be able to Move"))
		FLOW_LOG_IF( (InputData->RotationSpeedMultiplier ==0 ),	FLOW_LOG_ERROR("InputData->KeyMoveSpeedMultiplier is 0, the pawn might not be able to Move"))
	}
}

//------------------------------------------------------------------------------
// Functionality: Utility
//------------------------------------------------------------------------------

void AEDU_CORE_SpectatorCamera::ResetCamera()
{ FLOW_LOG
	EnableInterpRotation();
	EnableInterpMovement();

	TargetRotation.Pitch = AutoPitchMin;

	if(bAutoPitch)
	{
		bAutoPitchDisengaged = false;
	}
}

void AEDU_CORE_SpectatorCamera::EnableInterpRotation()
{ FLOW_LOG
	bInterpRot = true;
	InterpTimer = 1.f;
}

void AEDU_CORE_SpectatorCamera::EnableInterpMovement(float Time)
{ // FLOW_LOG
	bInterpMov = true;
	InterpTimer = Time;
}

void AEDU_CORE_SpectatorCamera::CameraTrace()
{ FLOW_LOG
	// Get the mouse position
	FVector2d MousePos;
	if (LocalController && LocalController->GetMousePosition(MousePos.X, MousePos.Y))
	{
		// Deproject the mouse position to a world position and direction
		FVector WorldLocation, WorldDirection;
		if (LocalController->DeprojectScreenPositionToWorld(MousePos.X, MousePos.Y, WorldLocation, WorldDirection))
		{
			// Define the start and end points of the trace
			FVector TraceStart = WorldLocation;
			FVector TraceEnd = TraceStart + (WorldDirection * (ZoomTraceLength+SpringArmComponent->TargetArmLength));
			
			FHitResult CameraTraceResult;
			FCollisionQueryParams CameraTraceCollisionParams;
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

void AEDU_CORE_SpectatorCamera::CursorTrace()
{ // FLOW_LOG_TICK
	FHitResult CameraTraceResult;
	FCollisionQueryParams CameraTraceCollisionParams;

	// Get the mouse position
	FVector2d MousePos;
	if (LocalController && LocalController->GetMousePosition(MousePos.X, MousePos.Y))
	{
		// Deproject the mouse position to a world position and direction
		FVector WorldLocation, WorldDirection;		
		if (LocalController->DeprojectScreenPositionToWorld(MousePos.X, MousePos.Y, WorldLocation, WorldDirection))
		{
			// Define the start and end points of the trace
			FVector TraceStart = WorldLocation;
			FVector TraceEnd = TraceStart + (WorldDirection * (ZoomTraceLength + SpringArmComponent->TargetArmLength));
			
			// Perform the CameraTrace // TODO: It would be best to make a custom CollisionProfile or channel for only Ground and Selectables.
			if(GetWorld()->LineTraceSingleByChannel(CameraTraceResult, TraceStart, TraceEnd, ECC_Visibility, CameraTraceCollisionParams))
			{
				// Check if we hit something
				if(CameraTraceResult.bBlockingHit)
				{
					// Save WorldPosition for other functions.
					CursorWorldPos = CameraTraceResult.ImpactPoint;
	
					// Highlight any selectable entity that we might have the cursor on.
					LastActor = CurrentActor;
					CurrentActor = Cast<AEDU_CORE_SelectableEntity>(CameraTraceResult.GetActor());
					/*------------------------------------------------------------------------------
					  Line Trace from Cursor, possible scenarios:
						  1. LastActor == null && CurrentActor && null
								Do Nothing.
						  2. LastActor == null && CurrentActor is valid
								Highlight CurrentActor
						  3. LastActor is valid && CurrentActor is null
								Unhighlight LastActor
						  4. Both actors are valid, but LastActor != CurrentActor
								Unhighlight LastActor, Highlight CurrentActor
						  5. Both actors are valid, and LastActor == CurrentActor
								Do nothing.
					------------------------------------------------------------------------------*/
					if(LastActor == nullptr)
					{
						if(CurrentActor == nullptr)
						{
							// Scenario 1; Do Nothing.
						}
						else
						{
							CurrentActor->MouseHighlightActor(); // Scenario 2: Highlight CurrentActor
						}
					}
					else // LastActor is valid...
					{   
						if(CurrentActor == nullptr) 
						{
							LastActor->MouseUnHighlightActor(); // Scenario 3: LastActor is valid && CurrentActor is null
						}
						else //  Both actors are valid...
						{
							if(LastActor != CurrentActor) // ...but LastActor != CurrentActor
							{  // Scenario 4:
								LastActor->MouseUnHighlightActor();
								CurrentActor->MouseHighlightActor();
							}
							// Else:Both actors are valid, and LastActor == CurrentActor
							// Scenario 5: Do nothing.
						}
					}
				}
			}
		}
	}
}

void AEDU_CORE_SpectatorCamera::MoveCameraAnchor(const FVector2d& Direction, const float& Speed)
{ // FLOW_LOG_TICK
	// We're rotating CameraAnchor not the SpringArm
	TargetLocation += CameraAnchor->GetRelativeRotation().RotateVector(FVector(Direction.X * Speed, Direction.Y * Speed, 0.0f)); // Z should be 0.
	GetTerrainPosition(TargetLocation, LastValidLocation);
	
	EnableInterpMovement();
}

void AEDU_CORE_SpectatorCamera::GetTerrainPosition(FVector& TargetPos, FVector& LastValidPos) const
{ // FLOW_LOG_TICK
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
	{ // RTS_TRACE_CHANNEL_TERRAIN is defined in EDU_CORE_StaticGameData 
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

void AEDU_CORE_SpectatorCamera::UpdateCameraRotation(const float DeltaTime) const
{ // FLOW_LOG_TICK
	// Rotate the Camera to target pitch
	// Observe the SpringArmComponent; it acts like a barrel, on a turret, only going up and down. It doesn't affect the rotation or the pitch of the CameraAnchor. 
	FRotator InterpPitch = FMath::RInterpTo(SpringArmComponent->GetRelativeRotation(), TargetRotation, DeltaTime, InputData->PitchInterpSpeed);
	InterpPitch.Yaw = 0.f;
	SpringArmComponent->SetRelativeRotation(InterpPitch);

	// Rotate the Camera to target rotation
	// Observe the CameraAnchor; since the new move position is in front of the CameraAnchor, we don't want it to pitch down into the ground, only rotate.
	float RotationSpeed = ModifierKey == EEDU_CORE_InputModifierKey::Mod_1 ? InputData->RotationInterpSpeed * InputData->RotationSpeedMultiplier : InputData->RotationInterpSpeed;
	FRotator InterpRotation = FMath::RInterpTo(CameraAnchor->GetRelativeRotation(), TargetRotation, DeltaTime, RotationSpeed);
	InterpRotation.Pitch = 0.f;
	CameraAnchor->SetRelativeRotation(InterpRotation);	
}

void AEDU_CORE_SpectatorCamera::UpdateCameraLocation(const float DeltaTime)
{ // FLOW_LOG_TICK
	// Move Pawn to target location
	// Interpolate vector from Current to Target, scaled by distance to Target, so it has a strong start speed and ease out.
	FVector InterpLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, InputData->MoveInterSpeed);
	SetActorLocation(InterpLocation);
}

void AEDU_CORE_SpectatorCamera::UpdateCameraZoom(const float DeltaTime)
{ // FLOW_LOG
	// <!> Note that this is a tick executed function, so be careful to set values that shouldn't be updated on tick.
	if (bZoomOut)
	{
		// Zoom camera to target zoom
		// Interpolate float from Current to Target, scaled by distance to Target, so it has a strong start speed and ease out.
		const float InterpZoom = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetZoom, DeltaTime, InputData->ZoomInterpSpeed);
		SpringArmComponent->TargetArmLength = InterpZoom;
		
		// Check if we should stop, we use 98%, because Interpolation algoritm will run close to forever as the value gets smaller.
		if(SpringArmComponent->TargetArmLength > TargetZoom * 0.98f)
		{
			bZoomOut = false;

			if(bAutoPitch && SpringArmComponent->TargetArmLength > InputData->MaxZoom * 0.981f)
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
		
		// Check if we should stop, we use 99.9%, because Interpolation algebra will run close to forever as the value get smaller.
		if(SpringArmComponent->TargetArmLength < TargetZoom * 1.01f)
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

void AEDU_CORE_SpectatorCamera::UpdateMouseDirection()
{
	/*----------------------- vs GetInputMouseDelta ------------------------------------
	  This function updates the Mouse Input as a value, without the need of a viewport,
	  meaning you can keep dragging as long as you have mouse pad space on your desk.

	  The APlayerController->GetInputMouseDelta() does a similar thing, but only if
	  a mouse button is held down. This one runs on tick instead, regardless if a
	  mouse button is held.
	---------------------------------------------------------------------------------*/
	FVector2d MousePos;
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

void AEDU_CORE_SpectatorCamera::DisableMouseEvents()
{ FLOW_LOG
	FVector2d MousePos;
	LocalController->GetMousePosition(MousePos.X, MousePos.Y);

	// Initial Position
	SavedMousePos.X = MousePos.X;
	SavedMousePos.Y = MousePos.Y;
	
	LocalController->bShowMouseCursor = false;
	LocalController->bEnableClickEvents = false;
	LocalController->bEnableTouchEvents = false;
	LocalController->bEnableMouseOverEvents = false;
}

void AEDU_CORE_SpectatorCamera::EnableMouseEvents() const
{ FLOW_LOG
	LocalController->SetMouseLocation(SavedMousePos.X, SavedMousePos.Y);
	
	LocalController->bShowMouseCursor = true;
	LocalController->bEnableClickEvents = true;
	LocalController->bEnableTouchEvents = true;
	LocalController->bEnableMouseOverEvents = true;
}

void AEDU_CORE_SpectatorCamera::AutoPitch(const float DeltaTime)
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

void AEDU_CORE_SpectatorCamera::EdgeScroll()
{ // FLOW_LOG_TICK
	FVector2d MousePos; // Stack allocated MousePos is faster than the heap.
	// Don't EdgeScroll if the mouse is outside the window.
	if(!LocalController->GetMousePosition(MousePos.X, MousePos.Y)) {return; };

	FIntVector2 ScreenSize;
	LocalController->GetViewportSize(ScreenSize.X, ScreenSize.Y);

	float DistanceToLeftEdge = MousePos.X;
	float DistanceToRightEdge = ScreenSize.X - MousePos.X;
	float DistanceToTopEdge = MousePos.Y;
	float DistanceToBottomEdge = ScreenSize.Y - MousePos.Y;
	
	if (DistanceToTopEdge <= ScreenEdgeArea)	{ ScrollDirection.X = ScreenEdgeArea - DistanceToTopEdge; } // Up
	if (DistanceToBottomEdge <= ScreenEdgeArea)	{ ScrollDirection.X = DistanceToBottomEdge - ScreenEdgeArea; } // Down
	if (DistanceToRightEdge <= ScreenEdgeArea)	{ ScrollDirection.Y = ScreenEdgeArea - DistanceToRightEdge; } // Right
	if (DistanceToLeftEdge <= ScreenEdgeArea)	{ ScrollDirection.Y = DistanceToLeftEdge - ScreenEdgeArea; } // Left
	
	if(ScrollDirection.X != 0 || ScrollDirection.Y != 0)
	{
		const float Speed = ModifierKey == EEDU_CORE_InputModifierKey::Mod_1 ?
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

void AEDU_CORE_SpectatorCamera::AutoScroll()
{ // Tick Function
// Don't EdgeScroll if the mouse is outside the window.
	UpdateMouseDirection();
	FIntVector2 ScreenSize;
	LocalController->GetViewportSize(ScreenSize.X, ScreenSize.Y);

	// We need to normalize the movement according to ScreenSize.
	ScrollDirection.X += MouseDirection.Y/ScreenSize.Y;
	ScrollDirection.Y += MouseDirection.X/ScreenSize.X;
	
	if(ScrollDirection.X != 0 || ScrollDirection.Y != 0)
	{ FLOW_LOG
		const float Speed = ModifierKey == EEDU_CORE_InputModifierKey::Mod_1 ?
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

void AEDU_CORE_SpectatorCamera::FreeLook()
{ // FLOW_LOG_TICK
	UpdateMouseDirection();
    
	// Modifier Keys
	float PitchSpeed = ModifierKey == EEDU_CORE_InputModifierKey::Mod_1 ?
		InputData->PitchSpeed * InputData->PitchSpeedMultiplier :
		InputData->PitchSpeed;
    
	float YawSpeed = ModifierKey == EEDU_CORE_InputModifierKey::Mod_1 ?
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

void AEDU_CORE_SpectatorCamera::MouseDrag()
{ // FLOW_LOG_TICK
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
// Functionality : Selection Controls
//------------------------------------------------------------------------------

void AEDU_CORE_SpectatorCamera::CopyEntitiesInHUDSelectionArray()
{ FLOW_LOG
	
	/*----------------------------------------------------------------------------
	  <!> Observer that this is all on the client. The server doesn't have access
	  to our local HUD, so it can't copy the entities in the array.  For that,
	  we need to tell the server to copy the Array on the server instance.
	----------------------------------------------------------------------------*/
	
	CameraSelectionArray = LocalHUD->GetHUDSelectionArray();
	SelectEntitiesInRectangle();
	ResetHUDSelectionArray();
}

void AEDU_CORE_SpectatorCamera::ReviseSelection()
{ FLOW_LOG
	for (AEDU_CORE_SelectableEntity* Entity : LocalHUD->GetHUDSelectionArray())
	{
		if (!CameraSelectionArray.Contains(Entity))
		{
			CameraSelectionArray.AddUnique(Entity);
			FLOW_LOG_WARNING("Adding")
		}
		else
		{
			CameraSelectionArray.Remove(Entity);
			FLOW_LOG_WARNING("Removing")
		}
	}
	SelectEntitiesInRectangle();
	ResetHUDSelectionArray();
}

void AEDU_CORE_SpectatorCamera::ResetCameraSelectionArray()
{ FLOW_LOG
	for(AEDU_CORE_SelectableEntity* Entity : CameraSelectionArray)
	{
		if(Entity) // Entities might be dead or deleted.
		{
			Entity->UnSelectActor();
		}
	}
	CameraSelectionArray.Reset();
}

void AEDU_CORE_SpectatorCamera::ResetHUDSelectionArray() const
{ FLOW_LOG
	for(AEDU_CORE_SelectableEntity* Entity : LocalHUD->GetHUDSelectionArray())
	{
		if(Entity) // Entities might be dead or deleted.
		{
			Entity->UnRectangleHighlightActor();
		}
	}
	LocalHUD->GetHUDSelectionArray().Reset();
}

void AEDU_CORE_SpectatorCamera::SelectEntitiesInRectangle()
{ FLOW_LOG
	for(AEDU_CORE_SelectableEntity* Entity : CameraSelectionArray)
	{
		if(Entity) // Entities might be dead or deleted.
		{
			Entity->SelectActor();
		}
	}
}

void AEDU_CORE_SpectatorCamera::CallCtrlGroup(const TArray<AEDU_CORE_SelectableEntity*>& CtrlGroupArray)
{ FLOW_LOG
	// Ready the Array.
	if(CameraSelectionArray.Num() > 0)
	{
		ResetCameraSelectionArray();
	}

	// Now Copy ctrl Group Array.
	if (CtrlGroupArray.Num() > 0)
	{
		CameraSelectionArray = CtrlGroupArray;
		
		// Select new Entities.
		for(AEDU_CORE_SelectableEntity* Entity : CameraSelectionArray)
		{
			if(Entity) // Make sure they are alive before calling functions on them.
			{
				Entity->UnRectangleHighlightActor();
				Entity->SelectActor();
			}
		}
	}
}

//------------------------------------------------------------------------------
// Functionality: Input Functions
//------------------------------------------------------------------------------
bool AEDU_CORE_SpectatorCamera::ShouldReceiveInput() const
{
	if(bFreeLook || bMouseDrag || bAutoScroll)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void AEDU_CORE_SpectatorCamera::Input_KeyMove(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	// Keyboard movement (aka WASD)
	if(SpringArmComponent != nullptr && InputData != nullptr && !bMouseDrag)
	{
		// Check Input
		const FVector2d Direction = InputActionValue.Get<FVector2d>();
		const float Speed = ModifierKey == EEDU_CORE_InputModifierKey::Mod_1 ?
			InputData->KeyMoveSpeed * InputData->KeyMoveSpeedMultiplier * SpringArmComponent->TargetArmLength * 0.0002f:
			InputData->KeyMoveSpeed * SpringArmComponent->TargetArmLength * 0.0002f;
		
		MoveCameraAnchor(Direction, Speed);
	}
}

void AEDU_CORE_SpectatorCamera::Input_KeyRotate(const FInputActionValue& InputActionValue)
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

void AEDU_CORE_SpectatorCamera::Input_Zoom(const FInputActionValue& InputActionValue)
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
	ModifierKey == EEDU_CORE_InputModifierKey::Mod_1 ?
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

void AEDU_CORE_SpectatorCamera::Input_MouseDrag_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bFreeLook || bAutoScroll) {return; }
	
	bMouseDrag = true;
	DisableMouseEvents();
}

void AEDU_CORE_SpectatorCamera::Input_MouseDrag_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bFreeLook || bAutoScroll) {return; }
	
	bMouseDrag = false;
	EnableMouseEvents();
}

void AEDU_CORE_SpectatorCamera::Input_FreeLook_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bMouseDrag || bAutoScroll) {return; }
	
	bFreeLook = true;
	bAutoPitchDisengaged = true;

	DisableMouseEvents();
}

void AEDU_CORE_SpectatorCamera::Input_FreeLook_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bMouseDrag || bAutoScroll) {return; }
	
	bFreeLook = false;

	EnableMouseEvents();
	EnableInterpRotation();
}

void AEDU_CORE_SpectatorCamera::Input_AutoScroll_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG // Make sure the Input Action in the Editor is a bool.
	if(bFreeLook || bMouseDrag) {return; }
	
	bAutoScroll = true;
	DisableMouseEvents();
}

void AEDU_CORE_SpectatorCamera::Input_AutoScroll_Released(const FInputActionValue& InputActionValue)
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

void AEDU_CORE_SpectatorCamera::Input_Mouse_1_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!ShouldReceiveInput()) { return; }
	// Check if Ctrl is held, and select all entities of the same class as the one under the cursor.
	if (ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)
	{
		// Trace Search
		FVector2d MousePos;
		LocalController->GetMousePosition(MousePos.X, MousePos.Y);
		float TraceLength = ZoomTraceLength + SpringArmComponent->TargetArmLength;
		LocalHUD->DetectEntityUnderCursor(MousePos, TraceLength);

		// The HUD needs to finish its search before we copy the Array, if the timer is too fast, Input_Mouse_1_Released still works.
		FTimerHandle WaitForHUD;
		GetWorld()->GetTimerManager().SetTimer(WaitForHUD, this, &ThisClass::CopyEntitiesInHUDSelectionArray, 0.1f, false);
		
		// Cache the selection array to avoid repeated calls
		auto HUDSelectionArray = LocalHUD->GetHUDSelectionArray();
		if (HUDSelectionArray.Num() > 0)
		{
			LocalHUD->bSearchWholeScreen = true;
			LocalHUD->SearchFilter = HUDSelectionArray[0]->GetClass();			
			CopyEntitiesInHUDSelectionArray();
		}
		return;
	}
	
	bMouse_1 = true;
	
	// Check if we are single-clicking
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - Mouse_1_StartTime > DoubleClickDelay)
	{
		FLOW_LOG_WARNING("Single Click")
		// Save the current Mouse Position in the Hud, in case we want to draw a SelectionMarquee.
		FVector2d MousePos;
		LocalController->GetMousePosition(MousePos.X, MousePos.Y);
		LocalHUD->DrawSelectionMarquee(MousePos);
		
		// Update the last click time
		Mouse_1_StartTime = CurrentTime;
		return;
	}

	// We only register DoubleClick if an entity is selected
	if(CameraSelectionArray.Num() > 0 && CameraSelectionArray[0]->GetClass())
	{
		Input_Mouse_1_DoubleClick();
	}
}

void AEDU_CORE_SpectatorCamera::Input_Mouse_1_DoubleClick()
{ FLOW_LOG_WARNING("DoubleClick!")
	LocalHUD->SearchFilter = CameraSelectionArray[0]->GetClass(); // This is the actor we are searching for.
	LocalHUD->bSearchWholeScreen = true; // This will make the HUD run a separate function in its DrawHUD().
}

void AEDU_CORE_SpectatorCamera::Input_Mouse_1_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!bMouse_1) { return; }
	bMouse_1 = false;

	// Trace for entities
	FVector2d MousePos;
	LocalController->GetMousePosition(MousePos.X, MousePos.Y);
	float TraceLength = ZoomTraceLength + SpringArmComponent->TargetArmLength;
	LocalHUD->DetectEntityUnderCursor(MousePos, TraceLength);
	
	LocalHUD->StopDrawingSelectionMarquee();
	
	if(LocalHUD->GetHUDSelectionArray().Num() > 0)
	{
		if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_1)
		{
			ReviseSelection();
		}
		else
		{
			CopyEntitiesInHUDSelectionArray();
		}
	}
	else
	{
		if(ModifierKey != EEDU_CORE_InputModifierKey::Mod_1)
		{
			ResetCameraSelectionArray();
		}
	}	
}

void AEDU_CORE_SpectatorCamera::Input_Mouse_2_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!ShouldReceiveInput()) { return; }
	
	bMouse_2 = true;
}

void AEDU_CORE_SpectatorCamera::Input_Mouse_2_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!bMouse_2) { return; }
	
	bMouse_2 = false;
}

void AEDU_CORE_SpectatorCamera::Input_Mouse_3_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!ShouldReceiveInput()) { return; }
	
	bMouse_3 = true;
}

void AEDU_CORE_SpectatorCamera::Input_Mouse_3_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!bMouse_3) { return; }
	
	bMouse_3 = false;
}

//---------------------------------------------------------------------------
// CTRL Group Assignment
//---------------------------------------------------------------------------
void AEDU_CORE_SpectatorCamera::Ctrl_Group_1_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 1
	{
		CTRL_Group_1.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_1.GroupArray);
	}
}

void AEDU_CORE_SpectatorCamera::Ctrl_Group_2_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 2
	{
		CTRL_Group_2.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_2.GroupArray);
	}
}

void AEDU_CORE_SpectatorCamera::Ctrl_Group_3_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 2
	{
		CTRL_Group_3.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_3.GroupArray);
	}
}

void AEDU_CORE_SpectatorCamera::Ctrl_Group_4_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 2
	{
		CTRL_Group_4.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_4.GroupArray);
	}
}

void AEDU_CORE_SpectatorCamera::Ctrl_Group_5_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 2
	{
		CTRL_Group_5.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_5.GroupArray);
	}
}

void AEDU_CORE_SpectatorCamera::Ctrl_Group_6_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 2
	{
		CTRL_Group_6.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_6.GroupArray);
	}
}

void AEDU_CORE_SpectatorCamera::Ctrl_Group_7_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 2
	{
		CTRL_Group_7.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_7.GroupArray);
	}
}

void AEDU_CORE_SpectatorCamera::Ctrl_Group_8_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 2
	{
		CTRL_Group_8.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_8.GroupArray);
	}
}

void AEDU_CORE_SpectatorCamera::Ctrl_Group_9_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 2
	{
		CTRL_Group_9.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_9.GroupArray);
	}
}

void AEDU_CORE_SpectatorCamera::Ctrl_Group_0_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)// Create Group 2
	{
		CTRL_Group_0.GroupArray = CameraSelectionArray;
	}
	
	else // Call Group 1...
	{
		ResetHUDSelectionArray();
		CallCtrlGroup(CTRL_Group_0.GroupArray);
	}
}

//------------------------------------------------------------------------------
// Functionality: Modifier Keys
//------------------------------------------------------------------------------

void AEDU_CORE_SpectatorCamera::Input_Mod_1_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_1 = true;
	SetModifierKey();
}

void AEDU_CORE_SpectatorCamera::Input_Mod_1_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!bMod_1) { return; }
	
	bMod_1 = false;
	SetModifierKey();
}

void AEDU_CORE_SpectatorCamera::Input_Mod_2_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_2 = true;
	SetModifierKey();
}

void AEDU_CORE_SpectatorCamera::Input_Mod_2_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!bMod_2) { return; }
	
	bMod_2 = false;
	SetModifierKey();
}

void AEDU_CORE_SpectatorCamera::Input_Mod_3_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG	
	bMod_3 = true;
	SetModifierKey();
}

void AEDU_CORE_SpectatorCamera::Input_Mod_3_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!bMod_3) { return; }

	bMod_3 = false;
	SetModifierKey();
}

void AEDU_CORE_SpectatorCamera::Input_Mod_4_Pressed(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	bMod_4 = true;
	SetModifierKey();
}

void AEDU_CORE_SpectatorCamera::Input_Mod_4_Released(const FInputActionValue& InputActionValue)
{ FLOW_LOG
	if(!bMod_4) { return; }
	
	bMod_4 = false;
	SetModifierKey();
}

void AEDU_CORE_SpectatorCamera::SetModifierKey()
{ FLOW_LOG
	/*--------------------------------------------------------------------------------
	  Note that this evaluation will start over every time a new key is pressed,
	  I tried using a switch, but switches can't be used with structs unless
	  overloaded, and it's not worth the headache.

	  I don't like nested ifs, but since this is only called on key press,
	  it's really not that performance heavy.  // Draug
	--------------------------------------------------------------------------------*/

	using enum EEDU_CORE_InputModifierKey; //  for readability
	
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
	
	// end using enum EEDU_CORE_InputModifierKey;
}

//---------------------------------------------------------------------------
// Functionality: ShortCuts
//---------------------------------------------------------------------------

void AEDU_CORE_SpectatorCamera::SelectAll()
{ FLOW_LOG
	if(ModifierKey == EEDU_CORE_InputModifierKey::Mod_2)
	{
		LocalHUD->bSearchWholeScreen = true; // This will make the Hud run a separate function in its DrawHUD().

		// The HUD needs to finish its search before we copy the Array, if the timer is too fast, Input_Mouse_1_Released still works.
		FTimerHandle WaitForHUD;
		GetWorld()->GetTimerManager().SetTimer(WaitForHUD, this, &ThisClass::CopyEntitiesInHUDSelectionArray, 0.1f, false);
	}
}
