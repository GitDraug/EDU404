#pragma once

/*--------------------------------------------------------------------
  Custom Log category, so we can monitor program flow in the console.
  The Macro makes the source less cluttered.

  For Category name, only these 3 need ot be changed
---------------------------------------------------------------------*/

//Declare External Log Category (Category name, Default verbosity, Maximum verbosity level)
DECLARE_LOG_CATEGORY_EXTERN(FlowLog_PLAYER, Display, All);

//Define External Log Category
inline DEFINE_LOG_CATEGORY(FlowLog_PLAYER);

// Define Internal Macro for this .h file
#define FLOWLOG_CATEGORY FlowLog_PLAYER

//-------------------------------------------------------
// Flow Log MACRO, only executed in debug mode.
//-------------------------------------------------------
#ifdef NDEBUG

  // Normal program flow, no message
    #define FLOW_LOG UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs"), *GetClass()->GetName(), __FUNCTION__); //   Custom Log category, so we can monitor program flow in the console. The Macro makes the source less cluttered.

  // Warning with messsage
    #define FLOW_LOG_WARNING_TOSTRING(Message) UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, Message);
    #define FLOW_LOG_WARNING(Message) UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, TEXT(Message));

  // Error with messsage
    #define FLOW_LOG_ERROR_TOSTRING(Message) UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, (Message)); 
    #define FLOW_LOG_ERROR(Message) UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, TEXT(Message)); 

//-------------------------------------------------------
// Used to track when a Tick starts
//-------------------------------------------------------
  #define FLOW_LOG_TICK if(!bFlowLogTick) UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - First tick initiated"), *GetClass()->GetName(), __FUNCTION__); bFlowLogTick = 1; // this one needs the FLOW_LOG_TICK_VARIABLE_FOR_DEBUG macro in the .h file to work.
 // #define FLOW_LOG UE_LOG(FlowLog, Display, TEXT("%s::%hs"), *GetPathNameSafe(GetClass()), __FUNCTION__); // this one-track filePath in the editor content explorer


  // Variable for .h File
  #define FLOW_LOG_TICK_VARIABLE_FOR_DEBUG bool bFlowLogTick; // for the FLOW_LOG_TICK macro.

//-------------------------------------------------------
// OnScreen Messages, when running live
//-------------------------------------------------------
  #define FLOW_LOG_ONSCREEN(Key, Color) if(GEngine) { GEngine->AddOnScreenDebugMessage(Key, 60.f, FColor::Color, FString::Printf(TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__)); };
  #define FLOW_ONSCREEN_TOSTRING(Key, Color, Message) if(GEngine) { GEngine->AddOnScreenDebugMessage(Key, 60.f, FColor::Color, FString::Printf(TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, *Message.ToString() )); };
#endif
