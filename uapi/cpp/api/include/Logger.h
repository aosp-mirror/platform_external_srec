/*---------------------------------------------------------------------------*
 *  Logger.h                                                                 *
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

#ifndef __UAPI__LOGGER
#define __UAPI__LOGGER

#if defined (ANDROID)
#define LOG_TAG "Uapi"  
#include <utils/Log.h>
#endif
#include "ReturnCode.h"
#include "exports.h"
#include "SmartProxy.h"

#define UAPI_LOG_FUNCTION_SCOPE

namespace android
{
  namespace speech
  {
    namespace recognition
    {
#ifdef UAPI_LOGGING_ENABLED

      class LoggerProxy;
      /**
       * Debug logging.
       *
       * <b>WARNING</b>: Loggers are hierarchical in nature. The parent logger
       * must not be deallocated before its children or a crash may result.
       * <b>Be extremely careful if you choose to allocate Logger instances on
       * the heap!</b>.
       */
      class Logger
      {
        public:
          enum UAPI_EXPORT LogLevel
          {
            /**
             * Does not log.
             */
            LEVEL_NONE,
            /**
             * Logs fatal issues. This level only logs ERROR.
             */
            LEVEL_ERROR,
            /**
             * Logs non-fatal issues. This level also logs ERROR.
             */
            LEVEL_WARN,
            /**
             * Logs debugging information, such as the values of variables.
             * This level also logs ERROR, WARN.
             */
            LEVEL_INFO,
            /**
             * Logs when loggers are created or destroyed. This level also logs
             * INFO, WARN, ERROR.
             */
            LEVEL_TRACE
          };

          /**
           * Returns the logger instance, creating one if necessary.
           *
           * @param returnCode returns SUCCESS unless a fatal error occurs returns SUCCESS
           * @return the singleton instance
           */
          UAPI_EXPORT static LoggerProxy getInstance(ReturnCode::Type& returnCode);

          /**
           * Returns the logger instance, if one already exists.
           *
           * @return the singleton instance. Can return 0 if the logger could not be created.
           */
          UAPI_EXPORT static Logger* getExistingInstance();

          /**
           * Sets the logging level.
           *
           * @param level the logging level
           * @param returnCode the return code
           * @see Logger#LogLevel
           */
          virtual void setLoggingLevel(LogLevel level, ReturnCode::Type& returnCode) = 0;

          /**
           * returns the current logging level
           *
           * @return the logging level of the logger.
           */
          virtual LogLevel getLoggingLevel() const = 0;

          /**
           * Sets the path that will contain the logs.
           *
           * @param path the path of the log file
           * @param returnCode SUCCESS unless a fatal error occurs.
           * ILLEGAL_ARGUMENT if path is null or empty.
           * INVALID_STATE if file handle is null.
           * FILE_NOT_FOUND if could not open the path.
           */
          virtual void setPath(const char* path, ReturnCode::Type& returnCode) = 0;

          /**
           * Logs a message using the LEVEL_ERROR logging level.
           *
           * @param format format of variable arguments that follow
           */
          virtual void error(const char* fn, const char* lpszFormat, ...) = 0;

          /**
           * Logs a message using the LEVEL_WARN logging level.
           *
           * @param format format of variable arguments that follow
           */
          virtual void warn(const char* fn, const char* lpszFormat, ...) = 0;

          /**
           * Logs a message using the LEVEL_INFO logging level.
           *
           * @param format format of variable arguments that follow
           */
          virtual void info(const char* fn, const char* lpszFormat, ...) = 0;

          /**
           * Logs a message using the LEVEL_TRACE logging level.
           *
           * @param format format of variable arguments that follow
           */
          virtual void trace(const char* fn, const char* lpszFormat, ...) = 0;
        protected:
          /**
           * Prevent construction.
           */
          Logger();
          /**
           * Prevent destruction.
           */
          virtual ~Logger();
        private:


          friend class SmartProxy;
          friend class LoggerProxy;
      };



      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, LoggerProxy, SmartProxy, Logger)
#else
        inline void UAPILog(const char *, const char* , ...)
        {
        }
				class Logger
				{
        public:
						enum UAPI_EXPORT LogLevel
						{
							LEVEL_NONE,
							LEVEL_ERROR,
							LEVEL_WARN,
							LEVEL_INFO,
							LEVEL_TRACE
						};
				};
#endif
    }
  }
}
      /**
       * call this macro if something should be logger as an error. It is usually not
       * fatal but in most cases the current operation has to be aborted because
       * there are no easy way to recover.
       */
#ifdef UAPI_LOGGING_ENABLED
#define UAPI_ERROR(...) { \
  if(android::speech::recognition::Logger::getExistingInstance()) \
    android::speech::recognition::Logger::getExistingInstance()->error(__VA_ARGS__); \
}
#else
//# define UAPI_ERROR(...) { if (false) UAPILog(__VA_ARGS__); }
#define UAPI_ERROR(fn, ...) { LOGE(__VA_ARGS__); }
#endif

      /**
       * call this macro if something should be logger as a warning. You have a
       * warning when something unexpected occurs but the overall execution of the
       * program should not be affected.
       */
#ifdef UAPI_LOGGING_ENABLED
#define UAPI_WARN(...) { \
  if(android::speech::recognition::Logger::getExistingInstance()) \
    android::speech::recognition::Logger::getExistingInstance()->warn(__VA_ARGS__); \
}
#else
//# define UAPI_WARN(...) { if (false) UAPILog(__VA_ARGS__); }
#define UAPI_WARN(fn, ...) { LOGW(__VA_ARGS__); }
#endif

      /**
       * call this macro if something should be logger as an information to the user.
       * Anything that is relevant can be logger as INFO.
       */
#ifdef UAPI_LOGGING_ENABLED
#define UAPI_INFO(...) { \
  if(android::speech::recognition::Logger::getExistingInstance()) \
    android::speech::recognition::Logger::getExistingInstance()->info(__VA_ARGS__); \
}
#else
//# define UAPI_INFO(...) { if (false) UAPILog(__VA_ARGS__); }
#define UAPI_INFO(fn, ...) { LOGV(__VA_ARGS__); }
#endif

      /**
       * call this macro if you want to be able to trace the code path.
       */
#ifdef UAPI_LOGGING_ENABLED
#define UAPI_TRACE(...) { \
  if(android::speech::recognition::Logger::getExistingInstance()) \
    android::speech::recognition::Logger::getExistingInstance()->trace(__VA_ARGS__); \
}
#else
//# define UAPI_TRACE(...) { if (false) UAPILog(__VA_ARGS__); }
#define UAPI_TRACE(fn, ...) { LOGV(__VA_ARGS__); }
#endif


# define UAPI_FN_NAME(name) const char* const fn = name;

#if defined(UAPI_LOGGING_ENABLED) && defined(UAPI_LOG_FUNCTION_SCOPE)
/** 
 * Create UAPI_TRACE objects on the stack to log when a scope is entered and
 * exited.
 */
namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class TraceScope
      {
        public:
          TraceScope(const char* fn)
          : functionName(fn)
          { UAPI_TRACE(functionName, "ENTER\n"); }
          ~TraceScope()
          { UAPI_TRACE(functionName, "EXIT\n"); }

        private:
          const char * functionName;
      };
    }
  }
}
# define UAPI_FN_SCOPE(name) const char* const fn = name; \
												android::speech::recognition::TraceScope traceScope(fn);
#else
inline void RemoveCompilerWarning(const char *) {}
# define UAPI_FN_SCOPE(name) const char* const fn = name;  RemoveCompilerWarning(fn);
#endif


#endif
