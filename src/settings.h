///////////////////////////////////////////////////////////////////
// Dandelion Settings
// Load/Save settings for Dandelion network visualiser
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2010
///////////////////////////////////////////////////////////////////

#ifndef SETTINGS_H
#define SETTINGS_H

///////////////////////////////////////////////////////////////////
// Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gnet.h>

#include "utils.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _SettingsPersist SettingsPersist;

typedef enum {
  SETTINGTYPE_INVALID = -1,

  SETTINGTYPE_INT,
  SETTINGTYPE_BOOL,
  SETTINGTYPE_FLOAT,
  SETTINGTYPE_STRING,

  SETTINGTYPE_NUM
} SETTINGTYPE;

typedef struct _SettingsLoadParser {
  void (* LoadProperty) (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
  void (* LoadSectionStart) (char const * szName, void * psData, SettingsPersist * psSettingsData);
  void (* LoadSectionEnd) (char const * szName, void * psData, SettingsPersist * psSettingsData);

  void * psData;
} SettingsLoadParser;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

// General
SettingsPersist * NewSettingsPersist (char const * szName, char const * szFilename);
void DeleteSettingsPersist (SettingsPersist * psSettingsData);

// Save routines
void SettingsSaveStart (SettingsPersist * psSettingsData);
void SettingsSaveEnd (SettingsPersist * psSettingsData);

void SettingsStartTag (SettingsPersist * psSettingsData, char const * szTag);
void SettingsEndTag (SettingsPersist * psSettingsData, char const * szTag);
void SettingsText (SettingsPersist * psSettingsData, char const * szText);
void SettingsPrintIn (SettingsPersist * psSettingsData, char * szFormat, ...);

void SettingsPrintInt (SettingsPersist * psSettingsData, char const * szName, int nValue);
void SettingsPrintBool (SettingsPersist * psSettingsData, char const * szName, bool boValue);
void SettingsPrintFloat (SettingsPersist * psSettingsData, char const * szName, float fValue);
void SettingsPrintString (SettingsPersist * psSettingsData, char const * szName, char const * szString);

// Load routines
bool SettingsLoad (SettingsPersist * psSettingsData);
void AddParser (SettingsLoadParser * psParser, SettingsPersist * psSettingsData);
void RemoveParser (SettingsPersist * psSettingsData);
SettingsLoadParser * GetParser (SettingsPersist * psSettingsData);


#endif /* SETTINGS_H */


