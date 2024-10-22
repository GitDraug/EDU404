#pragma once

//-------------------------------------------------------
// Flow Log MACRO, only executed in debug mode.
//-------------------------------------------------------
#if UE_BUILD_DEVELOPMENT

  // Normal program flow, no message
    #define FLOW_LOG if(HasAuthority()){ UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("Server: %s::%hs"), *GetName(), __FUNCTION__); } \
                                  else { UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("Client: %s::%hs"), *GetName(), __FUNCTION__); }


  // Warning with a message
    #define FLOW_LOG_WARNING(Message) if(HasAuthority()){ UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Server: %s::%hs - %s"), *GetName(), __FUNCTION__, TEXT(Message)); } \
                                                   else { UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Client: %s::%hs - %s"), *GetName(), __FUNCTION__, TEXT(Message)); }

  // Error with a message
    #define FLOW_LOG_ERROR(Message) if(HasAuthority()){ UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("Server: %s::%hs - %s"), *GetName(), __FUNCTION__, TEXT(Message)); } \
                                                 else { UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("Client: %s::%hs - %s"), *GetName(), __FUNCTION__, TEXT(Message)); }

//-------------------------------------------------------
// Used to track when a Tick starts
//-------------------------------------------------------
  #define FLOW_LOG_TICK if(HasAuthority()){ UE_LOG(FLOWLOG_TICK_CATEGORY, Display, TEXT("Server: %s::%hs"), *GetName(), __FUNCTION__); } \
                                     else { UE_LOG(FLOWLOG_TICK_CATEGORY, Display, TEXT("Client: %s::%hs"), *GetName(), __FUNCTION__); } 
//-------------------------------------------------------
// OnScreen Messages, when running live
//-------------------------------------------------------
  #define FLOW_LOG_ONSCREEN_MESSAGE(Message) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, FString::Printf(TEXT("%s::%hs - %s"), *GetName(), __FUNCTION__, TEXT(Message))); };
  #define FLOW_LOG_ONSCREEN_WARNING(Message) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("%s::%hs - %s"), *GetName(), __FUNCTION__, TEXT(Message))); };
  #define FLOW_LOG_ONSCREEN_ERROR(Message) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%s::%hs - %s"), *GetName(), __FUNCTION__, TEXT(Message))); };

  #define FLOW_ONSCREEN_TOSTRING(Key, Color, Message) if(GEngine) { GEngine->AddOnScreenDebugMessage(Key, 60.f, FColor::Color, FString::Printf(TEXT("%s::%hs - %s"), *GetName(), __FUNCTION__, *Message.ToString() )); };

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