#pragma once

/*--------------------------------------------------------------------
  Custom Log category, so we can monitor program flow in the console.
  The Macro makes the source less cluttered.

  For Category name, only these 3 need ot be changed
---------------------------------------------------------------------*/

//Declare External Log Category (Category name, Default verbosity, Maximum verbosity level)
DECLARE_LOG_CATEGORY_EXTERN(FlowLog_PLAYER, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(FlowLog_PLAYER_TICK, Display, All);

//Define External Log Category
inline DEFINE_LOG_CATEGORY(FlowLog_PLAYER);
inline DEFINE_LOG_CATEGORY(FlowLog_PLAYER_TICK);

// Define Internal Macro for this .h file
#define FLOWLOG_CATEGORY FlowLog_PLAYER
#define FLOWLOG_TICK_CATEGORY FlowLog_PLAYER_TICK

//-------------------------------------------------------
// Flow Log MACRO, only executed in debug mode.
//-------------------------------------------------------

#include "FLOWLOG.h"