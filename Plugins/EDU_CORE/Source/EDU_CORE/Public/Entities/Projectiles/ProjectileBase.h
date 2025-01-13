// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

/*------------------------------------------------------------------------------
  Projectile Archetype
--------------------------------------------------------------------------------
  Acts as an archetype for any kind of projectile, it's not supposed to be
  used in this form.

  TraceProjectiles are spawned from the Projectile Pool Manager. They are
  performance-centric, acting only as a container for a lincetrace with some
  effects.
  
  More advanced projectiles, such as rockets and missiles, have meshes and
  their own movement set.
------------------------------------------------------------------------------*/

UCLASS(Abstract)
class EDU_CORE_API AProjectileBase : public AActor
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

public:
	AProjectileBase();

protected:
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Editable Data: General
//------------------------------------------------------------------------------

	// the Actor resposible for sending this projectile
	UPROPERTY()
	TObjectPtr<AActor> Sender = nullptr;

	// Measures how long this projectile has been active in AsyncTime.
	float LifeTime = 0.0f;

	// Maximum number of AsyncSeconds before this projectile self-destructs.
	float MaxLifeTime = 0.0f;

};
