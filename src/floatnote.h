///////////////////////////////////////////////////////////////////
// FloatNote
// Display text arranged using forces
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Autumn 2010
///////////////////////////////////////////////////////////////////

#ifndef FLOATNOTE_H
#define FLOATNOTE_H

///////////////////////////////////////////////////////////////////
// Includes

#include "utils.h"
#include "settings.h"
#include "textures.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _NotesPersist NotesPersist;
typedef struct _FloatNote FloatNote;

struct _FloatNote {
  Vector3 vsAnchor;
  Vector3 vsPos;
  Vector3 vsVel;
	Vector3 vsSize;
	float fMarginInternal;
	float fMarginExternal;
	GString * szText;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

NotesPersist * NewNotesPersist (TexPersist * psTexData);
void DeleteNotesPersist (NotesPersist * psNotesData);

void RenderNotes (NotesPersist * psNotesData);
void UpdateNotes (NotesPersist * psNotesData);
FloatNote * AddFloatNote (Vector3 const * pvsAnchor, char * szText, NotesPersist * psNotesData);
void RemoveFloatNote (FloatNote * psNote, NotesPersist * psNotesData);
void SetNoteTethering (float fTethering, NotesPersist * psNotesData);
float GetNoteTethering (NotesPersist * psNotesData);
void SetNoteDisplay (bool boDisplay, NotesPersist * psNotesData);
bool GetNoteDisplay (NotesPersist * psNotesData);
void ToggleNoteDisplay (NotesPersist * psNotesData);
void SetNoteInverted (bool boInverted, NotesPersist * psNotesData);
bool GetNoteInverted (NotesPersist * psNotesData);
void ToggleNoteInverted (NotesPersist * psNotesData);
void SetNoteSelected (FloatNote * psSelectedNote, NotesPersist * psNotesData);
void SetNoteText (FloatNote * psNote, char const * szText);
void RemoveAllNotes (NotesPersist * psNotesData);
void SetNoteFocusFar (float fFocusFar, NotesPersist * psNotesData);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* FLOATNOTE_H */


