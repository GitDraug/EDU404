// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/EDU_CORE_PhysicsEntity.h"

#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Framework/Player/EDU_CORE_PlayerController.h"

// UE
#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "Net/UnrealNetwork.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_PhysicsEntity::AEDU_CORE_PhysicsEntity():
	bLerpLocation(true),
	bLerpRotation(true),
	bLerpScale(false)
{
	// Disable ticking at the start
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Enable Asynchronous Physics tick
	bAsyncPhysicsTickEnabled = true;
	
	// General variable replication
	bReplicates = true;

	// Always relevant for network (overrides bOnlyRelevantToOwner).
	// bAlwaysRelevant = true;

	// Whether this actor can take damage. Must be true for damage events (e. g. ReceiveDamage()) to be called.
	SetCanBeDamaged(false);
}

void AEDU_CORE_PhysicsEntity::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if(!bEnableClientIndependentPhysics)
	{
		DOREPLIFETIME(AEDU_CORE_PhysicsEntity, Rep_Location);
		DOREPLIFETIME(AEDU_CORE_PhysicsEntity, Rep_Rotation);
		DOREPLIFETIME(AEDU_CORE_PhysicsEntity, Rep_Scale);
	}
}

// Called when the game starts or when spawned
void AEDU_CORE_PhysicsEntity::BeginPlay()
{
	Super::BeginPlay();

	// To use physics on this entity, we need ot make sure the RootComponent is valid and derived from UPrimitiveComponent
	if (RootComponent && RootComponent->IsA<UPrimitiveComponent>())
	{
		// Attempt to cast to UStaticMeshComponent
		if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(RootComponent))
		{
			PhysicsComponent = MeshComponent;
		}
		// Attempt to cast to UShapeComponent
		else if (UShapeComponent* ShapeComponent = Cast<UShapeComponent>(RootComponent))
		{
			PhysicsComponent = ShapeComponent;
		}

		if (PhysicsComponent)
		{
			// Disable ticking at the start
			PhysicsComponent->PrimaryComponentTick.bStartWithTickEnabled = false;

			// We do custom replication
			PhysicsComponent->bReplicatePhysicsToAutonomousProxy = false;
		}
	}
	else
	{
		FLOW_LOG_ERROR("RootComponent is not a UPrimitiveComponent or derived from it; physics will not work.")
	}
	
	//------------------------------------------------------------------------------
	// No replicated physics, just local calculations!
	//------------------------------------------------------------------------------
	if(bEnableClientIndependentPhysics)
	{
		PhysicsComponent->SetSimulatePhysics(true);
		
		// Stop the server from sending updates and force resets on the client.
		SetNetUpdateFrequency(0);
	}
	
	//------------------------------------------------------------------------------
	// The GameMode Handles Physics on the Server!
	//------------------------------------------------------------------------------
	if(!bEnableClientIndependentPhysics && GetNetMode() != NM_Client)
	{
		PhysicsComponent->SetSimulatePhysics(true);

		// We need to at least send updates 20 times a second.
		if(GetNetUpdateFrequency() < 20) SetNetUpdateFrequency(20);
		
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToPhysicsEntityArray(this);
		}
	}
	
	//------------------------------------------------------------------------------
	// The Local PlayerController Handles Physics on the Client!
	//------------------------------------------------------------------------------
	else if(!bEnableClientIndependentPhysics && GetNetMode() == NM_Client)
	{
		PhysicsComponent->SetSimulatePhysics(false);
	
		Rep_Location = GetActorLocation();
		Rep_Rotation = GetActorRotation();
		Rep_Scale = GetActorScale();

		if (AEDU_CORE_PlayerController* LocalController = Cast<AEDU_CORE_PlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			LocalController->AddToPhysicsEntityArray(this);
		}
	}
}

//------------------------------------------------------------------------------
// Network Functionality: Server
//------------------------------------------------------------------------------	

void AEDU_CORE_PhysicsEntity::ServerPhysicsCalc(float DeltaTime)
{ // FlowLog_AI_TICK
	// Multithreaded ParallelFor in GameMode

	// There is no need for GameMode to calculate, the physics engine does it for us.
}

