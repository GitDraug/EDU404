// Fill out your copyright notice in the Description page of Project Settings.

// THIS
#include "Entities/Components/StatusComponent.h"
#include "Entities/Components/ComponentExtensions/StatusComponent_ConditionManagment.h"

// CORE
#include "Framework/Data/FLOWLOGS/FLOWLOG_COMPONENTS.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

// UE
#include "Framework/Pawns/EDU_CORE_C2_Camera.h"
#include "Net/UnrealNetwork.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

UStatusComponent::UStatusComponent()
{ // Don't Run logs in a component constructor, run it in OnRegister instead.

    // Never Tick!
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
}

void UStatusComponent::OnRegister()
{ FLOW_LOG
    Super::OnRegister();
    
    // SetIsReplicated shouldn't be run in constructor on components.
    SetIsReplicated(true);
}

void UStatusComponent::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();
    
    // Server Tick
    if(GetNetMode() != NM_Client)
    {
        if (AEDU_CORE_GameMode* GameModePtr = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
        {
            GameModePtr->AddToStatusComponentArray(this);
            GameModePtr->AddActorToTeamArray(GetOwner());
            GameMode = GameModePtr;
        }
    }
    
    // Save Owning Actor
    Owner = GetOwner();

    // Save pointer to Custom Player Pawn (C2_Camera)
    CheckLocalPlayer();

    // This doesn't seem strictly necessary, but it's good practice.
    ChangeTeam(ActiveTeam);
    
    if(ActiveTeam != EEDU_CORE_Team::Spectator)
    {
        // Units should always be hidden by default, else they will be visible for a moment before the replication kicks in.
        GetOwner()->SetActorHiddenInGame(true);
    }
}

void UStatusComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicating each visibility variable for each team
    DOREPLIFETIME(ThisClass, VisibilityForTeam_0);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_1);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_2);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_3);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_4);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_5);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_6);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_7);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_8);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_9);
    DOREPLIFETIME(ThisClass, VisibilityForTeam_10);
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void UStatusComponent::ServerStatusCalc(float DeltaTime)
{

}

void UStatusComponent::ServerStatusExec(float DeltaTime)
{
    // Decrement Visibility
    VisibilityTick();

    // Regenerate
    if(CurrentHealth < MaxHealth)
    {
        CurrentHealth += FMath::Clamp(HealthRegen, -MaxHealth, MaxHealth);
    }
}

//--------------------------------------------------------------------------
// Functionality > Damage Handling
//--------------------------------------------------------------------------

