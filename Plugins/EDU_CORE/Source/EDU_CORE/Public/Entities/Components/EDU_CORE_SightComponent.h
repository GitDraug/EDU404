// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "EDU_CORE_SightComponent.generated.h"

class USceneComponent;

/*------------------------------------------------------------------------------
  Sight component
--------------------------------------------------------------------------------
  Used to detect the visual presence of another actor.
------------------------------------------------------------------------------*/

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API USightComponent : public USceneComponent
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this component's properties
	USightComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
	
public:
	// How well can this entity see?
	UPROPERTY(EditAnywhere, Category = "Sight | Radius", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SightQuality = 1.f;

	// How far can this entity see?
	UPROPERTY(EditAnywhere, Category = "Sight | Radius")
	float SightLength = 1000.f;

	// How wide can this entity see?
	UPROPERTY(EditAnywhere, Category = "Sight | Radius")
	float SightWidth = 1000.f;

	// What collision channel are we viewing?
	UPROPERTY(EditAnywhere, Category = "Sight | Detection")
	TEnumAsByte<ETraceTypeQuery> SightDetectionVolumeChannel = TRACE_CHANNEL_SIGHTCONE ;
	
	// Enable/disable debug drawing
	UPROPERTY(EditAnywhere, Category = "Sight | Debug")
	bool bDrawDebug = false;
	
//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	
	UPROPERTY()
	TArray<AActor*> SensedActors;

	UPROPERTY()
	TArray<AActor*> IgnoredActors; // TODO this need ot be a ref to an array in gamestate.
	
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

	UFUNCTION()
	void PerformVisionFieldBoxTrace() const;

	UFUNCTION()
	bool VisualConfirmation() const;
};
