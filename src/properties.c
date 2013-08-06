///////////////////////////////////////////////////////////////////
// Dandelion Properties
// Add properties to nodes and links in Dandelion
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Autumn 2010
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <glib.h>

#include "properties.h"

///////////////////////////////////////////////////////////////////
// Defines

#define true (1)
#define false (0)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef enum {
  PROPTYPE_INVALID = -1,

  PROPTYPE_INT,
  PROPTYPE_FLOAT,
  PROPTYPE_STRING,

  PROPTYPE_NUM
} PROPTYPE;

struct _TProperty {
  char * szName;
  PROPTYPE eType;
  union {
  	int nValue;
  	float fValue;
  	char * szValue;
  };
};

struct _PropertiesPersist {
  GSList * psProperties;
};

typedef struct _SettingsProperty {
  TProperty * psProperty;
  PropertiesPersist * psPropertiesData;
} SettingsProperty;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

TProperty * NewTProperty (char const * szProperty);
void DeleteTProperty (TProperty * psProperty);
void DeleteTPropertyCallback (gpointer psPropertyData, gpointer psData);
void FreePropertyValue (TProperty * psProperty);
gint CompareProperties (gconstpointer a, gconstpointer b);
void SetPropertyName (char const * szName, TProperty * psProperty);
void SetPropertyInt (int nValue, TProperty * psProperty);
void SetPropertyFloat (float fValue, TProperty * psProperty);
void SetPropertyString (char const * szValue, TProperty * psProperty);
void SaveSettingsProperty (gpointer psData, gpointer psUserData);
void LoadSettingsStartProperties (SettingsPersist * psSettingsData, PropertiesPersist * psPropertiesData);
void LoadSettingsEndProperties (SettingsPersist * psSettingsData, PropertiesPersist * psPropertiesData);
void PropertyLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
void PropertyLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);
void PropertyLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);

///////////////////////////////////////////////////////////////////
// Function definitions

PropertiesPersist * NewPropertiesPersist () {
  PropertiesPersist * psPropertiesData;

  psPropertiesData = g_new0 (PropertiesPersist, 1);
  psPropertiesData->psProperties = NULL;

  return psPropertiesData;
}

void DeletePropertiesPersist (PropertiesPersist * psPropertiesData) {
	if (psPropertiesData->psProperties) {
		// Remove all of the properties
		g_slist_foreach (psPropertiesData->psProperties, DeleteTPropertyCallback, NULL);
		g_slist_free (psPropertiesData->psProperties);
	}

  g_free (psPropertiesData);
}

TProperty * NewTProperty (char const * szProperty) {
	TProperty * psProperty;

	psProperty = g_new0 (TProperty, 1);
	psProperty->szName = g_new (char, strlen (szProperty) + 1);
	strcpy (psProperty->szName, szProperty);
	psProperty->eType = PROPTYPE_INVALID;
	psProperty->nValue = 0;

	return psProperty;
}

void DeleteTProperty (TProperty * psProperty) {
	if (psProperty->szName) {
		g_free (psProperty->szName);
	}

	FreePropertyValue (psProperty);

	g_free (psProperty);
}

void DeleteTPropertyCallback (gpointer psPropertyData, gpointer psData) {
  // We don't use psData; it can just be NULL

  DeleteTProperty ((TProperty *)psPropertyData);
}

void FreePropertyValue (TProperty * psProperty) {
	if (psProperty) {
		switch (psProperty->eType) {
		case PROPTYPE_STRING:
			if (psProperty->szValue) {
				g_free (psProperty->szValue);
				psProperty->szValue = NULL;
			}
			break;
		default:
			// Do nothing
			break;
		}
	}

	psProperty->eType = PROPTYPE_INVALID;
	psProperty->nValue = 0;
}

gint CompareProperties (gconstpointer a, gconstpointer b) {
	return strcmp (((TProperty *)a)->szName, ((TProperty *)b)->szName);
}

void SetPropertyName (char const * szName, TProperty * psProperty) {
	if (psProperty->szName) {
		// Free the existing name
		g_free (psProperty->szName);
	}
	psProperty->szName = g_new (char, strlen (szName) + 1);
	psProperty->szName[0] = 0;
	strcpy (psProperty->szName, szName);
}

void SetPropertyInt (int nValue, TProperty * psProperty) {
	FreePropertyValue (psProperty);

	psProperty->eType = PROPTYPE_INT;
	psProperty->nValue = nValue;
}

