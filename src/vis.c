///////////////////////////////////////////////////////////////////
// Trustella Network
// Results log file animation viewer
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Autumn 2006
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

#include "vis.h"
#include "floatnote.h"
#include "textures.h"
#include "properties.h"

///////////////////////////////////////////////////////////////////
// Defines

#define true (1)
#define false (0)

#define NODERADIUS          (0.25)
#define NODESLICES          (10)
#define NODESTACKS          (10)

//#define TESTNODES           (128)
#define TESTNODES           (0)
#define TESTLINKS           (TESTNODES * 2)

#define ARROWLINKRADIUS     (0.05)
#define ARROWHEADRADIUS     (0.15)
#define ARROWHEADHEIGHT     (0.4)
#define ARROWMINLENGTH      (0.5)

#define ARROWHEADSLICES     (10)
#define ARROWHEADSTACKS     (1)
#define ARROWLINKSLICES     (10)
#define ARROWLINKSTACKS     (1)

#define MOUSE_ROTATE_SCALE  (100.0f)
#define RADIUSSTEP          (0.3f)
#define PSISTEP             (3.14159265f / 150.0f)
#define POINTTOHALFLIFE     (0.1f)
#define POINTTOMINMOVE      (0.002f)
#define POINTTOELLEVATION   (3.0f)

#define MGL_SCALE           (1.0f)
#define MGL_WIDTH           (0.0f)
#define MGL_HEIGHT          (0.0f)
#define MGL_DEPTH           (0.0f)

#define ARRANGEFORCE        (0.09f)
#define ARRANGELINKLEN      (3.0f)
#define ARRANGERIGIDITY     (0.055f)
#define ARRANGERESISTANCE   (0.9f)
#define ARRANGEMINDIST      (0.01f)
#define ARRANGECENTRING     (1.3f)
#define ARRANGECENTRINGFUNC ((psVisData->fArrangeCentring) / (psVisData->nNodes + 1))
#define ARRANGESHAKEVEL     (0.5f)
#define ARRANGEMOVESCALE    (0.5f)
#define ARRANGELINKSCALAR   (0.01f)

#define MAXKEYS             (256)
#define SELBUFSIZE          (512)

#define BOUNDSXMIN          (-1020.0f)
#define BOUNDSXMAX          (1020.0f)
#define BOUNDSYMIN          (-1020.0f)
#define BOUNDSYMAX          (1020.0f)
#define BOUNDSZMIN          (-1020.0f)
#define BOUNDSZMAX          (1020.0f)
#define BOUNDSVMIN          (-50.0f)
#define BOUNDSVMAX          (50.0f)

#define NEWLINK_DELAY       (0.02)
#define VIEW_RADIUS         (35.0f)
#define BOUNDSNEW           (5.0f)

#define NODETEXT_FONT       (GLUT_BITMAP_HELVETICA_10)
#define NODETEXT_LINE       (12)
#define NODETEXT_XOFF       (12)
#define NODETEXT_YOFF       (12)
#define NODETEXT_COLOUR     0.5, 1.0, 0.5
#define NODESELTEXT_COLOUR  0.8, 1.0, 0.8

#define LINKTEXT_FONT       (GLUT_BITMAP_HELVETICA_10)
#define LINKTEXT_LINE       (12)
#define LINKTEXT_XOFF       (6)
#define LINKTEXT_YOFF       (6)
#define LINKTEXT_COLOUR     0.5, 1.0, 0.5
#define LINKSELTEXT_COLOUR  0.8, 1.0, 0.8

#define MOMENTUM_MIN        (0.0005f)
#define MOMENTUM_RESISTANCE (0.98f)

#define GHOSTLINK_DURATION  (0.5f)
#define GHOSTLINK_HALFLIFE  (10.0f)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef enum {
  SELECTOBJECT_INVALID = -1,

  SELECTOBJECT_NODE,
  SELECTOBJECT_LINKOUT,
  SELECTOBJECT_LINKIN,

  SELECTOBJECT_NUM
} SELECTOBJECT;

typedef enum {
  DRAG_INVALID = -1,

  DRAG_NONE,
  DRAG_VIEW,
  DRAG_LINKIN,
  DRAG_LINKOUT,

  DRAG_NUM
} DRAG;

typedef enum {
  LINKTYPE_INVALID = -1,

  LINKTYPE_UNIDIRECTIONAL,
  LINKTYPE_BIDIRECTIONAL,

  LINKTYPE_NUM
} LINKTYPE;

typedef enum _GENLIST {
	GENLIST_INVALID = -1,

	GENLIST_ARROWLINK,
	GENLIST_ARROWHEAD,
	GENLIST_NODE,

	GENLIST_NUM
} GENLIST;

struct _TLink {
  GString * szName;
  TNode * psNodeFrom;
  TNode * psNodeTo;
  double fNewLinkTime;
  float afColour[4];
  FloatNote * psNote;
  LINKTYPE eType;
  PropertiesPersist * psPropertiesData;
};

struct _TNode {
  GString * szName;
  Vector3 vsPos;
  Vector3 vsVel;
  GSList * psLinksOut;
  int nLinksOut;
  int nLinksIn;
  float afColour[4];
  FloatNote * psNote;
  PropertiesPersist * psPropertiesData;
};

typedef struct _ChangeArrow {
  TNode * psOldFrom;
  TNode * psOldTo;
  TNode * psNewFrom;
  TNode * psNewTo;
  TLink * psLink;
  double fStartTime;
  double fDuration;
} ChangeArrow;

struct _VisPersist {
  GHashTable * psNodes;
  GHashTable * psLinks;
  GSList * psNodeList;
  int nNodes;
  double fCurrentTime;
  double fPrevTime;
  double fSpinTime;
  bool boNodeOverlay;
  bool boLinkOverlay;
  float fViewRadius;
  float fX;
  float fY;
  float fZ;
  float fXn;
  float fYn;
  float fZn;
  bool boSpin;
  int nXMouse;
  int nYMouse;
  DRAG eDrag;
  SELECTOBJECT eSelectObject;
  int nScreenWidth;
  int nScreenHeight;
  int nPrevScreenWidth;
  int nPrevScreenHeight;
  bool boFullScreen;
  bool boClearWhite;
  bool boAddToPlane;
  float fHalfLife;
  bool boArrived;
  TNode * psPointNode;
  bool boFollowNode;
  bool boDrawLinks;
  float fArrangeResistance;
  float fArrangeRigidity;
  float fArrangeForce;
  float fArrangeLinklen;
  float fArrangeCentring;
  float fArrangeLinkScalar;
  bool aboKeyDown[MAXKEYS];
  bool boFadeText;
  bool boNodeSelections;
  bool boLinkChanges;
  TNode * psNodeSelected;
  TLink * psLinkSelected;
  TNode * psLinkSelectedFrom;
  float fMomentum;
  float fXMomentum;
  float fYMomentum;
  float fZMomentum;
  GLuint uStartList;
  ChangeArrow * psChangeArrow;
  ServerPersist * psServerData;
  NotesPersist * psNotesData;
	TexPersist * psTexData;
};

typedef struct _SettingsNode {
  TNode * psNode;
  VisPersist * psVisData;
} SettingsNode;

typedef struct _SettingsLink {
  TNode * psNodeFrom;
  TLink * psLink;
  VisPersist * psVisData;
} SettingsLink;

typedef struct _SelectInfo {
  // Values in
  bool boSelectNodes;
  bool boSelectLinks;
  int nXPos;
  int nYPos;

  // Values out
  SELECTOBJECT eObjectType;
  TNode * psNode;
  TLink * psLink;
} SelectInfo;

///////////////////////////////////////////////////////////////////
// Global variables

static GLfloat gMatSpecular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat gMatShininess[] = { 50.0 };
static GLfloat gLinkDiffuse[] = { 0.5, 0.5, 1.0, 1.0 };
static GLfloat gSphereSpecular[] = { 0.7, 0.7, 0.7, 1.0 };
static GLfloat gSphereShininess[] = { 10.0 };
static GLfloat gSphereDiffuseOn[] = { 0.3, 0.9, 0.3, 1.0 };
static GLfloat gSphereDiffuseOff[] = { 0.9, 0.3, 0.3, 1.0 };
static GLfloat gLinkSelected[] = { 0.3, 0.9, 0.3, 0.5 };

///////////////////////////////////////////////////////////////////
// Function prototypes

TNode * NewTNode (char const * szName);
void DeleteTNode (TNode * psNode);
void DeleteTNodeCallback (gpointer psNodeData, gpointer psData);
TLink * NewTLink (char const * szName, TNode * psNodeFrom, TNode * psNodeTo);
void DeleteTLink (TLink * psLink);
void DeleteTLinkCallback (gpointer psLinkData, gpointer psData);
//static gint CompareNodeNames (gconstpointer psLink1, gconstpointer psLink2);
static gint CompareLinks (gconstpointer psLink1, gconstpointer psLink2);
static gint CompareLinksAnon (gconstpointer psLink1, gconstpointer psLink2);

void RemoveNode (char const * szName, VisPersist * psVisData);
void SetNodePosition (TNode * psNode, float fX, float fY, float fZ);
TLink * AddLinkDirect (char const * szName, TNode * psNodeFrom, TNode * psNodeTo, bool boBidirect, VisPersist * psVisData);
void RearrangeNetwork (VisPersist * psVisData);
void ApplyExpulsion (TNode * psNode, VisPersist * psVisData);
void ApplyLinkForce (TNode * psNode, VisPersist * psVisData);
void ApplyCentring (TNode * psNode, VisPersist * psVisData);
void RenderTextInSpace (char const * szText, GLdouble fX, GLdouble fY, GLdouble fZ);
void DrawTextOverlay (VisPersist * psVisData);
void ChangeView (float fTheta, float fPhi, float fPsi, float fRadius, VisPersist * psVisData);
void KeyIdle (VisPersist * psVisData);
bool PointTowards (float fXPos, float fYPos, float fZPos, float fRadius, VisPersist * psVisData);
void ResetAnimation (VisPersist * psVisData);
void Spin (VisPersist * psVisData);
void HashTableNodeDestroy (gpointer psData);
void HashTableLinkDestroy (gpointer psData);
void MomentumSpin (VisPersist * psVisData);
void VisLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
void VisLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
void VisLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);
void SaveSettingsNode (gpointer psData, gpointer psUserData);
void NodeLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
void NodeLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);
void NodeLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
void SaveSettingsLinks (gpointer psData, gpointer psUserData);
void LinkLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
void LinkLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);
void LinkLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
void InitialiseDisplayLists (VisPersist * psVisData);
void DeleteDisplayLists (VisPersist * psVisData);
void DrawArrow (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo, VisPersist * psVisData);
void DrawSelectArrow (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo, unsigned int uNodeCount, unsigned int uLinkCount, VisPersist * psVisData);
void DrawNode (GLdouble fXCentre, GLdouble fYCentre, GLdouble fZCentre, VisPersist * psVisData);
void SelectVisObject (SelectInfo * psSelectData, VisPersist * psVisData);
void MouseDown (int nXPos, int nYPos, VisPersist * psVisData);
void DragFinishLinkOut (int nXPos, int nYPos, VisPersist * psVisData);
void DragFinishLinkIn (int nXPos, int nYPos, VisPersist * psVisData);
void ChangeLink (TLink *psLink, TNode * psNewStart, TNode *psNewEnd, VisPersist * psVisData);
float DrawChangeArrow (ChangeArrow * psArrowData, VisPersist * psVisData);
void AnimateGhostLinkOut (int nXPos, int nYPos, VisPersist * psVisData);
void AnimateGhostLinkIn (int nXPos, int nYPos, VisPersist * psVisData);
void UpdateNoteAnchors (VisPersist * psVisData);
void SetNodeSelected (TNode * psNodeSelected, VisPersist * psVisData);
void SetLinkSelected (TNode * psNodeSelected, TLink * psLinkSelected, VisPersist * psVisData);
void SetLinkBidirect (TLink * psLink, bool boBidirect, VisPersist * psVisData);
void UpdateNodeNoteString (TNode * psNode);
void UpdateLinkNoteString (TLink * psLink);

///////////////////////////////////////////////////////////////////
// Function definitions

TNode * AddNode (char const * szName, VisPersist * psVisData) {
  TNode * psNodeNew;
  TNode * psNodeFound;

  psNodeFound = g_hash_table_lookup (psVisData->psNodes, szName);
  //psNodeFound = g_slist_find_custom (psVisData->psNodes, psNodeNew, CompareNodeNames);

  if (psNodeFound == NULL) {
		// Only add the node if a node with this name doesn't already exist
		psNodeNew = NewTNode (szName);

    // The node has a unique name, so add it
    if (psVisData->boAddToPlane) {
      // Option set to add new nodes to the X-Z plane, so set the Y position to zero
      psNodeNew->vsPos.fY = 0.0f;
    }
    // Add the FloatNote structure
    psNodeNew->psNote = AddFloatNote (psNodeNew->vsPos.fX, psNodeNew->vsPos.fY, "", psVisData->psNotesData);

		// Add the node to the lists
    psVisData->psNodeList = g_slist_prepend (psVisData->psNodeList, (gpointer)psNodeNew);
    g_hash_table_replace (psVisData->psNodes, psNodeNew->szName->str, psNodeNew);
    psVisData->nNodes++;
  }
  else {
    // The node already exists
    psNodeNew = psNodeFound;
  }

	return psNodeNew;
}

void RemoveNode (char const * szName, VisPersist * psVisData) {
  TNode * psNode;
  TNode * psNodeFrom;
  TLink * psLink;
  TLink * psCompareLink;
  GSList * psNodeListPos;
  GSList * psListItem;

  psNode = g_hash_table_lookup (psVisData->psNodes, szName);

  // Remove the node
  if (psNode) {
    // If it's the selected node, deselect it
    if (psNode == psVisData->psNodeSelected) {
			SetNodeSelected (NULL, psVisData);
    }

    // Delete any links going into the node
    psNodeListPos = psVisData->psNodeList;
    while (psNodeListPos) {
      psNodeFrom = (TNode *)psNodeListPos->data;
      psLink = FindLinkNodes (psNodeFrom, psNode, psVisData);

			// Remove all of the relevant links
		  psCompareLink = NewTLink ("", psNodeFrom, psNode);

		  psListItem = g_slist_find_custom (psNodeFrom->psLinksOut, psCompareLink, CompareLinksAnon);
			while (psListItem) {
		  	psLink = ((TLink *)psListItem->data);
	      RemoveLink (psLink, psVisData);

			  psListItem = g_slist_find_custom (psNodeFrom->psLinksOut, psCompareLink, CompareLinksAnon);
			}
		  DeleteTLink (psCompareLink);

      psNodeListPos = psNodeListPos->next;
    }
    
    // Remove the FloatNode structure
    RemoveFloatNote (psNode->psNote, psVisData->psNotesData);
    psNode->psNote = NULL;

    // Delete the node from the list and hash table
    psVisData->psNodeList = g_slist_remove (psVisData->psNodeList, (gconstpointer)psNode);
    g_hash_table_remove (psVisData->psNodes, szName);
  }
}

void MoveNode (char const * szName, double fX, double fY, double fZ, VisPersist * psVisData) {
  TNode * psNode;

  psNode = g_hash_table_lookup (psVisData->psNodes, szName);

  // Move the node
  if (psNode) {
    // Set the velocity of the node to create a movement
    psNode->vsVel.fX += ARRANGEMOVESCALE * fX;
    psNode->vsVel.fY += ARRANGEMOVESCALE * fY;
    psNode->vsVel.fZ += ARRANGEMOVESCALE * fZ;
  }
}

