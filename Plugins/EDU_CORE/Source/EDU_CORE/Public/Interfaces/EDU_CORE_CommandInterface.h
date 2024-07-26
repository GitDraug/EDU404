// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EDU_CORE_CommandInterface.generated.h"

class AEDU_CORE_PlayerController;
class AEDU_CORE_C2_Camera;
/*------------------------------------------------------------------------------
  <!> Make sure both U- and I-classes in an interface implement any Plugin API,
  else they will cause linking errors when imported into other plugins.

  The U-class integrates with Unreal’s reflection system. The I-class defines
  the actual interface functionality.
------------------------------------------------------------------------------*/
UINTERFACE()
class EDU_CORE_API UEDU_CORE_CommandInterface : public UInterface
{
	GENERATED_BODY()
};

/*------------------------------------------------------------------------------
  In Unreal Engine, an interface class allows you to define a contract that
  other classes can implement. Unlike traditional inheritance, where a class
  inherits both behavior and data from its parent class, an interface only
  specifies behavior (i.e., methods) that implementing classes must provide.

  Interfaces are particularly useful for ensuring that different classes share
  common functionality without enforcing a strict class hierarchy.

  The suffix = 0 means pure Abstract declaration, and forces all classes that
  inherit this interface to provide a definition.
------------------------------------------------------------------------------*/
class EDU_CORE_API IEDU_CORE_CommandInterface
{
	GENERATED_BODY()

public:
	virtual void Command_NavigateTo(AActor* Owner, const FVector& Position, const FRotator& Rotation, bool Que = false, float Delay = 0) = 0;
};