void UStatusComponent::ApplyDamage(const float DamageAmount, const float Penetration, const EDamageType DamageType, const float ProtectionRatio)
{ FLOW_LOG
    /*------------------------------------------------------------------------------------
      The damage system functions based on protection levels that must be defeated for
      damage to occur. If a projectile or other force cannot break through because
      the protection is too strong, no damage will occur.

      If a weapon can penetrate the protection, it means that a portion of its energy
      will pass through without being absorbed. However, the force must still have
      enough initial energy to penetrate the material, allowing a greater portion
      of energy to transfer to the protected target.

      A force with 100% penetration will transfer all its energy directly to the target
      without any absorption by the protection. Note that the force must still carry
      enough energy to overcome the protection.

      As the target sustains damage, its protection will wear down. The more energy
      the material absorbs, the more it degrades. Penetration enables a force to bypass
      some or all of the protection without being absorbed, causing less wear to
      the protective material.
    ------------------------------------------------------------------------------------
     About FMath::RandRange(0, 100) < Coverage
      This block executes if a random number between 0 and 100 is less than the
      Coverage value. Higher coverage increases the likelihood that the attack will be
      mitigated by protection.

      A person wearing a bulletproof vest still faces the risk of being shot in the face.
    ------------------------------------------------------------------------------------*/
    if (DamageAmount <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Damage amount must be greater than zero."));
        return; // Early exit if no valid damage amount
    }

    if(bCanDodge && FMath::FRand() < EvasionRating)
    {
        UE_LOG(LogTemp, Warning, TEXT("Lucky dice!"));
        return;
    }

    // Remember our health for percentage-based conditionals.
    float PreviousHealth = CurrentHealth;

    // Handle the damage type
    switch (DamageType)
    {
        case EDamageType::EDT_Kinetic:
            if(bKineticImmune) return;
        
            /*------------------------------------------------------------------------------------
              Kinetic energy covers all physical attacks, including shockwaves and mêlée weapons,
              wether piercing or blunt. A piercing force will naturally have higher penetration.
              
              Recomendations:
                Blunt: No, or low Penetration.          (0% - 20%)
                Slash, Rip, Tear: Medium Penetration.   (20% - 50%)
                Pierce:	High Penetration.               (50% - 90%)

              No kinetic energy should ever have 100% penetration.              
            ------------------------------------------------------------------------------------*/

            UE_LOG(LogTemp, Log, TEXT("Applying Kinetic Damage: %f with Penetration: %f"), DamageAmount, Penetration);
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, KineticDefence, ProtectionRatio);

        break;

        case EDamageType::EDT_Cold:
            if(bColdImmune) return;
        
            /*------------------------------------------------------------------------------------
              Cold damage refers to effects that pierce or overcome insulation. While cold can
              permeate materials and lead to temperature changes, it does not inherently destroy
              them. However, freezing can cause changes in material properties.

              For instance, metals can become more brittle at low temperatures, and certain
              materials may experience thermal contraction, which can lead to cracking or
              structural failure if they are subjected to rapid temperature changes.

              Recomendations:
                Cold (Köld): High to very high penetration. (35% - 90%)
                Freezing: Low penetration 0% - 35%
                
                No cold damage should ever have 100% penetration.
           ------------------------------------------------------------------------------------*/

            UE_LOG(LogTemp, Log, TEXT("Applying Cold Damage: %f with Penetration: %f"), DamageAmount, Penetration);
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, ColdResistance, ProtectionRatio);
                
        break;

        case EDamageType::EDT_Heat:
            if(bHeatImmune) return;

            /*------------------------------------------------------------------------------------
              Unlike cold, which can transfer through materials without inherently causing damage,
              heat and flame are destructive forces. When heat or flame interacts with a
              protective material, it tends to alter the material’s properties significantly,
              often leading to degradation or failure.

              Recomendations:
                Fire or Flame: very low penetration (causing combustion: 0% - 50%)
                Pure Heat: High penetration. (35% - 100%)
            ------------------------------------------------------------------------------------*/

            UE_LOG(LogTemp, Log, TEXT("Applying Heat Damage: %f with Penetration: %f"), DamageAmount, Penetration);
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, HeatResistance, ProtectionRatio);

            
        break;

        case EDamageType::EDT_Radiation:
            if(bRadiationImmune) return;

            /*------------------------------------------------------------------------------------
              The effects of Electromagnetic radiation (EMR) can penetrate materials based on its
              energy and wavelength, leading to a range of interactions from simple transmission
              to complex chemical reactions.

              While some forms of EMR can cause significant damage (especially in the case of
              ionizing radiation), others may interact with materials in ways that do not lead
              to immediate destruction.

              Recomendations:
                Radiation of right wavelenght should theoretically be able to pierce a protective
                barrier completely. (1% - 100%)
            ------------------------------------------------------------------------------------*/

            UE_LOG(LogTemp, Log, TEXT("Applying Radiation Damage: %f with Penetration: %f"), DamageAmount, Penetration);
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, RadiationResistance, ProtectionRatio);


        break;

        case EDamageType::EDT_Biological:
            if(bBioImmune) return;

            /*------------------------------------------------------------------------------------
              Bacteria, fungi, and other microorganisms can act as agents that "pierce" through
              materials by breaking down organic compounds or causing structural degradation.

              Examples:
               Termites and other pests can penetrate wooden structures, compromising their
               integrity without necessarily leaving large openings initially.
                
               Certain fungi can degrade the material properties of wood and other organic
               materials, leading to structural failures over time.

               Bacteria and viruses have developed various strategies to overcome protective
               barriers in host organisms. Bacteria can invade tissues through adhesion,
               enzymatic degradation, and immune evasion, while viruses rely on receptor
               binding and cellular mechanisms to gain entry into host cells.

              Recomendations:
                It is highly likely that all biological organisms will erode defenses before
                causing significant harm. Exceptions are certain viruses able to bypass defenses
                due to mutation or design. (0% - 100%)
            ------------------------------------------------------------------------------------*/

            UE_LOG(LogTemp, Log, TEXT("Applying Biological Damage: %f with Penetration: %f"), DamageAmount, Penetration);
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, BiologicalResistance, ProtectionRatio);

                
        break;

        case EDamageType::EDT_Chemical:
            if(bChemImmune) return;

            /*------------------------------------------------------------------------------------
              Chemical agents can effectively overcome protective barriers in biological and
              material systems through various mechanisms, including corrosion, dissolution,
              and disruption of cellular integrity. These interactions can lead to significant
              effects, including material degradation, toxicity, and impaired biological functions.

              Recomendations:
                The dual nature of chemical interactions will either cause significant harm to
                a material or bypass it entirely. Low (0 - 10%) OR Very High (90 - 100%)
            ------------------------------------------------------------------------------------*/

            UE_LOG(LogTemp, Log, TEXT("Applying Chemical Damage: %f with Penetration: %f"), DamageAmount, Penetration);
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, ChemicalResistance, ProtectionRatio);
        
        break;
        
         case EDamageType::EDT_Malware:
             if(bMalwareImmune) return;

            /*------------------------------------------------------------------------------------
              Malicious software operates under its own set of mechanisms that allow it to bypass
              defenses or cause significant harm. It does so by exploiting vulnerabilities, using
              social engineering, and using advanced techniques to evade detection.

              Recomendations:
                Depending on what the malware is designed to do, it can focus on avoiding barriers
                or destroy them entierly. (0% - 100%)
            ------------------------------------------------------------------------------------*/

            UE_LOG(LogTemp, Log, TEXT("Applying Malware Damage: %f with Penetration: %f"), DamageAmount, Penetration);
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, MalwareResistance, ProtectionRatio);


        break;

        case EDamageType::EDT_Chaos:
            if(bChaosImmune) return;

            /*------------------------------------------------------------------------------------
              Chaos damage manifests in various ways: psychologically, physically, environmentally,
              and existentially. These beings embody themes of insignificance, indifference, and
              the incomprehensible nature of the universe.

              The impact of such entities can lead to madness, societal collapse, and a redefined
              understanding of reality, leaving characters and readers alike grappling with the
              existential implications of a cosmos filled with ancient, uncaring forces.

              The Concecrated and the Concorted of Karkosa are excellent examples of physical
              beings being inrevertably altered by chaos.

              Recomendations:
                Chaos damage is inherintly chaotic, and both its damage and penetration level
                should be randomized.
            ------------------------------------------------------------------------------------*/

            UE_LOG(LogTemp, Log, TEXT("Applying Chaos Damage: %f with Penetration: %f"), DamageAmount, Penetration);
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, ChaosResistance, ProtectionRatio);
        
        break;
        
        default:
            UE_LOG(LogTemp, Error, TEXT("Unknown damage type!"));
        break;
    }

    /*---------------------------- Damage Magnitude ------------------------------------
      The magnitude of damage we take in a single hit can be used for stun, chock,
      panic, maiming, concussion and other lingering status effects.
    ------------------------------------------------------------------------------------*/
    float DamageMagnitude = CurrentHealth / PreviousHealth;
}