void SetNodeColour (char const * szName, double fRed, double fGreen, double fBlue, VisPersist * psVisData) {
  TNode * psNode;

  psNode = g_hash_table_lookup (psVisData->psNodes, szName);

  // Set the node colour
  if (psNode) {
    psNode->afColour[0] = fRed;
    psNode->afColour[1] = fGreen;
    psNode->afColour[2] = fBlue;
    psNode->afColour[3] = 1.0f;
  }
}

void AddLink (char const * szName, char const * szNodeFrom, char const * szNodeTo, bool boBidirect, VisPersist * psVisData) {
  TNode * psNodeFrom;
  TNode * psNodeTo;

  psNodeFrom = g_hash_table_lookup (psVisData->psNodes, szNodeFrom);
  psNodeTo = g_hash_table_lookup (psVisData->psNodes, szNodeTo);

  if (psNodeFrom && psNodeTo) {
    AddLinkDirect (szName, psNodeFrom, psNodeTo, boBidirect, psVisData);
  }
}

TLink * FindLinkNodesNamed (char const * szNodeFrom, char const * szNodeTo, VisPersist * psVisData) {
  TNode * psNodeFrom;
  TNode * psNodeTo;
  TLink * psLink = NULL;

  psNodeFrom = g_hash_table_lookup (psVisData->psNodes, szNodeFrom);
  psNodeTo = g_hash_table_lookup (psVisData->psNodes, szNodeTo);

  if (psNodeFrom && psNodeTo) {
		psLink = FindLinkNodes (psNodeFrom, psNodeTo, psVisData);
  }
  
  return psLink;
}

TLink * FindLinkNamed (char const * szName, VisPersist * psVisData) {
  TLink * psLink;

  psLink = g_hash_table_lookup (psVisData->psLinks, szName);
  
  return psLink;
}

TLink * FindLinkNodes (TNode * psNodeFrom, TNode * psNodeTo, VisPersist * psVisData) {
  GSList * psListItem;
  TLink * psCompareLink;
  TLink * psLink = NULL;

  psCompareLink = NewTLink ("", psNodeFrom, psNodeTo);

  // Find the first such link
  psListItem = g_slist_find_custom (psNodeFrom->psLinksOut, psCompareLink, CompareLinks);
  if (psListItem) {
  	psLink = ((TLink *)psListItem->data);
  }
  DeleteTLink (psCompareLink);

	return psLink;
}

void SetLinkColour (TLink * psLink, double fRed, double fGreen, double fBlue, VisPersist * psVisData) {
  psLink->afColour[0] = fRed;
  psLink->afColour[1] = fGreen;
  psLink->afColour[2] = fBlue;
  psLink->afColour[3] = 1.0f;
}

void SetLinkColourNamed (char const * szName, double fRed, double fGreen, double fBlue, VisPersist * psVisData) {
  TLink * psLink;

  psLink = FindLinkNamed (szName, psVisData);

  if (psLink) {
    SetLinkColour (psLink, fRed, fGreen, fBlue, psVisData);
  }
}

void RemoveAll (VisPersist * psVisData) {
  // Clear the selected node
	SetNodeSelected (NULL, psVisData);

  // Free all of the node data in the node list
  g_slist_free (psVisData->psNodeList);
  psVisData->psNodeList = NULL;

  // Free the nodes and node list hash table (will also free all links)
	// Note that this doesn't free the notes though
  g_hash_table_destroy (psVisData->psNodes);
  psVisData->psNodes = NULL;
  psVisData->nNodes = 0;
  // Free the link hash table
  g_hash_table_destroy (psVisData->psLinks);

	// Free the notes
	RemoveAllNotes (psVisData->psNotesData);

  // Create a new node hash table
  psVisData->psNodes = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, HashTableNodeDestroy);
	// Create a new link hash table
  psVisData->psLinks = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, HashTableLinkDestroy);

  ResetAnimation (psVisData);
}

void HashTableNodeDestroy (gpointer psData) {
  DeleteTNode ((TNode *)psData);
}

void HashTableLinkDestroy (gpointer psData) {
  DeleteTLink ((TLink *)psData);
}

VisPersist * NewVisPersist (void) {
  VisPersist * psVisData;

  psVisData = g_new0 (VisPersist, 1);

  psVisData->psNodes = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, HashTableNodeDestroy);
  psVisData->psLinks = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, HashTableLinkDestroy);
  psVisData->psNodeList = NULL;
  psVisData->nNodes = 0;
  psVisData->boNodeOverlay = TRUE;
  psVisData->boLinkOverlay = TRUE;
  psVisData->fViewRadius = VIEW_RADIUS;
  psVisData->fX = 0.0f;
  psVisData->fY = 1.0f;
  psVisData->fZ = 0.0f;
  psVisData->fXn = 1.0f;
  psVisData->fYn = 0.0f;
  psVisData->fZn = 0.0f;
  psVisData->boSpin = FALSE;
  psVisData->nXMouse = 0;
  psVisData->nYMouse = 0;
  psVisData->eDrag = DRAG_NONE;
  psVisData->eSelectObject = SELECTOBJECT_INVALID;
  psVisData->nScreenWidth = SCREENWIDTH;
  psVisData->nScreenHeight = SCREENHEIGHT;
  psVisData->nPrevScreenWidth = SCREENWIDTH;
  psVisData->nPrevScreenHeight = SCREENHEIGHT;
  psVisData->boFullScreen = false;
  psVisData->boClearWhite = false;
  psVisData->boAddToPlane = false;
  psVisData->fHalfLife = POINTTOHALFLIFE;
  psVisData->boArrived = true;
  psVisData->psPointNode = NULL;
  psVisData->boFollowNode = false;
  psVisData->boDrawLinks = true;
  psVisData->fArrangeResistance = ARRANGERESISTANCE;
  psVisData->fArrangeRigidity = ARRANGERIGIDITY;
  psVisData->fArrangeForce = ARRANGEFORCE;
  psVisData->fArrangeLinklen = ARRANGELINKLEN;
  psVisData->fArrangeCentring = ARRANGECENTRING;
  psVisData->fArrangeLinkScalar = ARRANGELINKSCALAR;
  psVisData->boFadeText = true;
  psVisData->boNodeSelections = true;
  psVisData->boLinkChanges = false;
  psVisData->psNodeSelected = NULL;
  psVisData->psLinkSelected = NULL;
  psVisData->psLinkSelectedFrom = NULL;
  psVisData->psChangeArrow = NULL;
  psVisData->psServerData = NULL;

  psVisData->fMomentum = 0.0f;
  psVisData->fXMomentum = 0.0f;
  psVisData->fYMomentum = 0.0f;
  psVisData->fZMomentum = 0.0f;

  psVisData->uStartList = 0;
  psVisData->psTexData = NewTexPersist (TEXNAME_NUM);
  psVisData->psNotesData = NewNotesPersist (psVisData->psTexData);

  return psVisData;
}

void DeleteVisPersist (VisPersist * psVisData) {
  // Free all of the node data in the node list
  //g_slist_foreach (psVisData->psNodes, DeleteTNodeCallback, NULL);
  // Free the list itself
  g_slist_free (psVisData->psNodeList);
  DeleteNotesPersist (psVisData->psNotesData);
  psVisData->psNotesData = NULL;

  // Free the nodes and node list hash table (will also free all links)
  g_hash_table_destroy (psVisData->psNodes);
  g_hash_table_destroy (psVisData->psLinks);

	DeleteTexPersist (psVisData->psTexData);

  g_free (psVisData);
}

TNode * NewTNode (char const * szName) {
  TNode * psNode;

  psNode = g_new0 (TNode, 1);

  psNode->szName = g_string_new (szName);
  psNode->vsPos.fX = BOUNDSNEW - ((BOUNDSNEW * 2.0) * ((float)rand () / (float)RAND_MAX));
  psNode->vsPos.fY = BOUNDSNEW - ((BOUNDSNEW * 2.0) * ((float)rand () / (float)RAND_MAX));
  psNode->vsPos.fZ = BOUNDSNEW - ((BOUNDSNEW * 2.0) * ((float)rand () / (float)RAND_MAX));
  psNode->vsVel.fX = 0.0f;
  psNode->vsVel.fY = 0.0f;
  psNode->vsVel.fZ = 0.0f;
  psNode->psLinksOut = NULL;
  psNode->nLinksIn = 0;
  psNode->nLinksOut = 0;
  psNode->afColour[0] = gSphereDiffuseOff[0];
  psNode->afColour[1] = gSphereDiffuseOff[1];
  psNode->afColour[2] = gSphereDiffuseOff[2];
  psNode->afColour[3] = gSphereDiffuseOff[3];
  psNode->psNote = NULL;
  psNode->psPropertiesData = NewPropertiesPersist ();

  return psNode;
}

void DeleteTNode (TNode * psNode) {
  // Free the list of links
  // Note that we don't free the links as they may be being remembered elsewhere
  // (such as in the Vis link hash table)
  // We also don't free the Note as this is also linked to in the FloatNotes list
  //g_slist_foreach (psNode->psLinksOut, DeleteTLinkCallback, NULL);
  g_slist_free (psNode->psLinksOut);

  g_string_free (psNode->szName, TRUE);
  psNode->szName = NULL;

	if (psNode->psPropertiesData) {
		DeletePropertiesPersist (psNode->psPropertiesData);
		psNode->psPropertiesData = NULL;
	}

  g_free (psNode);
}

void DeleteTNodeCallback (gpointer psNodeData, gpointer psData) {
  // We don't use psData; it can just be NULL

  DeleteTNode ((TNode *)psNodeData);
}

TLink * NewTLink (char const * szName, TNode * psNodeFrom, TNode * psNodeTo) {
  TLink * psLink;

  psLink = g_new0 (TLink, 1);

  psLink->szName = g_string_new (szName);
  psLink->psNodeFrom = psNodeFrom;
  psLink->psNodeTo = psNodeTo;
  psLink->fNewLinkTime = 0;

  psLink->afColour[0] = gLinkDiffuse[0];
  psLink->afColour[1] = gLinkDiffuse[1];
  psLink->afColour[2] = gLinkDiffuse[2];
  psLink->afColour[3] = gLinkDiffuse[3];
  psLink->psNote = NULL;
  psLink->psPropertiesData = NewPropertiesPersist ();

  return psLink;
}

void DeleteTLink (TLink * psLink) {
	g_string_free (psLink->szName, TRUE);

	if (psLink->psPropertiesData) {
		DeletePropertiesPersist (psLink->psPropertiesData);
		psLink->psPropertiesData = NULL;
	}

  g_free (psLink);
}

void DeleteTLinkCallback (gpointer psLinkData, gpointer psData) {
  // We don't use psData; it can just be NULL

  DeleteTLink ((TLink *)psLinkData);
}

void Realise (VisPersist * psVisData) {
  GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0, 0.5, 0.5, 0.5, 1.0 };
  GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0, 0.5, 0.3, 0.0, 1.0 };
  GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0, 0.3, 0.5, 1.0, 0.0 };
  GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0 };

  //glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightModelfv (GL_LIGHT_MODEL_AMBIENT, light_ambient);
  glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  //glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient + 4);
  //glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse + 4);
  //glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular + 4);
  //glLightfv(GL_LIGHT1, GL_POSITION, light_position + 4);

  glFrontFace (GL_CCW);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  //glEnable(GL_LIGHT1);
  glEnable (GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_TEXTURE_2D);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Load textures
	LoadTextures (psVisData->psTexData);

  InitialiseDisplayLists (psVisData);
}

void Unrealise (VisPersist * psVisData) {
  // Delete the OpenGL display lists
  DeleteDisplayLists (psVisData);
}

void Init (VisPersist * psVisData) {
  int nCount;
  struct timeb sTime;
#if (TESTNODES > 0)
  TNode * psNodeFrom;
  TNode * psNodeTo;
  char szName[256];
#endif

  for (nCount = 0; nCount < MAXKEYS; nCount++) {
    psVisData->aboKeyDown[nCount] = false;
  }

#if (TESTNODES > 0)
  // Create some random nodes
  for (nCount = 0; nCount < TESTNODES; nCount++) {
    sprintf (szName, "%d", nCount);
    AddNode (szName, psVisData);
  }

  // Create some random links
  for (nCount = 0; nCount < TESTLINKS; nCount++) {
    psNodeFrom = (TNode *)g_slist_nth_data (psVisData->psNodeList, (nCount % TESTNODES));
    psNodeTo = (TNode *)g_slist_nth_data (psVisData->psNodeList, (rand () % psVisData->nNodes));

    if (psNodeFrom && psNodeTo) {
      AddLinkDirect ("", psNodeFrom, psNodeTo, TRUE, psVisData);
    }
    else {
      printf ("Error: NULL node\n");
    }
  }
#endif

  ftime (& sTime);
  psVisData->fCurrentTime = (double)(sTime.time) + (double)(sTime.millitm) / 1000.0;
  if (psVisData->fCurrentTime < 0) psVisData->fCurrentTime = 0.0;
  psVisData->fPrevTime = psVisData->fCurrentTime;
  psVisData->fSpinTime = psVisData->fCurrentTime;
}

//static gint CompareNodeNames (gconstpointer psLink1, gconstpointer psLink2) {
//  return strcmp (((TNode *)psLink1)->szName->str, ((TNode *)psLink2)->szName->str);
//}

void VisSetServer (ServerPersist * psServerData, VisPersist * psVisData) {
  psVisData->psServerData = psServerData;
}

void SetNodePosition (TNode * psNode, float fX, float fY, float fZ) {
  psNode->vsPos.fX = fX;
  psNode->vsPos.fY = fY;
  psNode->vsPos.fZ = fZ;
}

static gint CompareLinks (gconstpointer psLink1, gconstpointer psLink2) {
	gint nDifference;

	nDifference = (((TLink *)psLink1)->psNodeTo) - (((TLink *)psLink2)->psNodeTo);
	if (nDifference == 0) {
		nDifference = strcmp (((TLink *)psLink1)->szName->str, ((TLink *)psLink2)->szName->str);
	}

  return nDifference;
}

static gint CompareLinksAnon (gconstpointer psLink1, gconstpointer psLink2) {
	gint nDifference;

	nDifference = (((TLink *)psLink1)->psNodeTo) - (((TLink *)psLink2)->psNodeTo);

  return nDifference;
}

