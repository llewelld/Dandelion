///////////////////////////////////////////////////////////////////
// Dandelion Properties
// Add properties to nodes and links in Dandelion
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Autumn 2010
///////////////////////////////////////////////////////////////////

#ifndef PROPERTIES_H
#define PROPERTIES_H

///////////////////////////////////////////////////////////////////
// Includes

#include "settings.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _TProperty TProperty;
typedef struct _PropertiesPersist PropertiesPersist;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

PropertiesPersist * NewPropertiesPersist ();
void DeletePropertiesPersist (PropertiesPersist * psPropertiesData);
void SetProperty (char const * szProperty, char const * szType, char const * szValue, PropertiesPersist * psPropertiesData);
void SubProperty (char const * szProperty, PropertiesPersist * psPropertiesData);
GString * GenerateNotePropertyString (GString * szString, PropertiesPersist * psPropertiesData);

void SaveSettingsProperties (SettingsPersist * psSettingsData, PropertiesPersist * psPropertiesData);
void PropertiesLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
void PropertiesLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
void PropertiesLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* PROPERTIES_H */




