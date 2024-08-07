// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_AbstractEntity.generated.h"

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  AbstractEntity makes use of ParallelTick, a ticking function connected to
  the GameMode that runs an Async Tick. It has no other function.

  Used for elements that come in large scale and require tick,
  such as bullets or shrapnel.
------------------------------------------------------------------------------*/

UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_AbstractEntity : public AActor
{
	GENERATED_BODY()
//------------------------------------------------------------------------------
// Init & Object lifetime management
//------------------------------------------------------------------------------
public:
	virtual void BeginPlay() override;
	
	// Called every frame by GameMode (Only exists on the Server)
	virtual void ServerTick(float DeltaTime);

	// Called every frame by LocalController (Only exists on the Client)
	virtual void ClientTick(float DeltaTime);

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
public:
	UPROPERTY(EditAnywhere,
		Category = "Aggregated Tick",
		meta = (DisplayName = "Aggregated Server Tick",
		ToolTip = "Used for batch processing of large scale abstract entities, like bullets, independent of Physics Tick."))
	bool bServerTickEnabled = false;

	UPROPERTY(EditAnywhere,
		Category = "Aggregated Tick",
		meta = (DisplayName = "Aggregated Client Tick",
		ToolTip = "Used for batch processing of large scale abstract entities, like bullets, independent of Physics Tick"))
	bool bClientTickEnabled = false;
	
};
