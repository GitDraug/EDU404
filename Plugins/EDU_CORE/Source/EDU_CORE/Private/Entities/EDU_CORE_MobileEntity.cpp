// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/EDU_CORE_MobileEntity.h"


// Sets default values
AMobileEntity::AMobileEntity()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AMobileEntity::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMobileEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

