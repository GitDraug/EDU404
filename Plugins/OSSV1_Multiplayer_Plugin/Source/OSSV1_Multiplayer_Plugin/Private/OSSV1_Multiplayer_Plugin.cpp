// Copyright Epic Games, Inc. All Rights Reserved.

#include "OSSV1_Multiplayer_Plugin.h"

#define LOCTEXT_NAMESPACE "FOSSV1_Multiplayer_PluginModule"

void FOSSV1_Multiplayer_PluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FOSSV1_Multiplayer_PluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOSSV1_Multiplayer_PluginModule, OSSV1_Multiplayer_Plugin)