float UStatusComponent::CalculateDamage(const float DamageAmount, float Penetration, float& Protection, const float ProtectionRatio) const
{ FLOW_LOG
    // This is the chance of us bypassing protection entierly (IE shooting someone carrying a chest plate in the face).
    if(FMath::RandRange(0, 100) > Coverage)
    {
        UE_LOG(LogTemp, Warning, TEXT("CRIT! Applying direct damage, bypassing protection!"));
        return DamageAmount;
    }
    
    // If we can't bypass protection, we have to defeat it; how much energy is left in that case?
    const float EffectiveDamage = DamageAmount - (Protection * ProtectionRatio);
    if(EffectiveDamage <= 0) return 0;

    // Make sure Penetration is viable for percentage calculations
    Penetration = FMath::Clamp(Penetration, 0.f, 1.f);
    
    // Determine the degradation ratio based on penetration.
    // If there is no penetration, then our protection is taking 100% of the energy.
    float ProtectionDegradationRatio = (Penetration <= 1.0f) ? (1 - Penetration) : 1.0f;
    
    // Calculate the absolute degradation absorbed by protection.
    // We must cap this because the protective material can't physically absorb infinite force.
    float ProtectionDegradation;
    if(EffectiveDamage < Protection)
    {
        // Absorb all the force that didn't penetrate the protection.
        ProtectionDegradation = EffectiveDamage * ProtectionDegradationRatio;
    }
    else
    {
        // Absorb as much as possible, until no more force can be absorbed. 
        ProtectionDegradation = Protection * ProtectionDegradationRatio;
    }
    
    // What is the new amount of protection after degredation (capped)?
    if(Protection - ProtectionDegradation >= Protection * DefenceDegradationCap)
    {
        // Normal degredation.
        Protection -= ProtectionDegradation;
    }
    else
    {
        // Capped degredation.
        Protection *= DefenceDegradationCap;
    }
    
    // Piercing damage/Beyond-Armour Effect (BAE): This is the damage able to bypass (penetrate) protection.
    float CriticalDamage = DamageAmount * Penetration;

    /*------------------------------ Excess Damage --------------------------------------
      If the incoming damage exceeds twice the protection rating, the excess energy
      will destroy the protective material and inflict additional, compounding damage
      to anything beyond it.
    ------------------------------------------------------------------------------------*/
    float ExcessDamage = (EffectiveDamage * ProtectionDegradationRatio) - ProtectionDegradation;
    
    return CriticalDamage + ExcessDamage;
}

