/*---------------------------------------------------------------------------*
 *  LoggerImpl.cpp                                                           *
 *                                                                           *
 *  Copyright 2007, 2008 Nuance Communciations, Inc.                               *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the 'License');          *
 *  you may not use this file except in compliance with the License.         *
 *                                                                           *
 *  You may obtain a copy of the License at                                  *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an 'AS IS' BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *---------------------------------------------------------------------------*/


//Memory leak detection
#if defined(_DEBUG) && defined(_WIN32)
#include "crtdbg.h"
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif
#include "LoggerImpl.h"
#include "System.h"

#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>

#if defined(UAPI_WIN32)
#include <sys/timeb.h>
#elif defined(UAPI_LINUX)
#include <inttypes.h>
#include <sys/time.h>
#endif
#include "Runnable.h"
#include "Mutex.h"
#include "ThreadLocal.h"
#include "FileASCII.h"


#if defined(ANDROID)
extern "C"
{
void add_common_log_info ( char *log_buffer );
}
#endif

using namespace android::speech::recognition;
using namespace android::speech::recognition::impl;
using namespace android::speech::recognition::utilities;


static const size_t BUFSIZE = 1024;
static const size_t BUFHDRSIZE = 8;

#ifdef UAPI_LOGGING_ENABLED


DEFINE_SMARTPROXY(impl, LoggerImplProxy, LoggerProxy, LoggerImpl)
Mutex* LoggerImpl::mutex = 0;
LoggerImpl::ComponentInitializer LoggerImpl::componentInitializer;
LoggerImpl * LoggerImpl::instance = 0;

LoggerImpl::ComponentInitializer::ComponentInitializer()
{
  mutex = Mutex::create(false, returnCode);
  if (returnCode)
  {
    fprintf(stderr, "LoggerImpl::ComponentInitializer::ComponentInitializer() - Could not create LoggerImpl::mutex\n");
    return;
  }

  LoggerImpl::instance = LoggerImpl::create( returnCode );
  if (returnCode != ReturnCode::SUCCESS)
  {
    fprintf(stderr, "LoggerImpl::ComponentInitializer::ComponentInitializer() - Could not create Logger, error %d\n", returnCode);
    delete mutex;
    mutex = 0;
    LoggerImpl::instance = 0;
    return;
  }
    
  LoggerProxy result(LoggerImpl::instance,false);
  if (!result)
  {
    fprintf(stderr, "LoggerImpl::ComponentInitializer::ComponentInitializer() - Could not create LoggerProxy\n");
    returnCode = ReturnCode::OUT_OF_MEMORY;
    delete mutex;
    mutex = 0;
    delete LoggerImpl::instance;
    LoggerImpl::instance = 0;
    return; 
  }
  LoggerImpl::instance->refCounter = result.getRoot();

  impl::LoggerImpl::componentInitializer.makeSureLoggerIsNeverDestroyedLoggerProxy = result;
}

LoggerImpl::ComponentInitializer::~ComponentInitializer()
{
  //get rid of the logger.
  impl::LoggerImpl::componentInitializer.makeSureLoggerIsNeverDestroyedLoggerProxy = LoggerProxy();

  if (mutex) 
      delete mutex;
  mutex = 0;
  returnCode = ReturnCode::UNKNOWN;
}

