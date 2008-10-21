/*---------------------------------------------------------------------------*
 *  types.h                                                                  *
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

#ifndef __UAPI__TYPES
#define __UAPI__TYPES

#include <math.h>


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
       * signed long.
       */
      typedef long LONG;
      
      /**
       * unsigned long.
       */
      typedef unsigned long ULONG;
      
      /**
       * 32-bit signed integer.
       */
      typedef int INT32;
      
      /**
       * The minimum value that a INT32 variable is guaranteed to support.
       */
      const INT32 INT32_MIN = -2147483647 - 1;
      // VS2003 gives a compiler warning unless we use -1
      
      /**
       * The maximum value that a INT32 variable is guaranteed to support.
       */
      const INT32 INT32_MAX = 2147483647;
      
      /**
       * The maximum number of digits needed to represent a INT32_DIGITS. +1 for the negative sign.
       */
      const int INT32_DIGITS = (int) ceil(32*log10(2.0) + 1);
      
      /**
       * 32-bit unsigned integer.
       */
      typedef unsigned int UINT32;
      
      /**
       * The minimum value that a UINT32 variable is guaranteed to support.
       */
      const UINT32 UINT32_MIN = 0;
      
      /**
       * The maximum value that a UINT32 variable is guaranteed to support.
       */
      const UINT32 UINT32_MAX = 4294967295ul;
      
      /**
       * The maximum number of digits needed to represent a UINT32_DIGITS. +1 for the positive sign.
       */
      const int UINT32_DIGITS = (int) ceil(32*log10(2.0) + 1);
      
      /**
       * 16-bit signed integer.
       */
      typedef short INT16;
      
      /**
       * The maximum number of digits needed to represent a INT16_DIGITS. +1 for the negative sign.
       */
      const int INT16_DIGITS = (int) ceil(16*log10(2.0) + 1);
      
      /**
       * 16-bit unsigned integer.
       */
      typedef unsigned short UINT16;
      
      /**
       * The minimum value that a UINT16 variable is guaranteed to support.
       */
      const UINT16 UINT16_MIN = 0;
      
      /**
       * The maximum value that a UINT16 variable is guaranteed to support.
       */
      const UINT16 UINT16_MAX = 65535;
      
      /**
       * 8-bit signed integer.
       */
      typedef char INT8;
      
      /**
       * 8-bit unsigned integer.
       */
      typedef unsigned char UINT8;
      
      /**
       * The minimum value that a UINT8 variable is guaranteed to support.
       */
      const UINT8 UINT8_MIN = 0;
      
      /**
       * The maximum value that a UINT8 variable is guaranteed to support.
       */
      const UINT8 UINT8_MAX = 255;
      
      /**
       * The maximum size of an array.
       */
      typedef int ARRAY_LIMIT;
      
      /**
       * The minimum value that a ARRAY_LIMIT variable is guaranteed to support.
       */
      const ARRAY_LIMIT ARRAY_LIMIT_MIN = 0;
      
      /**
       * The maximum value that a ARRAY_LIMIT variable is guaranteed to support.
       */
      const ARRAY_LIMIT ARRAY_LIMIT_MAX = 2147483647;
    }
  }
}

#endif