//--------------------------------------------------------------------------
// Functionality > Conditions & Traits
//--------------------------------------------------------------------------

void UStatusComponent::AddOrganicCondition(const EOrganicConditions Condition)
{
    FOrganicCondition OrganicCondition = StatusComponent_ConditionManagment::InitOrganicCondition(Condition);
    OrganicConditionSet.Add(OrganicCondition);

    // Regen/Degen Rates
    PhysiqueDegen    +=  OrganicCondition.PhysDegen;
    ConditionDegen   +=  OrganicCondition.CondDegen;
    HydrationDegen   +=  OrganicCondition.HydroDegen;
    NutritionDegen   +=  OrganicCondition.NutrDegen;

    // Flat Damage
    PhysiqueDamage   +=  OrganicCondition.PhysDamage;
    ConditionDamage  +=  OrganicCondition.CondDamage;
    SustinanceDamage +=  OrganicCondition.SustDamage;
    EnduranceDamage  +=  OrganicCondition.EnduDamage;
}

void UStatusComponent::RemoveOrganicCondition(const EOrganicConditions Condition)
{
    FOrganicCondition OrganicCondition = StatusComponent_ConditionManagment::InitOrganicCondition(Condition);
    if(OrganicConditionSet.Find(OrganicCondition))
    {
        OrganicConditionSet.Remove(OrganicCondition);
        
        // Regen/Degen Rates
        PhysiqueDegen    -=  OrganicCondition.PhysDegen;
        ConditionDegen   -=  OrganicCondition.CondDegen;
        HydrationDegen   -=  OrganicCondition.HydroDegen;
        NutritionDegen   -=  OrganicCondition.NutrDegen;

        // Flat Damage
        PhysiqueDamage   -=  OrganicCondition.PhysDamage;
        ConditionDamage  -=  OrganicCondition.CondDamage;
        SustinanceDamage -=  OrganicCondition.SustDamage;
        EnduranceDamage  -=  OrganicCondition.EnduDamage;
    }
}