LoggerProxy Logger::getInstance(ReturnCode::Type& returnCode)
{
  if (impl::LoggerImpl::componentInitializer.returnCode)
  {
    returnCode = impl::LoggerImpl::componentInitializer.returnCode;
    return LoggerProxy();
  }

  //we have to protect the construction of "instance". Make sure we don't have
  //multiple threads calling getInstance() while "instance" is equal to 0.
  LockScope ls(LoggerImpl::mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    fprintf(stderr, "Logger::getInstance - Failed to create LockScope of static mutex.\n");
    return LoggerProxy();
  }
  
  //it's now safe to check if instance == 0
  if (LoggerImpl::instance == 0)
  {
    LoggerImpl::instance = LoggerImpl::create( returnCode );
    if (returnCode != ReturnCode::SUCCESS)
    {
      fprintf(stderr, "Logger::getInstance - Could not create Logger, error %d\n", returnCode);
      return LoggerProxy();
    }
    
    LoggerProxy result(LoggerImpl::instance,false);
    if (!result)
    {
      returnCode = ReturnCode::OUT_OF_MEMORY;
      delete LoggerImpl::instance;
      LoggerImpl::instance = 0;
      return LoggerProxy();
    }
    LoggerImpl::instance->refCounter = result.getRoot();

    impl::LoggerImpl::componentInitializer.makeSureLoggerIsNeverDestroyedLoggerProxy = result;
    return result;
  }
  returnCode = ReturnCode::SUCCESS;
  LoggerProxy result(LoggerImpl::instance->refCounter);
  if (!result)
    returnCode = ReturnCode::OUT_OF_MEMORY;
  return result;
}

Logger* Logger::getExistingInstance()
{
  return LoggerImpl::instance;
}

LoggerImpl::~LoggerImpl()
{
  ReturnCode::Type rc;
  file.close(rc);
  LoggerImpl::instance = 0;
}

LoggerImpl * LoggerImpl::create(ReturnCode::Type & returnCode)
{
  LoggerImpl * result = 0;

#if defined(ANDROID)//Q device
    //result = new LoggerImpl("/tmp/uapi.log", Logger::LEVEL_WARN, returnCode);
    result = new LoggerImpl(NULL, Logger::LEVEL_WARN, returnCode);
#elif defined(UAPI_WIN32) || defined(UAPI_LINUX)
    result = new LoggerImpl("uapi.log", Logger::LEVEL_WARN, returnCode);
#endif
    if (result == 0)
    {
      fprintf(stderr, "Logger::getInstance - Could not create Logger, out of memory!!!\n");
      returnCode = ReturnCode::OUT_OF_MEMORY;
      return 0;
    }
    if (returnCode != ReturnCode::SUCCESS)
    {
      fprintf(stderr, "Logger::getInstance - Could not create Logger, error %d\n", returnCode);
      delete result;
      return 0;
    }

    return result;
}


LoggerImpl::LoggerImpl(const char* path, Logger::LogLevel level,
                       ReturnCode::Type& returnCode):
    loggingLevel(level),
    refCounter(0)
{
  internal_setPath(path, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    fprintf(stderr, "LoggerImpl::LoggerImpl - unable to create log file.\n");
    return;
  }
}

void LoggerImpl::internal_setPath(const char* path, ReturnCode::Type& returnCode)
{
  if (file.isOpened())
    file.close(returnCode);
    
  if (path) {
    file.open(path, "a+", returnCode);
    if (returnCode)
    {
      fprintf(stderr, "LoggerImpl::setPath - Failed to create file %s reason %d\n", path, returnCode);
      return;
    }
  }
  returnCode = ReturnCode::SUCCESS;

}

void LoggerImpl::setPath(const char* path, ReturnCode::Type& returnCode)
{
  LockScope ls(LoggerImpl::mutex, returnCode);
  if (returnCode)
    return;
  internal_setPath(path, returnCode);
}

void LoggerImpl::setLoggingLevel(Logger::LogLevel level, ReturnCode::Type & returnCode)
{
  LockScope ls(LoggerImpl::mutex, returnCode);
  if (returnCode)
    return;
  loggingLevel = level;
  returnCode = ReturnCode::SUCCESS;
}

Logger::LogLevel LoggerImpl::getLoggingLevel() const
{
  return loggingLevel;
}

void LoggerImpl::error(const char * fn, const char* format, ...)
{
  if (loggingLevel >= LEVEL_ERROR)
  {
    size_t length;
    va_list args;
    va_start(args, format);
    char* buf = (char*) malloc(BUFSIZE);
    if (!buf)
    {
      fprintf(stderr, "LoggerImpl::error() could not allocate buf\n");
      return;
    }
    
    //start with ERROR
#if defined(UAPI_WIN32)
    _snprintf(buf, BUFSIZE, "ERROR - %s : ", fn);
#elif defined (ANDROID)
    add_common_log_info ( buf );
    length = strlen(buf);
    snprintf(buf + length, BUFSIZE - length, "%s : ", fn);
#else
    snprintf(buf, BUFSIZE, "ERROR - %s : ", fn);
#endif
    length = strlen(buf);
    
#if defined(UAPI_WIN32)
    _vsnprintf(buf + length, BUFSIZE - length, format, args);
#else
    vsnprintf(buf + length, BUFSIZE - length, format, args);
#endif
#if defined (ANDROID)
    LOGE ( buf );
#else
    log(buf);
#endif
    free(buf);
    va_end(args);
  }
}

