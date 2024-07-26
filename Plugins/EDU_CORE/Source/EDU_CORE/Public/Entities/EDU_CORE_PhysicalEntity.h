// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_SelectableEntity.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "EDU_CORE_PhysicalEntity.generated.h"

class UBoxComponent;
class USphereComponent;
/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  En entity that implements gravity on the server and interpolates translation
  to clients. Derived class with self-propulsion is MobileEntity
------------------------------------------------------------------------------*/

UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_PhysicalEntity : public AEDU_CORE_SelectableEntity
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
protected:
	friend AEDU_CORE_GameMode; // GameMode is almighty.
	AEDU_CORE_PhysicalEntity();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void ServerPhysicsCalc(float DeltaTime);
	virtual void ServerPhysicsExec(float DeltaTime);

	// Override GetLifetimeReplicatedProps
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	// Pointer to the created physics component
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBoxComponent> PhysicsComponent;

//------------------------------------------------------------------------------
// Overlap Events
//------------------------------------------------------------------------------
protected:
	bool Overlapping;
	
	UPROPERTY()
	TArray<AActor*> OverlapArray;
	
	// Event handler functions
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
						AActor* OtherActor,
						UPrimitiveComponent* OtherComponent,
						int32 OtherBodyIndex,
						bool bFromSweep,
						const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
					  AActor* OtherActor,
					  UPrimitiveComponent* OtherComponent,
					  int32 OtherBodyIndex);

	virtual void SimpleCollisionReaction(AActor* OtherActor);
	
	//------------------------------------------------------------------------------
	// Simple gravity
	//------------------------------------------------------------------------------
	virtual void SimpleGravityCalc(float DeltaTime); // Multithreaded Calculations
	virtual void SimpleGravityExe(); // Final Server execution

	int16 FrameCounter;
	bool bTraceForGround = true;
	float FallSpeed = 0.f;
	FVector NewHeight; // ServerExe
	
//------------------------------------------------------------------------------
// Network Functionality
//------------------------------------------------------------------------------
public:
	UPROPERTY(EditAnywhere,
	Category = "Ground Offset",
	meta = (DisplayName = "Offset the Root component form the ground.",
	ToolTip = "The simplified server Physics gravity function traces fro ground, use this to offset the root physics component, if simulating the chest peice of an infantry unit for example."))
	float GroundOffset = 0.f;

	UPROPERTY(EditDefaultsOnly,
		Category = "Physics tick",
		meta = (DisplayName = "Enable Simplimfied Server Physics",
		ToolTip = "Uses simplified physics on the server instead of true physics."))
	bool bEnableSimpleServerPhysicsOnly = false;
	
	UPROPERTY(EditDefaultsOnly,
		Category = "Physics tick",
		meta = (DisplayName = "Enable Server Physics",
		ToolTip = "Adds this entity to the Aggregated Physics Tick in the GameMode"))
	bool bEnableServerPhysics = false;
	
	UPROPERTY(EditDefaultsOnly,
		Category = "Physics tick",
		meta = (DisplayName = "Enable Client Physics",
		ToolTip = "Adds this entity to the Aggregated Physics Tick in the PlayerController"))
	bool bEnableClientPhysics = false;

	UPROPERTY(EditDefaultsOnly,
		Category = "Decoupled, Non - Replicated, Client Independent Physics.",
		meta = (DisplayName = "Enable Client-Sided Independent Physics",
		ToolTip = "Enables Physics for this entity, but only on the local client. Decouples physics from server and other players. Effective for debris and other physics that doesn't need to be syncronized."))
	bool bEnableClientIndependentPhysics = false;
	
	UPROPERTY(EditDefaultsOnly,
		Category = "Replicated, Client - Sided, Physics tick",
		meta = (DisplayName = "Linear Interpolation for location",
		ToolTip = "Smooths location client-side, inbetween server updates. Only turn this on if you expect this entity to change location during play."))
	bool bLerpLocation;
	
	UPROPERTY(EditDefaultsOnly,
	Category = "Replicated, Client - Sided, Physics tick",
		meta = (DisplayName = "Linear Interpolation for scale",
		ToolTip = "Smooths sale client-side, inbetween server updates. Only turn this on if you expect this entity's hitbox to change scale or shape during play."))
	bool bLerpScale;
	
	UPROPERTY(EditDefaultsOnly,
		Category = "Replicated, Client - Sided, Physics tick",
		meta = (DisplayName = "Linear Interpolation for rotation",
		ToolTip = "Smooths rotation client-side, inbetween server updates. Only turn this on if you expect this entity to change rotation during play."))
	bool bLerpRotation;
	
	// Interpolate towards the replicated transform.
	FVector StartLocation;
	FRotator StartRotation;
	FVector StartScale;
	
	float BlendDuration_Location = 0.f;
	float BlendDuration_Rotation = 0.f;
	float BlendDuration_Scale = 0.f;

	// Elapsed time since the blend started.
	float ElapsedTime_Location = 0.f;	
	float ElapsedTime_Rotation = 0.f;
	float ElapsedTime_Scale = 0.f;
	
	FQuat StartQuat;
	FQuat TargetQuat;

	// Called by PlayerController on the client.
	void ClientLerpLocation(float DeltaTime);
	void ClientLerpRotation(float DeltaTime);
	void ClientLerpScale(float DeltaTime);

private:
	// Variables to replicate
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedLocation)
	FVector_NetQuantize10 Rep_Location;
	
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedRotation)
	FRotator Rep_Rotation;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedScale)
	FVector_NetQuantize10 Rep_Scale;

	// On Replication
	UFUNCTION()
	void OnRep_ReplicatedLocation();

	UFUNCTION()
	void OnRep_ReplicatedRotation();

	UFUNCTION()
	void OnRep_ReplicatedScale();
	
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
	
};