void UStatusComponent::CalculateOrganicStatus()
{
    // Lambda to calculate the status index based on the percentage
    constexpr uint8 NumStatuses = 5;
    auto GetStatusIndex = [](float CurrentValue, float MaxValue)
    {
        return FMath::Clamp(FMath::FloorToInt((CurrentValue / MaxValue) * NumStatuses), 0, NumStatuses - 1);
    };

    float PhysiquePercentage =  CurrentPhysique  / MaxPhysique;
    float ConditionPercentage = CurrentCondition / MaxCondition;
    float NutritionPercentage = CurrentNutrition / MaxNutrition;
    float HydrationPercentage = CurrentHydration / MaxHydration;
    float EndurancePercentage = CurrentEndurance / MaxEndurance;
    
    //-------------------------------------------------------------------
    // Physique: The body’s physical structure, build, and composition.
    //-------------------------------------------------------------------
    if (CurrentPhysique <= CappedPhysique)
    {
        CurrentPhysique += (PhysiqueRegen + PhysiqueDegen);
    }

    if (PhysiquePercentage > 0.f)
    {
        switch (GetStatusIndex(CurrentPhysique, MaxPhysique))
        {
            case 0: PhysiqueStatus = EOrganicStatus_Physique::Wounded;  break;
            case 1: PhysiqueStatus = EOrganicStatus_Physique::Injured;  break;
            case 2: PhysiqueStatus = EOrganicStatus_Physique::InPain;   break;
            case 3: PhysiqueStatus = EOrganicStatus_Physique::Grazed;   break;
            case 4: PhysiqueStatus = EOrganicStatus_Physique::Solid;    break;
        default: ;
        }
    }
    else
    {
        PhysiqueStatus = EOrganicStatus_Physique::Unknown;
    }

    //--------------------------------------------------------------
    // Condition: Illness, Disease, Affliction
    //--------------------------------------------------------------
    if (CurrentCondition <= MaxCondition)
    {
        CurrentCondition += (ConditionRegen + ConditionDegen);
    }

    if (ConditionPercentage > 0.f)
    {
        switch (GetStatusIndex(CurrentCondition, MaxCondition))
        {
            case 0: ConditionStatus = EOrganicStatus_Condition::Sick;        break;
            case 1: ConditionStatus = EOrganicStatus_Condition::Feverish;    break;
            case 2: ConditionStatus = EOrganicStatus_Condition::Nauseous;    break;
            case 3: ConditionStatus = EOrganicStatus_Condition::Discomforted;break;
            case 4: ConditionStatus = EOrganicStatus_Condition::Alert;       break;
        default: ;
        }
    }
    else
    {
        ConditionStatus = EOrganicStatus_Condition::Unknown;
    }

    //--------------------------------------------------------------
    // Sustinance: Nutrition
    //--------------------------------------------------------------
    if (CurrentNutrition <= MaxNutrition)
    {
        CurrentNutrition += (NutritionRegen + NutritionDegen);
    }

    if (NutritionPercentage > 0.f)
    {
        switch (GetStatusIndex(CurrentNutrition, MaxNutrition))
        {
            case 0: NutritionStatus = EOrganicStatus_Nutrition::Starving;       break;
            case 1: NutritionStatus = EOrganicStatus_Nutrition::Famished;       break;
            case 2: NutritionStatus = EOrganicStatus_Nutrition::VeryHungry;     break;
            case 3: NutritionStatus = EOrganicStatus_Nutrition::Hungry;         break;
            case 4: NutritionStatus = EOrganicStatus_Nutrition::Satiated;       break;
        default: ;
        }
    }
    else
    {
        NutritionStatus = EOrganicStatus_Nutrition::Unknown;
    }
    
    //--------------------------------------------------------------
    // Sustinance: Hydration
    //--------------------------------------------------------------
    if (CurrentHydration <= MaxHydration)
    {
        CurrentHydration += (HydrationRegen + HydrationDegen);
    }

    if (HydrationPercentage > 0.f)
    {
        switch (GetStatusIndex(CurrentNutrition, MaxHydration))
        {
            case 0: HydrationStatus = EOrganicStatus_Hydration::DesperateForWater; break;
            case 1: HydrationStatus = EOrganicStatus_Hydration::Dehydrated;        break;
            case 2: HydrationStatus = EOrganicStatus_Hydration::VeryThirsty;       break;
            case 3: HydrationStatus = EOrganicStatus_Hydration::Thirsty;           break;
            case 4: HydrationStatus = EOrganicStatus_Hydration::Satiated;          break;
        default: ;
        }
    }
    else
    {
        HydrationStatus = EOrganicStatus_Hydration::Unknown;
    }

    //--------------------------------------------------------------
    // Endurance
    //--------------------------------------------------------------
    if (CurrentEndurance <= MaxEndurance)
    {
        CurrentEndurance += (EnduranceRegen + EnduranceDegen);
    }

    if (EndurancePercentage > 0.f)
    {
        switch (GetStatusIndex(CurrentNutrition, MaxHydration))
        {
            case 0: EnduranceStatus = EOrganicStatus_Endurance::Exhausted;      break;
            case 1: EnduranceStatus = EOrganicStatus_Endurance::Tired;          break;
            case 2: EnduranceStatus = EOrganicStatus_Endurance::Winded;         break;
            case 3: EnduranceStatus = EOrganicStatus_Endurance::Warmedup;       break;
            case 4: EnduranceStatus = EOrganicStatus_Endurance::Rested;         break;
        default: ;
        }
    }
    else
    {
        EnduranceStatus = EOrganicStatus_Endurance::Unknown;
    }

    //--------------------------------------------------------------
    // Mental State
    //--------------------------------------------------------------
    if (CurrentMentalState <= 1.f)
    {
        CurrentMentalState += (EnduranceRegen + EnduranceDegen);
    }

    CurrentMentalState =
        (PhysiquePercentage
        + ConditionPercentage
        + NutritionPercentage
        + HydrationPercentage
        + EndurancePercentage) / 5.0f;

    if ((CurrentMentalState / 1.f) > 0.f)
    {
        switch (GetStatusIndex(CurrentMentalState, 1.f))
        {
            case 0: MentalStatus = EOrganicStatus_Mental::Panicked;       break;
            case 1: MentalStatus = EOrganicStatus_Mental::Scared;         break;
            case 2: MentalStatus = EOrganicStatus_Mental::Shaken;         break;
            case 3: MentalStatus = EOrganicStatus_Mental::Stressed;       break;
            case 4: MentalStatus = EOrganicStatus_Mental::Steady;         break;
        default: ;
        }
    }
    else
    {
        MentalStatus = EOrganicStatus_Mental::Unknown;
    }
}