void SetPropertyFloat (float fValue, TProperty * psProperty) {
	FreePropertyValue (psProperty);

	psProperty->eType = PROPTYPE_FLOAT;
	psProperty->fValue = fValue;
}

void SetPropertyString (char const * szValue, TProperty * psProperty) {
	FreePropertyValue (psProperty);

	psProperty->eType = PROPTYPE_STRING;
	psProperty->szValue = g_new (char, strlen (szValue) + 1);
	psProperty->szValue[0] = 0;
	strcpy (psProperty->szValue, szValue);
}

void SetProperty (char const * szProperty, char const * szType, char const * szValue, PropertiesPersist * psPropertiesData) {
	TProperty * psProperty;
	GSList * psFound;
	int nValue;
	float fValue;

	// Create an appropriate property structure
	psProperty = NewTProperty (szProperty);
	if (strcmp (szType, "int") == 0) {
		sscanf (szValue, "%d", & nValue);
		SetPropertyInt (nValue, psProperty);
	}
	else if (strcmp (szType, "float") == 0) {
		sscanf (szValue, "%f", & fValue);
		SetPropertyFloat (fValue, psProperty);
	}
	else if (strcmp (szType, "string") == 0) {
		SetPropertyString (szValue, psProperty);
	}
	else {
		FreePropertyValue (psProperty);
	}

	// Check whether the property already exists
	psFound = g_slist_find_custom (psPropertiesData->psProperties, psProperty, CompareProperties);

	if (psFound) {
		// Replace the data
		g_free (psFound->data);
		psFound->data = psProperty;
	}
	else {
		// Add the data to the list
		psPropertiesData->psProperties = g_slist_prepend (psPropertiesData->psProperties, psProperty);
	}
}

void SubProperty (char const * szProperty, PropertiesPersist * psPropertiesData) {
	TProperty * psProperty;
	GSList * psFound;

	// Create an appropriate property structure
	psProperty = NewTProperty (szProperty);

	// Find the property
	psFound = g_slist_find_custom (psPropertiesData->psProperties, psProperty, CompareProperties);

	if (psFound) {
		// Remove the data
		DeleteTProperty ((TProperty *)(psFound->data));
		psFound->data = NULL;
		psPropertiesData->psProperties = g_slist_delete_link (psPropertiesData->psProperties, psFound);
	}

	DeleteTProperty (psProperty);
}

GString * GenerateNotePropertyString (GString * szString, PropertiesPersist * psPropertiesData) {
	GSList * psProperties;
	TProperty * psProperty;

	// Find the list of properties
	psProperties = psPropertiesData->psProperties;

	if (szString == NULL) {
		szString = g_string_new ("");
	}

	// Erase the existing string
	g_string_erase (szString, 0, -1);

	// Cycle through all of the properties and add them to the string
	while (psProperties) {
		psProperty = ((TProperty *)psProperties->data);
		
		switch (psProperty->eType) {
		case PROPTYPE_INT:
			g_string_append_printf (szString, "%s: %d", psProperty->szName, psProperty->nValue);
			break;
		case PROPTYPE_FLOAT:
			g_string_append_printf (szString, "%s: %f", psProperty->szName, psProperty->fValue);
			break;
		case PROPTYPE_STRING:
			g_string_append_printf (szString, "%s: %s", psProperty->szName, psProperty->szValue);
			break;
		default:
			g_string_append_printf (szString, "%s: n/a", psProperty->szName);
			break;
		}
		
		psProperties = g_slist_next (psProperties);
		if (psProperties) {
			g_string_append (szString, "\n");
		}
	}

	return szString;
}

void SaveSettingsProperties (SettingsPersist * psSettingsData, PropertiesPersist * psPropertiesData) {
  // Save out the properties
	if (psPropertiesData->psProperties) {
	  SettingsStartTag (psSettingsData, "Properties");
	  g_slist_foreach (psPropertiesData->psProperties, SaveSettingsProperty, psSettingsData);
	  SettingsEndTag (psSettingsData, "Properties");
	}
}