TLink * AddLinkDirect (char const * szName, TNode * psNodeFrom, TNode * psNodeTo, bool boBidirect, VisPersist * psVisData) {
  GSList * psListItem;
  TLink * psNewLink = NULL;
  TLink * psCompareLink;

  // We're not going to allow links from a node to itself. It'll mess up our arc cosines.
  if (psNodeFrom != psNodeTo) {
		// Check whether the link has a name
		if (szName && strlen (szName) > 0) {
			// Named link (add to the hash table)

			// Only add link if there isn't one already
			psCompareLink = FindLinkNamed (szName, psVisData);
			
			if (psCompareLink != NULL) {
				// The link already exists, so we're just going to re-wire it
				ChangeLink (psCompareLink, psNodeFrom, psNodeTo, psVisData);
				psCompareLink->eType = boBidirect?LINKTYPE_BIDIRECTIONAL:LINKTYPE_UNIDIRECTIONAL;
				psNewLink = psCompareLink;
			}
			else {
				// This really is a new link
		    psNewLink = NewTLink (szName, psNodeFrom, psNodeTo);
		    psNewLink->fNewLinkTime = psVisData->fCurrentTime + NEWLINK_DELAY;
				psNewLink->eType = boBidirect?LINKTYPE_BIDIRECTIONAL:LINKTYPE_UNIDIRECTIONAL;

				// Add the FloatNote structure
				psNewLink->psNote = AddFloatNote ((psNodeFrom->vsPos.fX + psNodeTo->vsPos.fX) / 2.0f, (psNodeFrom->vsPos.fY + psNodeTo->vsPos.fY) / 2.0f, "", psVisData->psNotesData);

		    psNodeFrom->psLinksOut = g_slist_prepend (psNodeFrom->psLinksOut, psNewLink);
		    psNodeFrom->nLinksOut++;
		    psNodeTo->nLinksIn++;

		    g_hash_table_replace (psVisData->psLinks, psNewLink->szName->str, psNewLink);
			}
		}
		else {
			// Anonymous link (don't add to the hash table)

		  // Only add a link if there isn't one already
		  psCompareLink = NewTLink ("", psNodeFrom, psNodeTo);

		  psListItem = g_slist_find_custom (psNodeFrom->psLinksOut, psCompareLink, CompareLinks);

		  if (!psListItem) {
		    psNewLink = NewTLink ("", psNodeFrom, psNodeTo);
		    psNewLink->fNewLinkTime = psVisData->fCurrentTime + NEWLINK_DELAY;
				psNewLink->eType = boBidirect?LINKTYPE_BIDIRECTIONAL:LINKTYPE_UNIDIRECTIONAL;

				// Add the FloatNote structure
				psNewLink->psNote = AddFloatNote ((psNodeFrom->vsPos.fX + psNodeTo->vsPos.fX) / 2.0f, (psNodeFrom->vsPos.fY + psNodeTo->vsPos.fY) / 2.0f, "", psVisData->psNotesData);

		    psNodeFrom->psLinksOut = g_slist_prepend (psNodeFrom->psLinksOut, psNewLink);
		    psNodeFrom->nLinksOut++;
		    psNodeTo->nLinksIn++;
		  }
		  else {
		  	psNewLink = ((TLink *)(psListItem->data));
		  	psNewLink->eType = boBidirect?LINKTYPE_BIDIRECTIONAL:LINKTYPE_UNIDIRECTIONAL;
		  }
		  DeleteTLink (psCompareLink);
		}
  }

  if (psVisData->boFollowNode) {
    psVisData->boArrived = false;
    psVisData->psPointNode = psNodeTo;
  }
  
  return psNewLink;
}

void RemoveLink (TLink * psLink, VisPersist * psVisData) {
  GSList * psListItem;
	TNode * psNodeFrom;
	TNode * psNodeTo;
	gchar * szName;

	if (psLink) {
		psNodeFrom = psLink->psNodeFrom;
		psNodeTo = psLink->psNodeTo;

		// Remove the FloatNode structure
		RemoveFloatNote (psLink->psNote, psVisData->psNotesData);
		psLink->psNote = NULL;

		// Remove the link from the node's list of links
		psListItem = g_slist_find (psNodeFrom->psLinksOut, psLink);
		if (psListItem) {
			psNodeFrom->psLinksOut = g_slist_delete_link (psNodeFrom->psLinksOut, psListItem);
			psNodeFrom->nLinksOut--;
		}
		psNodeTo->nLinksIn--;

		// Remove the link from the hash table
		szName = g_strdup (psLink->szName->str);
		g_hash_table_remove (psVisData->psLinks, szName);
		g_free (szName);
	}
}

void SetLinkBidirect (TLink * psLink, bool boBidirect, VisPersist * psVisData) {
	psLink->eType = boBidirect?LINKTYPE_BIDIRECTIONAL:LINKTYPE_UNIDIRECTIONAL;
}

void Spin (VisPersist * psVisData) {
  double fTimeChange;

  fTimeChange = psVisData->fCurrentTime - psVisData->fSpinTime;
  if (fTimeChange > 0.005f) {
    if (fTimeChange > 0.05f) {
      fTimeChange = 0.05f;
    }
    ChangeView (0.2f * fTimeChange, 0.005f * sin (0.11f * psVisData->fCurrentTime), 0.0f, 0.0f,
      psVisData);
    psVisData->fSpinTime = psVisData->fCurrentTime;
  }
}

void DrawArrow (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo, VisPersist * psVisData) {
  float			fLength;
  float     fAngle;
  float     fXMid;
  float     fYMid;
  float     fZMid;

  fXMid = (fXTo + fXFrom)/2.0f;
  fYMid = (fYTo + fYFrom)/2.0f;
  fZMid = (fZTo + fZFrom)/2.0f;
  fLength = sqrt (((fXFrom - fXTo) * (fXFrom - fXTo))
    + ((fYFrom - fYTo) * (fYFrom - fYTo))
    + ((fZFrom - fZTo) * (fZFrom - fZTo)));
  fLength -= (2.0f * NODERADIUS);
  if (fLength < (ARROWHEADHEIGHT + ARROWMINLENGTH)) {
    fLength = (ARROWHEADHEIGHT + ARROWMINLENGTH);
  }

  glPushMatrix();

  glTranslatef (fXMid, fYMid, fZMid);

  fAngle = 360.0f * acos ((fXTo - fXFrom) / sqrt ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))))) / (2.0f * 3.1415926538);
  if (fZTo > fZFrom) fAngle = -fAngle;
  glRotatef (fAngle, 0.0, 1.0, 0.0);

  fAngle = 360.0f * acos ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))) / sqrt (((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom)))) * ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fYTo - fYFrom) * (fYTo - fYFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom)))))) / (2.0f * 3.1415926538);
  // The next line is annoying, but seems to be necessary to stop flickering due to floating point errors. Eergh.
  if (absf (fYTo - fYFrom) < 0.00001) fAngle = 0.0f;
  if (fYTo < fYFrom) fAngle = -fAngle;
  glRotatef (fAngle, 0.0, 0.0, 1.0);

  // Add the arrowhead
  glTranslatef (((fLength / 2.0f) - ARROWHEADHEIGHT), 0.0, 0.0);
  glRotatef (90.0f, 0.0, 1.0, 0.0);
  glCallList (psVisData->uStartList + GENLIST_ARROWHEAD);
  glRotatef (-90.0f, 0.0, 1.0, 0.0);
  glTranslatef (-((fLength / 2.0f) - ARROWHEADHEIGHT), 0.0, 0.0);

  // Shift half an arrowhead's worth so that the tube doesn't cross the arrowhead
  glTranslatef (-((ARROWHEADHEIGHT / 2.0f)), 0.0, 0.0);

  // Scale the tube to stretch between the two points
  glScalef ((fLength - (ARROWHEADHEIGHT / 2.0f)), 1.0f, 1.0f);
  //glScalef (fLength, 1.0f, 1.0f);

  // Move so the tube midpoint is at (fXMid, fYMid, fZMid)
  glTranslatef (-0.5, 0.0, 0.0);
  glRotatef (90.0f, 0.0, 1.0, 0.0);

  // Tubes aren't a glut primitive, but that's okay 'cos we built one earlier
  //glutSolidCone (0.3, 1.0, 10, 10);
  glCallList (psVisData->uStartList + GENLIST_ARROWLINK);

	glPopMatrix ();
}

void DrawNode (GLdouble fXCentre, GLdouble fYCentre, GLdouble fZCentre, VisPersist * psVisData) {
  glPushMatrix ();
  glTranslatef (fXCentre, fYCentre, fZCentre);
  glCallList (psVisData->uStartList + GENLIST_NODE);
  glPopMatrix ();
}

void RenderTextInSpace (char const * szText, GLdouble fX, GLdouble fY, GLdouble fZ) {
  GLdouble        afModel[16];
  GLdouble        afProjection[16];
  GLint           anViewpoert[4];

  glGetDoublev (GL_MODELVIEW_MATRIX, afModel);
  glGetDoublev (GL_PROJECTION_MATRIX, afProjection);
  glGetIntegerv (GL_VIEWPORT, anViewpoert);

  gluProject (fX, fY, fZ, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);

  glDisable (GL_LIGHTING);
  glColor3f (NODETEXT_COLOUR);
  fX += NODETEXT_XOFF;
  fY += NODETEXT_YOFF;

  RenderBitmapString ((float)fX, (float)fY, NODETEXT_FONT, szText);
  glEnable (GL_LIGHTING);
}

float DrawChangeArrow (ChangeArrow * psArrowData, VisPersist * psVisData) {
  float fCompletion;
  float fXFrom;
  float fYFrom;
  float fZFrom;
  float fXTo;
  float fYTo;
  float fZTo;
  float fProportion;

  // Find out how far complete the animation is between 0 and 1
  fCompletion = (psVisData->fCurrentTime - psArrowData->fStartTime) / psArrowData->fDuration;
  fCompletion = CLAMP (fCompletion, 0.0f, 1.0f);
  fProportion = 1.0f - (pow (0.5f, (fCompletion * GHOSTLINK_HALFLIFE)));
  fProportion = CLAMP (fProportion, 0.0f, 1.0f);

  fXFrom = psArrowData->psOldFrom->vsPos.fX + ((psArrowData->psNewFrom->vsPos.fX - psArrowData->psOldFrom->vsPos.fX) * fProportion);
  fYFrom = psArrowData->psOldFrom->vsPos.fY + ((psArrowData->psNewFrom->vsPos.fY - psArrowData->psOldFrom->vsPos.fY) * fProportion);
  fZFrom = psArrowData->psOldFrom->vsPos.fZ + ((psArrowData->psNewFrom->vsPos.fZ - psArrowData->psOldFrom->vsPos.fZ) * fProportion);

  fXTo = psArrowData->psOldTo->vsPos.fX + ((psArrowData->psNewTo->vsPos.fX - psArrowData->psOldTo->vsPos.fX) * fProportion);
  fYTo = psArrowData->psOldTo->vsPos.fY + ((psArrowData->psNewTo->vsPos.fY - psArrowData->psOldTo->vsPos.fY) * fProportion);
  fZTo = psArrowData->psOldTo->vsPos.fZ + ((psArrowData->psNewTo->vsPos.fZ - psArrowData->psOldTo->vsPos.fZ) * fProportion);

  DrawArrow (fXFrom, fYFrom, fZFrom, fXTo, fYTo, fZTo, psVisData);

  return fCompletion;
}

void InitialiseDisplayLists (VisPersist * psVisData) {
  GLUquadricObj * sQobj;

  psVisData->uStartList = glGenLists (GENLIST_NUM);

  int error = glGetError();
  if (error != GL_NO_ERROR) {
    printf ("Error %s\n", gluErrorString(error));
  }

  sQobj = gluNewQuadric ();
  gluQuadricDrawStyle (sQobj, GLU_FILL);
  gluQuadricNormals (sQobj, GLU_SMOOTH);

  glNewList (psVisData->uStartList + GENLIST_ARROWLINK, GL_COMPILE);
  gluCylinder (sQobj, ARROWLINKRADIUS, ARROWLINKRADIUS, 1.0, ARROWLINKSLICES, ARROWLINKSTACKS);
  glEndList ();

  gluDeleteQuadric (sQobj);

  glNewList (psVisData->uStartList + GENLIST_ARROWHEAD, GL_COMPILE);
  glutSolidCone (ARROWHEADRADIUS, ARROWHEADHEIGHT, ARROWHEADSLICES, ARROWHEADSTACKS);
  glEndList ();

  glNewList (psVisData->uStartList + GENLIST_NODE, GL_COMPILE);
  glutSolidSphere (NODERADIUS, NODESLICES, NODESTACKS);
  glEndList ();
}

void DeleteDisplayLists (VisPersist * psVisData) {
  glDeleteLists (psVisData->uStartList, GENLIST_NUM);
  psVisData->uStartList = 0;
}

