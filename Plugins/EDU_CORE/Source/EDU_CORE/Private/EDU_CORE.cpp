// Copyright Epic Games, Inc. All Rights Reserved.

#include "EDU_CORE.h"

#define LOCTEXT_NAMESPACE "FEDU_COREModule"

void FEDU_COREModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FEDU_COREModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEDU_COREModule, EDU_CORE)