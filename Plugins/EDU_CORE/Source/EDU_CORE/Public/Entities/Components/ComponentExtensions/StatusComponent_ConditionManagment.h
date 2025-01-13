#pragma once

#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"
#include "Framework/Data/DataTypes/OrganicStatusConditions.h"

class StatusComponent_ConditionManagment
{
public:

	// Initiates and return an OrganicCondition Struct to be used in a StatusComponent  
	static FOrganicCondition InitOrganicCondition(const EOrganicConditions OrganicCondition);
};