void Display (VisPersist * psVisData) {
  TNode * psLinkTo;
  TLink * psLink;
  TNode * psNode;
  GSList * psNodeListPos;
  GSList * psLinkListPos;
  GLdouble afModel[16];
  GLdouble afProjection[16];
  GLint anViewpoert[4];
  GLdouble fX;
  GLdouble fY;
  GLdouble fZ;
  float fFade;

  glLoadIdentity ();
  gluLookAt ((psVisData->fViewRadius) * psVisData->fX, (psVisData->fViewRadius) * psVisData->fY,
    (psVisData->fViewRadius) * psVisData->fZ, 0.0, 0.0, 0.0, psVisData->fXn, psVisData->fYn, psVisData->fZn);

  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, gSphereSpecular);
  glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, gSphereShininess);

  //glRotatef (gfR1, 0.0, 1.0, 0.0);
  //glRotatef (gfR2, 1.0, 0.0, 0.0);

  // Draw the nodes
  psNodeListPos = psVisData->psNodeList;
  while (psNodeListPos) {
    psNode = (TNode *)psNodeListPos->data;

    if (psNode == psVisData->psNodeSelected) {
      glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gSphereDiffuseOn);
    }
    else {
      glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, psNode->afColour);
    }
    DrawNode (psNode->vsPos.fX, psNode->vsPos.fY, psNode->vsPos.fZ, psVisData);

    psNodeListPos = psNodeListPos->next;
  }

  // Draw the links
  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, gMatSpecular);
  glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, gMatShininess);

  if (psVisData->boDrawLinks) {
    psNodeListPos = psVisData->psNodeList;
    while (psNodeListPos) {
      psNode = (TNode *)psNodeListPos->data;

      psLinkListPos = psNode->psLinksOut;
      while (psLinkListPos) {
        psLink = ((TLink *)psLinkListPos->data);
        psLinkTo = psLink->psNodeTo;

        if (psLink != psVisData->psLinkSelected) {
          glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, psLink->afColour);
          if (psLinkTo != psNode) {
            DrawArrow (psNode->vsPos.fX,
              psNode->vsPos.fY,
              psNode->vsPos.fZ,
              psLinkTo->vsPos.fX,
              psLinkTo->vsPos.fY,
              psLinkTo->vsPos.fZ,
              psVisData);
          }
        }
        psLinkListPos = psLinkListPos->next;
      }
      psNodeListPos = psNodeListPos->next;
    }
  }

  // Draw the selected link if there is one
  if (psVisData->psLinkSelected) {
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gLinkSelected);
    if ((psVisData->eDrag == DRAG_LINKIN) || (psVisData->eDrag == DRAG_LINKOUT)) {
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      DrawArrow (psVisData->psLinkSelectedFrom->vsPos.fX,
        psVisData->psLinkSelectedFrom->vsPos.fY,
        psVisData->psLinkSelectedFrom->vsPos.fZ,
        psVisData->psLinkSelected->psNodeTo->vsPos.fX,
        psVisData->psLinkSelected->psNodeTo->vsPos.fY,
        psVisData->psLinkSelected->psNodeTo->vsPos.fZ,
        psVisData);
      glDisable (GL_BLEND);
    }
    else {
      DrawArrow (psVisData->psLinkSelectedFrom->vsPos.fX,
        psVisData->psLinkSelectedFrom->vsPos.fY,
        psVisData->psLinkSelectedFrom->vsPos.fZ,
        psVisData->psLinkSelected->psNodeTo->vsPos.fX,
        psVisData->psLinkSelected->psNodeTo->vsPos.fY,
        psVisData->psLinkSelected->psNodeTo->vsPos.fZ,
        psVisData);
    }
  }

  // Draw the changing link if there is one
  if (psVisData->psChangeArrow) {
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gLinkSelected);
    DrawChangeArrow (psVisData->psChangeArrow, psVisData);
    glDisable (GL_BLEND);
  }

	// Draw the various text overlays
  glGetDoublev (GL_MODELVIEW_MATRIX, afModel);
  glGetDoublev (GL_PROJECTION_MATRIX, afProjection);
  glGetIntegerv (GL_VIEWPORT, anViewpoert);
  glDisable (GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  // Draw the node text overlay
  if (psVisData->boNodeOverlay) {
    psNodeListPos = psVisData->psNodeList;
    if (psVisData->boFadeText) {
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      while (psNodeListPos) {
        psNode = (TNode *)psNodeListPos->data;
        gluProject (psNode->vsPos.fX, psNode->vsPos.fY, psNode->vsPos.fZ, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);
        fX += NODETEXT_XOFF;
        fY += NODETEXT_YOFF;
        fFade = (1.0f - ((fZ - 0.95) * 60.0f));
        if (fFade > 0.0f) {
          glColor4f ((0.5), (1.0), (0.5), fFade);
          RenderBitmapString ((float)fX, (float)fY, NODETEXT_FONT, psNode->szName->str);
        }
        psNodeListPos = psNodeListPos->next;
      }
      glDisable (GL_BLEND);
    }
    else {
      glColor3f (NODETEXT_COLOUR);

      while (psNodeListPos) {
        psNode = (TNode *)psNodeListPos->data;
        gluProject (psNode->vsPos.fX, psNode->vsPos.fY, psNode->vsPos.fZ, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);
        fX += NODETEXT_XOFF;
        fY += NODETEXT_YOFF;
        RenderBitmapString ((float)fX, (float)fY, NODETEXT_FONT, psNode->szName->str);
        psNodeListPos = psNodeListPos->next;
      }
    }
  }

  // Draw the link text overlay
  if (psVisData->boLinkOverlay) {
    psNodeListPos = psVisData->psNodeList;
    if (psVisData->boFadeText) {
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      while (psNodeListPos) {
        psNode = (TNode *)psNodeListPos->data;

		    psLinkListPos = psNode->psLinksOut;
		    while (psLinkListPos) {
		      psLink = ((TLink *)psLinkListPos->data);
		      psLinkTo = psLink->psNodeTo;

					if (psLink->szName->len > 0) {
						gluProject ((psNode->vsPos.fX + psLinkTo->vsPos.fX) / 2.0, (psNode->vsPos.fY + psLinkTo->vsPos.fY) / 2.0, (psNode->vsPos.fZ + psLinkTo->vsPos.fZ) / 2.0, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);
						fX += LINKTEXT_XOFF;
						fY += LINKTEXT_YOFF;
						fFade = (1.0f - ((fZ - 0.95) * 60.0f));
						if (fFade > 0.0f) {
						  glColor4f ((0.5), (1.0), (0.5), fFade);
						  RenderBitmapString ((float)fX, (float)fY, LINKTEXT_FONT, psLink->szName->str);
						}
					}

		      psLinkListPos = psLinkListPos->next;
		    }
        psNodeListPos = psNodeListPos->next;
      }
      glDisable (GL_BLEND);
    }
    else {
      glColor3f (NODETEXT_COLOUR);

      while (psNodeListPos) {
        psNode = (TNode *)psNodeListPos->data;

		    psLinkListPos = psNode->psLinksOut;
		    while (psLinkListPos) {
		      psLink = ((TLink *)psLinkListPos->data);
		      psLinkTo = psLink->psNodeTo;

					if (psLink->szName->len > 0) {
						gluProject ((psNode->vsPos.fX + psLinkTo->vsPos.fX) / 2.0, (psNode->vsPos.fY + psLinkTo->vsPos.fY) / 2.0, (psNode->vsPos.fZ + psLinkTo->vsPos.fZ) / 2.0, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);
						fX += LINKTEXT_XOFF;
						fY += LINKTEXT_YOFF;
					  RenderBitmapString ((float)fX, (float)fY, LINKTEXT_FONT, psLink->szName->str);
					}

		      psLinkListPos = psLinkListPos->next;
		    }
        psNodeListPos = psNodeListPos->next;
      }
    }
  }

  if (psVisData->psNodeSelected) {
    psNode = psVisData->psNodeSelected;
    gluProject (psNode->vsPos.fX, psNode->vsPos.fY, psNode->vsPos.fZ, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);
    fX += NODETEXT_XOFF;
    fY += NODETEXT_YOFF;
    glColor3f (NODESELTEXT_COLOUR);
    RenderBitmapString ((float)fX, (float)fY, NODETEXT_FONT, psNode->szName->str);
  }

  if (psVisData->psLinkSelected) {
    psLink = psVisData->psLinkSelected;
    if (psLink->szName->len > 0) {
		  gluProject ((psLink->psNodeFrom->vsPos.fX + psLink->psNodeTo->vsPos.fX) / 2.0, (psLink->psNodeFrom->vsPos.fY + psLink->psNodeTo->vsPos.fY) / 2.0, (psLink->psNodeFrom->vsPos.fZ + psLink->psNodeTo->vsPos.fZ) / 2.0, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);
		  fX += LINKTEXT_XOFF;
		  fY += LINKTEXT_YOFF;
		  glColor3f (LINKSELTEXT_COLOUR);
		  RenderBitmapString ((float)fX, (float)fY, LINKTEXT_FONT, psLink->szName->str);
		}
  }

  glEnable(GL_DEPTH_TEST);
  glEnable (GL_LIGHTING);

	UpdateNoteAnchors (psVisData);
	RenderNotes (psVisData->psNotesData);

  glFlush ();

  //glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, gSphereSpecular);
  //glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, gSphereShininess);
  //glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gSphereDiffuseOn);
}


void SelectVisObject (SelectInfo * psSelectData, VisPersist * psVisData) {
  TNode * psNode;
  TLink * psLink;
  TNode * psLinkTo;
  GSList * psNodeListPos;
  GSList * psLinkListPos;
  GLint anViewPort[4];
  GLuint auSelectBuffer[SELBUFSIZE];
  unsigned int uCount;
  unsigned int uNodeCount;
  unsigned int uLinkCount;
  unsigned int uOrder;
  int nHits;
  int nBufferPos;
  int nNamesNum;
  int nNodeSelected;
  int nLinkSelected;
  SELECTOBJECT eObjectType;

  glSelectBuffer (SELBUFSIZE, auSelectBuffer);
	glRenderMode (GL_SELECT);

  glGetIntegerv (GL_VIEWPORT, anViewPort);

  glMatrixMode(GL_PROJECTION);
	glPushMatrix();
  glLoadIdentity ();
  gluPickMatrix (psSelectData->nXPos, anViewPort[3] - psSelectData->nYPos, 1, 1, anViewPort);

  gluPerspective(60, (float)psVisData->nScreenWidth / (float)psVisData->nScreenHeight, 1, 100);
  glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
  glLoadIdentity();

  glInitNames ();

  gluLookAt ((psVisData->fViewRadius) * psVisData->fX, (psVisData->fViewRadius) * psVisData->fY,
    (psVisData->fViewRadius) * psVisData->fZ, 0.0, 0.0, 0.0, psVisData->fXn, psVisData->fYn, psVisData->fZn);

  // Check for node selections
  if (psSelectData->boSelectNodes) {
    glPushName (SELECTOBJECT_NODE);
    uNodeCount = 0;
    glPushName (0);

    // Draw the nodes
    psNodeListPos = psVisData->psNodeList;
    while (psNodeListPos) {
      glLoadName (uNodeCount);
      psNode = (TNode *)psNodeListPos->data;

      DrawNode (psNode->vsPos.fX, psNode->vsPos.fY, psNode->vsPos.fZ, psVisData);

      psNodeListPos = psNodeListPos->next;
      uNodeCount++;
    }
    glPopName ();
    glPopName ();
  }

  // Check for link selections
  if (psSelectData->boSelectLinks) {
    glPushName (SELECTOBJECT_LINKOUT);
    uNodeCount = 0;

    // Draw the links
    psNodeListPos = psVisData->psNodeList;
    while (psNodeListPos) {
      uLinkCount = 0;

      psNode = (TNode *)psNodeListPos->data;

      psLinkListPos = psNode->psLinksOut;
      while (psLinkListPos) {
        psLink = ((TLink *)psLinkListPos->data);
        psLinkTo = psLink->psNodeTo;

        if (psLinkTo != psNode) {
          // Draw the link in two halves, so which know which end is being selected
          DrawSelectArrow (psNode->vsPos.fX,
            psNode->vsPos.fY,
            psNode->vsPos.fZ,
            psLinkTo->vsPos.fX,
            psLinkTo->vsPos.fY,
            psLinkTo->vsPos.fZ,
            uNodeCount,
            uLinkCount,
            psVisData);
        }
        psLinkListPos = psLinkListPos->next;
        uLinkCount++;
      }

      psNodeListPos = psNodeListPos->next;
      uNodeCount++;
    }

    glPopName ();
  }

  glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();
  glFlush ();

	nHits = glRenderMode (GL_RENDER);
  // Find the selected object
  nNodeSelected = -1;
  nLinkSelected = -1;
  psSelectData->psNode = NULL;
  psSelectData->psLink = NULL;
  eObjectType = SELECTOBJECT_INVALID;
  if (nHits != 0) {
    nBufferPos = 0;
    uOrder = -1;
    for (uCount = 0; uCount < nHits; uCount++) {
      nNamesNum = auSelectBuffer[nBufferPos];
      nBufferPos += 3;

      if ((auSelectBuffer[nBufferPos - 1] <= uOrder) && (nNamesNum >= 2)) {
        eObjectType = auSelectBuffer[nBufferPos];
        nNodeSelected = auSelectBuffer[nBufferPos + 1];
        if (nNamesNum >= 3) {
          nLinkSelected = auSelectBuffer[nBufferPos + 2];
        }
        uOrder = auSelectBuffer[nBufferPos - 1];
      }

      nBufferPos += nNamesNum;
    }

    if (eObjectType == SELECTOBJECT_NODE) {
      // Find the node
      uNodeCount = 0;
      psNodeListPos = psVisData->psNodeList;
      while (psNodeListPos) {
        if (nNodeSelected == uNodeCount) {
          psSelectData->psNode = (TNode *)psNodeListPos->data;
        }
        psNodeListPos = psNodeListPos->next;
        uNodeCount++;
      }
    }

    if ((eObjectType == SELECTOBJECT_LINKOUT) || (eObjectType == SELECTOBJECT_LINKIN)) {
      // Find the node
      uNodeCount = 0;
      psNodeListPos = psVisData->psNodeList;
      while (psNodeListPos) {
        if (nNodeSelected == uNodeCount) {
          psSelectData->psNode = (TNode *)psNodeListPos->data;

          // Find the link
          uLinkCount = 0;
          psLinkListPos = psSelectData->psNode->psLinksOut;
          while (psLinkListPos) {
            if (nLinkSelected == uLinkCount) {
              psSelectData->psLink = (TLink *)psLinkListPos->data;
            }
            psLinkListPos = psLinkListPos->next;
            uLinkCount++;
          }
        }
        psNodeListPos = psNodeListPos->next;
        uNodeCount++;
      }
    }
  }

  psSelectData->eObjectType = eObjectType;
}

void DrawSelectArrow (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo, unsigned int uNodeCount, unsigned int uLinkCount, VisPersist * psVisData) {
  float			fLength;
  float     fAngle;
  float     fXMid;
  float     fYMid;
  float     fZMid;

  fXMid = (fXTo + fXFrom)/2.0f;
  fYMid = (fYTo + fYFrom)/2.0f;
  fZMid = (fZTo + fZFrom)/2.0f;
  fLength = sqrt (((fXFrom - fXTo) * (fXFrom - fXTo))
    + ((fYFrom - fYTo) * (fYFrom - fYTo))
    + ((fZFrom - fZTo) * (fZFrom - fZTo)));
  fLength -= (2.0f * NODERADIUS);
  if (fLength < (ARROWHEADHEIGHT + ARROWMINLENGTH)) {
    fLength = (ARROWHEADHEIGHT + ARROWMINLENGTH);
  }

  glPushMatrix();

  glTranslatef (fXMid, fYMid, fZMid);

  fAngle = 360.0f * acos ((fXTo - fXFrom) / sqrt ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))))) / (2.0f * 3.1415926538);
  if (fZTo > fZFrom) fAngle = -fAngle;
  glRotatef (fAngle, 0.0, 1.0, 0.0);

  fAngle = 360.0f * acos ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))) / sqrt (((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom)))) * ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fYTo - fYFrom) * (fYTo - fYFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom)))))) / (2.0f * 3.1415926538);
  // The next line is annoying, but seems to be necessary to stop flickering due to floating point errors. Eergh.
  if (absf (fYTo - fYFrom) < 0.00001) fAngle = 0.0f;
  if (fYTo < fYFrom) fAngle = -fAngle;
  glRotatef (fAngle, 0.0, 0.0, 1.0);

  glLoadName (SELECTOBJECT_LINKIN);
  glPushName (uNodeCount);
  glPushName (uLinkCount);

  // Add the arrowhead
  glTranslatef (((fLength / 2.0f) - ARROWHEADHEIGHT), 0.0, 0.0);
  glRotatef (90.0f, 0.0, 1.0, 0.0);
  glCallList (psVisData->uStartList + GENLIST_ARROWHEAD);
  glRotatef (-90.0f, 0.0, 1.0, 0.0);
  glTranslatef (-((fLength / 2.0f) - ARROWHEADHEIGHT), 0.0, 0.0);

  // Shift half an arrowhead's worth so that the tube doesn't cross the arrowhead
  glTranslatef (-((ARROWHEADHEIGHT / 2.0f)), 0.0, 0.0);

  // Scale the tube to stretch between the two points
  glScalef (((fLength / 2.0f) - (ARROWHEADHEIGHT / 2.0f)), 1.0f, 1.0f);

  // Move so the tube midpoint is at (fXMid, fYMid, fZMid)
  //glTranslatef (0.0, 0.0, 0.0);
  glRotatef (90.0f, 0.0, 1.0, 0.0);

  // Render the end point half of the tube
  // Tubes aren't a glut primitive, but that's okay 'cos we built one earlier
  //glutSolidCone (0.3, 1.0, 10, 10);
  glCallList (psVisData->uStartList);

  glPopName ();
  glPopName ();

  glLoadName (SELECTOBJECT_LINKOUT);
  glPushName (uNodeCount);
  glPushName (uLinkCount);

  glScalef (0.9f, 0.9f, 1.0f);

  // Move so the tube midpoint is at (fXMid, fYMid, fZMid)
  glTranslatef (0.0, 0.0, -1.0);

  // Render the start point half of the tube
  glCallList (psVisData->uStartList);

	glPopMatrix ();

  glPopName ();
  glPopName ();
}

void DrawTextOverlay (VisPersist * psVisData) {
  struct tm * pTm;
  time_t ulTime;

  glDisable (GL_LIGHTING);
  if (psVisData->boClearWhite) {
    glColor3f (0.0, 0.0, 0.0);
  }
  else {
    glColor3f (1.0, 1.0, 1.0);
  }
  ulTime = (unsigned long)psVisData->fCurrentTime;
  pTm = gmtime (& ulTime);
  RenderBitmapString (2.0, psVisData->nScreenHeight - 20, GLUT_BITMAP_HELVETICA_10, asctime (pTm));

  glEnable (GL_LIGHTING);
}

void WindowPos2f (GLfloat fX, GLfloat fY) {
  GLfloat fXn, fYn;

  glPushAttrib (GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);
  glMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();

  glDepthRange (0, 0);
  glViewport ((int) fX - 1, (int) fY - 1, 2, 2);
  fXn = fX - (int) fX;
  fYn = fY - (int) fY;
  glRasterPos4f (fXn, fYn, 0.0, 1);
  glPopMatrix ();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();

  glPopAttrib ();
}

void RenderBitmapString (float fX, float fY, void * pFont, char const * szString) {
  int nPos;
  int nLine;

	nLine = 0;
  WindowPos2f (fX, fY);
  nPos = 0;
  while (szString[nPos] > 0) {
    glutBitmapCharacter (pFont, szString[nPos]);
    if (szString[nPos] == '\n') {
    	nLine++;
		  WindowPos2f (fX, fY - (nLine * 10));
    }
    nPos++;
  }
}

