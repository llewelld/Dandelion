///////////////////////////////////////////////////////////////////
// Trustella Network
// Results log file animation viewer
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Autumn 2006
///////////////////////////////////////////////////////////////////

#ifndef VIS_H
#define VIS_H

///////////////////////////////////////////////////////////////////
// Includes

typedef struct _VisPersist VisPersist;

#include "utils.h"
#include "settings.h"
#include "server.h"
#include "floatnote.h"

///////////////////////////////////////////////////////////////////
// Defines

//#define LEFT_BUTTON GLUT_LEFT_BUTTON
//#define RIGHT_BUTTON GLUT_RIGHT_BUTTON
//#define BUTTON_DOWN GLUT_DOWN
//#define BUTTON_UP GLUT_UP
#define LEFT_BUTTON 1
#define RIGHT_BUTTON 3
#define BUTTON_DOWN GDK_BUTTON_PRESS
#define BUTTON_UP GDK_BUTTON_RELEASE

#define SCREENWIDTH         (800)
#define SCREENHEIGHT        (600)
#define TEXT_LAYER_ZPOS	(0.95f)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _TNode TNode;
typedef struct _TLink TLink;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

VisPersist * NewVisPersist (void);
void DeleteVisPersist (VisPersist * psVisData);

void Redraw (VisPersist * psVisData);
void Mouse (int button, int state, int x, int y, VisPersist * psVisData);
void Key (unsigned char key, int x, int y, unsigned int uKeyModifiers, VisPersist * psVisData);
void KeyUp (unsigned char key, int x, int y, unsigned int uKeyModifiers, VisPersist * psVisData);
void Motion (int nX, int nY, VisPersist * psVisData);
void Reshape (int w, int h, VisPersist * psVisData);
void Idle (VisPersist * psVisData);
void Init (VisPersist * psVisData);
void Realise (VisPersist * psVisData);
void Unrealise (VisPersist * psVisData);
TNode * AddNode (char const * szName, VisPersist * psVisData);
void RemoveNode (char const * szName, VisPersist * psVisData);
void MoveNode (char const * szName, double fX, double fY, double fZ, VisPersist * psVisData);
void SetNodeColour (char const * szName, double fRed, double fGreen, double fBlue, VisPersist * psVisData);
void AddLink (char const * szName, char const * szNodeFrom, char const * szNodeTo, bool boBidirect, VisPersist * psVisData);
TLink * FindLinkNamed (char const * szName, VisPersist * psVisData);
TLink * FindLinkNodes (TNode * psNodeFrom, TNode * psNodeTo, VisPersist * psVisData);
TLink * FindLinkNodesNamed (char const * szNodeFrom, char const * szNodeTo, VisPersist * psVisData);
void RemoveLink (TLink * psLink, VisPersist * psVisData);
void SetLinkColour (TLink * psLink, double fRed, double fGreen, double fBlue, VisPersist * psVisData);
void SetLinkColourNamed (char const * szName, double fRed, double fGreen, double fBlue, VisPersist * psVisData);
void RemoveAll (VisPersist * psVisData);

void VisSetServer (ServerPersist * psServerData, VisPersist * psVisData);
void ToggleFullScreen (VisPersist * psVisData);
void Shake (VisPersist * psVisData);
void ToggleClearWhite (VisPersist * psVisData);
void ToggleAddToPlane (VisPersist * psVisData);
void ToggleSpin (VisPersist * psVisData);
void TogglePointTowards (VisPersist * psVisData);
void ToggleLinks (VisPersist * psVisData);
void ToggleNodeOverlay (VisPersist * psVisData);
void SetNodeOverlay (bool boNodeOverlay, VisPersist * psVisData);
bool GetNodeOverlay (VisPersist * psVisData);
void SetLinkOverlay (bool boLinkOverlay, VisPersist * psVisData);
bool GetLinkOverlay (VisPersist * psVisData);
void GetDisplayProperties (float * pfViewRadius, float * pfLinkLen, float * pfCentring, float * pfRigidity, float * pfForce, float * pfResistance, float * pfLinkScalar, VisPersist * psVisData);
void SetDisplayProperties (float fViewRadius, float fLinkLen, float fCentring, float fRigidity, float fForce, float fResistance, float fLinkScalar, VisPersist * psVisData);
void SetFadeText (bool boFadeText, VisPersist * psVisData);
bool GetFadeText (VisPersist * psVisData);
void SetNodeSelections (bool boNodeSelections, VisPersist * psVisData);
void SetLinkChanges (bool boLinkChanges, VisPersist * psVisData);
bool GetLinkChanges (VisPersist * psVisData);
bool GetNodeSelections (VisPersist * psVisData);
void SetClearWhite (bool boClearWhite, VisPersist * psVisData);
bool GetClearWhite (VisPersist * psVisData);
void SetAddToPlane (bool boAddToPlane, VisPersist * psVisData);
bool GetAddToPlane (VisPersist * psVisData);
bool GetFullScreen (VisPersist * psVisData);
gboolean GetMoving (VisPersist * psVisData);
void SaveSettingsVis (SettingsPersist * psSettingsData, VisPersist * psVisData);
void LoadSettingsStartVis (SettingsPersist * psSettingsData, VisPersist * psVisData);
void LoadSettingsEndVis (SettingsPersist * psSettingsData, VisPersist * psVisData);
void RenderBitmapString (float fX, float fY, float fZ, void * pFont, char const * szString);
NotesPersist * GetNotesPersist (VisPersist * psVisData);

void SetNodeProperty (char const * szName, char const * szProperty, char const * szType, char const * szValue, VisPersist * psVisData);
void SubNodeProperty (char const * szName, char const * szProperty, VisPersist * psVisData);
void SetLinkProperty (char const * szName, char const * szProperty, char const * szType, char const * szValue, VisPersist * psVisData);
void SubLinkProperty (char const * szName, char const * szProperty, VisPersist * psVisData);

float GetViewRadius (VisPersist * psVisData);
float * GetVariableViewRadius (VisPersist * psVisData);
float * GetVariableLinkLen (VisPersist * psVisData);
float * GetVariableCentring (VisPersist * psVisData);
float * GetVariableRigidity (VisPersist * psVisData);
float * GetVariableForce (VisPersist * psVisData);
float * GetVariableResistance (VisPersist * psVisData);
float * GetVariableLinkScalar (VisPersist * psVisData);
float * GetVariableFocusNear (VisPersist * psVisData);
float * GetVariableFocusFar (VisPersist * psVisData);
float * GetVariableFocusScaleNear (VisPersist * psVisData);
float * GetVariableFocusScaleFar (VisPersist * psVisData);
float * GetVariableFocusDarkenMax (VisPersist * psVisData);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* VIS_H */