void SaveSettingsProperty (gpointer psData, gpointer psUserData) {
  SettingsPersist * psSettingsData = (SettingsPersist *)psUserData;
  TProperty * psProperty = (TProperty *)psData;

  SettingsStartTag (psSettingsData, "Set");
  SettingsPrintString (psSettingsData, "Name", psProperty->szName);

	switch (psProperty->eType) {
	case PROPTYPE_INT:
	  SettingsPrintInt (psSettingsData, "Value", psProperty->nValue);
		break;
	case PROPTYPE_FLOAT:
	  SettingsPrintFloat (psSettingsData, "Value", psProperty->fValue);
		break;
	case PROPTYPE_STRING:
	  SettingsPrintString (psSettingsData, "Value", psProperty->szValue);
		break;
	default:
		// Do nothing
		break;
	}

  SettingsEndTag (psSettingsData, "Set");
}

void LoadSettingsStartProperties (SettingsPersist * psSettingsData, PropertiesPersist * psPropertiesData) {
  SettingsLoadParser * psLoadParser = NULL;

  psLoadParser = g_new0 (SettingsLoadParser, 1);

  psLoadParser->LoadProperty = PropertiesLoadProperty;
  psLoadParser->LoadSectionStart = PropertiesLoadSectionStart;
  psLoadParser->LoadSectionEnd = PropertiesLoadSectionEnd;
  psLoadParser->psData = psPropertiesData;
  AddParser (psLoadParser, psSettingsData);
}

void LoadSettingsEndProperties (SettingsPersist * psSettingsData, PropertiesPersist * psPropertiesData) {
  SettingsLoadParser * psLoadParser = NULL;

  psLoadParser = GetParser (psSettingsData);
  g_free (psLoadParser);
  RemoveParser (psSettingsData);
}

void PropertiesLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
  //PropertiesPersist * psPropertiesData = (PropertiesPersist *)psData;

	// Do nothing - therere's nothing other than properties here
}

void PropertiesLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  PropertiesPersist * psPropertiesData = (PropertiesPersist *)psData;
  SettingsProperty * psSettingsProperty;
  SettingsLoadParser * psLoadParser = NULL;

  if (stricmp (szName, "set") == 0) {
    // Move into the property section
		psSettingsProperty = g_new0 (SettingsProperty, 1);
		psSettingsProperty->psProperty = NewTProperty ("");
		psSettingsProperty->psPropertiesData = psPropertiesData;

    psLoadParser = g_new0 (SettingsLoadParser, 1);
    psLoadParser->LoadProperty = PropertyLoadProperty;
    psLoadParser->LoadSectionStart = PropertyLoadSectionStart;
    psLoadParser->LoadSectionEnd = PropertyLoadSectionEnd;
    psLoadParser->psData = psSettingsProperty;
    AddParser (psLoadParser, psSettingsData);
  }
}

void PropertiesLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  PropertiesPersist * psPropertiesData = (PropertiesPersist *)psData;

  if (stricmp (szName, "properties") == 0) {
    // Move out of the property section
    LoadSettingsEndProperties (psSettingsData, psPropertiesData);
  }
}

void PropertyLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  // Nothing to do at this level
}

void PropertyLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  SettingsProperty * psSettingsProperty = (SettingsProperty *)psData;
  SettingsLoadParser * psLoadParser = NULL;

  if (stricmp (szName, "set") == 0) {
		// Link the property into the properties list
		psSettingsProperty->psPropertiesData->psProperties = g_slist_prepend (psSettingsProperty->psPropertiesData->psProperties, psSettingsProperty->psProperty);

    // Move out of the node section
    g_free (psSettingsProperty);

    psLoadParser = GetParser (psSettingsData);
    g_free (psLoadParser);
    RemoveParser (psSettingsData);
  }
}

void PropertyLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
  SettingsProperty * psSettingsProperty = (SettingsProperty *)psData;

  switch (eType) {
  case SETTINGTYPE_STRING:
    if (stricmp (szName, "name") == 0) {
			SetPropertyName (((char *)(psValue)), psSettingsProperty->psProperty);
    }
    else if (stricmp (szName, "value") == 0) {
			SetPropertyString (((char *)(psValue)), psSettingsProperty->psProperty);
    }
    break;
  case SETTINGTYPE_INT:
		if (stricmp (szName, "value") == 0) {
			SetPropertyInt (*((int *)(psValue)), psSettingsProperty->psProperty);
    }
    break;
  case SETTINGTYPE_FLOAT:
		if (stricmp (szName, "value") == 0) {
			SetPropertyFloat (*((float *)(psValue)), psSettingsProperty->psProperty);
    }
    break;
  default:
    printf ("Unknown property %s\n", szName);
    break;
  }
}