void LoggerImpl::warn(const char * fn, const char* format, ...)
{
  if (loggingLevel >= LEVEL_WARN)
  {
    size_t length;
    va_list args;
    va_start(args, format);
    char* buf = (char*) malloc(sizeof(char) * BUFSIZE);
    if (!buf)
    {
      fprintf(stderr, "LoggerImpl::warn() could not allocate buf\n");
      return;
    }
    
    //start with WARN
#if defined(UAPI_WIN32)
    _snprintf(buf, BUFSIZE, "WARN  - %s : ", fn);
#elif defined (ANDROID)
    add_common_log_info ( buf );
    length = strlen(buf);
    snprintf(buf + length, BUFSIZE - length, "%s : ", fn);
#else
    snprintf(buf, BUFSIZE, "WARN  - %s : ", fn);
#endif
    length = strlen(buf);
    
#if defined(UAPI_WIN32)
    _vsnprintf(buf + length, BUFSIZE - length, format, args);
#else
    vsnprintf(buf + length, BUFSIZE - length, format, args);
#endif
#if defined (ANDROID)
    LOGW ( buf );
#else
    log(buf);
#endif
    free(buf);
    va_end(args);
  }
}

void LoggerImpl::info(const char * fn, const char* format, ...)
{
  if (loggingLevel >= LEVEL_INFO)
  {
    size_t length;
    va_list args;
    va_start(args, format);
    char* buf = (char*) malloc(sizeof(char) * BUFSIZE);
    if (!buf)
    {
      fprintf(stderr, "LoggerImpl::info() could not allocate buf\n");
      return;
    }
    
    //start with INFO
#if defined(UAPI_WIN32)
    _snprintf(buf, BUFSIZE, "INFO  - %s : ", fn);
#elif defined (ANDROID)
    add_common_log_info ( buf );
    length = strlen(buf);
    snprintf(buf + length, BUFSIZE - length, "%s : ", fn);
#else
    snprintf(buf, BUFSIZE, "INFO  - %s : ", fn);
#endif
    length = strlen(buf);
    
#if defined(UAPI_WIN32)
    _vsnprintf(buf + length, BUFSIZE - length, format, args);
#else
    vsnprintf(buf + length, BUFSIZE - length, format, args);
#endif
#if defined (ANDROID)
    LOGI ( buf );
#else
    log(buf);
#endif
    free(buf);
    va_end(args);
  }
}

void LoggerImpl::trace(const char * fn, const char* format, ...)
{
  if (loggingLevel >= LEVEL_TRACE)
  {
    size_t length;
    va_list args;
    va_start(args, format);
    char* buf = (char*) malloc(sizeof(char) * BUFSIZE);
    if (!buf)
    {
      fprintf(stderr, "LoggerImpl::trace() could not allocate buf\n");
      return;
    }
    
    //start with TRACE
#if defined(UAPI_WIN32)
    _snprintf(buf, BUFSIZE, "TRACE - %s : ", fn);
#elif defined (ANDROID)
    add_common_log_info ( buf );
    length = strlen(buf);
    snprintf(buf + length, BUFSIZE - length, "%s : ", fn);
#else
    snprintf(buf, BUFSIZE, "TRACE - %s : ", fn);
#endif
    length = strlen(buf);
    
#if defined(UAPI_WIN32)
    _vsnprintf(buf + length, BUFSIZE - length, format, args);
#else
    vsnprintf(buf + length, BUFSIZE - length, format, args);
#endif
#if defined (ANDROID)
    LOGD ( buf );
#else
    log(buf);
#endif
    free(buf);
    va_end(args);
  }
}


