// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/DataAssets/EDU_CORE_ControllerInputDataAsset.h"
#include "EDU404_ControllerInputDataAsset.generated.h"

/*------------------------------------------------------------------------------
  This DataAsset holds the Data and functionality for the Controller.

  This is not a PrimaryDataAsset, so it's ALWAYS loaded, but with such
  a small Data Asset it really doesn't matter.
------------------------------------------------------------------------------*/
UCLASS()
class EDU404_API UEDU_404_ControllerInputDataAsset : public UEDU_CORE_ControllerInputDataAsset
{
	GENERATED_BODY()
};