bool PointTowards (float fXPos, float fYPos, float fZPos, float fRadius, VisPersist * psVisData) {
  float fTheta;
  float fPhi;
  float fPsi;

  float fX;
  float fY;
  float fZ;
  float fXp;
  float fYp;
  float fZp;
  float fXn;
  float fYn;
  float fZn;
  bool boArrived = true;

  fXp = (-MGL_WIDTH/2.0f + (float)fXPos) / MGL_SCALE;
  fYp = POINTTOELLEVATION + (MGL_HEIGHT/2.0f - (float)fYPos) / MGL_SCALE;
  fZp = (-MGL_DEPTH/2.0f + (float)fZPos) / MGL_SCALE;

  fTheta = DotProdAngle (fXp, fZp, psVisData->fX, psVisData->fZ) * psVisData->fHalfLife;
  if (absf (fTheta) > POINTTOMINMOVE) {
    fX = (psVisData->fX * cos (fTheta)) + (psVisData->fZ * sin (fTheta));
    fZ = - (psVisData->fX * sin (fTheta)) + (psVisData->fZ * cos (fTheta));
    fXn = (psVisData->fXn * cos (fTheta)) + (psVisData->fZn * sin (fTheta));
    fZn = - (psVisData->fXn * sin (fTheta)) + (psVisData->fZn * cos (fTheta));
    psVisData->fX = fX;
    psVisData->fZ = fZ;
    psVisData->fXn = fXn;
    psVisData->fZn = fZn;
    boArrived = false;
  }

  fPhi = DotProdAngle (fYp, fZp, psVisData->fY, psVisData->fZ) * psVisData->fHalfLife;
  if (absf (fPhi) > POINTTOMINMOVE) {
    fY = (psVisData->fY * cos (fPhi)) + (psVisData->fZ * sin (fPhi));
    fZ = - (psVisData->fY * sin (fPhi)) + (psVisData->fZ * cos (fPhi));
    fYn = (psVisData->fYn * cos (fPhi)) + (psVisData->fZn * sin (fPhi));
    fZn = - (psVisData->fYn * sin (fPhi)) + (psVisData->fZn * cos (fPhi));
    psVisData->fY = fY;
    psVisData->fZ = fZ;
    psVisData->fYn = fYn;
    psVisData->fZn = fZn;
    boArrived = false;
  }

  fPsi = DotProdAngle (fXp, fYp, psVisData->fX, psVisData->fY) * psVisData->fHalfLife;
  if (absf (fPsi) > POINTTOMINMOVE) {
    fX = (psVisData->fX * cos (fPsi)) + (psVisData->fY * sin (fPsi));
    fY = - (psVisData->fX * sin (fPsi)) + (psVisData->fY * cos (fPsi));
    fXn = (psVisData->fXn * cos (fPsi)) + (psVisData->fYn * sin (fPsi));
    fYn = - (psVisData->fXn * sin (fPsi)) + (psVisData->fYn * cos (fPsi));
    psVisData->fX = fX;
    psVisData->fY = fY;
    psVisData->fXn = fXn;
    psVisData->fYn = fYn;
    boArrived = false;
  }

  //psVisData->fY = (psVisData->fY * cos (fPhi)) + (psVisData->fZ * sin (fPhi));
  //psVisData->fZ = - (psVisData->fY * sin (fPhi)) + (psVisData->fZ * cos (fPhi));

  //psVisData->fX = (psVisData->fX * cos (fPsi)) + (psVisData->fY * sin (fPsi));
  //psVisData->fY = - (psVisData->fX * sin (fPsi)) + (psVisData->fY * cos (fPsi));

  Normalise3f (& psVisData->fX, & psVisData->fY, & psVisData->fZ);

  return boArrived;
}

void Idle (VisPersist * psVisData) {
  struct timeb sTime;

  psVisData->fPrevTime = psVisData->fCurrentTime;
  ftime (& sTime);
  psVisData->fCurrentTime = (double)(sTime.time) + (double)(sTime.millitm) / 1000.0;
  if (psVisData->fCurrentTime < 0) psVisData->fCurrentTime = 0.0;

  KeyIdle (psVisData);

  RearrangeNetwork (psVisData);

  MomentumSpin (psVisData);

  if (!psVisData->boArrived) {
    psVisData->boArrived = PointTowards (psVisData->psPointNode->vsPos.fX, psVisData->psPointNode->vsPos.fY,
      psVisData->psPointNode->vsPos.fZ, 0.0f, psVisData);
  }
  else {
    if ((psVisData->boSpin) && (psVisData->eDrag != DRAG_VIEW)) {
      Spin (psVisData);
    }
  }

	UpdateNotes (psVisData->psNotesData);

  //glutPostRedisplay();
}

void RearrangeNetwork (VisPersist * psVisData) {
  TNode * psNode;
  GSList * psNodeListPos;
  float fForceScale;
  float fResistance;

	// Adjust the resistance slightly to tackle low numbers of nodes
	fResistance = psVisData->fArrangeResistance * (1.0f - (1.0f / (1.0 + pow (psVisData->nNodes, 10.0))));

  psNodeListPos = psVisData->psNodeList;
  while (psNodeListPos) {
    psNode = (TNode *)psNodeListPos->data;

    // Apply the arrangement forces
    ApplyExpulsion (psNode, psVisData);
    ApplyLinkForce (psNode, psVisData);
    ApplyCentring (psNode, psVisData);

    // Apply the general resistance
    psNode->vsVel.fX *= fResistance;
    psNode->vsVel.fY *= fResistance;
    psNode->vsVel.fZ *= fResistance;

    // Apply resistance proportional to the number of links in and out of the node
    // This reduces jitter caused by multiple competing forces
    fForceScale = (((psNode->nLinksIn + psNode->nLinksOut) * psVisData->fArrangeLinkScalar) + 1.0);
    psNode->vsVel.fX /= fForceScale;
    psNode->vsVel.fY /= fForceScale;
    psNode->vsVel.fZ /= fForceScale;

    // Restrict the velocity within certain limits
    if (psNode->vsVel.fX < BOUNDSZMIN) {
      psNode->vsVel.fX = BOUNDSZMIN;
    }
    if (psNode->vsVel.fX > BOUNDSVMAX) {
      psNode->vsVel.fX = BOUNDSVMAX;
    }
    if (psNode->vsVel.fY < BOUNDSZMIN) {
      psNode->vsVel.fY = BOUNDSZMIN;
    }
    if (psNode->vsVel.fY > BOUNDSVMAX) {
      psNode->vsVel.fY = BOUNDSVMAX;
    }
    if (psNode->vsVel.fZ < BOUNDSZMIN) {
      psNode->vsVel.fZ = BOUNDSZMIN;
    }
    if (psNode->vsVel.fZ > BOUNDSVMAX) {
      psNode->vsVel.fZ = BOUNDSVMAX;
    }

    // Update position based on velocity
    psNode->vsPos.fX += psNode->vsVel.fX;
    psNode->vsPos.fY += psNode->vsVel.fY;
    psNode->vsPos.fZ += psNode->vsVel.fZ;

    // Restrict the position within certain limits
    if (psNode->vsPos.fX < BOUNDSXMIN) {
      psNode->vsPos.fX = BOUNDSXMIN;
      psNode->vsVel.fX = 0;
    }
    if (psNode->vsPos.fX > BOUNDSXMAX) {
      psNode->vsPos.fX = BOUNDSXMAX;
      psNode->vsVel.fX = 0;
    }
    if (psNode->vsPos.fY < BOUNDSYMIN) {
      psNode->vsPos.fY = BOUNDSYMIN;
      psNode->vsVel.fY = 0;
    }
    if (psNode->vsPos.fY > BOUNDSYMAX) {
      psNode->vsPos.fY = BOUNDSYMAX;
      psNode->vsVel.fY = 0;
    }
    if (psNode->vsPos.fZ < BOUNDSZMIN) {
      psNode->vsPos.fZ = BOUNDSZMIN;
      psNode->vsVel.fZ = 0;
    }
    if (psNode->vsPos.fZ > BOUNDSZMAX) {
      psNode->vsPos.fZ = BOUNDSZMAX;
      psNode->vsVel.fZ = 0;
    }

    psNodeListPos = psNodeListPos->next;
  }
}

void ApplyExpulsion (TNode * psNode, VisPersist * psVisData) {
  float fXForce, fYForce, fZForce, fDisq;
  TNode * psNodeCount;
  GSList * psNodeListPos;

  fXForce = 0.0f;
  fYForce = 0.0f;
  fZForce = 0.0f;

  psNodeListPos = psVisData->psNodeList;
  while (psNodeListPos) {
    psNodeCount = (TNode *)psNodeListPos->data;

    if (psNodeCount != psNode) {
      fDisq = ((psNodeCount->vsPos.fX - psNode->vsPos.fX) * (psNodeCount->vsPos.fX - psNode->vsPos.fX))
        + ((psNodeCount->vsPos.fY - psNode->vsPos.fY) * (psNodeCount->vsPos.fY - psNode->vsPos.fY))
        + ((psNodeCount->vsPos.fZ - psNode->vsPos.fZ) * (psNodeCount->vsPos.fZ - psNode->vsPos.fZ));
      if (fDisq < ARRANGEMINDIST) fDisq = ARRANGEMINDIST;
      fXForce += (psNodeCount->vsPos.fX - psNode->vsPos.fX) / fDisq;
      fYForce += (psNodeCount->vsPos.fY - psNode->vsPos.fY) / fDisq;
      fZForce += (psNodeCount->vsPos.fZ - psNode->vsPos.fZ) / fDisq;
    }
    psNodeListPos = psNodeListPos->next;
  }

  psNode->vsVel.fX -= psVisData->fArrangeForce * fXForce;
  psNode->vsVel.fY -= psVisData->fArrangeForce * fYForce;
  psNode->vsVel.fZ -= psVisData->fArrangeForce * fZForce;
}

void ApplyLinkForce (TNode * psNode, VisPersist * psVisData) {
  TNode * psNodeLinked;
  float fForce, fXForce, fYForce, fZForce, fDist, fFactor;
  GSList * psLinkListPos;
  TLink * psLink;

  fXForce = 0.0f;
  fYForce = 0.0f;
  fZForce = 0.0f;
  psLinkListPos = psNode->psLinksOut;
  while (psLinkListPos) {
    psLink = (TLink *)psLinkListPos->data;

    psNodeLinked = psLink->psNodeTo;
    if (psVisData->fCurrentTime >= psLink->fNewLinkTime) {
      if (psNode != psNodeLinked) {
        fDist = sqrt (((psNodeLinked->vsPos.fX - psNode->vsPos.fX)
          * (psNodeLinked->vsPos.fX - psNode->vsPos.fX))
          + ((psNodeLinked->vsPos.fY - psNode->vsPos.fY)
          * (psNodeLinked->vsPos.fY - psNode->vsPos.fY))
          + ((psNodeLinked->vsPos.fZ - psNode->vsPos.fZ)
          * (psNodeLinked->vsPos.fZ - psNode->vsPos.fZ)));
        if (fDist < ARRANGEMINDIST) fDist = ARRANGEMINDIST;
        fFactor = (fDist - psVisData->fArrangeLinklen) * psVisData->fArrangeRigidity / fDist;
        // The 'equal and opposite' force is required to produce a stable system
        // A better solution might be to keep track of outgoing as well as incoming links
        fForce = ((psNodeLinked->vsPos.fX - psNode->vsPos.fX) * fFactor);
        psNodeLinked->vsVel.fX -= fForce;
        fXForce += fForce;

        fForce = ((psNodeLinked->vsPos.fY - psNode->vsPos.fY) * fFactor);
        psNodeLinked->vsVel.fY -= fForce;
        fYForce += fForce;

        fForce = ((psNodeLinked->vsPos.fZ - psNode->vsPos.fZ) * fFactor);
        psNodeLinked->vsVel.fZ -= fForce;
        fZForce += fForce;
      }
    }
    psLinkListPos = psLinkListPos->next;
  }

  psNode->vsVel.fX += fXForce;
  psNode->vsVel.fY += fYForce;
  psNode->vsVel.fZ += fZForce;
}

void ApplyCentring (TNode * psNode, VisPersist * psVisData) {
  float fDist;

  fDist = sqrt ((psNode->vsPos.fX * psNode->vsPos.fX)
    + (psNode->vsPos.fY * psNode->vsPos.fY)
    + (psNode->vsPos.fZ * psNode->vsPos.fZ));

  psNode->vsVel.fX -= psNode->vsPos.fX * fDist * ARRANGECENTRINGFUNC;
  psNode->vsVel.fY -= psNode->vsPos.fY * fDist * ARRANGECENTRINGFUNC;
  psNode->vsVel.fZ -= psNode->vsPos.fZ * fDist * ARRANGECENTRINGFUNC;
}

void Shake (VisPersist * psVisData) {
  TNode * psNode;
  GSList * psNodeListPos;

  psNodeListPos = psVisData->psNodeList;
  while (psNodeListPos) {
    psNode = (TNode *)psNodeListPos->data;

    psNode->vsVel.fX += ARRANGESHAKEVEL * 2.0f * (((float)rand () / (float)RAND_MAX) - 0.5f);
    psNode->vsVel.fY += ARRANGESHAKEVEL * 2.0f * (((float)rand () / (float)RAND_MAX) - 0.5f);
    psNode->vsVel.fZ += ARRANGESHAKEVEL * 2.0f * (((float)rand () / (float)RAND_MAX) - 0.5f);

    psNodeListPos = psNodeListPos->next;
  }
}

void Reshape (int nWidth, int nHeight, VisPersist * psVisData) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, nWidth, nHeight);

  psVisData->nScreenWidth = nWidth;
  psVisData->nScreenHeight = nHeight;

  if (!psVisData->boFullScreen) {
    psVisData->nPrevScreenWidth = psVisData->nScreenWidth;
    psVisData->nPrevScreenHeight = psVisData->nScreenHeight;
  }

  gluPerspective(60, (float)nWidth / (float)nHeight, 1, 100);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt ((psVisData->fViewRadius) * psVisData->fX, (psVisData->fViewRadius) * psVisData->fY,
    (psVisData->fViewRadius) * psVisData->fZ, 0.0, 0.0, 0.0, psVisData->fXn, psVisData->fYn, psVisData->fZn);
}

void Redraw (VisPersist * psVisData) {
  if (psVisData->boClearWhite) {
    glClearColor (1.0f, 1.0f, 1.0f, 0.0f);
  }
  else {
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
  }
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix ();
  Display (psVisData);
  DrawTextOverlay (psVisData);
  glPopMatrix ();

  //glutSwapBuffers();
}

void Mouse (int button, int state, int x, int y, VisPersist * psVisData) {
  if (button == LEFT_BUTTON && state == BUTTON_DOWN) {
    MouseDown (x, y, psVisData);
  }
  if (button == LEFT_BUTTON && state == BUTTON_UP) {
    // Mouse click up or finished drag
    switch (psVisData->eDrag) {
    default:
    case DRAG_VIEW:
      // Do nothing
      break;
    case DRAG_LINKOUT:
      DragFinishLinkOut (x, y, psVisData);
      break;
    case DRAG_LINKIN:
      DragFinishLinkIn (x, y, psVisData);
      break;
    }
    psVisData->eDrag = DRAG_NONE;
    psVisData->fMomentum = 1.0f;
  }
}

