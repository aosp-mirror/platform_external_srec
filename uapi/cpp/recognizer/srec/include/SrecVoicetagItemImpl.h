/*---------------------------------------------------------------------------*
 *  SrecVoicetagItemImpl.h                                                   *
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

#ifndef __UAPI__SRECVOICETAGITEMIMPL
#define __UAPI__SRECVOICETAGITEMIMPL

#include "exports.h"
#include "ReturnCode.h"
#include "VoicetagItemImpl.h"
#include "Task.h"
#include "VoicetagItemListener.h"

typedef struct SR_Nametag_t SR_Nametag;

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class File;
        class WorkerQueue;
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
      namespace srec
      {
        class SrecVoicetagItemImplProxy;
        /**
         * Voicetag that may be inserted into an embedded grammar slot.
         */
        class SrecVoicetagItemImpl: public android::speech::recognition::impl::VoicetagItemImpl
        {
          public:
            /**
             * Returns the underlying nametag.
             *
             * @return the underlying nametag
             */
           SREC_EXPORT SR_Nametag* getNametag();
           
           /**
           * (Optional operation) Returns the audio used to construct the Voicetag. The
           * audio is in PCM format and is start-pointed and end-pointed. The audio is
           * only generated if the enableGetWaveform recognition parameter is set
           * prior to recognition.
           *
           * @param waveform the read-only endpointed waveform
           * @param size the size of the waveform in bytes
           * @param returnCode the return code
           * @see RecognizerParameters.enableGetWaveform
           */
          SREC_EXPORT void getAudio(const INT16** waveform, ARRAY_LIMIT* size,
                                ReturnCode::Type& returnCode) const;
                                
          /**
           * (Optional operation) Sets the audio used to construct the Voicetag. The
           * audio is in PCM format and is start-pointed and end-pointed. The audio is
           * only generated if the enableGetWaveform recognition parameter is set
           * prior to recognition.
           *
           * @param waveform the endpointed waveform
           * @param size the size of the waveform in bytes
           * @param returnCode the return code
           * @see RecognizerParameters.enableGetWaveform
           */
          SREC_EXPORT void setAudio(const INT16* waveform, ARRAY_LIMIT size,
                                ReturnCode::Type& returnCode);
          /**
           * Save the Voicetag Item.
           *
           * @param the path where the Voicetag will be saved. We strongly recommend to set the filename with the same value of the VoicetagId. If the filename is ommited, then the VoicetagId (assigned upon creation) will be used as default. 
           * @param returnCode the return code
           */
           SREC_EXPORT void save(const char* path, ReturnCode::Type& returnCode);

          /**
           * Load a Voicetag Item.
           *
           * @param returnCode the return code
           */
          SREC_EXPORT void load(ReturnCode::Type& returnCode);
        
          /**
             * Creates a new VoicetagItemImpl from a file.
             *
             * @param filename the path of the voicetag file 
             * @param listener the listener object for the voicetag events
             * @param returnCode returns SUCCESS unless a fatal error occurs
             * @return a new voicetag item
             */
          SREC_EXPORT static SrecVoicetagItemImplProxy create(const char* filename,
                                                    VoicetagItemListenerProxy &listener,
                                                    ReturnCode::Type& returnCode);

           /**
             * Check if the getWaveform parameter is on
             * @return true if enabled; false otherwise
             */
          SREC_EXPORT static bool isEnableGetWaveformOn();

          private:
            /**
             * Prevent destruction.
             */
            virtual ~SrecVoicetagItemImpl();
            /**
             * Creates a new VoicetagItemImpl.
             *
             * @param result the recognition result used to construct the voicetag
             * @param waveform the audio used to generate the voicetag
             * @param size the length of the waveform in bytes
             * @param listener the listener object for the voicetag events
             * @param returnCode returns SUCCESS unless a fatal error occurs
             * @return a new voicetag item
             */
            static SrecVoicetagItemImplProxy create(SR_Nametag* nametag,
                                                    const INT16* waveform,
                                                    ARRAY_LIMIT size,
                                                    VoicetagItemListenerProxy &listener,
                                                    ReturnCode::Type& returnCode);
            /**
             * Prevent construction.
             */
            SrecVoicetagItemImpl(SR_Nametag* nametag, const INT16* _waveform,
                                 ARRAY_LIMIT size, VoicetagItemListenerProxy &listener);

            SrecVoicetagItemImpl(const char* filename, VoicetagItemListenerProxy &listener, ReturnCode::Type& returnCode);
      
            /**
             * Invoked by LoadVoicetagTask.
             */
            void onLoad();
            /**
             * Invoked by SaveVoicetagTask.
             */
            void onSave(const char* path);

             /**
             * The path to load the grammar from.
             */
            char* _filename;

            /**
             * The name tag
             */                     
            SR_Nametag* _nametag;
            
            /**
             * The listener for Voicetag events
             */
            VoicetagItemListenerProxy _listener;

            /**
             * File in which we will save the data
             */
            utilities::File* _file;
            /**
             * The thread and the queue used to process aysnc tasks.
             */
            utilities::WorkerQueue* _workerQueue;
            SmartProxy::Root* rootProxy;

            friend class SaveVoicetagTask;
            friend class LoadVoicetagTask;
            friend class SrecRecognitionResultImpl;
            friend class SrecVoicetagItemImplProxy;
        };
        
        /*
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(UAPI_EXPORT, SrecVoicetagItemImplProxy,
                           impl::VoicetagItemImplProxy,
                           SrecVoicetagItemImpl)

        class LoadVoicetagTask: public utilities::Task
        {
          public:
            LoadVoicetagTask(SrecVoicetagItemImplProxy& voicetag);
            virtual void run();
          private:
            SrecVoicetagItemImplProxy voicetag;
        };
        
        class SaveVoicetagTask: public utilities::Task
        {
          public:
            SaveVoicetagTask(SrecVoicetagItemImplProxy& voicetag, const char* path);
            virtual ~SaveVoicetagTask();
            virtual void run();
          private:
            const char* path;
            SrecVoicetagItemImplProxy voicetag;
        };

      }
    }
  }
}

#endif
