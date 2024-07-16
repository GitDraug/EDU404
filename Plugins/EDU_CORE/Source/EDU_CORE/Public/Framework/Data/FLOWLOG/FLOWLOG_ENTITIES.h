#pragma once

/*--------------------------------------------------------------------
  Custom Log category, so we can monitor program flow in the console.
  The Macro makes the source less cluttered.

  For Category name, only these 3 need ot be changed
---------------------------------------------------------------------*/

//Declare External Log Category (Category name, Default verbosity, Maximum verbosity level)
DECLARE_LOG_CATEGORY_EXTERN(FlowLog_ENTITIES, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(FlowLog_ENTITIES_TICK, Display, All);

//Define External Log Category
inline DEFINE_LOG_CATEGORY(FlowLog_ENTITIES);
inline DEFINE_LOG_CATEGORY(FlowLog_ENTITIES_TICK);

// Define Internal Macro for this .h file
#define FLOWLOG_CATEGORY FlowLog_ENTITIES
#define FLOWLOG_TICK_CATEGORY FlowLog_ENTITIES_TICK

//-------------------------------------------------------
// Flow Log MACRO, only executed in debug mode.
//-------------------------------------------------------
#if UE_BUILD_DEVELOPMENT

  // Normal program flow, no message
    #define FLOW_LOG UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs"), *GetClass()->GetName(), __FUNCTION__); // Custom Log category, so we can monitor program flow in the console. The Macro makes the source less cluttered.

  // Warning with a message
    #define FLOW_LOG_WARNING_TOSTRING(Message) UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, Message);
    #define FLOW_LOG_WARNING(Message) UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, TEXT(Message));

  // Error with a message
    #define FLOW_LOG_ERROR_TOSTRING(Message) UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, (Message)); 
    #define FLOW_LOG_ERROR(Message) UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, TEXT(Message)); 

//-------------------------------------------------------
// Used to track when a Tick starts
//-------------------------------------------------------
  #define FLOW_LOG_TICK UE_LOG(FLOWLOG_TICK_CATEGORY, Display, TEXT("%s::%hs"), *GetClass()->GetName(), __FUNCTION__);

//-------------------------------------------------------
// OnScreen Messages, when running live
//-------------------------------------------------------
  #define FLOW_LOG_ONSCREEN_MESSAGE(Message) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, FString::Printf(TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, TEXT(Message))); };
  #define FLOW_LOG_ONSCREEN_WARNING(Message) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, TEXT(Message))); };
  #define FLOW_LOG_ONSCREEN_ERROR(Message) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, TEXT(Message))); };

  #define FLOW_ONSCREEN_TOSTRING(Key, Color, Message) if(GEngine) { GEngine->AddOnScreenDebugMessage(Key, 60.f, FColor::Color, FString::Printf(TEXT("%s::%hs - %s"), *GetClass()->GetName(), __FUNCTION__, *Message.ToString() )); };

//-------------------------------------------------------
// If checks that only run in development
//-------------------------------------------------------
#define FLOW_LOG_IF(PARAM, ACTION) if(PARAM){ ACTION; }

#else // These are all undefined in a release build, essentially non-existent.
  #define FLOW_LOG

  #define FLOW_LOG_WARNING_TOSTRING
  #define FLOW_LOG_WARNING(Message)

  #define FLOW_LOG_ERROR_TOSTRING(Message)
  #define FLOW_LOG_ERROR(Message)

  #define FLOW_LOG_TICK 
  #define FLOW_LOG_TICK_VARIABLE_FOR_DEBUG

  #define FLOW_LOG_ONSCREEN_MESSAGE
  #define FLOW_LOG_ONSCREEN_WARNING
  #define FLOW_LOG_ONSCREEN_ERROR
  #define FLOW_ONSCREEN_TOSTRING

  #define FLOW_LOG_IF
#endif