void MouseDown (int nXPos, int nYPos, VisPersist * psVisData) {
  SelectInfo sSelectData;

	//MoveAnchor ((float)nXPos, (float)nYPos, psVisData);

  // Mouse click or start of drag
  psVisData->nXMouse = nXPos;
  psVisData->nYMouse = nYPos;
  psVisData->fMomentum = 0.0f;
  psVisData->fXMomentum = 0.0f;
  psVisData->fYMomentum = 0.0f;
  psVisData->fZMomentum = 0.0f;

  // Find out which object was selected
  sSelectData.boSelectNodes = psVisData->boNodeSelections;
  sSelectData.boSelectLinks = (psVisData->boDrawLinks && psVisData->boLinkChanges);
  sSelectData.nXPos = nXPos;
  sSelectData.nYPos = nYPos;
  SelectVisObject (& sSelectData, psVisData);

  psVisData->eSelectObject = sSelectData.eObjectType;

  switch (sSelectData.eObjectType) {
  case SELECTOBJECT_INVALID:
    psVisData->eDrag = DRAG_VIEW;
    break;
  case SELECTOBJECT_NODE:
    // Clear the selected link
		SetLinkSelected (NULL, NULL, psVisData);
    if (psVisData->psNodeSelected == sSelectData.psNode) {
      // If the node was already selected, deselect it
			SetNodeSelected (NULL, psVisData);
    }
    else {
      // If the node was not selected, select it
			SetNodeSelected (sSelectData.psNode, psVisData);
    }
    break;
  case SELECTOBJECT_LINKOUT:
		SetNodeSelected (sSelectData.psNode, psVisData);
		SetLinkSelected (sSelectData.psNode, sSelectData.psLink, psVisData);
    psVisData->eDrag = DRAG_LINKOUT;

    // Create a ghost link
    if (psVisData->psChangeArrow == NULL) {
      psVisData->psChangeArrow = g_new0 (ChangeArrow, 1);
    }
    psVisData->psChangeArrow->psOldFrom = sSelectData.psNode;
    psVisData->psChangeArrow->psOldTo = sSelectData.psLink->psNodeTo;
    psVisData->psChangeArrow->psNewFrom = sSelectData.psNode;
    psVisData->psChangeArrow->psNewTo = sSelectData.psLink->psNodeTo;
    psVisData->psChangeArrow->psLink = sSelectData.psLink;
    psVisData->psChangeArrow->fStartTime = psVisData->fCurrentTime;
    psVisData->psChangeArrow->fDuration = 1.0f;
    break;
  case SELECTOBJECT_LINKIN:
		SetNodeSelected (sSelectData.psLink->psNodeTo, psVisData);
		SetLinkSelected (sSelectData.psNode, sSelectData.psLink, psVisData);
    psVisData->eDrag = DRAG_LINKIN;

    // Create a ghost link
    if (psVisData->psChangeArrow == NULL) {
      psVisData->psChangeArrow = g_new0 (ChangeArrow, 1);
    }
    psVisData->psChangeArrow->psOldFrom = sSelectData.psNode;
    psVisData->psChangeArrow->psOldTo = sSelectData.psLink->psNodeTo;
    psVisData->psChangeArrow->psNewFrom = sSelectData.psNode;
    psVisData->psChangeArrow->psNewTo = sSelectData.psLink->psNodeTo;
    psVisData->psChangeArrow->psLink = sSelectData.psLink;
    psVisData->psChangeArrow->fStartTime = psVisData->fCurrentTime;
    psVisData->psChangeArrow->fDuration = 1.0f;
    break;
  default:
    // Do nothing;
    break;
  }
}

void AnimateGhostLinkOut (int nXPos, int nYPos, VisPersist * psVisData) {
  SelectInfo sSelectData;

  if (psVisData->psChangeArrow) {
    // Check whether we're hovering over a node
    sSelectData.boSelectNodes = true;
    sSelectData.boSelectLinks = false;
    sSelectData.nXPos = nXPos;
    sSelectData.nYPos = nYPos;
    SelectVisObject (& sSelectData, psVisData);

    if (sSelectData.eObjectType == SELECTOBJECT_NODE) {
      if (sSelectData.psNode != psVisData->psChangeArrow->psNewFrom) {
        psVisData->psChangeArrow->psOldFrom = psVisData->psChangeArrow->psNewFrom;
        psVisData->psChangeArrow->psNewFrom = sSelectData.psNode;
        psVisData->psChangeArrow->fStartTime = psVisData->fCurrentTime;
        psVisData->psChangeArrow->fDuration = 1.0f;
      }
    }
  }
}

void AnimateGhostLinkIn (int nXPos, int nYPos, VisPersist * psVisData) {
  SelectInfo sSelectData;

  if (psVisData->psChangeArrow) {
    // Check whether we're hovering over a node
    sSelectData.boSelectNodes = true;
    sSelectData.boSelectLinks = false;
    sSelectData.nXPos = nXPos;
    sSelectData.nYPos = nYPos;
    SelectVisObject (& sSelectData, psVisData);

    if (sSelectData.eObjectType == SELECTOBJECT_NODE) {
      if (sSelectData.psNode != psVisData->psChangeArrow->psNewTo) {
        psVisData->psChangeArrow->psOldTo = psVisData->psChangeArrow->psNewTo;
        psVisData->psChangeArrow->psNewTo = sSelectData.psNode;
        psVisData->psChangeArrow->fStartTime = psVisData->fCurrentTime;
        psVisData->psChangeArrow->fDuration = GHOSTLINK_DURATION;
      }
    }
  }
}

void DragFinishLinkOut (int nXPos, int nYPos, VisPersist * psVisData) {
  SelectInfo sSelectData;
  gboolean boChanged;

  // Remove the ghost link
  g_free (psVisData->psChangeArrow);
  psVisData->psChangeArrow = NULL;

  // Check whether we finished on a node
  sSelectData.boSelectNodes = true;
  sSelectData.boSelectLinks = false;
  sSelectData.nXPos = nXPos;
  sSelectData.nYPos = nYPos;
  SelectVisObject (& sSelectData, psVisData);

  psVisData->eSelectObject = sSelectData.eObjectType;

  if (sSelectData.eObjectType == SELECTOBJECT_NODE) {
    if (sSelectData.psNode != psVisData->psNodeSelected) {
      // Broadcast a CHANGELINK message to all connected client
      boChanged = ServerBroadcastCHANGELINK (psVisData->psLinkSelectedFrom->szName->str, psVisData->psLinkSelected->psNodeTo->szName->str, sSelectData.psNode->szName->str, psVisData->psLinkSelected->psNodeTo->szName->str, psVisData->psServerData);

      if (boChanged) {
        // Need to change the link
        ChangeLink (psVisData->psLinkSelected, sSelectData.psNode, psVisData->psLinkSelected->psNodeTo, psVisData);

        // Deselect the link and node
				SetNodeSelected (NULL, psVisData);
				SetLinkSelected (NULL, NULL, psVisData);
      }
    }
  }
}

void DragFinishLinkIn (int nXPos, int nYPos, VisPersist * psVisData) {
  SelectInfo sSelectData;
  gboolean boChanged;

  // Remove the ghost link
  g_free (psVisData->psChangeArrow);
  psVisData->psChangeArrow = NULL;

  // Check whether we finished on a node
  sSelectData.boSelectNodes = true;
  sSelectData.boSelectLinks = false;
  sSelectData.nXPos = nXPos;
  sSelectData.nYPos = nYPos;
  SelectVisObject (& sSelectData, psVisData);

  psVisData->eSelectObject = sSelectData.eObjectType;

  if (sSelectData.eObjectType == SELECTOBJECT_NODE) {
    if (sSelectData.psNode != psVisData->psNodeSelected) {
      // Broadcast a CHANGELINK message to all connected client
      boChanged = ServerBroadcastCHANGELINK (psVisData->psLinkSelectedFrom->szName->str, psVisData->psLinkSelected->psNodeTo->szName->str, psVisData->psLinkSelectedFrom->szName->str, sSelectData.psNode->szName->str, psVisData->psServerData);

      if (boChanged) {
        // Need to change the link

        ChangeLink (psVisData->psLinkSelected, psVisData->psLinkSelectedFrom, sSelectData.psNode, psVisData);

        // Deselect the link and node
				SetNodeSelected (NULL, psVisData);
				SetLinkSelected (NULL, NULL, psVisData);
      }
    }
  }
}

void ChangeLink (TLink *psLink, TNode * psNewFrom, TNode *psNewTo, VisPersist * psVisData) {
  GSList * psListItem;
  TNode * psOldFrom;

  psOldFrom = psLink->psNodeFrom;

  // We're not going to allow links from a node to itself. It'll mess up our arc cosines.
  // We also have to ensure this is actually a change
  if ((psNewFrom != psNewTo)
    && ((psNewFrom != psOldFrom) || (psLink->psNodeTo != psNewTo))) {
    // Move first matching link from previous list

    // Remove the link from the existing node
    psListItem = g_slist_find (psOldFrom->psLinksOut, psLink);
    if (psListItem) {
      psOldFrom->psLinksOut = g_slist_delete_link (psOldFrom->psLinksOut, psListItem);
      psOldFrom->nLinksOut--;
      psLink->psNodeTo->nLinksIn--;
    }

    // Set up the new data for the link
    psLink->psNodeTo = psNewTo;
    psLink->psNodeFrom = psNewFrom;
    psLink->fNewLinkTime = psVisData->fCurrentTime + NEWLINK_DELAY;

    // Attach the link to its new node
    psNewFrom->psLinksOut = g_slist_prepend (psNewFrom->psLinksOut, psLink);
    psNewFrom->nLinksOut++;
    psNewTo->nLinksIn++;
  }
}

void MomentumSpin (VisPersist * psVisData) {
  if (psVisData->eDrag == DRAG_VIEW) {
    psVisData->fXMomentum = 0.0f;
    psVisData->fYMomentum = 0.0f;
    psVisData->fZMomentum = 0.0f;
  }
  if (psVisData->fMomentum > MOMENTUM_MIN) {
    ChangeView ((psVisData->fXMomentum * psVisData->fMomentum),
      (psVisData->fYMomentum * psVisData->fMomentum),
      (psVisData->fZMomentum * psVisData->fMomentum), 0.0f, psVisData);

    psVisData->fMomentum *= (pow (MOMENTUM_RESISTANCE, 100.0 * (psVisData->fCurrentTime - psVisData->fPrevTime)));
  }
}

void ToggleFullScreen (VisPersist * psVisData) {
  //if (psVisData->boFullScreen) {
  //  glutReshapeWindow (psVisData->nPrevScreenWidth, psVisData->nPrevScreenHeight);
  //}
  //else {
  //  glutFullScreen ();
  //}
  psVisData->boFullScreen = !psVisData->boFullScreen;
}

bool GetFullScreen (VisPersist * psVisData) {
  return psVisData->boFullScreen;
}

void ToggleClearWhite (VisPersist * psVisData) {
  psVisData->boClearWhite = !psVisData->boClearWhite;
  SetNoteInverted (psVisData->boClearWhite, psVisData->psNotesData);
}

void ToggleAddToPlane (VisPersist * psVisData) {
  TNode * psNode;
  GSList * psNodeListPos;

  psVisData->boAddToPlane = !psVisData->boAddToPlane;

  if (psVisData->boAddToPlane) {
    // Force all of the nodes back onto the X-Z plane.
    psNodeListPos = psVisData->psNodeList;
    while (psNodeListPos) {
      psNode = (TNode *)psNodeListPos->data;
      psNode->vsPos.fY = 0;
      psNode->vsVel.fY = 0;
      psNodeListPos = psNodeListPos->next;
    }
  }
}

void ToggleSpin (VisPersist * psVisData) {
  psVisData->boSpin = !psVisData->boSpin;
}

void TogglePointTowards (VisPersist * psVisData) {
  psVisData->boFollowNode = !psVisData->boFollowNode;
  if (!psVisData->boFollowNode) {
    psVisData->boArrived = TRUE;
  }
}

void ToggleLinks (VisPersist * psVisData) {
  psVisData->boDrawLinks = !psVisData->boDrawLinks;
}

void ToggleNodeOverlay (VisPersist * psVisData) {
  psVisData->boNodeOverlay = !psVisData->boNodeOverlay;
}

void SetNodeOverlay (bool boNodeOverlay, VisPersist * psVisData) {
  psVisData->boNodeOverlay = boNodeOverlay;
}

bool GetNodeOverlay (VisPersist * psVisData) {
  return psVisData->boNodeOverlay;
}

void SetLinkOverlay (bool boLinkOverlay, VisPersist * psVisData) {
  psVisData->boLinkOverlay = boLinkOverlay;
}

bool GetLinkOverlay (VisPersist * psVisData) {
  return psVisData->boLinkOverlay;
}

void SetFadeText (bool boFadeText, VisPersist * psVisData) {
  psVisData->boFadeText = boFadeText;
}

bool GetFadeText (VisPersist * psVisData) {
  return psVisData->boFadeText;
}

void SetNodeSelections (bool boNodeSelections, VisPersist * psVisData) {
  psVisData->boNodeSelections = boNodeSelections;

  if ((!boNodeSelections) && (psVisData->psNodeSelected)) {
		SetNodeSelected (NULL, psVisData);
 }
}

bool GetNodeSelections (VisPersist * psVisData) {
  return psVisData->boNodeSelections;
}

void SetLinkChanges (bool boLinkChanges, VisPersist * psVisData) {
  psVisData->boLinkChanges = boLinkChanges;
}

bool GetLinkChanges (VisPersist * psVisData) {
  return psVisData->boLinkChanges;
}

void SetClearWhite (bool boClearWhite, VisPersist * psVisData) {
  psVisData->boClearWhite = boClearWhite;
  SetNoteInverted (boClearWhite, psVisData->psNotesData);
}

bool GetClearWhite (VisPersist * psVisData) {
  return psVisData->boClearWhite;
}

void SetAddToPlane (bool boAddToPlane, VisPersist * psVisData) {
  TNode * psNode;
  GSList * psNodeListPos;

  psVisData->boAddToPlane = boAddToPlane;

  if (psVisData->boAddToPlane) {
    // Force all of the nodes back onto the X-Z plane.
    psNodeListPos = psVisData->psNodeList;
    while (psNodeListPos) {
      psNode = (TNode *)psNodeListPos->data;
      psNode->vsPos.fY = 0;
      psNode->vsVel.fY = 0;
      psNodeListPos = psNodeListPos->next;
    }
  }
}

bool GetAddToPlane (VisPersist * psVisData) {
  return psVisData->boAddToPlane;
}

void GetDisplayProperties (float * pfViewRadius, float * pfLinkLen, float * pfCentring, float * pfRigidity, float * pfForce, float * pfResistance, float * pfLinkScalar, VisPersist * psVisData) {
  if (pfViewRadius) {
    * pfViewRadius = psVisData->fViewRadius;
  }
  if (pfLinkLen) {
    * pfLinkLen = psVisData->fArrangeLinklen;
  }
  if (pfCentring) {
    * pfCentring = psVisData->fArrangeCentring;
  }
  if (pfRigidity) {
    * pfRigidity = psVisData->fArrangeRigidity;
  }
  if (pfForce) {
    * pfForce = psVisData->fArrangeForce;
  }
  if (pfResistance) {
    * pfResistance = psVisData->fArrangeResistance;
  }
  if (pfLinkScalar) {
    * pfLinkScalar = psVisData->fArrangeLinkScalar;
  }
}

void SetDisplayProperties (float fViewRadius, float fLinkLen, float fCentring, float fRigidity, float fForce, float fResistance, float fLinkScalar, VisPersist * psVisData) {
  psVisData->fViewRadius = fViewRadius;
  psVisData->fArrangeLinklen = fLinkLen;
  psVisData->fArrangeCentring = fCentring;
  psVisData->fArrangeRigidity = fRigidity;
  psVisData->fArrangeForce = fForce;
  psVisData->fArrangeResistance = fResistance;
  psVisData->fArrangeLinkScalar = fLinkScalar;
}

