/**
 * @file Global.h 
 * @author your name (you@domain.com)
 * @brief this file contains the basic macro of the logger
 * @version 0.1
 * @date 2023-03-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "Loggers.h"

#ifndef Global_h
#define Global_h

using namespace LogCPP;

#define INFO_ASYN(msg)      AsynchronizedLogger::GetInstance().Log(INFO,      msg);
#define DEBUG_ASYN(msg)     AsynchronizedLogger::GetInstance().Log(DEBUG,     msg);
#define WARNING_ASYN(msg)   AsynchronizedLogger::GetInstance().Log(WARNING,   msg);
#define ERROR_ASYN(msg)     AsynchronizedLogger::GetInstance().Log(ERROR,     msg);
#define FATAL_ASYN(msg)     AsynchronizedLogger::GetInstance().Log(FATAL,     msg);
#define INFO_SYN(msg)       SynchronizedLogger::GetInstance().Log(INFO,      msg);
#define DEBUG_SYN(msg)      SynchronizedLogger::GetInstance().Log(DEBUG,     msg);
#define WARNING_SYN(msg)    SynchronizedLogger::GetInstance().Log(WARNING,   msg);
#define ERROR_SYN(msg)      SynchronizedLogger::GetInstance().Log(ERROR,     msg);
#define FATAL_SYN(msg)      SynchronizedLogger::GetInstance().Log(FATAL,     msg);
#define CLEAN_UP            SynchronizedLogger::GetInstance().SafeDelete(); \
                            AsynchronizedLogger::GetInstance().SafeDelete();

#endif
