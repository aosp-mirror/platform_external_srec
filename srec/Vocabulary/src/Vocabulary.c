/*---------------------------------------------------------------------------*
 *  Vocabulary.c  *
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

#include "plog.h"
#include "SR_Vocabulary.h"
#include "SR_VocabularyImpl.h"


ESR_ReturnCode SR_VocabularyCreate(ESR_Locale locale, SR_Vocabulary** self)
{
     SR_Vocabulary* Interface;
     SR_VocabularyImpl* impl;
     ESR_ReturnCode rc;

     CHK(rc, SR_VocabularyCreateImpl(&Interface));
     impl = (SR_VocabularyImpl*) Interface;
     impl->locale = locale;
     impl->ttp_lang = TTP_LANG(locale);

#ifdef USE_TTP
     /* impl->ttp_lang should be set to the current language before G2P is created */
     rc = SR_CreateG2P(Interface);
     if (rc != ESR_SUCCESS) 
     {
          SR_VocabularyDestroyImpl(Interface);
          goto CLEANUP;
     }
#endif

     *self = Interface;
     return ESR_SUCCESS;
 CLEANUP:
     return rc;
}

ESR_ReturnCode SR_VocabularyLoad(const LCHAR* filename, SR_Vocabulary** self)
{
     SR_Vocabulary* Interface;
     ESR_ReturnCode rc;

     CHK(rc, SR_VocabularyLoadImpl(filename, &Interface));

     *self = Interface;
     return ESR_SUCCESS;
 CLEANUP:
     return rc;
}

ESR_ReturnCode SR_VocabularySave(SR_Vocabulary* self, const LCHAR* filename)
{
  if (self==NULL)
  {
    PLogError(L("ESR_INVALID_ARGUMENT"));
    return ESR_INVALID_ARGUMENT;
  }
  return self->save(self, filename);
}

ESR_ReturnCode SR_VocabularyGetLanguage(SR_Vocabulary* self, ESR_Locale* locale)
{
  if (self==NULL)
  {
    PLogError(L("ESR_INVALID_ARGUMENT"));
    return ESR_INVALID_ARGUMENT;
  }
  return self->getLanguage(self, locale);
}

ESR_ReturnCode SR_VocabularyDestroy(SR_Vocabulary* self)
{
  if (self==NULL)
  {
    PLogError(L("ESR_INVALID_ARGUMENT"));
    return ESR_INVALID_ARGUMENT;
  }
  return self->destroy(self);
}

ESR_ReturnCode SR_VocabularyGetPronunciation(SR_Vocabulary* self, const LCHAR* word, LCHAR* phoneme, size_t* len)
{
  if (self==NULL)
  {
    PLogError(L("ESR_INVALID_ARGUMENT"));
    return ESR_INVALID_ARGUMENT;
  }
  return self->getPronunciation(self, word, phoneme, len);
}

/****************************
 * ETI to INFINITIVE Phoneme conversion stuff
 */
#define DICTIONARY_HANDLE void
#define MAX_ESR_LOCALES 16
#define MAX_LENGTH P_PATH_MAX
static SR_Vocabulary* g_etiinf_pronguesser[MAX_ESR_LOCALES];
const char*    g_etiinf_multichar[MAX_ESR_LOCALES][256];
static int initted = 0;
static int portable_initted = 0;

ESR_ReturnCode SR_Vocabulary_etiinf_conv_multichar(ESR_Locale locale, const LCHAR* i, LCHAR* output, size_t max_len)
{
     const LCHAR** m;

     //if (locale < 0) 
     //     return ESR_INVALID_ARGUMENT;
  
     m = g_etiinf_multichar[locale];

     for (*output='\0'; *i; ++i)
     {
          strcat(output, m[(int)*i]);
          if (*(i+1)) 
               strcat(output, " ");
     }
     return ESR_SUCCESS;
}

ESR_ReturnCode SR_Vocabulary_etiinf_conv_from_multichar(ESR_Locale locale, LCHAR* input, LCHAR* output)
{

     ESR_ReturnCode rc;
     size_t i;
     const LCHAR** m;

     //if (locale < 0) 
     //     return 1;

     CHKLOG(rc, SR_Vocabulary_etiinf_init_multichar(locale));
     m = g_etiinf_multichar[locale];

     for (i=0; i<256; ++i)
     {
          if (m[i] && !LSTRCMP(m[i], input))
          {
               *output = (LCHAR) i;
               return ESR_SUCCESS;
          }
     }
     return ESR_NO_MATCH_ERROR;
 CLEANUP:
     return rc;
}