//--------------------------------------------------------------------------
// Functionality > Organic Calculations
//--------------------------------------------------------------------------



//--------------------------------------------------------------------------
// Functionality > Utility
//--------------------------------------------------------------------------

void UStatusComponent::ChangeTeam(EEDU_CORE_Team NewTeam)
{ FLOW_LOG
    // Only the server should be allowed to do this.
    if(!HasAuthority()) return;

    GameMode->RemoveActorFromTeamArray(GetOwner(), ActiveTeam);
    ActiveTeam = NewTeam;
    GameMode->AddActorToTeamArray(GetOwner(), ActiveTeam);

    UpdateHostileTeams();
    
    ResetVisibilityForTeam(ActiveTeam);

}

void UStatusComponent::UpdateHostileTeams()
{ FLOW_LOG
    // Only the server should be allowed to do this.
    if(!HasAuthority()) return;
    
    // Cast 'Team' to uint8 if it's an enum class
    const uint8 OurTeam = static_cast<uint8>(ActiveTeam);
    constexpr uint8 Max = static_cast<uint8>(EEDU_CORE_Team::Max);

    // 0 is neutral
    for (uint8 Enum = 1; Enum < Max; ++Enum)
    {
        if (Enum != OurTeam) // Exclude the specified team
        {
            HostileTeamsArray.AddUnique(Enum);
        }
    }
}

void UStatusComponent::SetVisibleForTeam(EEDU_CORE_Team TeamIndex, uint8 Time)
{
    switch (TeamIndex)
    {
        case EEDU_CORE_Team::None:		VisibilityForTeam_0 = Time;     break;
        case EEDU_CORE_Team::Team_1:	VisibilityForTeam_1 = Time;     break;
        case EEDU_CORE_Team::Team_2:	VisibilityForTeam_2 = Time;     break;
        case EEDU_CORE_Team::Team_3:	VisibilityForTeam_3 = Time;     break;
        case EEDU_CORE_Team::Team_4:	VisibilityForTeam_4 = Time;     break;
        case EEDU_CORE_Team::Team_5:	VisibilityForTeam_5 = Time;     break;
        case EEDU_CORE_Team::Team_6:	VisibilityForTeam_6 = Time;     break;
        case EEDU_CORE_Team::Team_7:	VisibilityForTeam_7 = Time;     break;
        case EEDU_CORE_Team::Team_8:	VisibilityForTeam_8 = Time;     break;
        case EEDU_CORE_Team::Team_9:	VisibilityForTeam_9 = Time;     break;
        case EEDU_CORE_Team::Team_10:	VisibilityForTeam_10 = Time;    break;
			        
    default: ;
    }
}

uint8 UStatusComponent::GetVisibleForTeam(EEDU_CORE_Team TeamIndex) const
{
    switch (TeamIndex)
    {
        case EEDU_CORE_Team::None:		return VisibilityForTeam_0;     
        case EEDU_CORE_Team::Team_1:	return VisibilityForTeam_1;     
        case EEDU_CORE_Team::Team_2:	return VisibilityForTeam_2;    
        case EEDU_CORE_Team::Team_3:	return VisibilityForTeam_3;
        case EEDU_CORE_Team::Team_4:	return VisibilityForTeam_4;
        case EEDU_CORE_Team::Team_5:	return VisibilityForTeam_5;
        case EEDU_CORE_Team::Team_6:	return VisibilityForTeam_6;
        case EEDU_CORE_Team::Team_7:	return VisibilityForTeam_7;
        case EEDU_CORE_Team::Team_8:	return VisibilityForTeam_8; 
        case EEDU_CORE_Team::Team_9:	return VisibilityForTeam_9;
        case EEDU_CORE_Team::Team_10:	return VisibilityForTeam_10;
			        
    default: return VisibilityForTeam_0;
    }
}

