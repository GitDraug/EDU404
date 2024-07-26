// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/EDU_CORE_PhysicalEntity.h"
#include "Components/BoxComponent.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Framework/Player/EDU_CORE_PlayerController.h"
#include "Net/UnrealNetwork.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_PhysicalEntity::AEDU_CORE_PhysicalEntity()
{ FLOW_LOG
	// Functionality we don't need
	AActor::SetReplicateMovement(false); // We don't use a movement component.
	SetCanBeDamaged(false); // We use a custom damage component.
	PrimaryActorTick.bCanEverTick = false; // We use a custom aggregated tick.

	// General variable replication
	bReplicates = true;
	NetUpdateFrequency = 4; // Times we send updates per second.
	
	PhysicsComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicsComponent"));	
	PhysicsComponent->SetupAttachment(GetRootComponent());

	RootComponent = PhysicsComponent;
}

void AEDU_CORE_PhysicalEntity::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();
	
	//SetNetDormancy(DORM_Never);

	// For Simple Collision
	//PhysicsComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin);
	//PhysicsComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnOverlapEnd);
	
	//------------------------------------------------------------------------------
	// No aggregated tick, just local calculations!
	//------------------------------------------------------------------------------
	if(bEnableClientIndependentPhysics)
	{
		PhysicsComponent->SetSimulatePhysics(true); // Client
		
		if(GetNetMode() != NM_Client)
		{
			NetUpdateFrequency = 0; // This stops the server from sending updates and force resets on the client.
		}
		return;
	}
	
	//------------------------------------------------------------------------------
	// Simplified, homecooked physics
	//------------------------------------------------------------------------------
	if(bEnableSimpleServerPhysicsOnly && GetNetMode() != NM_Client)
	{
		PhysicsComponent->SetSimulatePhysics(false); // Client
		PhysicsComponent->SetEnableGravity(false);
		
		NewHeight = GetActorLocation(); // We need ot initiate this, else everyone will spawn at coordinate 0;
		
		if(NetUpdateFrequency < 4) NetUpdateFrequency = 4;
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToPhysicalEntityArray(this);
			PhysicsComponent->SetGenerateOverlapEvents(true);
		}
		return;
	}

	//------------------------------------------------------------------------------
	// The GameMode Handles Physics on the Server!
	//------------------------------------------------------------------------------
	if(bEnableServerPhysics && GetNetMode() != NM_Client)
	{
		PhysicsComponent->SetSimulatePhysics(true);
		
		if(NetUpdateFrequency < 4) NetUpdateFrequency = 4;
		
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToPhysicalEntityArray(this);
		}
	}
	//------------------------------------------------------------------------------
	// The Local PlayerController Handles Physics on the Client!
	//------------------------------------------------------------------------------
	else if(bEnableClientPhysics && GetNetMode() == NM_Client)
	{
		PhysicsComponent->SetSimulatePhysics(false);
	
		Rep_Location = GetActorLocation();
		Rep_Rotation = GetActorRotation();
		Rep_Scale = GetActorScale();

		if (AEDU_CORE_PlayerController* LocalController = Cast<AEDU_CORE_PlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			LocalController->AddToPhysicalEntityArray(this);
		}
	}
}

void AEDU_CORE_PhysicalEntity::ServerPhysicsCalc(float DeltaTime)
{ // FlowLog_AI_TICK
	// Multithreaded ParallelFor in GameMode
	if(bEnableSimpleServerPhysicsOnly)
	{
		if(bTraceForGround)
		{
			SimpleGravityCalc(DeltaTime);
		}
		else
		{	// Timer, so we only trace in intervals if we're not in free fall.
			if(FrameCounter < 15)
			{
				FrameCounter++;
				return;
			}
			bTraceForGround = true;
			FrameCounter = 0;
		}
	}
}

void AEDU_CORE_PhysicalEntity::ServerPhysicsExec(float DeltaTime)
{ // FlowLog_AI_TICK
	// Update ReplicatedTransform with the new transform from server-side physics simulation
	Rep_Location = GetActorLocation();
	Rep_Rotation = GetActorRotation();
	Rep_Scale = GetActorScale();

	if(bEnableSimpleServerPhysicsOnly && bTraceForGround)
	{
		SimpleGravityExe();
	}
	// DrawDebugLine(GetWorld(), GetActorLocation(),FinalLocation, FColor::Green, false, 1.f, 0, 1.0f);
	//GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	// FString::Printf(TEXT("FallSpeed: %f m/s"), FallSpeed));
	
	//GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	//FString::Printf(TEXT("FrameCounter: %d"), FrameCounter));
}

//------------------------------------------------------------------------------
// Overlap Events
//------------------------------------------------------------------------------

void AEDU_CORE_PhysicalEntity::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{ FLOW_LOG
	if (OtherActor && OtherActor != this)
	{
		FLOW_LOG_WARNING("Overlapping!")
		Overlapping = true;
		OverlapArray.Add(OtherActor);
		SimpleCollisionReaction(OtherActor);
	}
}

void AEDU_CORE_PhysicalEntity::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{ FLOW_LOG
	// Handle the overlap end event
	if (OtherActor && OtherActor != this)
	{
		OverlapArray.Remove(OtherActor);
		
		if(OverlapArray.Num() == 0)
		{
			Overlapping = false;
			OverlapArray.Reset();
		}
	}		
}

//------------------------------------------------------------------------------
// Simple Collision
//------------------------------------------------------------------------------

