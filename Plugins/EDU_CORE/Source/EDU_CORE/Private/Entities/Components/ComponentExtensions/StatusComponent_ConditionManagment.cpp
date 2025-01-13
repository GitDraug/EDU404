
#include "Entities/Components/ComponentExtensions/StatusComponent_ConditionManagment.h"
#include "Framework/Data/DataTypes/OrganicStatusConditions.h"

FOrganicCondition StatusComponent_ConditionManagment::InitOrganicCondition(const EOrganicConditions OrganicCondition)
{
	// Struct to Return
	FOrganicCondition Condition;

	//-------------------------------------------------------------------
	// Helper function to fill in properties from ConditionLibrary.
	//-------------------------------------------------------------------
	auto InitConditionWith = [&Condition](auto& BaseCondition)
	{
		// Desctiption
		Condition.Name			= BaseCondition.Name;
		Condition.Description	= BaseCondition.Description;
		Condition.Severity		= BaseCondition.Severity;
		Condition.Treatment		= BaseCondition.Treatment;

		// Regen / Degen
		Condition.PhysDegen		= BaseCondition.PhysDegen;
		Condition.CondDegen		= BaseCondition.CondDegen;
		Condition.HydroDegen	= BaseCondition.HydroDegen;
		Condition.NutrDegen		= BaseCondition.NutrDegen;

		// Flat Damage
		Condition.PhysDamage	= BaseCondition.PhysDamage;
		Condition.CondDamage	= BaseCondition.CondDamage;
		Condition.EnduDamage	= BaseCondition.EnduDamage;
		Condition.SustDamage	= BaseCondition.SustDamage;
	};

	//-------------------------------------------------------------------
	// Switch that fills in FOrganicCondition depending on Enum
	//-------------------------------------------------------------------

	// Get a Base from condition Library
	// Initiate the condition we will return with Base
	using enum EOrganicConditions;
	switch (OrganicCondition)
	{
		case Abrasion:	{ FOrganicCondition_Abrasion	AbrasionBase;	InitConditionWith(AbrasionBase);	break; }
		case Cut:		{ FOrganicCondition_Cut			CutBase;		InitConditionWith(CutBase);			break; }

		default: ;
	}

	return Condition;
}