float UStatusComponent::GetDefenceAgainst(EDamageType DamageType) const
{
    switch (DamageType)
    {
        case EDamageType::EDT_Kinetic:
            if(bKineticImmune) return FLT_MAX;
            return KineticDefence;
        ;

        case EDamageType::EDT_Cold:
            if(bColdImmune) return FLT_MAX;
            return ColdResistance;
        ;
        
        case EDamageType::EDT_Heat:
            if(bHeatImmune) return FLT_MAX;
            return ColdResistance;
        ;

        case EDamageType::EDT_Radiation:
            if(bRadiationImmune) return FLT_MAX;
            return RadiationResistance;
        ;

        case EDamageType::EDT_Biological:
            if(bBioImmune) return FLT_MAX;
            return BiologicalResistance;
        ;

        case EDamageType::EDT_Chemical:
            if(bChemImmune) return FLT_MAX;
            return ChemicalResistance;
        ;

        case EDamageType::EDT_Malware:
            if(bMalwareImmune) return FLT_MAX;
            return MalwareResistance;
        ;

        case EDamageType::EDT_Chaos:
            if (bChaosImmune) return FLT_MAX;
            return ChaosResistance;
        
        default: return FLT_MAX;;
    }
}

void UStatusComponent::ResetVisibilityForTeam(EEDU_CORE_Team TeamIndex)
{ // FLOW_LOG

    switch (TeamIndex)
    {
        case EEDU_CORE_Team::None:		VisibilityForTeam_0 = VisibilityTimer; 	break;
        case EEDU_CORE_Team::Team_1:	VisibilityForTeam_1 = VisibilityTimer; 	break;
        case EEDU_CORE_Team::Team_2:	VisibilityForTeam_2 = VisibilityTimer; 	break;
        case EEDU_CORE_Team::Team_3:	VisibilityForTeam_3 = VisibilityTimer;  break;
        case EEDU_CORE_Team::Team_4:	VisibilityForTeam_4 = VisibilityTimer; 	break;
        case EEDU_CORE_Team::Team_5:	VisibilityForTeam_5 = VisibilityTimer;  break;
        case EEDU_CORE_Team::Team_6:	VisibilityForTeam_6 = VisibilityTimer;  break;
        case EEDU_CORE_Team::Team_7:	VisibilityForTeam_7 = VisibilityTimer; 	break;
        case EEDU_CORE_Team::Team_8:	VisibilityForTeam_8 = VisibilityTimer; 	break;
        case EEDU_CORE_Team::Team_9:	VisibilityForTeam_9 = VisibilityTimer;  break;
        case EEDU_CORE_Team::Team_10:	VisibilityForTeam_10 = VisibilityTimer;	break;
			        
    default: ;
    }
}

//--------------------------------------------------------------------------
// Functionality > Networking
//--------------------------------------------------------------------------

void UStatusComponent::CheckLocalPlayer()
{ FLOW_LOG
    
    // This should never fail, unless we are a dedicated server.
    LocalPlayerController = GetWorld()->GetFirstPlayerController();

    if(LocalPlayerController && LocalPlayerController->GetPawn())
    {
        LocalCamera = Cast<AEDU_CORE_C2_Camera>(LocalPlayerController->GetPawn());
    }
    else
    {
        FLOW_LOG_ERROR("Cast to Camera failed?")
    }
}