ESR_ReturnCode SR_Vocabulary_etiinf_init_multichar(ESR_Locale locale)
{
     size_t i;
     const LCHAR** m;

     //if (locale<0) 
     //     return 1;
  
     m = &g_etiinf_multichar[locale][0];
     for(i=0;i<255;i++)    m[i] = "";
  
     switch (locale)
     {
     case ESR_LOCALE_EN_US:
     case ESR_LOCALE_EN_GB:
          /* enu_d2f_fray_g.pht */
          m['}']="um";  m['?']="OW";  m['~']="un";  m['@']="uh";  m['A']="EY";
          m['C']="ch";  m['D']="dh";  m['E']="EE";  m['I']="AY";  m['J']="jnk";
          m['L']="ul";  m['N']="ng";  m['O']="OH";  m['P']="ur";  m['S']="sh";
          m['T']="th";  m['U']="OOH"; m['V']="UR";  m['Z']="zh";  m[']']="oh";
          m['^']="ENV"; m['#']="sil"; m['a']="AA";  m['b']="b";   m['c']="eh";
          m['d']="d";   m['e']="EH";  m['f']="f";   m[')']="AH";  m['g']="g";
          m['h']="h";   m['i']="IH";  m['j']="j";   m[',']="AE";  m['k']="k";
          m['l']="l";   m['m']="m";   m['/']="ee";  m['n']="n";   m['o']="AW";
          m['p']="p";   m['q']="OO";  m['r']="r";   m['s']="s";   m['t']="t";
          m['6']="ih";  m['u']="UH";  m['v']="v";   m['w']="w";   m['y']="y";
          m['z']="z";   m['<']="OY";  m['{']="AWH";
          break;
     case ESR_LOCALE_FR_FR:
          /* fra_t22_m.pht */
          m['A']="ACI"; m[3]="OEE";   m[6]="OEN";   m['E']="EAC"; m['J']="jnk";
          m['M']="gn";  m[16]="QQ";   m['N']="ng";  m['O']="OCI"; m[19]="AE";
          m['S']="sh";  m['U']="UY";  m['W']="yw";  m['Y']="EN";  m['Z']="ge";
          m[31]="OE";   m['^']="ENV"; m['#']="sil"; m['a']="AGR"; m['b']="b";
          m['d']="d";   m['e']="ECI"; m['f']="f";   m[')']="AN";  m['g']="g";
          m['i']="II";  m['k']="k";   m['l']="l";   m['m']="m";   m['n']="n";
          m['o']="OO";  m['p']="p";   m['r']="r";   m['s']="s";   m['t']="t";
          m['u']="UGR"; m['v']="v";   m['w']="w";   m['y']="y";   m['z']="z";
          m['{']="ON";
          break;

     case ESR_LOCALE_DE_DE:
          m['@']="utt"; m['A']="AH";  m[4]="eu";    m['C']="ich"; m[6]="EU";
          m['E']="EH";  m['H']="ue";  m['I']="IH";  m['J']="jnk"; m['K']="ach";
          m['N']="ng";  m['O']="OH";  m['S']="sch"; m['T']="hr";  m['U']="UH";
          m['V']="UEH"; m['W']="wu";  m['Z']="zh";  m['[']="ott"; m['^']="ENV";
          m['!']="att"; m['#']="sil"; m['a']="ATT"; m['b']="b";   m['c']="ett";
          m['d']="d";   m['e']="ETT"; m['f']="f";   m['g']="g";   m['h']="h";
          m['i']="ITT"; m['j']="j";   m[',']="AEH"; m['k']="k";   m['l']="l";
          m['m']="m";   m['n']="n";   m['o']="OTT"; m['p']="p";   m['q']="UE";
          m['r']="r";   m['s']="s";   m['t']="t";   m['6']="itt"; m['u']="UTT";
          m['w']="w";   m['x']="@@";  m[':']="oe";  m['z']="z";   m['<']="OE";
          m['{']="OEH";
          break;
     case ESR_LOCALE_ES_ES:
          m['@']="uu";  m['C']="ch";  m['D']="rr";  m['E']="EY";  m['J']="jnk";
          m['M']="ks";  m['N']="nn";  m['T']="Z";   m['[']="oo";  m['^']="ENV";
          m['!']="aa";  m['#']="sil"; m['a']="AA";  m['b']="b";   m['c']="ee";
          m['d']="d";   m['e']="EE";  m['f']="f";   m[')']="AU";  m['g']="g";
          m['i']="II";  m['j']="j";   m['k']="k";   m['l']="l";   m['m']="m";
          m['n']="n";   m['o']="OO";  m['p']="p";   m['r']="r";   m['s']="s";
          m['6']="ii";  m['t']="t";   m['u']="UU";  m['w']="w";   m['y']="y";
          break;
     case ESR_LOCALE_NL_NL:
          m['S']="S";   m['a']="a";   m['N']="nK";  m['d']="d";   m['E']="E";   
          m['2']="ep";  m['j']="j";   m['y']="y";   m['Z']="Z";   m['u']="u";   
          m['1']="AA";  m['k']="k";   m['g']="g";   m['t']="t";   m['e']="e";   
          m['J']="jnk"; m['v']="v";   m['s']="s";   m['^']="ENV"; m['b']="b";   
          m['I']="I";   m['G']="G";   m['z']="z";   m['w']="w";   m['$']="$";   
          m['r']="r";   m['x']="x";   m['h']="h";   m['f']="f";   m['i']="i";   
          m['A']="A";   m['6']="A%t"; m['O']="O";   m['n']="n";   m['3']="Ei";  
          m['#']="sil"; m['m']="m";   m['8']="O%t"; m['l']="l";   m['4']="yy";  
          m['p']="p";   m['5']="Au";  m['o']="o";   
          break;
     case ESR_LOCALE_IT_IT:
          m['@']="uu";  m['A']="AI";  m['C']="ci";  m['E']="EI";  m['J']="jnk";
          m['K']="rr";  m['M']="gi";  m['N']="gn";  m['O']="OI";  m[21]="gl";
          m['S']="sci"; m['Y']="ETT"; m['[']="oo";  m['^']="ENV"; m['!']="aa";
          m['#']="sil"; m['a']="AA";  m['b']="b";   m['c']="ee";  m['d']="d";
          m['e']="EE";  m['f']="f";   m[')']="AU";  m['g']="g";   m['i']="II";
          m['j']="j";   m['k']="k";   m['l']="l";   m['m']="m";   m['n']="n";
          m['o']="OO";  m['p']="p";   m['r']="r";   m['s']="s";   m['t']="t";
          m['6']="ii";  m['u']="UU";  m['v']="v";   m['w']="w";   m['z']="z";
          m['{']="OTT";
          break;
     case ESR_LOCALE_PT_PT:
          m['A']="ao";  m['B']="ojn"; m['E']="eh";  m['I']="ix";  m['J']="jnk";
          m['L']="lj";  m['N']="nj";  m['O']="on";  m['R']="rr";  m['S']="sh";
          m['U']="un";  m['Z']="zh";  m['^']="ENV"; m['#']="sil"; m['a']="a";
          m['b']="b";   m['c']="ew";  m['d']="d";   m['e']="e";   m['f']="f";
          m['g']="g";   m['h']="in";  m['i']="i";   m['j']="j";   m['k']="k";
          m['l']="l";   m['m']="m";   m['n']="n";   m['1']="aj";  m['o']="o";
          m['p']="p";   m['2']="ajn"; m['3']="an";  m['q']="iw";  m['r']="r";
          m['4']="aw";  m['s']="s";   m['5']="awn"; m['t']="t";   m['6']="ax";
          m['u']="u";   m['7']="axn"; m['v']="v";   m['8']="ej";  m['w']="w";
          m['9']="en";  m['x']="ls";  m['y']="oj";  m['z']="z";
          break;
     case ESR_LOCALE_JA_JP:
          return ESR_NOT_SUPPORTED;
          break;
     }
     m['#']="iwt"; m['&']="&";
  
     return ESR_SUCCESS;
}

