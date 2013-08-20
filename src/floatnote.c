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
#define NOTE_LAYER_ZPOS	(0.0f)


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
	float fFocusFar;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

void RenderNote (FloatNote * psNote, NotesPersist * psNotesData);
void RenderNoteAnchor (FloatNote * psNote, NotesPersist * psNotesData);
void RenderNoteCallback (gpointer psData, gpointer psUserData);
FloatNote * NewFloatNote (Vector3 const * pvsAnchor, char * szText);
void RenderNoteAnchorCallback (gpointer psData, gpointer psUserData);
void ApplyNoteExpulsion (FloatNote * psNote, NotesPersist * psNotesData);
void ApplyTethering (FloatNote * psNote, NotesPersist * psNotesData);
void MoveAnchor (float fX, float fY, NotesPersist * psNotesData);
void DeleteFloatNote (FloatNote * psNote);
void RenderBox (Vector3 const * pvsPos1, Vector3 const * pvsPos2, float fBorder, GLuint uTexture);
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
	psNotesData->fFocusFar = 0.95f;

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
	Vector3 vsBoxTopLeft;
	Vector3 vsBoxBottomRight;

	if (psNote->szText->len > 0) {
		// Draw a transparent rectangle on the screen
		fLength = ((psNote->vsPos.fX + psNote->fMarginExternal + NOTE_XSHIFT - psNote->vsAnchor.fX)
			* (psNote->vsPos.fX + psNote->fMarginExternal + NOTE_XSHIFT - psNote->vsAnchor.fX)) 
			+ ((psNote->vsPos.fY + psNote->fMarginExternal + NOTE_YSHIFT - psNote->vsAnchor.fY)
			* (psNote->vsPos.fY + psNote->fMarginExternal + NOTE_YSHIFT - psNote->vsAnchor.fY));
		if (fLength < (200.0f * 200.0f)) {
			fFade = (1.0f - ((psNote->vsAnchor.fZ - psNotesData->fFocusFar) * 60.0f));
			if (psNote == psNotesData->psSelectedNote) {
				fFade = (1.0f / NOTE_BACKFADEMAX);
			}
			if ((fFade > 0.0f) && (fFade < 50.0f)) {
				if ((fFade > 1.0f) && (psNote != psNotesData->psSelectedNote)) {
					fFade = 1.0f;
				}
				glColor4f (1.0, 1.0, 1.0, fFade * NOTE_BACKFADEMAX);

				// Set the z position of the note
			  glDepthRange (psNote->vsAnchor.fZ, psNote->vsAnchor.fZ);

				// Render the floating note
				uTexture = GetTexture (TEXNAME_NOTES, NULL, psNotesData->psTexData);
				glBindTexture (GL_TEXTURE_2D, uTexture);
				glEnable (GL_TEXTURE_2D);
				glEnableClientState (GL_TEXTURE_COORD_ARRAY);

				vsBoxTopLeft.fX = psNote->vsPos.fX + psNote->fMarginExternal + NOTE_XSHIFT;
				vsBoxTopLeft.fY = psNote->vsPos.fY + psNote->fMarginExternal + NOTE_YSHIFT;
				vsBoxTopLeft.fZ = psNote->vsPos.fZ;
				vsBoxBottomRight.fX = psNote->vsPos.fX + psNote->vsSize.fX - psNote->fMarginExternal + NOTE_XSHIFT;
				vsBoxBottomRight.fY = psNote->vsPos.fY + psNote->vsSize.fY - psNote->fMarginExternal + NOTE_YSHIFT;
				vsBoxBottomRight.fZ = psNote->vsPos.fZ;
				RenderBox (& vsBoxTopLeft, & vsBoxBottomRight, NOTE_BORDERWIDTH, uTexture);

				afVertices[0]  = psNote->vsAnchor.fX;
				afVertices[1]  = psNote->vsAnchor.fY;
				afVertices[2]  = NOTE_LAYER_ZPOS;
				afVertices[3]  = psNote->vsPos.fX + psNote->fMarginExternal + NOTE_XSHIFT;
				afVertices[4]  = psNote->vsPos.fY + psNote->fMarginExternal + NOTE_YSHIFT;
				afVertices[5]  = NOTE_LAYER_ZPOS;

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
			  glDepthRange (0.0, 1.0);

				RenderBitmapString (psNote->vsPos.fX + psNote->fMarginInternal + psNote->fMarginExternal + NOTE_XSHIFT, psNote->vsPos.fY + psNote->vsSize.fY - psNote->fMarginExternal - psNote->fMarginInternal + NOTE_YSHIFT - NOTETEXT_LINEHEIGHT + 2.0f, psNote->vsAnchor.fZ, NOTETEXT_FONT, psNote->szText->str);
			}
		}
	}
}

