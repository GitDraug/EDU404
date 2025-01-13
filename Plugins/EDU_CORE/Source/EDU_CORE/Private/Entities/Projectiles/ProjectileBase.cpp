// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Projectiles/ProjectileBase.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------


AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}
