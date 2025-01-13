// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "StatusComponent.generated.h"


enum class EOrganicConditions : uint16;
class USphereComponent;
class UBoxComponent;
class AEDU_CORE_GameMode;
class AEDU_CORE_SpectatorCamera;
class AEDU_CORE_C2_Camera;

/*------------------------------------------------------------------------------
  Status Component
--------------------------------------------------------------------------------
  Acts as a manager for self-centered stats, such as Health and Damage.
  
  All Health and Damage is self-inflicted, while engagements ond offensive
  actions aimed at other enteties are managed in the EngagementComponent.
------------------------------------------------------------------------------*/

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:
	// Gets the entities C2 Camera
	FORCEINLINE  TObjectPtr<AEDU_CORE_C2_Camera> GetC2Camera() const { return LocalCamera; };
	
	// Gets how long we are visible when detected
	FORCEINLINE uint8 GetVisibilityTimer() const { return VisibilityTimer; };
	
	// Gets the entities Maximum Health
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; };
	
	// Gets the entities Visual Camouflage rating
	FORCEINLINE int32 GetVisualCamouflage() const { return VisualCamouflage; };
	
	// Gets the entities Thermal Camouflage rating
	FORCEINLINE int32 GetThermalCamouflage() const { return ThermalCamouflage; };
	
	// Gets the entities Noise Camouflage rating
	FORCEINLINE int32 GetNoiseCamouflage() const { return NoiseCamouflage; };
	
	// Gets the team we're currently on
	FORCEINLINE EEDU_CORE_Team GetActiveTeam() const { return ActiveTeam; };

	// Sets visibility for a specific team for a duration
	void SetVisibleForTeam(EEDU_CORE_Team TeamIndex, uint8 Time = 10);
	
	// Gets visibility for a specific team for a duration
	uint8 GetVisibleForTeam(EEDU_CORE_Team TeamInde) const;
	
	// Returns the Defence against a certain damage type 
	float GetDefenceAgainst(EDamageType DamageType) const;

	// Sets visibility for a specific team using the default duration
	void ResetVisibilityForTeam(EEDU_CORE_Team TeamIndex);

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	
	// Sets default values for this component's properties
	UStatusComponent();

protected:

	// Triggered both in Editor and Runtime
	virtual void OnRegister() override;
	
	// Called when the game starts
	virtual void BeginPlay() override;

	// Override GetLifetimeReplicatedProps
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:
	virtual void ServerStatusCalc(float DeltaTime);
	virtual void ServerStatusExec(float DeltaTime);

protected:
	// Issued by GameMode for time sliced tick
	UPROPERTY(VisibleAnywhere)
	int32 BatchIndex = 0;
	