void Key (unsigned char key, int x, int y, unsigned int uKeyModifiers, VisPersist * psVisData) {
  char szName[256];
  TNode * psNodeFrom = NULL;
  TNode * psNodeTo = NULL;

  //GLint nKeyModifiers;
  //nKeyModifiers = glutGetModifiers ();

  //if (key < MAXKEYS)
  psVisData->aboKeyDown[key] = true;

  switch (key) {
    case 27:
      exit(0);
      break;
    case 's':
      Shake (psVisData);
      break;
    case 'p':
      psVisData->boFollowNode = !psVisData->boFollowNode;
      if (!psVisData->boFollowNode) {
        psVisData->boArrived = TRUE;
      }
      break;
    case 't':
	    psVisData->boLinkOverlay = !psVisData->boLinkOverlay;
      if (psVisData->boLinkOverlay) {
	      psVisData->boNodeOverlay = !psVisData->boNodeOverlay;
		    if (psVisData->boNodeOverlay) {
			    ToggleNoteDisplay (psVisData->psNotesData);
		    }
      }
      break;
    //case '\r':
    //  if (uKeyModifiers & GLUT_ACTIVE_ALT) ToggleFullScreen (VisPersist * psVisData);
    //  break;
    case 'w':
      psVisData->boClearWhite = !psVisData->boClearWhite;
		  SetNoteInverted (psVisData->boClearWhite, psVisData->psNotesData);
      break;
    case 'l':
      psVisData->boDrawLinks = !psVisData->boDrawLinks;
      break;
    case '[':
      psVisData->fArrangeResistance -= 0.02f;
      if (psVisData->fArrangeResistance < 0.0f) psVisData->fArrangeResistance = 0.0f;
      break;
    case ']':
      psVisData->fArrangeResistance += 0.02f;
      if (psVisData->fArrangeResistance > 1.0f) psVisData->fArrangeResistance = 1.0f;
      break;
    case '-':
      psVisData->fArrangeRigidity -= 0.005f;
      if (psVisData->fArrangeRigidity < 0.0f) psVisData->fArrangeRigidity = 0.0f;
      break;
    case '=':
      psVisData->fArrangeRigidity += 0.005f;
      if (psVisData->fArrangeRigidity > 1.0f) psVisData->fArrangeRigidity = 1.0f;
      break;
    case '{':
      psVisData->fArrangeForce -= 0.005f;
      if (psVisData->fArrangeForce < 0.0f) psVisData->fArrangeForce = 0.0f;
      break;
    case '}':
      psVisData->fArrangeForce += 0.005f;
      if (psVisData->fArrangeForce > 1.0f) psVisData->fArrangeForce = 1.0f;
      break;
    case 'r':
      ResetAnimation (psVisData);
      break;
    case 'o':
      psVisData->boSpin = !psVisData->boSpin;
      break;
    case '1':
      psNodeTo = NULL;
      if (psVisData->nNodes > 0) {
        psNodeTo = (TNode *)g_slist_nth_data (psVisData->psNodeList, (rand () % psVisData->nNodes));
      }
      sprintf (szName, "%d", psVisData->nNodes);
      AddNode (szName, psVisData);
      psNodeFrom = g_hash_table_lookup (psVisData->psNodes, szName);
      if (psNodeFrom && psNodeTo) {
        AddLinkDirect ("", psNodeFrom, psNodeTo, true, psVisData);
      }
      break;
    case '2':
      psNodeFrom = (TNode *)g_slist_nth_data (psVisData->psNodeList, (rand () % psVisData->nNodes));
      psNodeTo = (TNode *)g_slist_nth_data (psVisData->psNodeList, (rand () % psVisData->nNodes));

      if (psNodeFrom && psNodeTo) {
        AddLinkDirect ("", psNodeFrom, psNodeTo, false, psVisData);
      }
      break;
    case '0':
      RemoveAll (psVisData);
      break;
  }
}

void KeyUp (unsigned char key, int x, int y, unsigned int uKeyModifiers, VisPersist * psVisData) {
  //if (key < MAXKEYS)
  psVisData->aboKeyDown[key] = false;

  switch (key) {
    case 's':
      // Do nothing
      break;
  }
}

void KeyIdle (VisPersist * psVisData) {
  if (psVisData->aboKeyDown['m']) ChangeView (0.0f, 0.0f, 0.0f, -RADIUSSTEP, psVisData);
  if (psVisData->aboKeyDown['n']) ChangeView (0.0f, 0.0f, 0.0f, +RADIUSSTEP, psVisData);
  if (psVisData->aboKeyDown[',']) ChangeView (0.0f, 0.0f, +PSISTEP, 0.0f, psVisData);
  if (psVisData->aboKeyDown['.']) ChangeView (0.0f, 0.0f, -PSISTEP, 0.0f, psVisData);

  //if (gboKeyLeft) {
  //  gvVel.fX -= SPHERESPEED;
  //  gvVel.fX *= SPHEREAIRRES;
  //}
  //if (gboKeyRight) {
  //  gvVel.fX += SPHERESPEED;
  //  gvVel.fX *= SPHEREAIRRES;
  //}
  //if (gboKeyUp) {
  //  gvVel.fY += SPHERESPEED;
  //  gvVel.fY *= SPHEREAIRRES;
  //}
  //if (gboKeyDown) {
  //  gvVel.fY -= SPHERESPEED;
  //  gvVel.fY *= SPHEREAIRRES;
  //}

  //if (gboMouseRight) {
  //  gfVVel[(int)(WIDTH / 2) + 2][(int)(HEIGHT / 2) - 3] += STRENGTH;
  //}
}

void ChangeView (float fTheta, float fPhi, float fPsi, float fRadius, VisPersist * psVisData) {
  float fA;
  float fB;
  float fX;
  float fY;
  float fZ;
  float fXn;
  float fYn;
  float fZn;

  float fXv;
  float fYv;
  float fZv;

  psVisData->fViewRadius += fRadius;

  // Phi
  fA = cos (fPhi);
  fB = sin (fPhi);

  fX = (fA * psVisData->fX) + (fB * psVisData->fXn);
  fY = (fA * psVisData->fY) + (fB * psVisData->fYn);
  fZ = (fA * psVisData->fZ) + (fB * psVisData->fZn);

  fXn = - (fB * psVisData->fX) + (fA * psVisData->fXn);
  fYn = - (fB * psVisData->fY) + (fA * psVisData->fYn);
  fZn = - (fB * psVisData->fZ) + (fA * psVisData->fZn);

  psVisData->fX = fX;
  psVisData->fY = fY;
  psVisData->fZ = fZ;

  psVisData->fXn = fXn;
  psVisData->fYn = fYn;
  psVisData->fZn = fZn;

  // Theta
  fXv = (psVisData->fY * psVisData->fZn) - (psVisData->fZ * psVisData->fYn);
  fYv = (psVisData->fZ * psVisData->fXn) - (psVisData->fX * psVisData->fZn);
  fZv = (psVisData->fX * psVisData->fYn) - (psVisData->fY * psVisData->fXn);

  fA = cos (fTheta);
  fB = sin (fTheta);

  fX = (fA * psVisData->fX) + (fB * fXv);
  fY = (fA * psVisData->fY) + (fB * fYv);
  fZ = (fA * psVisData->fZ) + (fB * fZv);

  psVisData->fX = fX;
  psVisData->fY = fY;
  psVisData->fZ = fZ;

  // Psi
  fA = cos (fPsi);
  fB = sin (fPsi);

  fXv = (psVisData->fY * psVisData->fZn) - (psVisData->fZ * psVisData->fYn);
  fYv = (psVisData->fZ * psVisData->fXn) - (psVisData->fX * psVisData->fZn);
  fZv = (psVisData->fX * psVisData->fYn) - (psVisData->fY * psVisData->fXn);

  fXn = (fA * fXn) - (fB * fXv);
  fYn = (fA * fYn) - (fB * fYv);
  fZn = (fA * fZn) - (fB * fZv);

  psVisData->fXn = fXn;
  psVisData->fYn = fYn;
  psVisData->fZn = fZn;

  // Normalise vectors (they should already be, but we make sure to avoid
  // cumulative rounding errors)

  Normalise3f (& psVisData->fX, & psVisData->fY, & psVisData->fZ);
  Normalise3f (& psVisData->fXn, & psVisData->fYn, & psVisData->fZn);

	UpdateNotes (psVisData->psNotesData);
}

void Motion (int nX, int nY, VisPersist * psVisData) {
  switch (psVisData->eDrag) {
  case DRAG_VIEW:
  	//MoveAnchor ((float)nX, (float)nY, psVisData);

    ChangeView ((nX - psVisData->nXMouse) / MOUSE_ROTATE_SCALE,
      (nY - psVisData->nYMouse) / MOUSE_ROTATE_SCALE, 0.0f, 0.0f, psVisData);

    psVisData->fXMomentum = ((float)nX - (float)psVisData->nXMouse) / MOUSE_ROTATE_SCALE;
    psVisData->fYMomentum = ((float)nY - (float)psVisData->nYMouse) / MOUSE_ROTATE_SCALE;
    psVisData->fZMomentum = 0.0f;

    psVisData->nXMouse = nX;
    psVisData->nYMouse = nY;
    //glutPostRedisplay ();
    break;
  case DRAG_LINKOUT:
    AnimateGhostLinkOut (nX, nY, psVisData);
    break;
  case DRAG_LINKIN:
    AnimateGhostLinkIn (nX, nY, psVisData);
    break;
  default:
    // Do nothing
    break;
  }
}

gboolean GetMoving (VisPersist * psVisData) {
  return (psVisData->eDrag != DRAG_NONE);
}

/*
int main (int argc, char **argv) {

  char szResults[_MAX_PATH];

  srand (time (NULL));
  glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow ("test");
  glutDisplayFunc (Redraw);
  glutMouseFunc (Mouse);
  glutKeyboardFunc (Key);
  glutKeyboardUpFunc (KeyUp);
  glutIgnoreKeyRepeat (true);
  glutMotionFunc (Motion);
  glutReshapeFunc (Reshape);
  glutIdleFunc (Idle);
  //glutCreateMenu (menu);
  //glutAddMenuEntry ("Nothing", 1);
  //glutAddMenuEntry ("Nothing else", 2);
  //glutAttachMenu (GLUT_RIGHT_BUTTON);

  if (argc < 2) {
    szResults[0] = 0;
  }
  else {
    strncpy (szResults, argv[1], _MAX_PATH);
  }
  Init (szResults);

  glutMainLoop ();
  return 0;
}
*/

void ResetAnimation (VisPersist * psVisData) {
  struct timeb sTime;

  // Reset the time
  ftime (& sTime);

  psVisData->fCurrentTime = (double)(sTime.time) + (double)(sTime.millitm) / 1000.0;
  if (psVisData->fCurrentTime < 0) psVisData->fCurrentTime = 0.0;
  psVisData->fPrevTime = psVisData->fCurrentTime;
  psVisData->fSpinTime = psVisData->fCurrentTime;
}

void SaveSettingsVis (SettingsPersist * psSettingsData, VisPersist * psVisData) {
  SettingsPrintBool (psSettingsData, "FullScreen", psVisData->boFullScreen);
  SettingsPrintBool (psSettingsData, "ClearWhite", psVisData->boClearWhite);
  SettingsPrintBool (psSettingsData, "AddToPlane", psVisData->boAddToPlane);
  SettingsPrintBool (psSettingsData, "FollowNode", psVisData->boFollowNode);
  SettingsPrintBool (psSettingsData, "DrawLinks", psVisData->boDrawLinks);
  SettingsPrintBool (psSettingsData, "NodeOverlay", psVisData->boNodeOverlay);
  SettingsPrintBool (psSettingsData, "LinkOverlay", psVisData->boLinkOverlay);
  SettingsPrintBool (psSettingsData, "FadeText", psVisData->boFadeText);
  SettingsPrintBool (psSettingsData, "NodeSelections", psVisData->boNodeSelections);
  SettingsPrintBool (psSettingsData, "LinkChanges", psVisData->boLinkChanges);
  SettingsPrintBool (psSettingsData, "ShowNotes", GetNoteDisplay (psVisData->psNotesData));

  SettingsPrintFloat (psSettingsData, "Resistance", psVisData->fArrangeResistance);
  SettingsPrintFloat (psSettingsData, "Rigidity", psVisData->fArrangeRigidity);
  SettingsPrintFloat (psSettingsData, "Force", psVisData->fArrangeForce);
  SettingsPrintFloat (psSettingsData, "LinkLen", psVisData->fArrangeLinklen);
  SettingsPrintFloat (psSettingsData, "Centring", psVisData->fArrangeCentring);
  SettingsPrintFloat (psSettingsData, "LinkScalar", psVisData->fArrangeLinkScalar);
  SettingsPrintFloat (psSettingsData, "ViewRadius", psVisData->fViewRadius);
  SettingsPrintFloat (psSettingsData, "Tethering", GetNoteTethering (psVisData->psNotesData));

  // Save out the nodes
  g_slist_foreach (psVisData->psNodeList, SaveSettingsNode, psSettingsData);

  // Save out the links
  g_slist_foreach (psVisData->psNodeList, SaveSettingsLinks, psSettingsData);
}

void SaveSettingsNode (gpointer psData, gpointer psUserData) {
  SettingsPersist * psSettingsData = (SettingsPersist *)psUserData;
  TNode * psNode = (TNode *)psData;

  SettingsStartTag (psSettingsData, "Node");
  SettingsPrintString (psSettingsData, "Name", psNode->szName->str);

  SettingsPrintFloat (psSettingsData, "Red", psNode->afColour[0]);
  SettingsPrintFloat (psSettingsData, "Green", psNode->afColour[1]);
  SettingsPrintFloat (psSettingsData, "Blue", psNode->afColour[2]);
  SettingsPrintFloat (psSettingsData, "Alpha", psNode->afColour[3]);

	SaveSettingsProperties (psSettingsData, psNode->psPropertiesData);
  SettingsEndTag (psSettingsData, "Node");
}

void SaveSettingsLinks (gpointer psData, gpointer psUserData) {

  SettingsPersist * psSettingsData = (SettingsPersist *)psUserData;
  TNode * psNode = (TNode *)psData;
  TLink * psLink = NULL;
  GSList * psListLink;

  // Cycle through each of the links for this node and save its details
  psListLink = psNode->psLinksOut;
  while (psListLink) {
    psLink = (TLink *)(psListLink->data);

    SettingsStartTag (psSettingsData, "Link");
    if ((psLink->szName) && (psLink->szName->len > 0)) {
	    SettingsPrintString (psSettingsData, "Name", psLink->szName->str);
    }
    SettingsPrintString (psSettingsData, "From", psNode->szName->str);
    SettingsPrintString (psSettingsData, "To", psLink->psNodeTo->szName->str);

    SettingsPrintFloat (psSettingsData, "Red", psLink->afColour[0]);
    SettingsPrintFloat (psSettingsData, "Green", psLink->afColour[1]);
    SettingsPrintFloat (psSettingsData, "Blue", psLink->afColour[2]);
    SettingsPrintFloat (psSettingsData, "Alpha", psLink->afColour[3]);

		SaveSettingsProperties (psSettingsData, psLink->psPropertiesData);
    SettingsEndTag (psSettingsData, "Link");

    psListLink = g_slist_next (psListLink);
  }
}

void LoadSettingsStartVis (SettingsPersist * psSettingsData, VisPersist * psVisData) {
  SettingsLoadParser * psLoadParser = NULL;

  psLoadParser = g_new0 (SettingsLoadParser, 1);

  psLoadParser->LoadProperty = VisLoadProperty;
  psLoadParser->LoadSectionStart = VisLoadSectionStart;
  psLoadParser->LoadSectionEnd = VisLoadSectionEnd;
  psLoadParser->psData = psVisData;
  AddParser (psLoadParser, psSettingsData);
}

void LoadSettingsEndVis (SettingsPersist * psSettingsData, VisPersist * psVisData) {
  SettingsLoadParser * psLoadParser = NULL;

  psLoadParser = GetParser (psSettingsData);
  g_free (psLoadParser);
  RemoveParser (psSettingsData);
}

void VisLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
  VisPersist * psVisData = (VisPersist *)psData;

  switch (eType) {
  case SETTINGTYPE_BOOL:
    if (stricmp (szName, "FullScreen") == 0) {
      psVisData->boFullScreen = *((bool*)(psValue));
    }
    else if (stricmp (szName, "ClearWhite") == 0) {
      psVisData->boClearWhite = *((bool*)(psValue));
		  SetNoteInverted (psVisData->boClearWhite, psVisData->psNotesData);
    }
    else if (stricmp (szName, "AddToPlane") == 0) {
      psVisData->boAddToPlane = *((bool*)(psValue));
    }
    else if (stricmp (szName, "FollowNode") == 0) {
      psVisData->boFollowNode = *((bool*)(psValue));
    }
    else if (stricmp (szName, "DrawLinks") == 0) {
      psVisData->boDrawLinks = *((bool*)(psValue));
    }
    else if (stricmp (szName, "NodeOverlay") == 0) {
      psVisData->boNodeOverlay = *((bool*)(psValue));
    }
    else if (stricmp (szName, "LinkOverlay") == 0) {
      psVisData->boLinkOverlay = *((bool*)(psValue));
    }
    else if (stricmp (szName, "FadeText") == 0) {
      psVisData->boFadeText = *((bool*)(psValue));
    }
    else if (stricmp (szName, "NodeSelections") == 0) {
      psVisData->boNodeSelections = *((bool*)(psValue));
    }
    else if (stricmp (szName, "LinkChanges") == 0) {
      psVisData->boLinkChanges = *((bool*)(psValue));
    }
    else if (stricmp (szName, "ShowNotes") == 0) {
    	SetNoteDisplay (*((bool*)(psValue)), psVisData->psNotesData);
    }
    break;
  case SETTINGTYPE_FLOAT:
    if (stricmp (szName, "Resistance") == 0) {
      psVisData->fArrangeResistance = *((float*)(psValue));
    }
    else if (stricmp (szName, "Rigidity") == 0) {
      psVisData->fArrangeRigidity = *((float*)(psValue));
    }
    else if (stricmp (szName, "Force") == 0) {
      psVisData->fArrangeForce = *((float*)(psValue));
    }
    else if (stricmp (szName, "LinkLen") == 0) {
      psVisData->fArrangeLinklen = *((float*)(psValue));
    }
    else if (stricmp (szName, "Centring") == 0) {
      psVisData->fArrangeCentring = *((float*)(psValue));
    }
    else if (stricmp (szName, "LinkScalar") == 0) {
      psVisData->fArrangeLinkScalar = *((float*)(psValue));
    }
    else if (stricmp (szName, "ViewRadius") == 0) {
      psVisData->fViewRadius = *((float*)(psValue));
    }
    else if (stricmp (szName, "Tethering") == 0) {
    	SetNoteTethering (*((float*)(psValue)), psVisData->psNotesData);
    }
    break;
  default:
    printf ("Unknown vis property %s\n", szName);
    break;
  }
}

void VisLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  VisPersist * psVisData = (VisPersist *)psData;
  SettingsNode * psSettingsNode;
  SettingsLink * psSettingsLink;
  SettingsLoadParser * psLoadParser = NULL;

  if (stricmp (szName, "node") == 0) {
    // Move into the node section
    psSettingsNode = g_new0 (SettingsNode, 1);
    psSettingsNode->psNode = NewTNode ("");
    psSettingsNode->psVisData = psVisData;

    psLoadParser = g_new0 (SettingsLoadParser, 1);
    psLoadParser->LoadProperty = NodeLoadProperty;
    psLoadParser->LoadSectionStart = NodeLoadSectionStart;
    psLoadParser->LoadSectionEnd = NodeLoadSectionEnd;
    psLoadParser->psData = psSettingsNode;
    AddParser (psLoadParser, psSettingsData);
  }
  else if (stricmp (szName, "link") == 0) {
    // Move into the link section
    psSettingsLink = g_new0 (SettingsLink, 1);
    psSettingsLink->psNodeFrom = NULL;
    psSettingsLink->psLink = NewTLink ("", NULL, NULL);
    psSettingsLink->psVisData = psVisData;

    psLoadParser = g_new0 (SettingsLoadParser, 1);
    psLoadParser->LoadProperty = LinkLoadProperty;
    psLoadParser->LoadSectionStart = LinkLoadSectionStart;
    psLoadParser->LoadSectionEnd = LinkLoadSectionEnd;

    psLoadParser->psData = psSettingsLink;
    AddParser (psLoadParser, psSettingsData);
  }
}

void VisLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  VisPersist * psVisData = (VisPersist *)psData;

  if (stricmp (szName, "vis") == 0) {
    // Move out of the vis section
    LoadSettingsEndVis (psSettingsData, psVisData);
  }
}

void NodeLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  SettingsNode * psSettingsNode = (SettingsNode *)psData;
  SettingsLoadParser * psLoadParser = NULL;

  if (stricmp (szName, "properties") == 0) {
    // Move into the properties section
    psLoadParser = g_new0 (SettingsLoadParser, 1);
    psLoadParser->LoadProperty = PropertiesLoadProperty;
    psLoadParser->LoadSectionStart = PropertiesLoadSectionStart;
    psLoadParser->LoadSectionEnd = PropertiesLoadSectionEnd;
    psLoadParser->psData = psSettingsNode->psNode->psPropertiesData;
    AddParser (psLoadParser, psSettingsData);
  }
}

void NodeLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
	TNode * psNewNode;

  SettingsNode * psSettingsNode = (SettingsNode *)psData;
  SettingsLoadParser * psLoadParser = NULL;

  if (stricmp (szName, "node") == 0) {
    // Create the node
    psNewNode = AddNode (psSettingsNode->psNode->szName->str, psSettingsNode->psVisData);
    // Transfer the colour over
    memcpy (psNewNode->afColour, psSettingsNode->psNode->afColour, sizeof (float) * 4);

		if ((psSettingsNode->psNode->psPropertiesData) && psNewNode) {
			// Transfer the properties over
			DeletePropertiesPersist (psNewNode->psPropertiesData);
			psNewNode->psPropertiesData = psSettingsNode->psNode->psPropertiesData;
			psSettingsNode->psNode->psPropertiesData = NULL;
			UpdateNodeNoteString (psNewNode);
		}

    // Move out of the node section
    DeleteTNode (psSettingsNode->psNode);
    g_free (psSettingsNode);

    psLoadParser = GetParser (psSettingsData);
    g_free (psLoadParser);
    RemoveParser (psSettingsData);
  }
}

void NodeLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
  SettingsNode * psSettingsNode = (SettingsNode *)psData;

  switch (eType) {
  case SETTINGTYPE_STRING:
    if (stricmp (szName, "Name") == 0) {
      g_string_assign (psSettingsNode->psNode->szName, ((char *)(psValue)));
    }
    break;
  case SETTINGTYPE_FLOAT:
    if (stricmp (szName, "Red") == 0) {
    	psSettingsNode->psNode->afColour[0] = (*(float *)(psValue));
    }
    else if (stricmp (szName, "Green") == 0) {
    	psSettingsNode->psNode->afColour[1] = (*(float *)(psValue));
    }
    else if (stricmp (szName, "Blue") == 0) {
    	psSettingsNode->psNode->afColour[2] = (*(float *)(psValue));
    }
    else if (stricmp (szName, "Alpha") == 0) {
    	psSettingsNode->psNode->afColour[3] = (*(float *)(psValue));
    }
    break;
  default:
    printf ("Unknown node property %s\n", szName);
    break;
  }
}


void LinkLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  SettingsLink * psSettingsLink = (SettingsLink *)psData;
  SettingsLoadParser * psLoadParser = NULL;

  if (stricmp (szName, "properties") == 0) {
    // Move into the properties section
    psLoadParser = g_new0 (SettingsLoadParser, 1);
    psLoadParser->LoadProperty = PropertiesLoadProperty;
    psLoadParser->LoadSectionStart = PropertiesLoadSectionStart;
    psLoadParser->LoadSectionEnd = PropertiesLoadSectionEnd;
    psLoadParser->psData = psSettingsLink->psLink->psPropertiesData;
    AddParser (psLoadParser, psSettingsData);
  }
}

void LinkLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  SettingsLink * psSettingsLink = (SettingsLink *)psData;
  SettingsLoadParser * psLoadParser = NULL;
  TLink * psNewLink;

  if (stricmp (szName, "link") == 0) {
    // Create the link

    if (psSettingsLink->psNodeFrom && psSettingsLink->psLink->psNodeTo) {
      psNewLink = AddLinkDirect (psSettingsLink->psLink->szName->str, psSettingsLink->psNodeFrom, psSettingsLink->psLink->psNodeTo, FALSE, psSettingsLink->psVisData);
      // Transfer the colour over
      memcpy (psNewLink->afColour, psSettingsLink->psLink->afColour, sizeof (float) * 4);

			// Transfer the properties over
			if ((psSettingsLink->psLink->psPropertiesData) && psNewLink) {
				DeletePropertiesPersist (psNewLink->psPropertiesData);
				psNewLink->psPropertiesData = psSettingsLink->psLink->psPropertiesData;
				psSettingsLink->psLink->psPropertiesData = NULL;
				UpdateLinkNoteString (psNewLink);
			}
    }

    // Move out of the link section
    DeleteTLink (psSettingsLink->psLink);
    g_free (psSettingsLink);

    psLoadParser = GetParser (psSettingsData);
    g_free (psLoadParser);
    RemoveParser (psSettingsData);
  }
}

void LinkLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
  SettingsLink * psSettingsLink = (SettingsLink *)psData;

  switch (eType) {
  case SETTINGTYPE_STRING:
    if (stricmp (szName, "Name") == 0) {
      g_string_assign (psSettingsLink->psLink->szName, ((char *)(psValue)));
    }
    else if (stricmp (szName, "From") == 0) {
      psSettingsLink->psNodeFrom = g_hash_table_lookup (psSettingsLink->psVisData->psNodes, ((char *)(psValue)));
    }
    else if (stricmp (szName, "To") == 0) {
      psSettingsLink->psLink->psNodeTo = g_hash_table_lookup (psSettingsLink->psVisData->psNodes, ((char *)(psValue)));
    }
    break;
  case SETTINGTYPE_FLOAT:
    if (stricmp (szName, "Red") == 0) {
    	psSettingsLink->psLink->afColour[0] = (*(float *)(psValue));
    }
    else if (stricmp (szName, "Green") == 0) {
    	psSettingsLink->psLink->afColour[1] = (*(float *)(psValue));
    }
    else if (stricmp (szName, "Blue") == 0) {
    	psSettingsLink->psLink->afColour[2] = (*(float *)(psValue));
    }
    else if (stricmp (szName, "Alpha") == 0) {
    	psSettingsLink->psLink->afColour[3] = (*(float *)(psValue));
    }
    break;
  default:
    printf ("Unknown link property %s\n", szName);
    break;
  }
}

void UpdateNoteAnchors (VisPersist * psVisData) {
  GLdouble        afModel[16];
  GLdouble        afProjection[16];
  GLint           anViewpoert[4];
  TNode * psNode;
  TLink * psLink;
  TNode * psLinkTo;
  FloatNote * psNote;
  GSList * psNodeListPos;
  GSList * psLinkListPos;
  GLdouble fX, fY, fZ;

  glGetDoublev (GL_MODELVIEW_MATRIX, afModel);
  glGetDoublev (GL_PROJECTION_MATRIX, afProjection);
  glGetIntegerv (GL_VIEWPORT, anViewpoert);

	// Cycle through all of the nodes
  psNodeListPos = psVisData->psNodeList;
  while (psNodeListPos) {
  	psNode = (TNode *)psNodeListPos->data;
  	
  	// Update node anchors
    psNote = (FloatNote *)psNode->psNote;

		gluProject (psNode->vsPos.fX, psNode->vsPos.fY, psNode->vsPos.fZ, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);
		
		psNote->vsAnchor.fX = fX;
		psNote->vsAnchor.fY = fY;
		psNote->vsAnchor.fZ = fZ;

		// Cycle through all of the links leaving this node
    psLinkListPos = psNode->psLinksOut;
    while (psLinkListPos) {
      psLink = ((TLink *)psLinkListPos->data);
      psLinkTo = psLink->psNodeTo;

			// Update link anchors
		  psNote = (FloatNote *)psLink->psNote;

			gluProject ((psNode->vsPos.fX + psLinkTo->vsPos.fX) / 2.0f, (psNode->vsPos.fY + psLinkTo->vsPos.fY) / 2.0f, (psNode->vsPos.fZ + psLinkTo->vsPos.fZ) / 2.0f, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);
		
			psNote->vsAnchor.fX = fX;
			psNote->vsAnchor.fY = fY;
			psNote->vsAnchor.fZ = fZ;

      psLinkListPos = psLinkListPos->next;
    }
    psNodeListPos = psNodeListPos->next;
  }
}

NotesPersist * GetNotesPersist (VisPersist * psVisData) {
	return psVisData->psNotesData;
}

void SetNodeSelected (TNode * psNodeSelected, VisPersist * psVisData) {
	psVisData->psNodeSelected = psNodeSelected;
	if (psNodeSelected) {
		SetNoteSelected (psNodeSelected->psNote, psVisData->psNotesData);
	}
	else {
		SetNoteSelected (NULL, psVisData->psNotesData);
	}
}

void SetLinkSelected (TNode * psNodeSelected, TLink * psLinkSelected, VisPersist * psVisData) {
	psVisData->psLinkSelected = psLinkSelected;
	psVisData->psLinkSelectedFrom = psNodeSelected;
	if (psLinkSelected) {
		SetNoteSelected (psLinkSelected->psNote, psVisData->psNotesData);
	}
	else {
		SetNoteSelected (NULL, psVisData->psNotesData);
	}
}

void SetNodeProperty (char const * szName, char const * szProperty, char const * szType, char const * szValue, VisPersist * psVisData) {
	TNode * psNode;

	// Find the node
  psNode = g_hash_table_lookup (psVisData->psNodes, szName);
	
	if (psNode) {
		// Set the property for the node
		SetProperty (szProperty, szType, szValue, psNode->psPropertiesData);

		// Update the note string
		UpdateNodeNoteString (psNode);
	}
}

void SubNodeProperty (char const * szName, char const * szProperty, VisPersist * psVisData) {
	TNode * psNode;

	// Find the node
  psNode = g_hash_table_lookup (psVisData->psNodes, szName);
	
	if (psNode) {
		// Remove the property for the node
		SubProperty (szProperty, psNode->psPropertiesData);

		// Update the note string
		UpdateNodeNoteString (psNode);
	}
}

void SetLinkProperty (char const * szName, char const * szProperty, char const * szType, char const * szValue, VisPersist * psVisData) {
	TLink * psLink;

	// Find the link
	psLink = FindLinkNamed (szName, psVisData);

	if (psLink) {
		// Set the property for the link
		SetProperty (szProperty, szType, szValue, psLink->psPropertiesData);

		// Update the note string
		UpdateLinkNoteString (psLink);
	}
}

void SubLinkProperty (char const * szName, char const * szProperty, VisPersist * psVisData) {
	TLink * psLink;

	// Find the link
	psLink = FindLinkNamed (szName, psVisData);

	if (psLink) {
		// Remove the property for the link
		SubProperty (szProperty, psLink->psPropertiesData);

		// Update the note string
		UpdateLinkNoteString (psLink);
	}
}

void UpdateNodeNoteString (TNode * psNode) {
	GString * szText = NULL;

	// Update the note string
	szText = g_string_new ("");
	szText = GenerateNotePropertyString (szText, psNode->psPropertiesData);
	SetNoteText (psNode->psNote, szText->str);
	g_string_free (szText, TRUE);
}

void UpdateLinkNoteString (TLink * psLink) {
	GString * szText = NULL;

	// Update the note string
	szText = g_string_new ("");
	szText = GenerateNotePropertyString (szText, psLink->psPropertiesData);
	SetNoteText (psLink->psNote, szText->str);
	g_string_free (szText, TRUE);
}

