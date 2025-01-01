// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_SelectableEntity.h"
#include "EDU_CORE_PhysicsEntity.generated.h"

class UPrimitiveComponent;

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  En entity that implements Async Physics on the server and interpolates its
  translation to clients. Derived class with self-propulsion is MobileEntity.
------------------------------------------------------------------------------*/

UCLASS()
class EDU_CORE_API AEDU_CORE_PhysicsEntity : public AEDU_CORE_SelectableEntity
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this actor's properties
	AEDU_CORE_PhysicsEntity();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Override GetLifetimeReplicatedProps
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	// Pointer to the ShapeComponent that will be manipulated by the Physics Engine.
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> PhysicsComponent;

//------------------------------------------------------------------------------
// Network Functionality
//------------------------------------------------------------------------------
public:

	//---------------------------------------------------------------
	// Get/Set
	//--------------------------------------------------------------

	FORCEINLINE TObjectPtr<UPrimitiveComponent> GetPhysicsComponent() const { return PhysicsComponent; };
	
	//--------------------------------------------------------------
	// Server Aggregated Tick
	//--------------------------------------------------------------
	virtual void ServerPhysicsCalc(float DeltaTime);
	virtual void ServerPhysicsExec(float DeltaTime);	

	//---------------------------------------------------------------
	// Public Editor Settings
	//--------------------------------------------------------------	
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
		meta = (DisplayName = "Linear Interpolation for rotation",
		ToolTip = "Smooths rotation client-side, inbetween server updates. Only turn this on if you expect this entity to change rotation during play."))
	bool bLerpRotation;
	
	UPROPERTY(EditDefaultsOnly,
	Category = "Replicated, Client - Sided, Physics tick",
		meta = (DisplayName = "Linear Interpolation for scale",
		ToolTip = "Smooths sale client-side, inbetween server updates. Only turn this on if you expect this entity's hitbox to change scale or shape during play."))
	bool bLerpScale;
	
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
