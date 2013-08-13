///////////////////////////////////////////////////////////////////
// FloatNote
// Display text arranged using forces
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Autumn 2010
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <glib.h>

#include "floatnote.h"
#include "vis.h"
#include "textures.h"

///////////////////////////////////////////////////////////////////
// Defines

#define NOTETEXT_FONT       (GLUT_BITMAP_HELVETICA_10)
#define NOTETEXT_LINEHEIGHT (10)
#define NOTE_BORDERINT (4.0f)
#define NOTE_BORDEREXT (10.0f)
#define NOTE_PICSIZE (24.0f/32.0f)
#define NOTE_BORDERWIDTH (8.0f)
#define NOTE_BACKFADEMAX (0.5f)
#define NOTE_TEXTFADEMAX (0.7f)
#define NOTE_XSHIFT (16.0f)
#define NOTE_YSHIFT (16.0f)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

struct _NotesPersist {
	GSList * psNoteList;
	TexPersist * psTexData;
	bool boDisplay;
	float fTethering;
	float fExpulsion;
	FloatNote * psSelectedNote;
	bool boInverted;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

void RenderNote (FloatNote * psNote, NotesPersist * psNotesData);
void RenderNoteAnchor (FloatNote * psNote, NotesPersist * psNotesData);
void RenderNoteCallback (gpointer psData, gpointer psUserData);
FloatNote * NewFloatNote (float fXAnchor, float fYAnchor, char * szText);
void RenderNoteAnchorCallback (gpointer psData, gpointer psUserData);
void ApplyNoteExpulsion (FloatNote * psNote, NotesPersist * psNotesData);
void ApplyTethering (FloatNote * psNote, NotesPersist * psNotesData);
void MoveAnchor (float fX, float fY, NotesPersist * psNotesData);
void DeleteFloatNote (FloatNote * psNote);
void RenderBox (float fX1, float fY1, float fX2, float fY2, float fBorder, GLuint uTexture);
void DeleteNoteCallback (gpointer psData, gpointer psUserData);

///////////////////////////////////////////////////////////////////
// Function definitions

NotesPersist * NewNotesPersist (TexPersist * psTexData) {
  NotesPersist * psNotesData;

  psNotesData = g_new0 (NotesPersist, 1);
  psNotesData->psNoteList = NULL;
	psNotesData->psTexData = psTexData;
	psNotesData->boDisplay = false;
	psNotesData->fTethering = 0.1f;
	psNotesData->fExpulsion = 1.0f;
	psNotesData->psSelectedNote = NULL;
	psNotesData->boInverted = FALSE;

  return psNotesData;
}

void DeleteNotesPersist (NotesPersist * psNotesData) {
	// Free all of the notes in the note list
	RemoveAllNotes (psNotesData);

  g_free (psNotesData);
}

void RemoveAllNotes (NotesPersist * psNotesData) {
	// Note that this doesn't delete links to notes from nodes and links
	// This needs to be done separately
  g_slist_foreach (psNotesData->psNoteList, DeleteNoteCallback, psNotesData);
  g_slist_free (psNotesData->psNoteList);
  psNotesData->psNoteList = NULL;
}

void DeleteNoteCallback (gpointer psData, gpointer psUserData) {
	FloatNote * psNote = (FloatNote *)psData;
	//NotesPersist * psNotesData = (NotesPersist *)psUserData;

	DeleteFloatNote (psNote);
}

void RenderNote (FloatNote * psNote, NotesPersist * psNotesData) {
	// Render the note to the screen
	float fFade;
	float fLength;
	GLuint uTexture;
	GLfloat afVertices[6];

	if (psNote->szText->len > 0) {
		// Draw a transparent rectangle on the screen
		fLength = ((psNote->vsPos.fX + psNote->fMarginExternal + NOTE_XSHIFT - psNote->vsAnchor.fX)
			* (psNote->vsPos.fX + psNote->fMarginExternal + NOTE_XSHIFT - psNote->vsAnchor.fX)) 
			+ ((psNote->vsPos.fY + psNote->fMarginExternal + NOTE_YSHIFT - psNote->vsAnchor.fY)
			* (psNote->vsPos.fY + psNote->fMarginExternal + NOTE_YSHIFT - psNote->vsAnchor.fY));
		if (fLength < (200.0f * 200.0f)) {
			fFade = (1.0f - ((psNote->vsAnchor.fZ - 0.95) * 60.0f));
			if (psNote == psNotesData->psSelectedNote) {
				fFade = (1.0f / NOTE_BACKFADEMAX);
			}
			if ((fFade > 0.0f) && (fFade < 50.0f)) {
				if ((fFade > 1.0f) && (psNote != psNotesData->psSelectedNote)) {
					fFade = 1.0f;
				}
				glColor4f (1.0, 1.0, 1.0, fFade * NOTE_BACKFADEMAX);

				// Render the floating note
				uTexture = GetTexture (TEXNAME_NOTES, NULL, psNotesData->psTexData);
				glBindTexture (GL_TEXTURE_2D, uTexture);
				glEnable (GL_TEXTURE_2D);
				glEnableClientState (GL_TEXTURE_COORD_ARRAY);
				RenderBox (psNote->vsPos.fX + psNote->fMarginExternal + NOTE_XSHIFT, psNote->vsPos.fY + psNote->fMarginExternal + NOTE_YSHIFT, psNote->vsPos.fX + psNote->vsSize.fX - psNote->fMarginExternal + NOTE_XSHIFT, psNote->vsPos.fY + psNote->vsSize.fY - psNote->fMarginExternal + NOTE_YSHIFT, NOTE_BORDERWIDTH, uTexture);

				afVertices[0]  = psNote->vsAnchor.fX;
				afVertices[1]  = psNote->vsAnchor.fY;
				afVertices[2]  = 0.0f;
				afVertices[3]  = psNote->vsPos.fX + psNote->fMarginExternal + NOTE_XSHIFT;
				afVertices[4]  = psNote->vsPos.fY + psNote->fMarginExternal + NOTE_YSHIFT;
				afVertices[5]  = 0.0f;

				glDisable (GL_TEXTURE_2D);
				glDisableClientState (GL_TEXTURE_COORD_ARRAY);
				glColor4f (0.5, 0.5, 1.0, fFade * 0.7f);
				glVertexPointer (3, GL_FLOAT, 0, afVertices);
				glDrawArrays (GL_LINE_STRIP, 0, 2);

				if (psNotesData->boInverted) {
					glColor4f (0.0, 0.0, 0.0, fFade * NOTE_TEXTFADEMAX);
				}
				else {
					glColor4f (1.0, 1.0, 1.0, fFade * NOTE_TEXTFADEMAX);
				}
				RenderBitmapString (psNote->vsPos.fX + psNote->fMarginInternal + psNote->fMarginExternal + NOTE_XSHIFT, psNote->vsPos.fY + psNote->vsSize.fY - psNote->fMarginExternal - psNote->fMarginInternal + NOTE_YSHIFT - NOTETEXT_LINEHEIGHT + 2.0f, TEXT_LAYER_ZPOS, NOTETEXT_FONT, psNote->szText->str);
			}
		}
	}
}

void RenderNoteAnchor (FloatNote * psNote, NotesPersist * psNotesData) {
	// Render the note to the screen
	float fFade;
	GLuint uTexture;
	GLfloat afVertices[6];

	if (psNote->szText->len > 0) {
		// Draw a transparent rectangle on the screen
		fFade = (1.0f - ((psNote->vsAnchor.fZ - 0.95) * 60.0f));
		if (psNote == psNotesData->psSelectedNote) {
			fFade = (1.0f / NOTE_BACKFADEMAX);
		}
		if ((fFade > 0.0f) && (fFade < 50.0f)) {
			if ((fFade > 1.0f) && (psNote != psNotesData->psSelectedNote)) {
				fFade = 1.0f;
			}
			glColor4f (1.0, 1.0, 1.0, fFade * NOTE_BACKFADEMAX);

			// Render the floating note
			uTexture = GetTexture (TEXNAME_NOTES, NULL, psNotesData->psTexData);
			glBindTexture (GL_TEXTURE_2D, uTexture);
			glEnable (GL_TEXTURE_2D);
		  glEnableClientState (GL_TEXTURE_COORD_ARRAY);
			RenderBox (psNote->vsAnchor.fX + psNote->fMarginExternal + NOTE_XSHIFT, psNote->vsAnchor.fY + psNote->fMarginExternal + NOTE_YSHIFT, psNote->vsAnchor.fX + psNote->vsSize.fX - psNote->fMarginExternal + NOTE_XSHIFT, psNote->vsAnchor.fY + psNote->vsSize.fY - psNote->fMarginExternal + NOTE_YSHIFT, NOTE_BORDERWIDTH, uTexture);

			afVertices[0]  = psNote->vsAnchor.fX;
			afVertices[1]  = psNote->vsAnchor.fY;
			afVertices[2]  = 0.0f;
			afVertices[3]  = psNote->vsAnchor.fX + psNote->fMarginExternal + NOTE_XSHIFT;
			afVertices[4]  = psNote->vsAnchor.fY + psNote->fMarginExternal + NOTE_YSHIFT;
			afVertices[5]  = 0.0f;

			glDisable (GL_TEXTURE_2D);
		  glDisableClientState (GL_TEXTURE_COORD_ARRAY);
			glColor4f (0.5, 0.5, 1.0, fFade * NOTE_BACKFADEMAX);
			glVertexPointer (3, GL_FLOAT, 0, afVertices);
			glDrawArrays (GL_LINE_STRIP, 0, 2);

			if (psNotesData->boInverted) {
				glColor4f (0.0, 0.0, 0.0, fFade * NOTE_TEXTFADEMAX);
			}
			else {
				glColor4f (1.0, 1.0, 1.0, fFade * NOTE_TEXTFADEMAX);
			}
			RenderBitmapString (psNote->vsAnchor.fX + psNote->fMarginInternal + psNote->fMarginExternal + NOTE_XSHIFT, psNote->vsAnchor.fY + psNote->vsSize.fY - psNote->fMarginExternal - psNote->fMarginInternal + NOTE_YSHIFT - NOTETEXT_LINEHEIGHT + 2.0f, TEXT_LAYER_ZPOS, NOTETEXT_FONT, psNote->szText->str);
		}
	}
}

void RenderBox (float fX1, float fY1, float fX2, float fY2, float fBorder, GLuint uTexture) {
	GLfloat afVertices[(16 * 3)];
	GLfloat afTexCoords[(16 * 2)];
	GLubyte auIndices[(6 * 9)] = {
		0, 1, 4, 4, 1, 5, 
		1, 2, 5, 5, 2, 6, 
		2, 3, 6, 6, 3, 7, 
		4, 5, 8, 8, 5, 9, 
		5, 6, 9, 9, 6, 10, 
		6, 7, 10, 10, 7, 11, 
		8, 9, 12, 12, 9, 13, 
		9, 10, 13, 13, 10, 14, 
		10, 11, 14, 14, 11, 15};

	// Line fY1
	afVertices[0]  = fX1;
	afVertices[1]  = fY1;
	afVertices[2]  = 0.0f;

	afVertices[3]  = fX1 + fBorder;
	afVertices[4]  = fY1;
	afVertices[5]  = 0.0f;

	afVertices[6]  = fX2 - fBorder;
	afVertices[7]  = fY1;
	afVertices[8]  = 0.0f;

	afVertices[9]  = fX2;
	afVertices[10] = fY1;
	afVertices[11] = 0.0f;

	// Line fY1 + fBorder
	afVertices[12] = fX1;
	afVertices[13] = fY1 + fBorder;
	afVertices[14] = 0.0f;

	afVertices[15] = fX1 + fBorder;
	afVertices[16] = fY1 + fBorder;
	afVertices[17] = 0.0f;

	afVertices[18] = fX2 - fBorder;
	afVertices[19] = fY1 + fBorder;
	afVertices[20] = 0.0f;

	afVertices[21] = fX2;
	afVertices[22] = fY1 + fBorder;
	afVertices[23] = 0.0f;

	// Line fY2 - fBorder
	afVertices[24] = fX1;
	afVertices[25] = fY2 - fBorder;
	afVertices[26] = 0.0f;

	afVertices[27] = fX1 + fBorder;
	afVertices[28] = fY2 - fBorder;
	afVertices[29] = 0.0f;

	afVertices[30] = fX2 - fBorder;
	afVertices[31] = fY2 - fBorder;
	afVertices[32] = 0.0f;

	afVertices[33] = fX2;
	afVertices[34] = fY2 - fBorder;
	afVertices[35] = 0.0f;

	// Line fY2
	afVertices[36] = fX1;
	afVertices[37] = fY2;
	afVertices[38] = 0.0f;

	afVertices[39] = fX1 + fBorder;
	afVertices[40] = fY2;
	afVertices[41] = 0.0f;

	afVertices[42] = fX2 - fBorder;
	afVertices[43] = fY2;
	afVertices[44] = 0.0f;

	afVertices[45] = fX2;
	afVertices[46] = fY2;
	afVertices[47] = 0.0f;

	// Texture line fY1
	afTexCoords[0]  = 0.0f;
	afTexCoords[1]  = (3.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[2]  = (1.0f/3.0f) * NOTE_PICSIZE;
	afTexCoords[3]  = (3.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[4]  = (2.0f/3.0f) * NOTE_PICSIZE;
	afTexCoords[5]  = (3.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[6]  = 1.0f * NOTE_PICSIZE;
	afTexCoords[7]  = (3.0f/3.0f) * NOTE_PICSIZE;

	// Texture line fY1 + fBorder
	afTexCoords[8]  = 0.0f;
	afTexCoords[9]  = (2.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[10] = (1.0f/3.0f) * NOTE_PICSIZE;
	afTexCoords[11] = (2.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[12] = (2.0f/3.0f) * NOTE_PICSIZE;
	afTexCoords[13] = (2.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[14] = 1.0f * NOTE_PICSIZE;
	afTexCoords[15] = (2.0f/3.0f) * NOTE_PICSIZE;

	// Texture line fY2 - fBorder
	afTexCoords[16] = 0.0f;
	afTexCoords[17] = (1.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[18] = (1.0f/3.0f) * NOTE_PICSIZE;
	afTexCoords[19] = (1.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[20] = (2.0f/3.0f) * NOTE_PICSIZE;
	afTexCoords[21] = (1.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[22] = 1.0f * NOTE_PICSIZE;
	afTexCoords[23] = (1.0f/3.0f) * NOTE_PICSIZE;

	// Texture line fY2
	afTexCoords[24] = 0.0f;
	afTexCoords[25] = (0.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[26] = (1.0f/3.0f) * NOTE_PICSIZE;
	afTexCoords[27] = (0.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[28] = (2.0f/3.0f) * NOTE_PICSIZE;
	afTexCoords[29] = (0.0f/3.0f) * NOTE_PICSIZE;

	afTexCoords[30] = 1.0f * NOTE_PICSIZE;
	afTexCoords[31] = (0.0f/3.0f) * NOTE_PICSIZE;

	glBindTexture (GL_TEXTURE_2D, uTexture);
	glVertexPointer (3, GL_FLOAT, 0, afVertices);
	glTexCoordPointer (2, GL_FLOAT, 0, afTexCoords);
	glDrawElements (GL_TRIANGLES, (6 * 9), GL_UNSIGNED_BYTE, auIndices);
}

void RenderNoteCallback (gpointer psData, gpointer psUserData) {
	FloatNote * psNote = (FloatNote *)psData;
	NotesPersist * psNotesData = (NotesPersist *)psUserData;

	RenderNote (psNote, psNotesData);
}

void RenderNoteAnchorCallback (gpointer psData, gpointer psUserData) {
	FloatNote * psNote = (FloatNote *)psData;
	NotesPersist * psNotesData = (NotesPersist *)psUserData;

	RenderNoteAnchor (psNote, psNotesData);
}

void RenderNotes (NotesPersist * psNotesData) {
  GLint anViewPort[4];

	// Create a 2D overlay layer
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();

	glGetIntegerv (GL_VIEWPORT, anViewPort);
	glOrtho (anViewPort[0], anViewPort[2], anViewPort[1], anViewPort[3], 0, 1);

	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();
	glDisable (GL_DEPTH_TEST);
	glDepthMask (GL_FALSE);
	glDisable (GL_LIGHTING);

	// Allow transparency
	glEnable (GL_BLEND);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Set up the texture
	glEnable (GL_TEXTURE_2D);

	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_COLOR_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glEnableClientState  (GL_VERTEX_ARRAY);

	if (psNotesData->boDisplay) {
		// Render all of the notes to the screen
		if ((psNotesData->fTethering >= 1.0f) && (psNotesData->fExpulsion <= 0.0f)) {
			g_slist_foreach (psNotesData->psNoteList, RenderNoteAnchorCallback, (gpointer)psNotesData);
		}
		else {
			g_slist_foreach (psNotesData->psNoteList, RenderNoteCallback, (gpointer)psNotesData);
		}
	}
	else {
		if (psNotesData->psSelectedNote) {
			RenderNoteAnchor (psNotesData->psSelectedNote, psNotesData);
		}
	}

	glDisable (GL_TEXTURE_2D);

	// Return to 3D
	glDisable (GL_BLEND);
	glPopMatrix ();
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glEnable (GL_LIGHTING);
	glEnable (GL_DEPTH_TEST);
	glDepthMask (GL_TRUE);
}

float CalculateOverlap (float fStart1, float fLength1, float fStart2, float fLength2) {
	float fOverlap;
	float fEnd1;
	float fEnd2;

	fEnd1 = fStart1 + fLength1;
	fEnd2 = fStart2 + fLength2;
	fOverlap = 0.0f;
	if (fStart1 <= fStart2) {
		if (fEnd1 > fStart2) {
			if (fEnd1 > fEnd2) {
				fOverlap = fLength2;
			}
			else {
				fOverlap = fEnd1 - fStart2;
			}
		}
	}
	else {
		if (fEnd2 > fStart1) {
			if (fEnd1 < fEnd2) {
				fOverlap = fLength1;
			}
			else {
				fOverlap = fEnd2 - fStart1;
			}
		}
	}

	return fOverlap;
}

void ApplyNoteExpulsion (FloatNote * psNote, NotesPersist * psNotesData) {
  float fXForce, fYForce;
  FloatNote * psNoteCount;
  GSList * psNoteListPos;
	float fCoverage;
	float fXDirection, fYDirection, fLength;
	float fFade;

  fXForce = 0.0f;
  fYForce = 0.0f;

  psNoteListPos = psNotesData->psNoteList;
  while (psNoteListPos) {
    psNoteCount = (FloatNote *)psNoteListPos->data;

    if (psNoteCount != psNote) {
    	// Calculate fade
		  fFade = (1.0f - ((psNoteCount->vsAnchor.fZ - 0.95) * 60.0f));

			if ((psNoteCount->szText->len > 0) && (fFade > 0.0f) && (fFade < 50.0f))  {
				// Calculate coverage
				fCoverage = CalculateOverlap (psNote->vsPos.fX, psNote->vsSize.fX, psNoteCount->vsPos.fX, psNoteCount->vsSize.fX) * CalculateOverlap (psNote->vsPos.fY, psNote->vsSize.fY, psNoteCount->vsPos.fY, psNoteCount->vsSize.fY);
				if (fCoverage > 0.0f) {
					fXDirection = ((psNote->vsPos.fX + (psNote->vsSize.fX / 2.0f)) - (psNoteCount->vsPos.fX + (psNoteCount->vsSize.fX / 2.0f)));
					fYDirection = ((psNote->vsPos.fY + (psNote->vsSize.fY / 2.0f)) - (psNoteCount->vsPos.fY + (psNoteCount->vsSize.fY / 2.0f)));
					fLength = sqrt ((fXDirection * fXDirection) + (fYDirection * fYDirection));
					fXForce += psNotesData->fExpulsion * (40.0f + fCoverage) * fXDirection * fFade / fLength;
					fYForce += psNotesData->fExpulsion * (40.0f + fCoverage) * fYDirection * fFade / fLength;
				}
			}
    }
    psNoteListPos = psNoteListPos->next;
  }

  psNote->vsVel.fX += 0.01f * fXForce;
  psNote->vsVel.fY += 0.01f * fYForce;
}

void ApplyTethering (FloatNote * psNote, NotesPersist * psNotesData) {
	psNote->vsVel.fX -=  psNotesData->fTethering * (psNote->vsPos.fX - psNote->vsAnchor.fX);
	psNote->vsVel.fY -=  psNotesData->fTethering * (psNote->vsPos.fY - psNote->vsAnchor.fY);
}

void UpdateNotes (NotesPersist * psNotesData) {
	// Float all of the notes around each other
  FloatNote * psNote;
  GSList * psNoteListPos;

	if ((psNotesData->fTethering >= 1.0f) && (psNotesData->fExpulsion <= 0.0f)) {
		psNoteListPos = psNotesData->psNoteList;
		while (psNoteListPos) {
		  psNote = (FloatNote *)psNoteListPos->data;

		  // Update position to match the anchor
		  psNote->vsPos.fX = psNote->vsAnchor.fX;
		  psNote->vsPos.fY = psNote->vsAnchor.fY;

		  psNoteListPos = psNoteListPos->next;
		}
	}
	else {
		psNoteListPos = psNotesData->psNoteList;
		while (psNoteListPos) {
		  psNote = (FloatNote *)psNoteListPos->data;

		  // Apply the arrangement forces
		  if (psNote->szText->len > 0) {
				ApplyNoteExpulsion (psNote, psNotesData);
			}

			// Apply resistance
		  psNote->vsVel.fX *= 0.4f;
		  psNote->vsVel.fY *= 0.4f;

			// Apply tethering using ancors
			ApplyTethering (psNote, psNotesData);

		  // Update position based on velocity
		  psNote->vsPos.fX += psNote->vsVel.fX;
		  psNote->vsPos.fY += psNote->vsVel.fY;

		  psNoteListPos = psNoteListPos->next;
		}
	}
}

FloatNote * NewFloatNote (float fXAnchor, float fYAnchor, char * szText) {
  FloatNote * psNote;

  psNote = g_new0 (FloatNote, 1);

  psNote->vsAnchor.fX = fXAnchor;
  psNote->vsAnchor.fY = fYAnchor;
  psNote->vsAnchor.fZ = 0.0f;
  psNote->vsPos.fX = 20.0f * ((float)rand () / (float)RAND_MAX);
  psNote->vsPos.fY = 20.0f * ((float)rand () / (float)RAND_MAX);
  psNote->vsPos.fZ = 0.0f;
  psNote->vsVel.fX = 0.0f;
  psNote->vsVel.fY = 0.0f;
  psNote->vsVel.fZ = 0.0f;
  psNote->fMarginInternal = NOTE_BORDERINT;
  psNote->fMarginExternal = NOTE_BORDEREXT;
  psNote->szText = NULL;
	SetNoteText (psNote, szText);
  
  return psNote;
}

void DeleteFloatNote (FloatNote * psNote) {
  g_string_free (psNote->szText, TRUE);

  g_free (psNote);
}

FloatNote * AddFloatNote (float fXAnchor, float fYAnchor, char * szText, NotesPersist * psNotesData) {
  FloatNote * psNoteNew;

	psNoteNew = NewFloatNote (fXAnchor, fYAnchor, szText);
  psNotesData->psNoteList = g_slist_prepend (psNotesData->psNoteList, (gpointer)psNoteNew);
  
  return psNoteNew;
}

void RemoveFloatNote (FloatNote * psNote, NotesPersist * psNotesData) {
	psNotesData->psNoteList = g_slist_remove (psNotesData->psNoteList, (gconstpointer)psNote);

	DeleteFloatNote (psNote);
}

void MoveAnchor (float fX, float fY, NotesPersist * psNotesData) {
  FloatNote * psNote = (FloatNote *)psNotesData->psNoteList->data;
  
  psNote->vsAnchor.fX = fX;
  psNote->vsAnchor.fY = fY;
}

void SetNoteTethering (float fTethering, NotesPersist * psNotesData) {
	psNotesData->fTethering = fTethering;
	psNotesData->fExpulsion = (1.0f - ((fTethering - 0.1f) / 0.9f));
}

float GetNoteTethering (NotesPersist * psNotesData) {
	return psNotesData->fTethering;
}

void SetNoteDisplay (bool boDisplay, NotesPersist * psNotesData) {
	psNotesData->boDisplay = boDisplay;
}

bool GetNoteDisplay (NotesPersist * psNotesData) {
	return psNotesData->boDisplay;
}

void ToggleNoteDisplay (NotesPersist * psNotesData) {
	psNotesData->boDisplay = !psNotesData->boDisplay;
}

void SetNoteInverted (bool boInverted, NotesPersist * psNotesData) {
	psNotesData->boInverted = boInverted;
}

bool GetNoteInverted (NotesPersist * psNotesData) {
	return psNotesData->boInverted;
}

void ToggleNoteInverted (NotesPersist * psNotesData) {
	psNotesData->boInverted = !psNotesData->boInverted;
}

void SetNoteSelected (FloatNote * psSelectedNote, NotesPersist * psNotesData) {
	psNotesData->psSelectedNote = psSelectedNote;
}

void SetNoteText (FloatNote * psNote, char const * szText) {
  int nWidth;
  int nPos;

	if (psNote && szText) {
		psNote->vsSize.fX = 0.0f;
		psNote->vsSize.fY = 0.0f;
		psNote->vsSize.fZ = 0.0f;
		if (psNote->szText != NULL) {
			g_string_assign (psNote->szText, szText);
		}
		else {
			psNote->szText = g_string_new (szText);
		}
  
		nWidth = glutBitmapLength (NOTETEXT_FONT, (char unsigned *)szText);
		psNote->vsSize.fX = (float)nWidth + (2.0f * (psNote->fMarginInternal + psNote->fMarginExternal));

		// Figure out the height of the string
		nPos = 0;
		psNote->vsSize.fY = NOTETEXT_LINEHEIGHT + (2.0f * (psNote->fMarginInternal + psNote->fMarginExternal));
		while (szText[nPos] > 0) {
			if (szText[nPos] == '\n') {
				psNote->vsSize.fY += NOTETEXT_LINEHEIGHT;
			}
			nPos++;
		}
	}
}

