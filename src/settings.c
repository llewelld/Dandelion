///////////////////////////////////////////////////////////////////
// Dandelion Settings
// Load/Save settings for Dandelion network visualiser
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2010
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkgl.h>
#include <glade/glade.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#include "settings.h"

///////////////////////////////////////////////////////////////////
// Defines

#define FILE_BUFFER_SIZE (1024)
#define MAX_LEVELS (999999)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

struct _SettingsPersist {
  GString * szName;
  GString * szFilename;
  FILE * fhFile;
  int nIndent;
  SettingsLoadParser * psParser;
  // Note that this is a backwards stack
  // The current parser is at the head of the list
  GSList * psParsers;
  int nLevel;
  GString * szText;
  SETTINGTYPE eType;
  GString * szID;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

void PrintIn (FILE * fhFile, int nIndent, char const * szFormat, ...);
void StartElement (GMarkupParseContext * psContext, gchar const * szElementName, gchar const * * aszAttributeNames, gchar const * * aszAttributeValues, gpointer psData, GError * * ppsError);
void EndElement (GMarkupParseContext * psContext, gchar const * szElementName, gpointer psData, GError * * ppsError);
void Text (GMarkupParseContext * pContext, gchar const * szText, gsize nTextLen, gpointer psData, GError * * ppsError);
void DestroyUserData (gpointer psData);

///////////////////////////////////////////////////////////////////
// General function definitions

SettingsPersist * NewSettingsPersist (char const * szName, char const * szFilename) {
  SettingsPersist * psSettingsData = NULL;

  psSettingsData = g_new0 (SettingsPersist, 1);
  psSettingsData->nIndent = 0;
  psSettingsData->fhFile = NULL;
  psSettingsData->psParser = NULL;
  psSettingsData->psParsers = NULL;
  psSettingsData->nLevel = 0;
  psSettingsData->szText = g_string_new ("");
  psSettingsData->szFilename = g_string_new (szFilename);
  psSettingsData->szName = g_string_new (szName);
  psSettingsData->szID = g_string_new (szFilename);
  return psSettingsData;
}

void DeleteSettingsPersist (SettingsPersist * psSettingsData) {
  if (psSettingsData->fhFile) {
    SettingsEndTag (psSettingsData, psSettingsData->szName->str);
    fclose (psSettingsData->fhFile);
  }

  g_string_free (psSettingsData->szName, TRUE);
  g_string_free (psSettingsData->szID, TRUE);
  g_string_free (psSettingsData->szFilename, TRUE);
  g_string_free (psSettingsData->szText, TRUE);
  g_free (psSettingsData);
}

///////////////////////////////////////////////////////////////////
// Saving function definitions

void SettingsSaveStart (SettingsPersist * psSettingsData) {
  if (psSettingsData->fhFile) {
    SettingsEndTag (psSettingsData, psSettingsData->szName->str);
    fclose (psSettingsData->fhFile);
  }

  psSettingsData->fhFile = fopen (psSettingsData->szFilename->str, "wb");
  if (psSettingsData->fhFile) {
    // Preamble
    psSettingsData->nIndent = 0;
    PrintIn (psSettingsData->fhFile, psSettingsData->nIndent, "<?xml version='1.0' encoding='UTF-8' ?>\n\n");

    SettingsStartTag (psSettingsData, psSettingsData->szName->str);
  }
}

void SettingsSaveEnd (SettingsPersist * psSettingsData) {
  if (psSettingsData->fhFile) {
    SettingsEndTag (psSettingsData, psSettingsData->szName->str);
    fclose (psSettingsData->fhFile);
    psSettingsData->fhFile = NULL;
  }
  psSettingsData->nIndent = 0;
}

void PrintIn (FILE * fhFile, int nIndent, char const * szFormat, ...) {
  va_list sArgP;
  int nIndentCount;

  if (fhFile) {
    // Indent
    for (nIndentCount = 0; nIndentCount < nIndent; nIndentCount++) {
      fprintf (fhFile, "\t");
    }

    va_start (sArgP, szFormat);
    vfprintf (fhFile, szFormat, sArgP);
    va_end (sArgP);
  }
}

void SettingsStartTag (SettingsPersist * psSettingsData, char const * szTag) {
  PrintIn (psSettingsData->fhFile, psSettingsData->nIndent, "<%s>\n", szTag);
  psSettingsData->nIndent++;
}

void SettingsEndTag (SettingsPersist * psSettingsData, char const * szTag) {
  psSettingsData->nIndent--;
  PrintIn (psSettingsData->fhFile, psSettingsData->nIndent, "</%s>\n", szTag);
}

void SettingsText (SettingsPersist * psSettingsData, char const * szText) {
  gchar * szEscaped;
  szEscaped = g_markup_printf_escaped ("%s", szText);
  
  PrintIn (psSettingsData->fhFile, psSettingsData->nIndent, szEscaped);

  g_free (szEscaped);
}

void SettingsPrintIn (SettingsPersist * psSettingsData, char * szFormat, ...) {
  gchar * szEscaped;
  va_list sArgP;

  va_start (sArgP, szFormat);
  szEscaped = g_markup_vprintf_escaped (szFormat, sArgP);
  va_end (sArgP);

  PrintIn (psSettingsData->fhFile, psSettingsData->nIndent, szEscaped);
  
  g_free (szEscaped);
}

void SettingsPrintInt (SettingsPersist * psSettingsData, char const * szName, int nValue) {
  PrintIn (psSettingsData->fhFile, psSettingsData->nIndent, "<Property id=\"%s\" type=\"int\">%d</Property>\n", szName, nValue);
}

void SettingsPrintBool (SettingsPersist * psSettingsData, char const * szName, bool boValue) {
  PrintIn (psSettingsData->fhFile, psSettingsData->nIndent, "<Property id=\"%s\" type=\"bool\">%d</Property>\n", szName, boValue);
}

void SettingsPrintFloat (SettingsPersist * psSettingsData, char const * szName, float fValue) {
  PrintIn (psSettingsData->fhFile, psSettingsData->nIndent, "<Property id=\"%s\" type=\"float\">%f</Property>\n", szName, fValue);
}

void SettingsPrintString (SettingsPersist * psSettingsData, char const * szName, char const * szString) {
  gchar * szEscaped;

  szEscaped = g_markup_printf_escaped ("%s", szString);
  PrintIn (psSettingsData->fhFile, psSettingsData->nIndent, "<Property id=\"%s\" type=\"string\">%s</Property>\n", szName, szEscaped);

  g_free (szEscaped);
}

///////////////////////////////////////////////////////////////////
// Loading function definitions

bool SettingsLoad (SettingsPersist * psSettingsData) {
  bool boSuccess = FALSE;
  GMarkupParseContext * psParseContext;
  GMarkupParser * psParser;
  FILE * fhFile;
  gchar szFileBuffer[FILE_BUFFER_SIZE];
  int nRead;

  psSettingsData->nLevel = 0;

  psParser = g_new0 (GMarkupParser, 1);
  psParser->start_element = StartElement;
  psParser->end_element = EndElement;
  psParser->text = Text;
  psParser->passthrough = NULL;
  psParser->error = NULL;

  fhFile = fopen (psSettingsData->szFilename->str, "rb");
  if (fhFile) {
    boSuccess = TRUE;
    psParseContext = g_markup_parse_context_new (psParser, 0, (gpointer)psSettingsData, DestroyUserData);

    nRead = fread (szFileBuffer, sizeof (char), FILE_BUFFER_SIZE, fhFile);
    while (boSuccess && (nRead > 0)) {
      boSuccess = g_markup_parse_context_parse (psParseContext, szFileBuffer, nRead, NULL);
      nRead = fread (szFileBuffer, sizeof (char), FILE_BUFFER_SIZE, fhFile);
    }

    if (boSuccess) {
      g_markup_parse_context_end_parse (psParseContext, NULL);
    }
    g_markup_parse_context_free (psParseContext);
    fclose (fhFile);
  }
  g_free (psParser);
  // The context is destroyed automatically by the parser

  return boSuccess;
}

void StartElement (GMarkupParseContext * psContext, gchar const * szElementName, gchar const * * aszAttributeNames, gchar const * * aszAttributeValues, gpointer psData, GError * * ppsError) {
  SettingsPersist * psSettingsData = (SettingsPersist *)psData;
  int nAttribute;

  psSettingsData->nLevel++;

  g_string_set_size (psSettingsData->szText, 0);

  if ((psSettingsData->nLevel >= 0) && (psSettingsData->nLevel < MAX_LEVELS)) {
    if (stricmp (szElementName, "property") == 0) {
      // Check the attributes for a type value
      psSettingsData->eType = SETTINGTYPE_INVALID;
      nAttribute = 0;
      while (aszAttributeNames[nAttribute] != NULL) {
        if (stricmp (aszAttributeNames[nAttribute], "type") == 0) {
          if (stricmp (aszAttributeValues[nAttribute], "int") == 0) {
            psSettingsData->eType = SETTINGTYPE_INT;
          }
          else if (stricmp (aszAttributeValues[nAttribute], "bool") == 0) {
            psSettingsData->eType = SETTINGTYPE_BOOL;
          }
          else if (stricmp (aszAttributeValues[nAttribute], "float") == 0) {
            psSettingsData->eType = SETTINGTYPE_FLOAT;
          }
          else if (stricmp (aszAttributeValues[nAttribute], "string") == 0) {
            psSettingsData->eType = SETTINGTYPE_STRING;
          }
        }
        else if (stricmp (aszAttributeNames[nAttribute], "id") == 0) {
          g_string_assign (psSettingsData->szID, aszAttributeValues[nAttribute]);
        }
        nAttribute++;
      }
    }
    else if (stricmp (szElementName, psSettingsData->szName->str) == 0) {
      // Start of the file
      psSettingsData->nLevel = -1;
    }
    else {
      // Section start
      if (psSettingsData->psParser && psSettingsData->psParser->LoadSectionStart) {
        psSettingsData->psParser->LoadSectionStart (szElementName, psSettingsData->psParser->psData, psSettingsData);
      }
    }
  }
}

void EndElement (GMarkupParseContext * psContext, gchar const * szElementName, gpointer psData, GError * * ppsError) {
  SettingsPersist * psSettingsData = (SettingsPersist *)psData;
  void * psValue;
  int nValue;
  bool boValue;
  float fValue;

  if ((psSettingsData->nLevel >= 0) && (psSettingsData->nLevel < MAX_LEVELS)) {
    if (stricmp (szElementName, "property") == 0) {
      psValue = NULL;
      // Property
      if (psSettingsData->psParser && psSettingsData->psParser->LoadProperty) {
        switch (psSettingsData->eType) {
        case SETTINGTYPE_INT:
          sscanf (psSettingsData->szText->str, "%d", & nValue);
          psValue = & nValue;
          break;
        case SETTINGTYPE_BOOL:
          sscanf (psSettingsData->szText->str, "%d", & nValue);
          boValue = nValue;
          psValue = & boValue;
          break;
        case SETTINGTYPE_FLOAT:
          sscanf (psSettingsData->szText->str, "%f", & fValue);
          psValue = & fValue;
          break;
        case SETTINGTYPE_STRING:
          psValue = psSettingsData->szText->str;
          break;
        default:
          // Do nothing;
          break;
        }
        psSettingsData->psParser->LoadProperty (psSettingsData->eType, psSettingsData->szID->str, psValue, psSettingsData->psParser->psData, psSettingsData);
      }
    }
    else if (stricmp (szElementName, psSettingsData->szName->str) == 0) {
      // End of the file
      psSettingsData->nLevel = -1;
    }
    else {
      // Section end
      if (psSettingsData->psParser && psSettingsData->psParser->LoadSectionEnd) {
        psSettingsData->psParser->LoadSectionEnd (szElementName, psSettingsData->psParser->psData, psSettingsData);
      }
    }
  }
  psSettingsData->nLevel--;

  g_string_set_size (psSettingsData->szText, 0);
}

void Text (GMarkupParseContext * pContext, gchar const * szText, gsize nTextLen, gpointer psData, GError * * ppsError) {
  SettingsPersist * psSettingsData = (SettingsPersist *)psData;

  g_string_append_len (psSettingsData->szText, szText, nTextLen);
}

void DestroyUserData (gpointer psData) {
  //SettingsPersist * psSettingsData = (SettingsPersist *)psData;
  
  // This will be deleted by the program later  
  //DeleteSettingsPersist (psSettingsData);
}

void AddParser (SettingsLoadParser * psParser, SettingsPersist * psSettingsData) {
  // Add a parser to the top of the stack
  psSettingsData->psParsers = g_slist_prepend (psSettingsData->psParsers, (gpointer)psParser);
  psSettingsData->psParser = psParser;
}

void RemoveParser (SettingsPersist * psSettingsData) {
  // Remove the head of the list
  psSettingsData->psParsers = g_slist_delete_link (psSettingsData->psParsers, psSettingsData->psParsers);
  // Set the parser to the new head of the list
  if (psSettingsData->psParsers) {
    psSettingsData->psParser = (SettingsLoadParser *)psSettingsData->psParsers->data;
  }
  else {
    psSettingsData->psParser = NULL;
  }
}

SettingsLoadParser * GetParser (SettingsPersist * psSettingsData) {
  return psSettingsData->psParser;
}


