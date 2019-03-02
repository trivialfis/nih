#ifndef C_LOGGING_H_
#define C_LOGGING_H_

#include "nih/C/nih.h"

NIH_API NIH_ErrCode nih_log(char const* msg, int verbosity);
NIH_API NIH_ErrCode nih_logDefault(char const* msg);

NIH_API NIH_ErrCode nih_setLogVerbosity(char const* msg);

#endif  // C_LOGGING_H_