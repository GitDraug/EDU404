// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDU_CORE_StatusComponent.generated.h"

// Enum for different damage types
UENUM(BlueprintType) // Allow this enum to be used in Blueprints
enum class EDamageType : uint8
{
	EDT_Kinetic			UMETA(DisplayName = "Kinetic"),
	EDT_Cold			UMETA(DisplayName = "Cold"),
	EDT_Heat			UMETA(DisplayName = "Heat"),
	EDT_Radiation		UMETA(DisplayName = "Radiation"),
	EDT_Biological		UMETA(DisplayName = "Biological"),
	EDT_Chemical		UMETA(DisplayName = "Chemical"),
	EDT_Malware			UMETA(DisplayName = "Malware"),
	EDT_Chaos			UMETA(DisplayName = "Chaos"),
	DT_Max				UMETA(Hidden) // Optional: Use for bounds checking, not visible in editor
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API UEDU_CORE_StatusComponent : public UActorComponent
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this component's properties
	UEDU_CORE_StatusComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:
	
	// Gets the entities camouflage rating
	FORCEINLINE float GetCamouflage() const { return VisualCamouflage; };
	
//--------------------------------------------------------------------------
// Editable Data > Defense > Health & Resistances
//--------------------------------------------------------------------------
protected:
	// Max Health Pool
	UPROPERTY(EditAnywhere, Category = "Defence | Health")
	float MaxHealth = 0;
	
	// Our Armor will degrade with each hit, but stop degrading at [Default: 25%]
	UPROPERTY(EditAnywhere, Category = "Defence | Health", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefenceDegregationCap = 0.25f;
	
	// Immune to Kinetic damage?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bKineticImmune = false;

	// Protection against kinetic energy IE padded clothing, body armor, etc.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bPhysImmune"))
	float KineticDefence = 0;
	
	// Immune to Cold?
    UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
    bool bColdImmune = false;

	// Insulation against cold.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bColdImmune"))
	float ColdResistance = 0; 

	// Immune to Fire, Flame and Heat?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bHeatImmune = false;
	
	// Fire, Flame and Heat Resistance.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bHeatImmune"))
	float HeatResistance = 0;

	// Immune to Electromagnetic and Particle Radiation?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bRadiationImmune = false;

	// Resistance to Electromagnetic and Particle Radiation.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bRadiationImmune"))
	float RadiationResistance = 0;

	// Immune to Biological contaminants, such as bacteria, fungus and viruses?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	float BioImmune = false;

	// Resistance to Biological contaminants, such as bacteria, fungus and viruses.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!BioImmune"))
	float BiologicalResistance = 0;

	// Immune to Corrosion, Acid and various Toxins?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bChemImmune = false;

	// Resistance to Corrosion, Acid and various Toxins
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bChemImmune"))
	float ChemicalResistance = 0;

	// Immune to Malicious Software?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bMalwareImmune = false;
	
	// Resistance to Malicious Software.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bMalwareImmune"))
	float MalwareResistance = 0;
	
	// Immune to Chaos?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bChaosImmune = false;
	
	// Resistance to Chaotic, unkown damage, caused by unidentified source beyond human comprehension.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bChaosImmune"))
	float ChaosResistance = 0;

//--------------------------------------------------------------------------
// Editable Data > Defense > Sight
//--------------------------------------------------------------------------
protected:
	// Protection against Visual Checks
	UPROPERTY(EditAnywhere, Category = "Defence | Sight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VisualCamouflage = 0.f;
	
//--------------------------------------------------------------------------
// Data > Damage & Defence
//--------------------------------------------------------------------------
protected:

	// Our Current Health
	UPROPERTY()
	float CurrentHealth = MaxHealth;

	// Our Health before Applied Damage
	UPROPERTY()
	float PreviousHealth = CurrentHealth;

//--------------------------------------------------------------------------
// Data > Damage & Defence
//--------------------------------------------------------------------------
protected:

	// Apply Damage to entity based of a damage type.
	UFUNCTION()
	void ApplyDamage(float DamageAmount = 0.f, float PenetrationPercentage = 0.f, EDamageType DamageType = EDamageType::EDT_Kinetic);

	// Helper function for ApplyDamage.
	UFUNCTION()
	float CalculateDamage(float EffectiveDamage, float PenetrationPercentage, float& Protection) const;
};
