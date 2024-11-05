// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Components/EDU_CORE_StatusComponent.h"


// Sets default values for this component's properties
UEDU_CORE_StatusComponent::UEDU_CORE_StatusComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UEDU_CORE_StatusComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UEDU_CORE_StatusComponent::ApplyDamage(float DamageAmount, float Penetration, EDamageType DamageType)
{
    /*------------------------------------------------------------------------------------
      The damage system functions based on protection levels that must be defeated for
      damage to occur. If a projectile or other force cannot break through because
      the protection is too strong, no damage will occur.

      If a weapon can penetrate the protection, it means that a portion of its energy
      will pass through without being absorbed. However, the force must still have
      sufficient initial energy to penetrate the material, allowing a greater portion
      of energy to transfer to the protected target.

      A force with 100% penetration will transfer all its energy directly to the target
      without any absorption by the protection. Note that the force must still carry
      enough energy to overcome the protection.

      As the target sustains damage, its protection will wear down. The more energy
      the material absorbs, the more it degrades. Penetration enables a force to bypass
      some or all of the protection without being absorbed, causing less wear to
      the protective material.
    ------------------------------------------------------------------------------------*/
    if (DamageAmount <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Damage amount must be greater than zero."));
        return; // Early exit if no valid damage amount
    }

    // Make sure Penetration is viable for percentage calculations
    Penetration = FMath::Clamp(Penetration, 0.f, 1.f);

    PreviousHealth = CurrentHealth;

    // Handle the damage type
    switch (DamageType)
    {
        case EDamageType::EDT_Kinetic:
            if(bKineticImmune || DamageAmount - KineticDefence < 0) return;

            /*------------------------------------------------------------------------------------
              Kinetic energy covers all physical attacks, including shockwaves and mêlée weapons,
              wether piercing or blunt. A piercing force will naturally have higher penetration.
              
              Recomendations:
                Blunt: No, or low Penetration.          (0% - 20%)
                Slash, Rip, Tear: Medium Penetration.   (20% - 50%)
                Pierce:	High Penetration.               (50% - 90%)

              No kinetic energy should ever have 100% penetration.              
            ------------------------------------------------------------------------------------*/
        
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, KineticDefence);
            UE_LOG(LogTemp, Log, TEXT("Applying Kinetic Damage: %f with Penetration: %f"), DamageAmount, Penetration);
        break;

        case EDamageType::EDT_Cold:
            if(bColdImmune || DamageAmount - ColdResistance < 0) return;

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
        
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, ColdResistance);
            UE_LOG(LogTemp, Log, TEXT("Applying Cold Damage: %f with Penetration: %f"), DamageAmount, Penetration);
        break;

        case EDamageType::EDT_Heat:
            if(bHeatImmune || DamageAmount - HeatResistance < 0) return;

            /*------------------------------------------------------------------------------------
              Unlike cold, which can transfer through materials without inherently causing damage,
              heat and flame are destructive forces. When heat or flame interacts with a
              protective material, it tends to alter the material’s properties significantly,
              often leading to degradation or failure.

              Recomendations:
                Fire or Flame: very low penetration (causing combustion: 0% - 50%)
                Pure Heat: High penetration. (35% - 100%)
            ------------------------------------------------------------------------------------*/

            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, HeatResistance);
            UE_LOG(LogTemp, Log, TEXT("Applying Heat Damage: %f with Penetration: %f"), DamageAmount, Penetration);
        break;

        case EDamageType::EDT_Radiation:
            if(bRadiationImmune || DamageAmount - RadiationResistance < 0) return;

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
        
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, RadiationResistance);
            UE_LOG(LogTemp, Log, TEXT("Applying Radiation Damage: %f with Penetration: %f"), DamageAmount, Penetration);
        break;

        case EDamageType::EDT_Biological:
            if(BioImmune || DamageAmount - BiologicalResistance < 0) return;

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
        
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, BiologicalResistance);
            UE_LOG(LogTemp, Log, TEXT("Applying Biological Damage: %f with Penetration: %f"), DamageAmount, Penetration);
        break;

        case EDamageType::EDT_Chemical:
            if(bChemImmune || DamageAmount - ChemicalResistance < 0) return;

        /*------------------------------------------------------------------------------------
          Chemical agents can effectively overcome protective barriers in biological and
          material systems through various mechanisms, including corrosion, dissolution,
          and disruption of cellular integrity. These interactions can lead to significant
          effects, including material degradation, toxicity, and impaired biological functions.

          Recomendations:
            The dual nature of chemical interactions will either cause significant harm to
            a material, or bypass it entirely. Low (0 - 10%) OR Very High (90 - 100%)
        ------------------------------------------------------------------------------------*/
        
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, ChemicalResistance);
            UE_LOG(LogTemp, Log, TEXT("Applying Chemical Damage: %f with Penetration: %f"), DamageAmount, Penetration);
        break;
        
         case EDamageType::EDT_Malware:
            if(bMalwareImmune || DamageAmount - MalwareResistance < 0) return;

        /*------------------------------------------------------------------------------------
          Malicious software operates under its own set of mechanisms that allow it to bypass
          defenses or cause significant harm. It does so by exploiting vulnerabilities, using
          social engineering, and using advanced techniques to evade detection.

          Recomendations:
            Depending on what the malware is designed to do, it can focus on avoiding barriers
            or destroy them entierly. (0% - 100%)
        ------------------------------------------------------------------------------------*/
        
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, MalwareResistance);
            UE_LOG(LogTemp, Log, TEXT("Applying Chemical Damage: %f with Penetration: %f"), DamageAmount, Penetration);
        break;

        case EDamageType::EDT_Chaos:
            if(bChaosImmune || DamageAmount - ChaosResistance < 0) return;

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
            
            CurrentHealth -= CalculateDamage(DamageAmount, Penetration, ChaosResistance);
            UE_LOG(LogTemp, Error, TEXT("Applying Chaos Damage: %f with Penetration: %f"), DamageAmount, Penetration);
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

float UEDU_CORE_StatusComponent::CalculateDamage(float DamageAmount, float Penetration, float& Protection) const
{
    // Calculate initial effective damage after bypassing protection.
    const float EffectiveDamage = DamageAmount - Protection;

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
    if(Protection - ProtectionDegradation >= Protection * DefenceDegregationCap)
    {
        // Normal degredation.
        Protection -= ProtectionDegradation;
    }
    else
    {
        // Capped degredation.
        Protection *= DefenceDegregationCap;
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