ESR_Locale SR_Vocabulary_etiinf_get_locale_or_exit(const LCHAR* language)
    {
    LCHAR local_chars[4];

    /* language string comes from userdict/language.c, eg: en.us */
    local_chars [0] = (char)tolower(language[0]);
    local_chars [1] = (char)tolower(language[1]);
    local_chars [2] = (char)tolower(language[2]);
    local_chars [3] = (char)tolower(language[3]);

    if ( memcmp ( local_chars, "enus", 4 ) == 0 )
        return ESR_LOCALE_EN_US;
    else if ( ( memcmp ( local_chars, "enuk", 4 ) == 0 ) || ( memcmp ( local_chars, "engb", 4 ) == 0 ) )
        return ESR_LOCALE_EN_GB;
    else if ( memcmp ( local_chars, "frfr", 4 ) == 0 )
        return ESR_LOCALE_FR_FR;
    else if ( memcmp ( local_chars, "dede", 4 ) == 0 )
        return ESR_LOCALE_DE_DE;
    else if ( memcmp ( local_chars, "ptpt", 4 ) == 0 )
        return ESR_LOCALE_PT_PT;
    else if ( memcmp ( local_chars, "nlnl", 4 ) == 0 )
        return ESR_LOCALE_NL_NL;
    else if ( memcmp ( local_chars, "itit", 4 ) == 0 )
        return ESR_LOCALE_IT_IT;
    else if ( memcmp ( local_chars, "eses", 4 ) == 0 )
        return ESR_LOCALE_ES_ES;
    else
        {
        PLogError(L("ESR_NOT_IMPLEMENTED (error getting locale for language '%s')"), language);
        return ESR_NOT_IMPLEMENTED;
        }
    }

