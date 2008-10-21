/*---------------------------------------------------------------------------*
 *  ESR_Locale.c  *
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


#include "ESR_Locale.h"
#include "plog.h"
#include "LCHAR.h"

#define LOCALE_COUNT 9
static LCHAR* localeStringMapping[LOCALE_COUNT+1] =
  {
    L("ESR_LOCALE_EN_US"),
    L("ESR_LOCALE_FR_FR"),
    L("ESR_LOCALE_DE_DE"),
    L("ESR_LOCALE_EN_GB"),
    L("ESR_LOCALE_IT_IT"),
    L("ESR_LOCALE_NL_NL"),
    L("ESR_LOCALE_PT_PT"),
    L("ESR_LOCALE_ES_ES"),
    L("ESR_LOCALE_JA_JP"),
    L("invalid locale code") /* must remain last element for ESR_locale2str() to function */
  };
  
LCHAR* ESR_locale2str(const ESR_Locale locale)
{
  if (locale >= LOCALE_COUNT)
    return localeStringMapping[LOCALE_COUNT];
  return localeStringMapping[locale];
}


ESR_ReturnCode ESR_str2locale(const LCHAR* str, ESR_Locale* locale)
{
  int result;
  
  if (lstrcasecmp(str, L("EN-US"), &result) == ESR_SUCCESS && !result)
  {
    *locale = ESR_LOCALE_EN_US;
    return ESR_SUCCESS;
  }
  else if (lstrcasecmp(str, L("FR-FR"), &result) == ESR_SUCCESS && !result)
  {
    *locale = ESR_LOCALE_FR_FR;
    return ESR_SUCCESS;
  }
  else if (lstrcasecmp(str, L("DE-DE"), &result) == ESR_SUCCESS && !result)
  {
    *locale = ESR_LOCALE_DE_DE;
    return ESR_SUCCESS;
  }
  else if (lstrcasecmp(str, L("EN-GB"), &result) == ESR_SUCCESS && !result)
  {
    *locale = ESR_LOCALE_EN_GB;
    return ESR_SUCCESS;
  }
  else if (lstrcasecmp(str, L("JA-JP"), &result) == ESR_SUCCESS && !result)
  {
    *locale = ESR_LOCALE_JA_JP;
    return ESR_SUCCESS;
  }
  else if (lstrcasecmp(str, L("PT-PT"), &result) == ESR_SUCCESS && !result)
  {
    *locale = ESR_LOCALE_PT_PT;
    return ESR_SUCCESS;
  }
  else if (lstrcasecmp(str, L("ES-ES"), &result) == ESR_SUCCESS && !result)
  {
    *locale = ESR_LOCALE_ES_ES;
    return ESR_SUCCESS;
  }
  else if (lstrcasecmp(str, L("IT-IT"), &result) == ESR_SUCCESS && !result)
  {
    *locale = ESR_LOCALE_IT_IT;
    return ESR_SUCCESS;
  }
  else if (lstrcasecmp(str, L("NL-NL"), &result) == ESR_SUCCESS && !result)
  {
    *locale = ESR_LOCALE_NL_NL;
    return ESR_SUCCESS;
  }
  PLogError(L("no locale defined for %s"), str);
  return ESR_INVALID_ARGUMENT;
}
