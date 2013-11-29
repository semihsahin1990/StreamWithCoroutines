#pragma once

#include "streamc/runtime/RuntimeLogger.h"

#define SC_APPLOG(lvl, body) \
  BOOST_LOG_SEV(RuntimeLogger::getApplicationInternalLogger(), lvl) << \
      __BASE_FILE__ << ":" << __LINE__ << " - " << body;