void UStatusComponent::VisibilityTick()
{    
    if(!HasAuthority()) return;

    /*------------------------- Dividing by Bitwise shift -----------------------------------
      In normal arithmetic, dividing a number by 2 gives you half of that number.

      A bitwise right shift (using the >> operator) shifts the bits of a number to the right
      by a specified number of positions. When you shift a number by 1 position (as in >> 1),
      it divides the number by 2 (for integers).
    --------------------------------------------------------------------------------------*/

    // Inlined lambda for removing an actor if visibility is greater than half
    auto Remove = [&](const uint8& VisibilityForTeam, const EEDU_CORE_Team Team) {
        if (VisibilityForTeam < (VisibilityTimer >> 1)) // Divide by 2
        {
            // UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("VisibilityForTeam %d = %d"), ActiveTeam, VisibilityForTeam);
            GameMode->RemoveActorFromTeamVisibleActorsArray(GetOwner(), Team);
        }
    };
    
    if(VisibilityForTeam_0 > 0 && ActiveTeam != EEDU_CORE_Team::None){ --VisibilityForTeam_0; Remove(VisibilityForTeam_0, EEDU_CORE_Team::None); }
    if(VisibilityForTeam_1 > 0 && ActiveTeam != EEDU_CORE_Team::Team_1){ --VisibilityForTeam_1; Remove(VisibilityForTeam_1, EEDU_CORE_Team::Team_1); }
    if(VisibilityForTeam_2 > 0 && ActiveTeam != EEDU_CORE_Team::Team_2){ --VisibilityForTeam_2; Remove(VisibilityForTeam_2, EEDU_CORE_Team::Team_2); }
    if(VisibilityForTeam_3 > 0 && ActiveTeam != EEDU_CORE_Team::Team_3){ --VisibilityForTeam_3; Remove(VisibilityForTeam_3, EEDU_CORE_Team::Team_3); }
    if(VisibilityForTeam_4 > 0 && ActiveTeam != EEDU_CORE_Team::Team_4){ --VisibilityForTeam_4; Remove(VisibilityForTeam_4, EEDU_CORE_Team::Team_4); }
    if(VisibilityForTeam_5 > 0 && ActiveTeam != EEDU_CORE_Team::Team_5){ --VisibilityForTeam_5; Remove(VisibilityForTeam_5, EEDU_CORE_Team::Team_5); }
    if(VisibilityForTeam_6 > 0 && ActiveTeam != EEDU_CORE_Team::Team_6){ --VisibilityForTeam_6; Remove(VisibilityForTeam_6, EEDU_CORE_Team::Team_6); }
    if(VisibilityForTeam_7 > 0 && ActiveTeam != EEDU_CORE_Team::Team_7){ --VisibilityForTeam_7; Remove(VisibilityForTeam_7, EEDU_CORE_Team::Team_7); }
    if(VisibilityForTeam_8 > 0 && ActiveTeam != EEDU_CORE_Team::Team_8){ --VisibilityForTeam_8; Remove(VisibilityForTeam_8, EEDU_CORE_Team::Team_8); }
    if(VisibilityForTeam_9 > 0 && ActiveTeam != EEDU_CORE_Team::Team_9){ --VisibilityForTeam_9; Remove(VisibilityForTeam_9, EEDU_CORE_Team::Team_9); }
    if(VisibilityForTeam_10 > 0 && ActiveTeam != EEDU_CORE_Team::Team_10){ --VisibilityForTeam_10; Remove(VisibilityForTeam_10, EEDU_CORE_Team::Team_10); }

    Server_VisibilityForTeamUpdate();
}

void UStatusComponent::Server_VisibilityForTeamUpdate_Implementation() const
{
    if(GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone)
    {
        switch (LocalCamera->GetTeam())
        {
            case EEDU_CORE_Team::None: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_0 == 0); break;
            case EEDU_CORE_Team::Team_1: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_1 == 0); break;
            case EEDU_CORE_Team::Team_2: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_2 == 0); break;
            case EEDU_CORE_Team::Team_3: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_3 == 0); break;
            case EEDU_CORE_Team::Team_4: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_4 == 0); break;
            case EEDU_CORE_Team::Team_5: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_5 == 0); break;
            case EEDU_CORE_Team::Team_6: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_6 == 0); break;
            case EEDU_CORE_Team::Team_7: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_7 == 0); break;
            case EEDU_CORE_Team::Team_8: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_8 == 0); break;
            case EEDU_CORE_Team::Team_9: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_9 == 0); break;
            case EEDU_CORE_Team::Team_10: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_10 == 0); break;
        
        default: ;
        }
    }
}

void UStatusComponent::OnRep_VisibilityForTeamUpdate() const
{ FLOW_LOG
    
    // <!> Client only
    switch (LocalCamera->GetTeam())
    {
        case EEDU_CORE_Team::None: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_0 == 0); break;
        case EEDU_CORE_Team::Team_1: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_1 == 0); break;
        case EEDU_CORE_Team::Team_2: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_2 == 0); break;
        case EEDU_CORE_Team::Team_3: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_3 == 0); break;
        case EEDU_CORE_Team::Team_4: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_4 == 0); break;
        case EEDU_CORE_Team::Team_5: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_5 == 0); break;
        case EEDU_CORE_Team::Team_6: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_6 == 0); break;
        case EEDU_CORE_Team::Team_7: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_7 == 0); break;
        case EEDU_CORE_Team::Team_8: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_8 == 0); break;
        case EEDU_CORE_Team::Team_9: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_9 == 0); break;
        case EEDU_CORE_Team::Team_10: GetOwner()->SetActorHiddenInGame(VisibilityForTeam_10 == 0); break;
    default: ;
    }
}