void AEDU_CORE_PhysicalEntity::SimpleCollisionReaction(AActor* OtherActor)
{ FLOW_LOG
	
	if (!OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetActor is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("TargetActor %s"), *OtherActor->GetName());
	
	FVector MyLocation = GetActorLocation();
	FVector CollisionLocation = OtherActor->GetActorLocation();
	UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("CollisionLocation: %f, %f, %f"), CollisionLocation.Z, CollisionLocation.X, CollisionLocation.Y);

	// Perform the line trace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore this actor during trace

	bool Trace = GetWorld()->LineTraceSingleByChannel(HitResult, MyLocation, CollisionLocation, ECC_Visibility, QueryParams);
	DrawDebugLine(GetWorld(), MyLocation, CollisionLocation, FColor::Green, false, 1.0f, 0, 1.0f);

	if (true)
	{
		
		UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Target hit!"));
		// Calculate the distance between the two actors
		float Distance = FVector::Dist(MyLocation, CollisionLocation);

		// Calculate the direction to move in (opposite of the trace direction)
		FVector MoveDirection = (CollisionLocation - MyLocation).GetSafeNormal();
		
		// Calculate the new location to move to
		FVector NewLocation = MyLocation - MoveDirection * Distance;
		DrawDebugSphere(GetWorld(), NewLocation, 30.f, 12, FColor::Green, false, 10.f);

		// Move the actor with sweep
		FHitResult SweepHitResult;
		RootComponent->SetRelativeLocation(NewLocation, true, &SweepHitResult);
        
		if (SweepHitResult.IsValidBlockingHit())
		{
			UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Sweep hit something while moving: %s"), *SweepHitResult.GetActor()->GetName());
		}
	}
	else
	{
		UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Line trace did not hit anything."));
	}
}

void AEDU_CORE_PhysicalEntity::SimpleGravityCalc(float DeltaTime)
{
	FVector Origin, BoxExtent;
	GetActorBounds(false, Origin, BoxExtent);
	
	FVector TraceStartLocation = GetActorLocation();
	FVector TraceEndLocation = TraceStartLocation - FVector(0.f, 0.f, BoxExtent.Z + FallSpeed + GroundOffset);  // BoxExtent – Set to half the actor's size in 3d space, so this is the bottom.
	
	// Perform the line trace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore the actor itself
	
	// Line trace downwards
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, ECC_Visibility, QueryParams))
	{
		if(HitResult.bBlockingHit)
		{
			// If we hit something, set the new center location to the impact point
			NewHeight = HitResult.ImpactPoint + FVector(0.f, 0.f, BoxExtent.Z + GroundOffset) ;
			FallSpeed = 0.1f; // We give it a gap, so it doesn't clip through the floor.
			bTraceForGround = false;
			// TODO: there should be a death above a certain speed here.
				/*---------------------- Bounce solution for lols ----------------------------
				 We can make the impact bounce by inverting the fallspeed.
					float OldFallSpeed = FallSpeed;
					FallSpeed = OldFallSpeed * -0.8f; // The inverted fraction here is the bounce.
					//bTraceForGround = false;
				----------------------------------------------------------------------------*/
		}
	}
	else
	{
		// If nothing was hit, it means we are falling.
		if(FallSpeed < 100'00.f)
		{
			FallSpeed += 9.0f * DeltaTime;
		}
		//FinalLocation.X = GetActorLocation().X;
		//FinalLocation.Y = GetActorLocation().Y;
		NewHeight.Z = TraceStartLocation.Z - FallSpeed;
		bTraceForGround = true;
	}
} 

void AEDU_CORE_PhysicalEntity::SimpleGravityExe()
{
	SetActorLocation(NewHeight, true);
}

//------------------------------------------------------------------------------
// Network Functionality
//------------------------------------------------------------------------------

// Function to get lifetime replicated props
void AEDU_CORE_PhysicalEntity::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if(!bEnableClientIndependentPhysics)
	{
		DOREPLIFETIME(AEDU_CORE_PhysicalEntity, Rep_Location);
		DOREPLIFETIME(AEDU_CORE_PhysicalEntity, Rep_Rotation);
		DOREPLIFETIME(AEDU_CORE_PhysicalEntity, Rep_Scale);
	}
}

void AEDU_CORE_PhysicalEntity::OnRep_ReplicatedLocation()
{ // FLOW_LOG 
	// Handle transform update on clients
	StartLocation = GetActorLocation();
	BlendDuration_Location = ElapsedTime_Location;
	ElapsedTime_Location = 0.f;
	
	//UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - BlendDuration: %f"), *GetClass()->GetName(), __FUNCTION__, BlendDuration);
}

void AEDU_CORE_PhysicalEntity::OnRep_ReplicatedRotation()
{ // FLOW_LOG
	StartRotation = GetActorRotation();
	StartQuat = FQuat(GetActorRotation());
	TargetQuat = FQuat(Rep_Rotation);
	
	BlendDuration_Rotation = ElapsedTime_Rotation;
	ElapsedTime_Rotation = 0.f;
}

void AEDU_CORE_PhysicalEntity::OnRep_ReplicatedScale()
{ // FLOW_LOG
	StartScale = GetActorScale();
	BlendDuration_Scale = ElapsedTime_Scale;
	ElapsedTime_Scale = 0.f;
}

//------------------------------------------------------------------------------
// Client Functionality
//------------------------------------------------------------------------------

void AEDU_CORE_PhysicalEntity::ClientLerpLocation(float DeltaTime)
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

void AEDU_CORE_PhysicalEntity::ClientLerpRotation(float DeltaTime)
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

void AEDU_CORE_PhysicalEntity::ClientLerpScale(float DeltaTime)
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