void RenderNoteAnchor (FloatNote * psNote, NotesPersist * psNotesData) {
	// Render the note to the screen
	float fFade;
	GLuint uTexture;
	GLfloat afVertices[6];
	Vector3 vsBoxTopLeft;
	Vector3 vsBoxBottomRight;

	if (psNote->szText->len > 0) {
		// Draw a transparent rectangle on the screen
		fFade = (1.0f - ((psNote->vsAnchor.fZ - psNotesData->fFocusFar) * 60.0f));
		if (psNote == psNotesData->psSelectedNote) {
			fFade = (1.0f / NOTE_BACKFADEMAX);
		}
		if ((fFade > 0.0f) && (fFade < 50.0f)) {
			if ((fFade > 1.0f) && (psNote != psNotesData->psSelectedNote)) {
				fFade = 1.0f;
			}
			glColor4f (1.0, 1.0, 1.0, fFade * NOTE_BACKFADEMAX);

			// Set the z position of the note
		  glDepthRange (psNote->vsAnchor.fZ, psNote->vsAnchor.fZ);

			// Render the floating note
			uTexture = GetTexture (TEXNAME_NOTES, NULL, psNotesData->psTexData);
			glBindTexture (GL_TEXTURE_2D, uTexture);
			glEnable (GL_TEXTURE_2D);
		  glEnableClientState (GL_TEXTURE_COORD_ARRAY);

			vsBoxTopLeft.fX = psNote->vsAnchor.fX + psNote->fMarginExternal + NOTE_XSHIFT;
			vsBoxTopLeft.fY = psNote->vsAnchor.fY + psNote->fMarginExternal + NOTE_YSHIFT;
			vsBoxTopLeft.fZ = psNote->vsAnchor.fZ;
			vsBoxBottomRight.fX = psNote->vsAnchor.fX + psNote->vsSize.fX - psNote->fMarginExternal + NOTE_XSHIFT;
			vsBoxBottomRight.fY = psNote->vsAnchor.fY + psNote->vsSize.fY - psNote->fMarginExternal + NOTE_YSHIFT;
			vsBoxBottomRight.fZ = psNote->vsAnchor.fZ;
			RenderBox (& vsBoxTopLeft, & vsBoxBottomRight, NOTE_BORDERWIDTH, uTexture);

			afVertices[0]  = psNote->vsAnchor.fX;
			afVertices[1]  = psNote->vsAnchor.fY;
			afVertices[2]  = NOTE_LAYER_ZPOS;
			afVertices[3]  = psNote->vsAnchor.fX + psNote->fMarginExternal + NOTE_XSHIFT;
			afVertices[4]  = psNote->vsAnchor.fY + psNote->fMarginExternal + NOTE_YSHIFT;
			afVertices[5]  = NOTE_LAYER_ZPOS;

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
		  glDepthRange (0.0, 1.0);

			RenderBitmapString (psNote->vsAnchor.fX + psNote->fMarginInternal + psNote->fMarginExternal + NOTE_XSHIFT, psNote->vsAnchor.fY + psNote->vsSize.fY - psNote->fMarginExternal - psNote->fMarginInternal + NOTE_YSHIFT - NOTETEXT_LINEHEIGHT + 2.0f, psNote->vsAnchor.fZ, NOTETEXT_FONT, psNote->szText->str);
		}
	}
}