void LoggerImpl::log(const char* toLog)
{
#ifdef UAPI_WIN32
  // get current time
  struct timeb timebuf;
  ftime(&timebuf);
#elif defined(ANDROID)
return;
#elif defined(UAPI_LINUX)
  // struct timeb isn't available on some Linux systems, so there is no milliseconds field
  struct timeval time_now;
  gettimeofday ( &time_now, NULL );
#endif
  
  struct tm *ptm;

#ifdef UAPI_WIN32
  ptm = localtime(&timebuf.time);
#elif defined(ANDROID)
  ;
#elif defined(UAPI_LINUX)
  ptm = localtime ( (const time_t *)&time_now.tv_sec );	// This conversion is safe for a few decades. SteveR
#endif
  
  // now you have the year, month, day etc.
  char date[30];
  
  ::strftime(date, 30, "%Y/%m/%d %H:%M:%S", ptm);
  
  char pszMsec[10];
#ifdef UAPI_WIN32
  _snprintf(pszMsec, 6, ".%03u ", timebuf.millitm);
#elif defined(ANDROID)
  ;
#elif defined(UAPI_LINUX)
  snprintf ( pszMsec, 8, ".%06ld ", time_now.tv_usec );
#endif
  
  char threadId[20];
  ReturnCode::Type returnCode;
  Runnable::getCurrentThreadId(threadId, 20, returnCode);
  if (returnCode)
  {
    fprintf(stderr, "LoggerImpl::log - Runnable::getcurrentThreadId() failed: %s\n",
            ReturnCode::toString(returnCode));
    return;
  }
  
  char* finalString = new char[strlen(date) + strlen(pszMsec) + strlen(threadId) + strlen(" ") + strlen(toLog) + 1];
  
  strcpy(finalString, "");
  strcat(finalString, date);
  strcat(finalString, pszMsec);
  strcat(finalString, threadId);
  strcat(finalString, " ");
  strcat(finalString, toLog);
 
  {
    LockScope ls(LoggerImpl::mutex, returnCode);
    if (returnCode)
    {
      fprintf(stderr, "LoggerImpl::log - Failed to lock mutex\n");
      delete[] finalString;
      return;
    }

    if (!file.isOpened())
    {
      if (returnCode)
        fprintf(stderr, "LoggerImpl::log - unlock() failed: %s\n", ReturnCode::toString(returnCode));
      delete[] finalString;
      return;
    }
    
    UINT32 length = (UINT32) strlen(finalString);
    file.write(finalString, sizeof(char), length, returnCode);
    if (returnCode)
    {
      fprintf(stderr, "LoggerImpl::log - Could not write log into a file: %s\n",
              ReturnCode::toString(returnCode));
    }
    file.flush();
  }
  delete[] finalString;
}
#endif


#if defined(ANDROID)
extern "C"
{
void add_common_log_info ( char *log_buffer )
{
  ReturnCode::Type returnCode;
  struct timeval time_now;
  struct tm *ptm;
  char date[30];
  char pszMsec[10];
  char threadId[20];

// Google has its own time stamp mechanism. Just commented out in case its
// useful in the future. SteveR
//  gettimeofday ( &time_now, NULL );
//  ptm = localtime ( (const time_t *)&time_now.tv_sec );	// This conversion is safe for a few decades. SteveR
  
  // now you have the year, month, day etc.
//  ::strftime(date, 30, "%Y/%m/%d %H:%M:%S", ptm);
//  snprintf ( pszMsec, 8, ".%06ld ", time_now.tv_usec );
  
  Runnable::getCurrentThreadId(threadId, 20, returnCode);
  if (returnCode)
  {
    fprintf(stderr, "LoggerImpl::log - Runnable::getcurrentThreadId() failed: %s\n",
            ReturnCode::toString(returnCode));
    return;
  }
//  strcpy ( log_buffer, date );
//  strcat ( log_buffer, pszMsec );
  strcpy ( log_buffer, threadId );
  strcat ( log_buffer, " " );
}
}
#endif