ESR_ReturnCode SR_Vocabulary_etiinf_get_phonemes(const char *input, /* word to generate pronunciations for */
                                                 const char* language,
                                                 DICTIONARY_HANDLE *h,  	/* dictionary to search, in addition to table above */
                                                 char **output_array, 		/* buffer to store pronunciations */
                                                 unsigned int *num_prons, 	/* number of prounciations to be returned */
                                                 unsigned int max_len, 	/* the length of each buffer line */
                                                 unsigned int max_prons,	/* maximum number of pronunciations allowed */
                                                 void *text_to_phoneme_data)	/* text to phoneme data */			  
{
     LCHAR raw_output[MAX_LENGTH];
     size_t len;
     int stat;
     LCHAR *output = output_array[0]; /* will only generate one pronunciation from the rules */
     ESR_ReturnCode rc;
     ESR_Locale locale;
  
     locale = SR_Vocabulary_etiinf_get_locale_or_exit(language);

     raw_output[0] = '\0';
     *num_prons = 1;

     if (initted == 0)
     {
          PLogError(L("DICT: NAV rules called but not initialized"));
          *output = '\0';
          return ESR_FATAL_ERROR;
     }

     /*PTHREAD_INIT_PRIVATE( pthread_key, pglobals, sizeof (GLOBALS), NULL );*/

     len = MAX_LENGTH;
     stat = (int)SR_VocabularyGetPronunciation( g_etiinf_pronguesser[locale], input, 
                                                raw_output, &len);
     if(stat != 0)
     {
          PLogError(L("DICT: SR_VocabularyGetPronunciation() failed"));
          return ESR_FATAL_ERROR;
     }
     else 
     {
          /* convert the phoneme to multi char representation */
          rc = SR_Vocabulary_etiinf_conv_multichar(locale, raw_output, output, max_len);
          if (rc!=ESR_SUCCESS)
          {
               PLogError(L("DICT: failed to multichar output of SR_VocabularyGetPronounciation() %s"), raw_output);
               return rc;
          }
     }
     return ESR_SUCCESS;
}

ESR_ReturnCode SR_Vocabulary_etiinf_destroy_pronguesser( const char* language) 
{
     if (portable_initted == 1) 
          PMemShutdown();
     portable_initted = 0;
     return ESR_SUCCESS;
}

ESR_ReturnCode SR_Vocabulary_etiinf_init_pronguesser( const char* language)
{
     size_t i,j;
     ESR_ReturnCode rc;
     ESR_Locale locale;

     if (!portable_initted)
     {
          rc = PMemInit();
          if (rc!=ESR_SUCCESS)
               return rc;
          for (i=0; i<MAX_ESR_LOCALES; ++i)
               g_etiinf_pronguesser[i] = 0;
          for (i=0; i<MAX_ESR_LOCALES; ++i)
               for (j=0; j<255; ++j)
                    g_etiinf_multichar[i][j] = 0;
          portable_initted = 1;
     }
  
     locale = SR_Vocabulary_etiinf_get_locale_or_exit(language);
  
     CHKLOG(rc, SR_VocabularyCreate(locale, &g_etiinf_pronguesser[locale]));
     CHKLOG(rc, SR_Vocabulary_etiinf_init_multichar(locale));
     return ESR_SUCCESS;
 CLEANUP:
     return rc;
}