void RenderBox (Vector3 const * pvsPos1, Vector3 const * pvsPos2, float fBorder, GLuint uTexture) {
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
	afVertices[0]  = pvsPos1->fX;
	afVertices[1]  = pvsPos1->fY;
	afVertices[2]  = NOTE_LAYER_ZPOS;

	afVertices[3]  = pvsPos1->fX + fBorder;
	afVertices[4]  = pvsPos1->fY;
	afVertices[5]  = NOTE_LAYER_ZPOS;

	afVertices[6]  = pvsPos2->fX - fBorder;
	afVertices[7]  = pvsPos1->fY;
	afVertices[8]  = NOTE_LAYER_ZPOS;

	afVertices[9]  = pvsPos2->fX;
	afVertices[10] = pvsPos1->fY;
	afVertices[11] = NOTE_LAYER_ZPOS;

	// Line fY1 + fBorder
	afVertices[12] = pvsPos1->fX;
	afVertices[13] = pvsPos1->fY + fBorder;
	afVertices[14] = NOTE_LAYER_ZPOS;

	afVertices[15] = pvsPos1->fX + fBorder;
	afVertices[16] = pvsPos1->fY + fBorder;
	afVertices[17] = NOTE_LAYER_ZPOS;

	afVertices[18] = pvsPos2->fX - fBorder;
	afVertices[19] = pvsPos1->fY + fBorder;
	afVertices[20] = NOTE_LAYER_ZPOS;

	afVertices[21] = pvsPos2->fX;
	afVertices[22] = pvsPos1->fY + fBorder;
	afVertices[23] = NOTE_LAYER_ZPOS;

	// Line fY2 - fBorder
	afVertices[24] = pvsPos1->fX;
	afVertices[25] = pvsPos2->fY - fBorder;
	afVertices[26] = NOTE_LAYER_ZPOS;

	afVertices[27] = pvsPos1->fX + fBorder;
	afVertices[28] = pvsPos2->fY - fBorder;
	afVertices[29] = NOTE_LAYER_ZPOS;

	afVertices[30] = pvsPos2->fX - fBorder;
	afVertices[31] = pvsPos2->fY - fBorder;
	afVertices[32] = NOTE_LAYER_ZPOS;

	afVertices[33] = pvsPos2->fX;
	afVertices[34] = pvsPos2->fY - fBorder;
	afVertices[35] = NOTE_LAYER_ZPOS;

	// Line fY2
	afVertices[36] = pvsPos1->fX;
	afVertices[37] = pvsPos2->fY;
	afVertices[38] = NOTE_LAYER_ZPOS;

	afVertices[39] = pvsPos1->fX + fBorder;
	afVertices[40] = pvsPos2->fY;
	afVertices[41] = NOTE_LAYER_ZPOS;

	afVertices[42] = pvsPos2->fX - fBorder;
	afVertices[43] = pvsPos2->fY;
	afVertices[44] = NOTE_LAYER_ZPOS;

	afVertices[45] = pvsPos2->fX;
	afVertices[46] = pvsPos2->fY;
	afVertices[47] = NOTE_LAYER_ZPOS;

	// Texture line fY1
	afTexCoords[0]  = 0.0f;
	afTexCoords[1]  = (3.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[2]  = (1.0f / 3.0f) * NOTE_PICSIZE;
	afTexCoords[3]  = (3.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[4]  = (2.0f / 3.0f) * NOTE_PICSIZE;
	afTexCoords[5]  = (3.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[6]  = 1.0f * NOTE_PICSIZE;
	afTexCoords[7]  = (3.0f / 3.0f) * NOTE_PICSIZE;

	// Texture line fY1 + fBorder
	afTexCoords[8]  = 0.0f;
	afTexCoords[9]  = (2.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[10] = (1.0f / 3.0f) * NOTE_PICSIZE;
	afTexCoords[11] = (2.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[12] = (2.0f / 3.0f) * NOTE_PICSIZE;
	afTexCoords[13] = (2.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[14] = 1.0f * NOTE_PICSIZE;
	afTexCoords[15] = (2.0f / 3.0f) * NOTE_PICSIZE;

	// Texture line fY2 - fBorder
	afTexCoords[16] = 0.0f;
	afTexCoords[17] = (1.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[18] = (1.0f / 3.0f) * NOTE_PICSIZE;
	afTexCoords[19] = (1.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[20] = (2.0f / 3.0f) * NOTE_PICSIZE;
	afTexCoords[21] = (1.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[22] = 1.0f * NOTE_PICSIZE;
	afTexCoords[23] = (1.0f / 3.0f) * NOTE_PICSIZE;

	// Texture line fY2
	afTexCoords[24] = 0.0f;
	afTexCoords[25] = (0.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[26] = (1.0f / 3.0f) * NOTE_PICSIZE;
	afTexCoords[27] = (0.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[28] = (2.0f / 3.0f) * NOTE_PICSIZE;
	afTexCoords[29] = (0.0f / 3.0f) * NOTE_PICSIZE;

	afTexCoords[30] = 1.0f * NOTE_PICSIZE;
	afTexCoords[31] = (0.0f / 3.0f) * NOTE_PICSIZE;

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
	glOrtho (anViewPort[0], anViewPort[2], anViewPort[1], anViewPort[3], 0.0, 100.0);

	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();
	//glDisable (GL_DEPTH_TEST);
  glDepthFunc (GL_ALWAYS);

	//glDepthMask (GL_FALSE);
	glDisable (GL_LIGHTING);

	// Allow transparency
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
  glDepthFunc (GL_LESS);
	//glEnable (GL_DEPTH_TEST);
	//glDepthMask (GL_TRUE);
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
		  fFade = (1.0f - ((psNoteCount->vsAnchor.fZ - psNotesData->fFocusFar) * 60.0f));

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
		  psNote->vsPos.fZ = psNote->vsAnchor.fZ;

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

			// Apply tethering using anchors
			ApplyTethering (psNote, psNotesData);

		  // Update position based on velocity
		  psNote->vsPos.fX += psNote->vsVel.fX;
		  psNote->vsPos.fY += psNote->vsVel.fY;
		  psNote->vsPos.fZ = psNote->vsAnchor.fZ;

		  psNoteListPos = psNoteListPos->next;
		}
	}
}

FloatNote * NewFloatNote (Vector3 const * pvsAnchor, char * szText) {
  FloatNote * psNote;

  psNote = g_new0 (FloatNote, 1);

  psNote->vsAnchor.fX = pvsAnchor->fX;
  psNote->vsAnchor.fY = pvsAnchor->fY;
  psNote->vsAnchor.fZ = pvsAnchor->fZ;
  psNote->vsPos.fX = 20.0f * ((float)rand () / (float)RAND_MAX);
  psNote->vsPos.fY = 20.0f * ((float)rand () / (float)RAND_MAX);
  psNote->vsPos.fZ = psNote->vsAnchor.fZ;
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

FloatNote * AddFloatNote (Vector3 const * pvsAnchor, char * szText, NotesPersist * psNotesData) {
  FloatNote * psNoteNew;

	psNoteNew = NewFloatNote (pvsAnchor, szText);
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

void SetNoteFocusFar (float fFocusFar, NotesPersist * psNotesData) {
	psNotesData->fFocusFar = fFocusFar;
}