//--------------------------------------------------------------------------
// Editable Data > Defense > Health & Resistances
//--------------------------------------------------------------------------
protected:
	
	// Max Health Pool
	UPROPERTY(EditAnywhere, Category = "Defence | Health")
	float MaxHealth = 100.f;

	// The amount of Health this entity regenerates per second
	UPROPERTY(EditAnywhere, Category = "Defence | Health")
	float HealthRegen = 1.f;

	// The amount of Health this entity degenerates per second
	UPROPERTY(EditAnywhere, Category = "Defence | Health")
	float HealthDegen = -1.f;
	
	// How many percentages of our body are protected?
	UPROPERTY(EditAnywhere, Category = "Defence | Health", meta = (ClampMin = "0", ClampMax = "100"))
	uint8 Coverage = 100;

	// Is this unit able to dodge attacks? (Lucky)
	UPROPERTY(EditAnywhere, Category = "Defence | Health")
	bool bCanDodge = false;

	// How large is the chance that this unit is able to doge an attack?
	UPROPERTY(EditAnywhere, Category = "Defence | Health", meta = (ClampMin = "0.0", ClampMax = "1.0"), meta = (EditCondition = "!bCanDodge"))
	float EvasionRating = 0.f;
	
	// Our Armor will degrade with each hit, but stop degrading at [Default: 25%]
	UPROPERTY(EditAnywhere, Category = "Defence | Health", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefenceDegradationCap = 0.25f;
	
	// Immune to Kinetic damage?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bKineticImmune = false;

	// Protection against kinetic energy IE padded clothing, body armor, etc.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bKineticImmune"))
	float KineticDefence = 0.f;
	
	// Immune to Cold?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bColdImmune = false;

	// Insulation against cold.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bColdImmune"))
	float ColdResistance = 0.f; 

	// Immune to Fire, Flame and Heat?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bHeatImmune = false;
	
	// Fire, Flame and Heat Resistance.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bHeatImmune"))
	float HeatResistance = 0.f;

	// Immune to Electromagnetic and Particle Radiation?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bRadiationImmune = false;

	// Resistance to Electromagnetic and Particle Radiation.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bRadiationImmune"))
	float RadiationResistance = 0.f;

	// Immune to Biological contaminants, such as bacteria, fungus and viruses?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bBioImmune = false;

	// Resistance to Biological contaminants, such as bacteria, fungus and viruses.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bBioImmune"))
	float BiologicalResistance = 0.f;

	// Immune to Corrosion, Acid and various Toxins?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bChemImmune = false;

	// Resistance to Corrosion, Acid and various Toxins
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bChemImmune"))
	float ChemicalResistance = 0.f;

	// Immune to Malicious Software?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bMalwareImmune = false;
	
	// Resistance to Malicious Software.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bMalwareImmune"))
	float MalwareResistance = 0.f;
	
	// Immune to Chaos?
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance")
	bool bChaosImmune = false;
	
	// Resistance to Chaotic, unkown damage, caused by unidentified source beyond human comprehension.
	UPROPERTY(EditAnywhere, Category = "Defence | Resistance", meta = (EditCondition = "!bChaosImmune"))
	float ChaosResistance = 0.f;

//--------------------------------------------------------------------------
// Editable Data > Organic Data
//--------------------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Status")
	EOrganicStatus_Physique PhysiqueStatus;
	
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Status")
	EOrganicStatus_Condition ConditionStatus;
	
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Status")
	EOrganicStatus_Nutrition NutritionStatus;
	
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Status")
	EOrganicStatus_Hydration HydrationStatus;
	
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Status")
	EOrganicStatus_Endurance EnduranceStatus;
	
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Status")
	EOrganicStatus_Mental MentalStatus;
	
	//-------------------------------------------------------------------
	// Physique: The body’s physical structure, build, and composition.
	//-------------------------------------------------------------------
	
	// The Maximum Physique when healthy
	UPROPERTY(EditAnywhere, Category = "Organic Data | Physique")
	float MaxPhysique = 100.f;

	// Flat Physice Damage caused by a Condition
	UPROPERTY(EditAnywhere, Category = "Organic Data | Physique")
	float PhysiqueDamage = 0.f;
	
	// The Maximum Physique when effected by a Condition
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Physique")
	float CappedPhysique = MaxPhysique - PhysiqueDamage;
	
	// Our Current Physique
	UPROPERTY(EditAnywhere, Category = "Organic Data | Physique")
	float CurrentPhysique = CappedPhysique;

	// The bodys the ability to heal injured cells
	UPROPERTY(EditAnywhere, Category = "Organic Data | Physique")
	float PhysiqueRegen = 0.f;

	// Damage over Time (DoT) due to injury
	UPROPERTY(EditAnywhere, Category = "Organic Data | Physique")
	float PhysiqueDegen = 0.f;
	
	//--------------------------------------------------------------
	// Condition: Illness, Disease, Affliction
	//--------------------------------------------------------------
	
	// The Maximum Condition when healthy
	UPROPERTY(EditAnywhere, Category = "Organic Data | Condition")
	float MaxCondition = 10.f;

	// Flat Condition Damage caused by a Condition
	UPROPERTY(EditAnywhere, Category = "Organic Data | Condition")
	float ConditionDamage = 0.f;
	
	// The Maximum Condition when effected by a Condition
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Condition")
	float CappedCondition = MaxCondition - CappedCondition;

	// Our Current Condition
	UPROPERTY(EditAnywhere, Category = "Organic Data | Condition")
	float CurrentCondition = CappedCondition;

	// The body's ability to combat Disease
	UPROPERTY(EditAnywhere, Category = "Organic Data | Condition")
	float ConditionRegen = 0.f;

	// Damage over Time (DoT) due to Illness
	UPROPERTY(EditAnywhere, Category = "Organic Data | Condition")
	float ConditionDegen = 0.f;

	//--------------------------------------------------------------
	// Sustinance: Nutrition and Hydration
	//--------------------------------------------------------------

	// The Maximum Hydration and Nutrition the body can hold
	UPROPERTY(EditAnywhere, Category = "Organic Data | Sustinance")
	float MaxSustinance = 10.f;

	// Flat Condition Damage caused by a Condition
	UPROPERTY(EditAnywhere, Category = "Organic Data | Sustinance")
	float SustinanceDamage = 0.f;
	
	// The Maximum Hydration and Nutrition the body can hold when effected by a Condition
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Sustinance")
	float CappedSustinance = MaxSustinance - SustinanceDamage;

	// Our Current Condition
	UPROPERTY(EditAnywhere, Category = "Organic Data | Sustinance")
	float CurrentSustinance = CappedSustinance;

	//----------------
	// Nutrition
	//----------------
	
	// How many Calories the body can hold
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Sustinance | Nutrition")
	float MaxNutrition = MaxSustinance / 2.f;
	
	// How many Calories the body havs available for 
	UPROPERTY(EditAnywhere, Category = "Organic Data | Sustinance | Nutrition")
	float CurrentNutrition = MaxNutrition;

	// The body's ability to digest Nutrition
	UPROPERTY(EditAnywhere, Category = "Organic Data | Sustinance | Nutrition")
	float NutritionRegen = 0.f;
	
	// How fast the Body is burning Calories
	UPROPERTY(EditAnywhere, Category = "Organic Data | Sustinance | Nutrition")
	float NutritionDegen = 0.f;

	//----------------
	// Hydration
	//----------------
	
	// How mych Water the body can hold
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Sustinance | Hydration")
	float MaxHydration = MaxSustinance / 2.f;
	
	// How Water the body has available for digestion
	UPROPERTY(EditAnywhere, Category = "Organic Data | Sustinance | Hydration")
	float CurrentHydration = MaxHydration;
	
	// The body's ability to digest water
	UPROPERTY(EditAnywhere, Category = "Organic Data | Sustinance | Hydration")
	float HydrationRegen = 0.f;

	// How fast the Body is losing water
	UPROPERTY(EditAnywhere, Category = "Organic Data | Sustinance | Hydration")
	float HydrationDegen = 0.f;
	
	//--------------------------------------------------------------
	// Endurance: The ability to produce and consume energy
	//--------------------------------------------------------------

	// The Maximum Endurance when healthy
	UPROPERTY(EditAnywhere, Category = "Organic Data | Endurance")
	float MaxEndurance = 10.f;

	// Flat Condition Damage caused by a Condition
	UPROPERTY(EditAnywhere, Category = "Organic Data | Endurance")
	float EnduranceDamage = 0.f;

	// The Maximum Endurance when effected by a Condition
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Endurance")
	float CappedEndurance = MaxEndurance - EnduranceDamage;

	// Our Current Condition
	UPROPERTY(EditAnywhere, Category = "Organic Data | Endurance")
	float CurrentEndurance = CappedEndurance;
	
	// The body's ability to digest water
	UPROPERTY(EditAnywhere, Category = "Organic Data | Endurance")
	float EnduranceRegen = 0.f;

	// How fast the Body is losing water
	UPROPERTY(EditAnywhere, Category = "Organic Data | Endurance")
	float EnduranceDegen = 0.f;

	//--------------------------------------------------------------
	// Mental State
	//--------------------------------------------------------------
	
	// Current Mental State, based of all factors.
	UPROPERTY(VisibleAnywhere, Category = "Organic Data | Mental State")
	float CurrentMentalState = 1.f;
	
//--------------------------------------------------------------------------
// Editable Data > Veterancy
//--------------------------------------------------------------------------
protected:
	
	UPROPERTY(EditAnywhere, Category = "Veterancy")
	float MaxLevel = 0;
	
	// Resistance to Chaotic, unkown damage, caused by unidentified source beyond human comprehension.
	UPROPERTY(EditAnywhere, Category = "Veterancy")
	float CurrentLevel = 0;
	
//--------------------------------------------------------------------------
// Editable Data > Defense > Visibility
//--------------------------------------------------------------------------
protected:
	
	// Protection against Visual Checks
	UPROPERTY(EditAnywhere, Category = "Defence | Visibility", meta = (ClampMin = "0", ClampMax = "100"))
	int32 VisualCamouflage = 0;

	// Protection against Thermal Checks
	UPROPERTY(EditAnywhere, Category = "Defence | Visibility", meta = (ClampMin = "0", ClampMax = "100"))
	int32 ThermalCamouflage = 0;

	// Protection against Vibration Checks
	UPROPERTY(EditAnywhere, Category = "Defence | Visibility", meta = (ClampMin = "0", ClampMax = "100"))
	int32 NoiseCamouflage = 0;

	// How long does this entity stay visible after being detected?
	// <!> This shouldn't go below 4.
	UPROPERTY(EditAnywhere, Category = "Defence | Visibility", meta = (ClampMin = "1.0"))
	uint8 VisibilityTimer = 10;
	
	/*--------- Deprecated: TArray<bool> versus TArray<uint8> ----------
	  While TArray<bool> works fine, using an array of bool can be less
	  memory-efficient. Unreal Engine has optimizations for arrays of
	  certain types, and bool arrays might not be packed efficiently
	  in memory, especially when many elements are set to false.
	--------------------------------------------------------------------
	
	// Array to store Visibility for 11 teams
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_VisibleForTeamArray)
	FReplicatedArray<uint8> VisibleForTeamArray;

	// Array to store Fading Visibility for 11 teams
	UPROPERTY(VisibleAnywhere)
	FReplicatedArray<uint8> FadingForTeamArray; 
	*/
	
	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_0 = 0;
	
	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_1 = 0;
	
	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_2 = 0;

	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_3 = 0;

	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_4 = 0;

	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_5 = 0;

	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_6 = 0;

	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_7 = 0;

	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_8 = 0;

	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_9 = 0;

	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_10 = 0;

	// This one should never be decremented.
	UPROPERTY(EditAnywhere, Category = "Defence | Visibility| Team Timers", ReplicatedUsing = OnRep_VisibilityForTeamUpdate)
	uint8 VisibilityForTeam_Spectator = 1;
	
//--------------------------------------------------------------------------
// Editable Data > Team setting
//--------------------------------------------------------------------------
protected:
	
	// What Team does this entity belong to?
	UPROPERTY(EditAnywhere, Category = "Team Setting")
	EEDU_CORE_Team ActiveTeam = EEDU_CORE_Team::None;

	// What teams are Hostile?
	UPROPERTY(EditAnywhere, Category = "Team Setting")
	TArray<uint8> HostileTeamsArray;
	
//--------------------------------------------------------------------------
// Data > Damage & Defence
//--------------------------------------------------------------------------
protected:

	// Our Current Health
	UPROPERTY()
	float CurrentHealth = MaxHealth;

//--------------------------------------------------------------------------
// Components
//--------------------------------------------------------------------------
protected:

	// Used for Time Gated Functions such as Damage Over Time (DOT)
	UPROPERTY()
	float DeltaTimer = 0.f;
	
	// Pointer to owning Actor
	UPROPERTY()
	TObjectPtr<AActor> Owner = GetOwner();
	
	// Pointer to GameMode for easy access to Team Arrays
	UPROPERTY()
	TObjectPtr<AEDU_CORE_GameMode> GameMode = nullptr;

	// Pointer to LocalPlayerController
	UPROPERTY()
	TObjectPtr<APlayerController> LocalPlayerController;

	// Pointer to the Camera we are using
	UPROPERTY()
	TObjectPtr<AEDU_CORE_C2_Camera> LocalCamera;

	// All Organic Conditions this entity is suffering from
	UPROPERTY(EditAnywhere, Category = "Status")
	TSet<FOrganicCondition> OrganicConditionSet;
	
	
//--------------------------------------------------------------------------
// Functionality > Damage Handling
//--------------------------------------------------------------------------
public:

	// Apply Damage to entity based of a damage type.
	UFUNCTION()
	void ApplyDamage(const float DamageAmount = 0.f, const float PenetrationPercentage = 0.f, const EDamageType DamageType = EDamageType::EDT_Kinetic, const float ProtectionRatio = 1.f);

protected:
	// Calculate Damage dealt based of protection and penetration.
	UFUNCTION()
	float CalculateDamage(const float EffectiveDamage, float PenetrationPercentage, float& Protection, const float ProtectionRatio = 1.f) const;
	
//--------------------------------------------------------------------------
// Functionality > Conditions & Traits
//--------------------------------------------------------------------------
protected:

	// Adds a Status Condition from the Condition Library, initiating it on the fly.
	UFUNCTION()
	void AddOrganicCondition(const EOrganicConditions Condition);
	
	// Removes a Status Condition from the OrganicConditionSet, and removing harmful effect.
	UFUNCTION()
	void RemoveOrganicCondition(const EOrganicConditions Condition);

//--------------------------------------------------------------------------
// Functionality > Organic Calculations
//--------------------------------------------------------------------------

	void CalculateOrganicStatus();
	
//--------------------------------------------------------------------------
// Functionality > Utility
//--------------------------------------------------------------------------
protected:
	
	void ChangeTeam(EEDU_CORE_Team NewTeam);
	
	void UpdateHostileTeams();

	void VisibilityTick();

//------------------------------------------------------------------------------
// Network Functionality
//------------------------------------------------------------------------------
protected:
	
	// Checks and saves for the Local Player and the Local Camera
	UFUNCTION()
	void CheckLocalPlayer();

	// Sets visibility on the Clients, reacting to VisibilityForTeam
	UFUNCTION()
	void OnRep_VisibilityForTeamUpdate() const;

	// Sets visibility on the Listen Server without replicating to clients
	UFUNCTION(Server, Unreliable)
	void Server_VisibilityForTeamUpdate() const;
	
};
