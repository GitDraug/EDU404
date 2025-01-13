#pragma once

#include "OrganicStatusConditions.generated.h"

UENUM()
enum class EOrganicConditions : uint16
{
	// Default - Individual is Healthy
	None			UMETA(DisplayName = "None"),

//------------------------------------------------------------------
// Mild Organic Conditions
//------------------------------------------------------------------
	
	// A superficial injury to the skin caused by friction or scraping.
	Abrasion		UMETA(DisplayName = "Abrasion"),

	//  A wound caused by a sharp object, resulting in a clean break in the skin.
	Cut				UMETA(DisplayName = "Cut"),

	// A viral infection affecting the upper respiratory tract, causing symptoms like sneezing, congestion, and sore throat.
	CommonCold		UMETA(DisplayName = "Common Cold"),
	
	// A traumatic brain injury resulting from a blow to the head or violent shaking of the head.
	Concussion		UMETA(DisplayName = "Concussion"),
};

USTRUCT(BlueprintType)
struct FOrganicCondition_Abrasion
{
	GENERATED_BODY()

	//--------------------------------------------------------------------------------------
	// Description
	//--------------------------------------------------------------------------------------
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "Abrasion";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "A superficial injury to the skin caused by friction or scraping.";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Severity = "Typically mild; involves damage to the outer layer of the skin (epidermis) and may result in pain and minor bleeding.";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Treatment = "Clean the wound with water, apply an antiseptic, and cover with a sterile dressing or bandage.";
	
	//--------------------------------------------------------------------------------------
	// Regen / Degen
	//--------------------------------------------------------------------------------------

	// Physical
	UPROPERTY()
	float PhysDegen = 0.f;

	UPROPERTY()
	float CondDegen = 0.f;

	// Sustinance
	UPROPERTY()
	float HydroDegen = 0.f;

	UPROPERTY()
	float NutrDegen = 0.f;

		
	//--------------------------------------------------------------------------------------
	// Flat Damage
	//--------------------------------------------------------------------------------------
	
	UPROPERTY()
	float PhysDamage = -3.f;

	UPROPERTY()
	float CondDamage = 0.f;
	
	UPROPERTY()
	float EnduDamage = 0.f;

	UPROPERTY()
	float SustDamage = 0.f;
	
};

USTRUCT(BlueprintType)
struct FOrganicCondition_Cut
{
	GENERATED_BODY()

	//--------------------------------------------------------------------------------------
	// Description
	//--------------------------------------------------------------------------------------
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "Cut";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "A wound caused by a sharp object, resulting in a clean break in the skin.";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Severity = "Can vary from mild to severe, depending on the depth and location of the cut; may require medical attention.";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Treatment = "Control bleeding with direct pressure, clean the wound with water if available, and cover with a sterile dressing.";
	
	//--------------------------------------------------------------------------------------
	// Regen / Degen
	//--------------------------------------------------------------------------------------

	// Physical
	UPROPERTY()
	float PhysDegen = 0.f;

	UPROPERTY()
	float CondDegen = 0.f;

	// Sustinance
	UPROPERTY()
	float HydroDegen = 0.f;

	UPROPERTY()
	float NutrDegen = 0.f;
	
	//--------------------------------------------------------------------------------------
	// Flat Damage
	//--------------------------------------------------------------------------------------
	
	UPROPERTY()
	float PhysDamage = -3.f;

	UPROPERTY()
	float CondDamage = 0.f;
	
	UPROPERTY()
	float EnduDamage = 0.f;

	UPROPERTY()
	float SustDamage = 0.f;
	
};
