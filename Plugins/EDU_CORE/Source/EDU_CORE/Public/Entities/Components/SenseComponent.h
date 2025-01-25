// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// CORE
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"

// UE
#include "CoreMinimal.h"
#include "Engine/OverlapResult.h"

// THIS
#include "SenseComponent.generated.h"

class AEDU_CORE_GameMode;
class UStatusComponent;
class AEDU_CORE_SelectableEntity;
class UTurretWeaponComponent;


/*------------------------------------------------------------------------------
  Sight component
--------------------------------------------------------------------------------
  Used to detect the presence of another actor.

  Each SenseComponent acts as a separate sensor; therefore, settings should not
  be configured in the StatusComponent but in each SenseComponent separately.
------------------------------------------------------------------------------*/

// Enum for different kinds of vision cones
UENUM(BlueprintType) // Allow this enum to be used in Blueprints
enum class EFieldOfVisionType : uint8
{
	EFOV_Sphere		UMETA(DisplayName = "Sphere - normal FOV"),
	EFOV_Capsule	UMETA(DisplayName = "Capsule - Long and Narrow FOV"),
	EFOV_Max		UMETA(Hidden)
};

// Enum for different kinds of sening
UENUM(BlueprintType) // Allow this enum to be used in Blueprints
enum class EFSenseType : uint8
{

	ESense_Sight			UMETA(DisplayName = "Sight Only"), // This sensor can detect visible light.
	ESense_SightAndHearing	UMETA(DisplayName = "Sight and Hearing"), // This sensor can detect visible light and vibrations.
	ESense_SightAndThermal	UMETA(DisplayName = "Sight and Thermal Vision"), // This sensor can detect visible light, infrared light (heat).
	ESense_All				UMETA(DisplayName = "Sight, Thermal Vision and Hearing"), // This sensor can detect visible light, infrared light (heat) and vibrations.
	ESense_Max				UMETA(Hidden)
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API USenseComponent : public UActorComponent
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this component's properties
	USenseComponent(const FObjectInitializer& ObjectInitializer);

protected:

	// Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called.
	virtual void OnRegister() override;
	
	// Called when the game starts
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:

	// Sets a new Sight Focus
	// <!> Observe that the entities total sight is the SightFocusLength + SightRadius.
	//     SightFocusLength 500 + SightRadius 500 will enable the entity to see 1000 straight forward. 
	FORCEINLINE void SetSightLenght(const float NewSightFocusLength) { SightFocusLength = NewSightFocusLength; }; 
	
//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:
	virtual void ServerSightCalc(float DeltaTime);
	virtual void ServerSightExec(float DeltaTime);
	
	// Get Batch Index from GameMode
	virtual void UpdateBatchIndex(const int32 ServerBatchIndex);

protected:
	// Issued by GameMode for time sliced tick
	UPROPERTY(VisibleAnywhere)
	int32 BatchIndex = 0;
	
//------------------------------------------------------------------------------
// Editable Data: General
//------------------------------------------------------------------------------
public:
	// What object tyoe are we searching for?
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> SenseObjectType = ECC_Pawn ;
	
	// What kind of Sense/Sensor is this component?
	UPROPERTY(EditAnywhere)
	EFSenseType SenseType = EFSenseType::ESense_Sight;

	// Name of the Turret that acts as a mount for this component.
	// If left empty, the SenseComponent will attach to the RootComponent.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAttachedToTurret = false;
	
	// Name of the Turret that acts as a mount for this component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bAttachedToMount", EditConditionHides))
	FString TurretName = "";
	
	// Rotation and Location relative to whatever the Component is attached to.
	UPROPERTY(EditAnywhere)
	FTransform RelativeTransform;

//------------------------------------------------------------------------------
// Editable Data: Sight
//------------------------------------------------------------------------------
public:
	// How well can this entity see?
	UPROPERTY(EditAnywhere, Category = "Sight | Radius", meta = (ClampMin = "0", ClampMax = "100"))
	int32 SightQuality = 100;

	// How well can this entity see themal signatures?
	UPROPERTY(EditAnywhere, Category = "Sight | Radius", meta = (ClampMin = "0", ClampMax = "100"))
	int32 ThermalQuality = 100;
	
	//<!> Observe that the entities total sight lenght is the SightFocusLength + SightRadius.
	UPROPERTY(EditAnywhere, Category = "Sight | Radius")
	float SightFocusLength = 10000.f;

	// How wide can this entity see?
	UPROPERTY(EditAnywhere, Category = "Sight | Radius")
	float SightRadius = 10000.f;

	// Shape of the FOV check
	UPROPERTY(EditAnywhere, Category = "Sight | Detection")
	EFieldOfVisionType FieldOfVisionType = EFieldOfVisionType::EFOV_Sphere;

	// Enable/disable debug shape
	UPROPERTY(EditAnywhere, Category = "Sight | Debug")
	bool bDrawSightDebugShape = false;

	// Enable/disable debug line
	UPROPERTY(EditAnywhere, Category = "Sight | Debug")
	bool bDrawSightDebugLine = false;

//------------------------------------------------------------------------------
// Editable Data: Hearing
//------------------------------------------------------------------------------
public:

	/*------------------------------------------------------------------
	  Hearning acts as 360 degrees of vibration detection, without the
	  need of visual confirmaiton.
	  
	  Note that some enteties, like certain Qubits, can sense vibrations
	  or "hear" through the ground, not just air.
	------------------------------------------------------------------*/

	// How well can this entity hear?
	UPROPERTY(EditAnywhere, Category = "Hearing | Radius", meta = (ClampMin = "0", ClampMax = "100"))
	int32 HearingQuality = 100;
	
	// How far can we hear?
	UPROPERTY(EditAnywhere, Category = "Hearing | Radius")
	float HearingRadius = 5000.f;
	
	// Enable/disable debug shape
	UPROPERTY(EditAnywhere, Category = "Hearing | Debug")
	bool bDrawHearningDebugShape = false;
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:

	// Pointer to owning Actor
	UPROPERTY()
	AActor* Owner = GetOwner();
	
	UPROPERTY()
	TObjectPtr<UTurretWeaponComponent> Parent;
	
	// Pointer to GameMode for easy access to Team Arrays
	UPROPERTY()
	TObjectPtr<AEDU_CORE_GameMode> GameMode = nullptr;
	
	// Pointer to StatusComponent
	UPROPERTY()
	TObjectPtr<UStatusComponent> StatusComponent = nullptr;

	// Cached ParentTransform for batched update
	UPROPERTY()
	FTransform PreviousParentTransform;

	// Temp Array for for DetectActorsInFOV()
	UPROPERTY(VisibleAnywhere)
	TArray<FOverlapResult> SensedActorsArray;
	
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
protected:

	// Makes use of a shape to trace an area for Object Types.
	UFUNCTION()
	void DetectActorsInFOV();

	// Makes use of a sphere to trace an area for Object Types.
	UFUNCTION()
	void Listen();

	// Helper function for DetectActorsInFOV() 
	UFUNCTION()
	bool GetVisualConfirmation(const FVector& StartLocation, const FVector& EndLocation, const AActor* ActorToConfirm) const;
	
	//
	void SetEntityTeamVisibility(AEDU_CORE_SelectableEntity* SelectableEntity, EEDU_CORE_Team OurTeam, UStatusComponent* TargetStatusComponent) const;


	
//------------------------------------------------------------------------------
// Functionality > Setup
//------------------------------------------------------------------------------
protected:
	
	// Ensure that we have a statusComponent active. This component will not function properly without it.
	void EnsureStatusComponent();
	
};
