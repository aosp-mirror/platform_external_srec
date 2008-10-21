/*---------------------------------------------------------------------------*
 *  exports.h                                                                *
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

#ifndef __UAPI_EXPORTS
#define __UAPI_EXPORTS
#if defined(UAPI_WIN32)
# ifdef UAPI_EXPORTS
#   define UAPI_EXPORT __declspec(dllexport)
# else
#   define UAPI_EXPORT __declspec(dllimport)
# endif
# ifdef JNI_EXPORTS
#   define JNI_EXPORT __declspec(dllexport)
# else
#   define JNI_EXPORT __declspec(dllimport)
# endif
#elif defined(UAPI_LINUX)
# ifdef UAPI_EXPORTS
#  define UAPI_EXPORT
# else
#  define UAPI_EXPORT
# endif
# ifdef JNI_EXPORTS
#  define JNI_EXPORT
# else
#  define JNI_EXPORT
# endif
#else
# error not defined
#endif

#endif
