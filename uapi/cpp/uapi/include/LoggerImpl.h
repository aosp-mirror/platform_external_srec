/*---------------------------------------------------------------------------*
 *  LoggerImpl.h                                                             *
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

#ifndef _UAPI_LOGGERIMPL_H_
#define _UAPI_LOGGERIMPL_H_

#include "Logger.h"
#include "ReturnCode.h"
#include "SmartProxy.h"
#include "FileASCII.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class ThreadLocal;
        class FileASCII;
        class Mutex;
        class RefCounter;
      }
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace impl
      {
#ifdef UAPI_LOGGING_ENABLED
        class LoggerImplProxy;

        /**
         * Debug logging.
         */
        class LoggerImpl: public Logger
        {
          public:
            /**
             * destructor
             */
            virtual ~LoggerImpl();

            /**
             * Sets the logging level.
             *
             * @param level logging level. If it's set to LEVEL_INFO, it will log,
             * ERRORs WARNs and INFO.
             */
            virtual void setLoggingLevel(Logger::LogLevel level, ReturnCode::Type & returnCode);

						/**
						 * returns the current logging level
						 *
						 * @return the logging level of the logger.
						 */
            virtual LogLevel getLoggingLevel() const;

            /**
             * Sets the path that will contain the logs.
             *
             * @param path the path of the log file
             * @param returnCode the return code.
             */
            virtual void setPath(const char* path, ReturnCode::Type& returnCode);

            /**
             * Method called by the UAPI_ERROR macro used to do logging. It takes
             * variable argument list as a parameter.
             *
             * @param fn the function name
             * @param lpszFormat variable argument list of things to log.
             */
            virtual void error(const char* fn, const char* lpszFormat, ...);

            /**
             * Method called by the UAPI_WARN macro used to do logging. It takes
             * variable argument list as a parameter.
             *
             * @param fn the function name
             * @param lpszFormat variable argument list of things to log.
             */
            virtual void warn(const char* fn, const char* lpszFormat, ...);

            /**
             * Method called by the UAPI_INFO macro used to do logging. It takes
             * variable argument list as a parameter.
             *
             * @param fn the function name
             * @param lpszFormat variable argument list of things to log.
             */
            virtual void info(const char* fn, const char* lpszFormat, ...);

            /**
             * Method called by the UAPI_TRACE macro used to do logging. It takes
             * variable argument list as a parameter.
             *
             * @param fn the function name
             * @param lpszFormat variable argument list of things to log.
             */
            virtual void trace(const char* fn, const char* lpszFormat, ...);


          private:
            /**
             * Contructor
             *
             * @param path path of the log file
             * @param level logging level. If it's set to LEVEL_INFO, it will log,
             * ERRORs WARNs and INFO.
             * @param returnCode the return code.
             */
            LoggerImpl(const char* path, Logger::LogLevel level, ReturnCode::Type& returnCode);

            /**
             * helper function used to create an instance of this class
             */
            static LoggerImpl * create(ReturnCode::Type & returnCode );

	    /**
	     * Internal fuction used to set the path without locking the mutex
	     */
            void internal_setPath(const char* path, ReturnCode::Type& returnCode);

            /**
             * Initializes the component when the library is loaded.
             */
            class ComponentInitializer
            {
              public:
                ComponentInitializer();
                ~ComponentInitializer();
                
                ReturnCode::Type returnCode;
                LoggerProxy      makeSureLoggerIsNeverDestroyedLoggerProxy;
            };

            /**
             * Method that actually writes into the file
             *
             * @param toLog actual string that we want to save into a file.
             */
            void log(const char* toLog);

            /**
             * Variable that contains the current logging level
             */
            Logger::LogLevel loggingLevel;

            /**
             * file in which the logs will be saved.
             */
            utilities::FileASCII file;
            SmartProxy::Root* refCounter;

            static ComponentInitializer componentInitializer;

            static utilities::Mutex * mutex;

            static LoggerImpl * instance;

            friend class Logger;
            friend class LoggerImplProxy;
            friend class ComponentInitializer;
        };

        /**
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(UAPI_EXPORT, LoggerImplProxy, LoggerProxy, LoggerImpl)

#else
        class LoggerImpl : public Logger
        {
          public:
            LoggerImpl();
        };
#endif
      }
    }
  }
}

#endif