void AEDU_CORE_PhysicsEntity::ServerPhysicsExec(float DeltaTime)
{ // FlowLog_AI_TICK
	
	// Updating Rep_ variables on the server will trigger updates on the client.
	Rep_Location = GetActorLocation();
	Rep_Rotation = GetActorRotation();
	Rep_Scale = GetActorScale();
}

void AEDU_CORE_PhysicsEntity::OnRep_ReplicatedLocation()
{ // FLOW_LOG 
	// Handle transform update on clients
	StartLocation = GetActorLocation();
	BlendDuration_Location = ElapsedTime_Location;
	ElapsedTime_Location = 0.f;
	
	//UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - BlendDuration: %f"), *GetClass()->GetName(), __FUNCTION__, BlendDuration);
}

void AEDU_CORE_PhysicsEntity::OnRep_ReplicatedRotation()
{ // FLOW_LOG
	StartRotation = GetActorRotation();
	StartQuat = FQuat(GetActorRotation());
	TargetQuat = FQuat(Rep_Rotation);
	
	BlendDuration_Rotation = ElapsedTime_Rotation;
	ElapsedTime_Rotation = 0.f;
}

void AEDU_CORE_PhysicsEntity::OnRep_ReplicatedScale()
{ // FLOW_LOG
	StartScale = GetActorScale();
	BlendDuration_Scale = ElapsedTime_Scale;
	ElapsedTime_Scale = 0.f;
}

//------------------------------------------------------------------------------
// Network Functionality: Client
//------------------------------------------------------------------------------

void AEDU_CORE_PhysicsEntity::ClientLerpLocation(float DeltaTime)
{ // FLOW_LOG
	if(!bLerpLocation) return;

	ElapsedTime_Location += DeltaTime;
	float Alpha = FMath::Clamp(ElapsedTime_Location / BlendDuration_Location, 0.0f, 1.0f);
			
	// Interpolate towards the replicated transform
	FVector CurrentLocation = FMath::Lerp(StartLocation, Rep_Location, Alpha);
	SetActorLocation(CurrentLocation);

	/*
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT(
	"X %f, Y %f, Z %f "), CurrentLocation.X, CurrentLocation.Z, CurrentLocation.Y));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT(
	"Alpha_Location %f "), Alpha));
	*/
}

void AEDU_CORE_PhysicsEntity::ClientLerpRotation(float DeltaTime)
{ // FLOW_LOG
	if(!bLerpRotation) return;
	
	ElapsedTime_Rotation += DeltaTime;
	float Alpha = FMath::Clamp(ElapsedTime_Rotation / BlendDuration_Rotation, 0.0f, 1.0f);

	// Interpolate towards the replicated transform
	//FRotator LerpedRotation = FMath::Lerp(CurrentRotation, Rep_Rotation, Alpha_Rotation);
	FQuat LerpedQuat = FQuat::Slerp(StartQuat, TargetQuat, Alpha);
	FRotator CurrentRotation = LerpedQuat.Rotator();
	SetActorRotation(CurrentRotation);

	/*
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT(
	"Pitch: %f, Yaw %f, Roll %f "), CurrentRotation.Pitch, CurrentRotation.Yaw, CurrentRotation.Roll));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT(
	"Alpha_Rotation %f "), Alpha));
	*/
}

void AEDU_CORE_PhysicsEntity::ClientLerpScale(float DeltaTime)
{ // FLOW_LOG
	if(!bLerpScale) return;
	
	ElapsedTime_Scale += DeltaTime;
	float Alpha = FMath::Clamp(ElapsedTime_Scale / BlendDuration_Scale, 0.0f, 1.0f);
			
	// Interpolate towards the replicated transform
	FVector CurrentScale = FMath::Lerp(CurrentScale, Rep_Scale, Alpha);
	SetActorScale3D(CurrentScale);

	/*
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT(
	"Width %f, Height %f, Depth %f "), CurrentScale.X, CurrentScale.Z, CurrentScale.Y));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, FString::Printf(TEXT(
	"Alpha_Scale %f "), Alpha));
	*/
}
