// Fill out your copyright notice in the Description page of Project Settings.

// THIS
#include "Entities/Components/EDU_CORE_StatusComponent.h"

// CORE
#include "Framework/Data/FLOWLOGS/FLOWLOG_COMPONENTS.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

// UE
#include "Framework/Pawns/EDU_CORE_C2_Camera.h"
#include "Net/UnrealNetwork.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

UEDU_CORE_StatusComponent::UEDU_CORE_StatusComponent()
{ // Don't Run logs in a component constructor, run it in OnRegister instead.

    // Never Tick!
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
}

void UEDU_CORE_StatusComponent::OnRegister()
{ FLOW_LOG
    Super::OnRegister();
    
    // SetIsReplicated shouldn't be run in constructor on components.
    SetIsReplicated(true);
}

void UEDU_CORE_StatusComponent::BeginPlay()
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

void UEDU_CORE_StatusComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
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

void UEDU_CORE_StatusComponent::ServerStatusCalc(float DeltaTime)
{

}

void UEDU_CORE_StatusComponent::ServerStatusExec(float DeltaTime)
{
    // Decrement Visibility
    VisibilityTick();
}

//--------------------------------------------------------------------------
// Functionality > Damage Handling
//--------------------------------------------------------------------------

void UEDU_CORE_StatusComponent::ApplyDamage(const float DamageAmount, const float Penetration, const EDamageType DamageType, const float ProtectionRatio)
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

float UEDU_CORE_StatusComponent::CalculateDamage(const float DamageAmount, float Penetration, float& Protection, const float ProtectionRatio) const
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
// Functionality > Utility
//--------------------------------------------------------------------------

void UEDU_CORE_StatusComponent::ChangeTeam(EEDU_CORE_Team NewTeam)
{ FLOW_LOG
    // Only the server should be allowed to do this.
    if(!HasAuthority()) return;

    ActiveTeam = NewTeam;

    UpdateHostileTeams();
    
    ResetVisibilityForTeam(ActiveTeam);

}

void UEDU_CORE_StatusComponent::UpdateHostileTeams()
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

void UEDU_CORE_StatusComponent::SetVisibleForTeam(EEDU_CORE_Team TeamIndex, uint8 Time)
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

void UEDU_CORE_StatusComponent::ResetVisibilityForTeam(EEDU_CORE_Team TeamIndex)
{ FLOW_LOG

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

void UEDU_CORE_StatusComponent::CheckLocalPlayer()
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

void UEDU_CORE_StatusComponent::VisibilityTick()
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

void UEDU_CORE_StatusComponent::Server_VisibilityForTeamUpdate_Implementation() const
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

void UEDU_CORE_StatusComponent::OnRep_VisibilityForTeamUpdate() const
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