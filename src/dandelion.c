///////////////////////////////////////////////////////////////////
// Dandelion
// Network Visualiser
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Autumn 2006, Spring 2008
//
// Based on GtkGLExt logo demo
// by Naofumi Yasufuku <naofumi@users.sourceforge.net>
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

#include "vis.h"
#include "server.h"
#include "settings.h"
#include "spinslider.h"

///////////////////////////////////////////////////////////////////
// Defines

#define UPDATE_TIMEOUT (10)
#define DEFAULT_PORT (4972)
#define FULLSCREEN_BUTTONBARSHOW (8)
#define RADIUS_SCROLL (1)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _DandePersist DandePersist;

struct _DandePersist {
  VisPersist * psVisData;
  GtkWidget * psDrawingArea;
  gboolean boTimeoutContinue;
  guint TimeoutID;
  GladeXML * psXML;
  gboolean boButtonBarHidden;
  SpinSliderPersist * psSpinSliderData;
  SpinSliderLink * psZoomSpinSlider;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

DandePersist * NewDandePersist (void);
void DeleteDandePersist (DandePersist * psDandeData);
static void Realize (GtkWidget * psWidget, gpointer psData);
static void Unrealize (GtkWidget * psWidget, gpointer psData) ;
static gboolean ConfigureEvent (GtkWidget * psWidget, GdkEventConfigure * psEvent, gpointer psData);
static gboolean ExposeEvent (GtkWidget * psWidget, GdkEventExpose * psEvent, gpointer psData);
static gboolean ButtonPressEvent (GtkWidget * psWidget, GdkEventButton * psEvent, gpointer * psData);
static gboolean ButtonReleaseEvent (GtkWidget * psWidget, GdkEventButton * psEvent, gpointer * psData);
static gboolean MotionNotifyEvent (GtkWidget * psWidget, GdkEventMotion * psEvent, gpointer psData);
static gboolean ScrollEvent (GtkWidget * psWidget, GdkEventScroll * psEvent, gpointer psData);
static gboolean KeyPressEvent (GtkWidget * psWidget, GdkEventKey * psEvent, gpointer psData);
static gboolean KeyReleaseEvent (GtkWidget * psWidget, GdkEventKey * psEvent, gpointer psData);
static gboolean Timeout (gpointer psData);
static void TimeoutAdd (DandePersist * psDandeData);
static void TimeoutRemove (DandePersist * psDandeData);
static gboolean MapEvent (GtkWidget * psWidget, GdkEventAny * psEvent, gpointer psData);
static gboolean UnmapEvent (GtkWidget * psWidget, GdkEventAny * psEvent, gpointer psData);
static gboolean VisibilityNotifyEvent (GtkWidget * psWidget, GdkEventVisibility * psEvent, gpointer psData);
//static gboolean InvertBackground (GtkWidget * psWidget, gpointer psData);
static gboolean SpinVisualisation (GtkWidget * psWidget, gpointer psData);
static gboolean FocusOnNode (GtkWidget * psWidget, gpointer psData);
static gboolean ShowLinks (GtkWidget * psWidget, gpointer psData);
static gboolean ShowLabels (GtkWidget * psWidget, gpointer psData);
static gboolean ShakeNodes (GtkWidget * psWidget, gpointer psData);
static gboolean ClearNodes (GtkWidget * psWidget, gpointer psData);
void SetDisplayPropertiesDialogue (DandePersist * psDandeData);
void SetDisplayPropertiesValues (DandePersist * psDandeData);
//static gboolean DisplayPropertiesSpinChanged (GtkWidget * psWidget, gpointer psData);
//static gboolean DisplayPropertiesSliderChanged (GtkWidget * psWidget, gpointer psData);
static gboolean ConfigureDialogueOpen (GtkWidget * psWidget, gpointer psData);
static gboolean ConfigureDialogueOK (GtkWidget * psWidget, gpointer psData);
void SetConfigureDialogue (DandePersist * psDandeData);
void SetConfigureValues (DandePersist * psDandeData);
void ToggleFullScreenWindow (DandePersist * psDandeData);
void SaveSettingsAll (DandePersist * psDandeData);

void LoadSettingsAll (DandePersist * psDandeData);
void DandeLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
void DandeLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
void DandeLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);

///////////////////////////////////////////////////////////////////
// Function definitions

DandePersist * NewDandePersist (void) {
  DandePersist * psDandeData;

  psDandeData = g_new0 (DandePersist, 1);

  psDandeData->psVisData = NULL;
  psDandeData->psDrawingArea = NULL;
  psDandeData->boTimeoutContinue = FALSE;
  psDandeData->TimeoutID = 0;
  psDandeData->boButtonBarHidden = FALSE;
  psDandeData->psSpinSliderData = NewSpinSliderPersist ();

  return psDandeData;
}

void DeleteDandePersist (DandePersist * psDandeData) {
  if (psDandeData->psVisData) {
    DeleteVisPersist (psDandeData->psVisData);
    psDandeData->psVisData = NULL;
  }
  
  if (psDandeData->psSpinSliderData) {
  	DeleteSpinSliderPersist (psDandeData->psSpinSliderData);
  	psDandeData->psSpinSliderData = NULL;
  }

  g_free (psDandeData);
}

static void Realize (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  GdkGLContext * psGlContext = gtk_widget_get_gl_context (psWidget);
  GdkGLDrawable * psGlDrawable = gtk_widget_get_gl_drawable (psWidget);

  // OpenGL BEGIN
  if (!gdk_gl_drawable_gl_begin (psGlDrawable, psGlContext)) {
    return;
  }

  Realise (psDandeData->psVisData);

  gdk_gl_drawable_gl_end (psGlDrawable);
  // OpenGL END
}

static void Unrealize (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  Unrealise (psDandeData->psVisData);
}

static gboolean ConfigureEvent (GtkWidget * psWidget, GdkEventConfigure * psEvent, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  GdkGLContext * psGlContext = gtk_widget_get_gl_context (psWidget);
  GdkGLDrawable * psGlDrawable = gtk_widget_get_gl_drawable (psWidget);

  GLfloat fWidth = psWidget->allocation.width;
  GLfloat fHeight = psWidget->allocation.height;

  // OpenGL BEGIN
  if (!gdk_gl_drawable_gl_begin (psGlDrawable, psGlContext)) {
    return FALSE;
  }

  Reshape (fWidth, fHeight, psDandeData->psVisData);

  gdk_gl_drawable_gl_end (psGlDrawable);
  // OpenGL END

  return TRUE;
}

static gboolean ExposeEvent (GtkWidget * psWidget, GdkEventExpose * psEvent, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  GdkGLContext * psGlContext = gtk_widget_get_gl_context (psWidget);
  GdkGLDrawable * psGlDrawable = gtk_widget_get_gl_drawable (psWidget);

  // OpenGL BEGIN
  if (!gdk_gl_drawable_gl_begin (psGlDrawable, psGlContext)) {
    return FALSE;
  }

  // Redraw the visualisation
  Redraw (psDandeData->psVisData);

  // Swap buffers
  if (gdk_gl_drawable_is_double_buffered (psGlDrawable)) {
    gdk_gl_drawable_swap_buffers (psGlDrawable);
  }
  else {
    glFlush ();
  }

  gdk_gl_drawable_gl_end (psGlDrawable);
  // OpenGL END

  return TRUE;
}

static gboolean ButtonPressEvent (GtkWidget * psWidget, GdkEventButton * psEvent, gpointer * psData) {
  DandePersist * psDandeData = (DandePersist * )psData;
  gboolean boFullScreen;
  GtkWidget * psWidgetSet;

  boFullScreen = GetFullScreen (psDandeData->psVisData);

  if (boFullScreen) {
    if (psEvent->y > (psWidget->allocation.height - FULLSCREEN_BUTTONBARSHOW)) {
      if (psDandeData->boButtonBarHidden) {
        psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "ButtonBar");
        gtk_widget_show (psWidgetSet);
        psDandeData->boButtonBarHidden = FALSE;
      }
    }
    else {
      if (!psDandeData->boButtonBarHidden) {
        psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "ButtonBar");
        gtk_widget_hide (psWidgetSet);
        psDandeData->boButtonBarHidden = TRUE;
      }
    }
  }

  Mouse (psEvent->button, psEvent->type, psEvent->x, psEvent->y, psDandeData->psVisData);

  return FALSE;
}

static gboolean ButtonReleaseEvent (GtkWidget * psWidget, GdkEventButton * psEvent, gpointer * psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  Mouse (psEvent->button, psEvent->type, psEvent->x, psEvent->y, psDandeData->psVisData);

  return FALSE;
}

static gboolean MotionNotifyEvent (GtkWidget * psWidget, GdkEventMotion * psEvent, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  //float fWidth = psWidget->allocation.width;
  //float fHeight = psWidget->allocation.height;
  float fX = psEvent->x;
  float fY = psEvent->y;

  Motion ((int)fX, (int)fY, psDandeData->psVisData);
  gdk_window_invalidate_rect (psWidget->window, & psWidget->allocation, FALSE);

  return TRUE;
}

static gboolean KeyPressEvent (GtkWidget * psWidget, GdkEventKey * psEvent, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;
  guint uModifiers;

  uModifiers = gtk_accelerator_get_default_mod_mask ();

  switch (psEvent->keyval) {
    case GDK_Return:
      if ((psEvent->state & uModifiers) == GDK_MOD1_MASK) {
        ToggleFullScreenWindow (psDandeData);
      }
      break;
    default:
      Key (psEvent->keyval, 0, 0, (psEvent->state & uModifiers), psDandeData->psVisData);
      break;
  }

  return TRUE;
}

static gboolean KeyReleaseEvent (GtkWidget * psWidget, GdkEventKey * psEvent, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;
  guint uModifiers;

  uModifiers = gtk_accelerator_get_default_mod_mask ();

  KeyUp (psEvent->keyval, 0, 0, (psEvent->state & uModifiers), psDandeData->psVisData);

  return TRUE;
}

static gboolean ScrollEvent (GtkWidget * psWidget, GdkEventScroll * psEvent, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;
  float fRadius;

  switch (psEvent->direction) {
    case GDK_SCROLL_UP:
			fRadius = GetViewRadius (psDandeData->psVisData);
      fRadius += RADIUS_SCROLL;
			SpinSliderSetValue (fRadius, psDandeData->psZoomSpinSlider);
      break;
    case GDK_SCROLL_DOWN:
			fRadius = GetViewRadius (psDandeData->psVisData);
      fRadius -= RADIUS_SCROLL;
			SpinSliderSetValue (fRadius, psDandeData->psZoomSpinSlider);
      break;
    default:
      // Do nothing
      break;
  }

  return TRUE;
}

void ToggleFullScreenWindow (DandePersist * psDandeData) {
  GtkWidget * psWindow;
  GtkWidget * psWidget;
  gboolean boCurrentState;

  boCurrentState = GetFullScreen (psDandeData->psVisData);

  if (boCurrentState) {
    psWindow = glade_xml_get_widget (psDandeData->psXML, "MainWindow");
    gtk_window_unfullscreen (GTK_WINDOW (psWindow));
    psWidget = glade_xml_get_widget (psDandeData->psXML, "ButtonBar");
    gtk_widget_show (psWidget);
    psDandeData->boButtonBarHidden = FALSE;
  }
  else {
    psWindow = glade_xml_get_widget (psDandeData->psXML, "MainWindow");
    gtk_window_fullscreen (GTK_WINDOW (psWindow));
    psWidget = glade_xml_get_widget (psDandeData->psXML, "ButtonBar");
    gtk_widget_hide (psWidget);
    psDandeData->boButtonBarHidden = TRUE;
  }

  psWindow = glade_xml_get_widget (psDandeData->psXML, "Properties");
  gtk_window_set_keep_above (GTK_WINDOW (psWindow), !boCurrentState);
  psWindow = glade_xml_get_widget (psDandeData->psXML, "Configure");
  gtk_window_set_keep_above (GTK_WINDOW (psWindow), !boCurrentState);

  ToggleFullScreen (psDandeData->psVisData);
}

static gboolean Timeout (gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  if (psDandeData->boTimeoutContinue) {
    Idle (psDandeData->psVisData);

    // Invalidate the drawing area
    gdk_window_invalidate_rect (psDandeData->psDrawingArea->window, & psDandeData->psDrawingArea->allocation, 
      FALSE);

    // Update drawing area synchronously
    gdk_window_process_updates (psDandeData->psDrawingArea->window, FALSE);
  }
  else {
    psDandeData->TimeoutID = 0;
  }

  return psDandeData->boTimeoutContinue;
}

static void TimeoutAdd (DandePersist * psDandeData) {
  psDandeData->boTimeoutContinue = TRUE;
  if (psDandeData->TimeoutID == 0) {
    //psDandeData->TimeoutID = g_timeout_add (UPDATE_TIMEOUT, Timeout, psDandeData);
    psDandeData->TimeoutID = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, UPDATE_TIMEOUT, Timeout, 
      psDandeData, NULL);
    //psDandeData->TimeoutID = g_idle_add (Timeout, psDandeData);
  }
}

static void TimeoutRemove (DandePersist * psDandeData) {
  psDandeData->boTimeoutContinue = FALSE;
}

static gboolean MapEvent (GtkWidget * psWidget, GdkEventAny * psEvent, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  TimeoutAdd (psDandeData);

  return TRUE;
}

static gboolean UnmapEvent (GtkWidget * psWidget, GdkEventAny * psEvent, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  TimeoutRemove (psDandeData);

  return TRUE;
}

static gboolean VisibilityNotifyEvent (GtkWidget * psWidget, GdkEventVisibility * psEvent, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  if (psEvent->state == GDK_VISIBILITY_FULLY_OBSCURED) {
  	TimeoutRemove (psDandeData);
  }
  else {
  	TimeoutAdd (psDandeData);
  }

  return TRUE;
}

//static gboolean InvertBackground (GtkWidget * psWidget, gpointer psData) {
//  DandePersist * psDandeData = (DandePersist * )psData;
//
//  ToggleClearWhite (psDandeData->psVisData);
//
//  return TRUE;
//}

static gboolean SpinVisualisation (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  ToggleSpin (psDandeData->psVisData);

  return TRUE;
}

static gboolean FocusOnNode (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  TogglePointTowards (psDandeData->psVisData);

  return TRUE;
}

static gboolean ShowLinks (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  ToggleLinks (psDandeData->psVisData);

  return TRUE;
}

static gboolean ShowLabels (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  ToggleNodeOverlay (psDandeData->psVisData);

  return TRUE;
}

static gboolean ShakeNodes (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  Shake (psDandeData->psVisData);

  return TRUE;
}

static gboolean ClearNodes (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;

  RemoveAll (psDandeData->psVisData);

  return TRUE;
}

static gboolean ConfigureDialogueOpen (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;
  GtkWidget * psWindow;

  SetConfigureDialogue (psDandeData);

  psWindow = glade_xml_get_widget (psDandeData->psXML, "Configure");
  gtk_widget_show (psWindow);

  return TRUE;
}

static gboolean ConfigureDialogueOK (GtkWidget * psWidget, gpointer psData) {
  DandePersist * psDandeData = (DandePersist * )psData;
  GtkWidget * psWindow;

  SetConfigureValues (psDandeData);

  psWindow = glade_xml_get_widget (psDandeData->psXML, "Configure");
  gtk_widget_hide (psWindow);

  return TRUE;
}

void SetConfigureDialogue (DandePersist * psDandeData) {
  GtkWidget * psWidgetSet;
  bool boValue;
  float fValue;

  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "NodeOverlay");
  boValue = GetNodeOverlay (psDandeData->psVisData);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "LinkOverlay");
  boValue = GetLinkOverlay (psDandeData->psVisData);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "FadeText");
  boValue = GetFadeText (psDandeData->psVisData);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "NodeSelections");
  boValue = GetNodeSelections (psDandeData->psVisData);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "LinkChanges");
  boValue = GetLinkChanges (psDandeData->psVisData);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Invert");
  boValue = GetClearWhite (psDandeData->psVisData);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Plane");
  boValue = GetAddToPlane (psDandeData->psVisData);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "ShowNotes");
  boValue = GetNoteDisplay (GetNotesPersist (psDandeData->psVisData));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "FloatNotes");
  fValue = GetNoteTethering (GetNotesPersist (psDandeData->psVisData));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), (fValue < 0.5f));
}

void SetConfigureValues (DandePersist * psDandeData) {
  GtkWidget * psWidgetSet;
  bool boValue;

  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "NodeOverlay");
  boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
  SetNodeOverlay (boValue, psDandeData->psVisData);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "LinkOverlay");
  boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
  SetLinkOverlay (boValue, psDandeData->psVisData);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "FadeText");
  boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
  SetFadeText (boValue, psDandeData->psVisData);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "NodeSelections");
  boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
  SetNodeSelections (boValue, psDandeData->psVisData);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "LinkChanges");
  boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
  SetLinkChanges (boValue, psDandeData->psVisData);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Invert");
  boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
  SetClearWhite (boValue, psDandeData->psVisData);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Plane");
  boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
  SetAddToPlane (boValue, psDandeData->psVisData);
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "ShowNotes");
  boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
  SetNoteDisplay (boValue, GetNotesPersist (psDandeData->psVisData));
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "FloatNotes");
  boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
  SetNoteTethering ((boValue ? 0.1f : 1.0f), GetNotesPersist (psDandeData->psVisData));
}

void SetDisplayPropertiesDialogue (DandePersist * psDandeData) {
  GtkWidget * psWidget;
  float fViewRadius;
  float fLinkLen;
  float fCentring;
  float fRigidity;
  float fForce;
  float fResistance;
  float fLinkScalar;

  GetDisplayProperties (& fViewRadius, & fLinkLen, & fCentring, & fRigidity, & fForce, & fResistance, 
    & fLinkScalar, psDandeData->psVisData);

  // Spin buttons
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Zoom");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fViewRadius);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "LinkLen");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fLinkLen);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Centring");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fCentring);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Rigidity");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fRigidity);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Force");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fForce);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Resistance");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fResistance);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Scalar");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fLinkScalar);

  // Sliders
  psWidget = glade_xml_get_widget (psDandeData->psXML, "ZoomSlider");
  gtk_range_set_value (GTK_RANGE (psWidget), fViewRadius);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "LinkLenSlider");
  gtk_range_set_value (GTK_RANGE (psWidget), fLinkLen);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "CentringSlider");
  gtk_range_set_value (GTK_RANGE (psWidget), fCentring);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "RigiditySlider");
  gtk_range_set_value (GTK_RANGE (psWidget), fRigidity);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "ForceSlider");
  gtk_range_set_value (GTK_RANGE (psWidget), fForce);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "ResistanceSlider");
  gtk_range_set_value (GTK_RANGE (psWidget), fResistance);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "ScalarSlider");
  gtk_range_set_value (GTK_RANGE (psWidget), fLinkScalar);
}

void SetDisplayPropertiesValues (DandePersist * psDandeData) {
  GtkWidget * psWidgetSet;
  float fViewRadius;
  float fLinkLen;
  float fCentring;
  float fRigidity;
  float fForce;
  float fResistance;
  float fScalar;

  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Zoom");
  fViewRadius = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "LinkLen");
  fLinkLen = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Centring");
  fCentring = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Rigidity");
  fRigidity = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Force");
  fForce = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Resistance");
  fResistance = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, "Scalar");
  fScalar = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));

  SetDisplayProperties (fViewRadius, fLinkLen, fCentring, fRigidity, fForce, fResistance, fScalar, 
    psDandeData->psVisData);
}

//static gboolean DisplayPropertiesSpinChanged (GtkWidget * psWidget, gpointer psData) {
//  DandePersist * psDandeData = (DandePersist * )psData;
//  GtkWidget * psWidgetSet;
//  float fValue;
//  const char * szName;
//  GString * szTransfer;

//  // Transfer value to slider
//  szName = glade_get_widget_name (psWidget);

//  szTransfer = g_string_new (szName);
//  g_string_append (szTransfer, "Slider");

//  fValue = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidget));
//  psWidgetSet = glade_xml_get_widget (psDandeData->psXML, szTransfer->str);
//  gtk_range_set_value (GTK_RANGE (psWidgetSet), fValue);

//  g_string_free (szTransfer, TRUE);

//  // Update the values
//  SetDisplayPropertiesValues (psDandeData);

//  return TRUE;
//}

//static gboolean DisplayPropertiesSliderChanged (GtkWidget * psWidget, gpointer psData) {
//  DandePersist * psDandeData = (DandePersist * )psData;
//  GtkWidget * psWidgetSet;
//  float fValue;
//  const char * szName;
//  GString * szTransfer;
//  int nLength;

//  // Transfer value to spin button
//  szName = glade_get_widget_name (psWidget);

//  szTransfer = g_string_new (szName);
//  nLength = szTransfer->len - sizeof ("Slider") + 1;
//  if (nLength > 0) {
//    g_string_truncate (szTransfer, nLength);

//    fValue = gtk_range_get_value (GTK_RANGE (psWidget));
//    psWidgetSet = glade_xml_get_widget (psDandeData->psXML, szTransfer->str);
//    gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), fValue);
//  }

//  g_string_free (szTransfer, TRUE);

//  // The values will be updated if the Spin Button value changes

//  return TRUE;
//}

int main (int argc, char *argv[]) {
  GdkGLConfig * GlConfig;
  GtkWidget * psWindow;
  GtkWidget * psWidget;
  VisPersist * psVisData;
  DandePersist * psDandeData;
  ServerPersist * psServerData = NULL;
  float * pfVariable;

  // Initialise various libraries
  gtk_init (&argc, &argv);
  gtk_gl_init (&argc, &argv);
  glutInit (&argc, argv);

  // Create new persistent structures
  psVisData = NewVisPersist ();
  psDandeData = NewDandePersist ();
  psDandeData->psVisData = psVisData;

  // Start up the server
  psServerData = NewServerPersist (DEFAULT_PORT, psVisData);
  VisSetServer (psServerData, psVisData);

  // First try double buffering
  GlConfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH  | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_ALPHA | GDK_GL_MODE_STENCIL);
  if (GlConfig == NULL) {
    g_print ("*** Cannot find the double-buffered visual.\n");
    g_print ("*** Trying single-buffered visual.\n");

    // If that fails, we'll try single buffered
    GlConfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_ALPHA | GDK_GL_MODE_STENCIL);
    if (GlConfig == NULL) {
  	  g_print ("*** No appropriate OpenGL-capable visual found.\n");
  	  exit (1);
  	}
  }

  // Load the glade interface
  psDandeData->psXML = glade_xml_new (DANDEDIR "/Dandelion.glade", NULL, NULL);

  // Main window
  psWindow = glade_xml_get_widget (psDandeData->psXML, "MainWindow");
  gtk_container_set_reallocate_redraws (GTK_CONTAINER (psWindow), TRUE);
  psDandeData->psDrawingArea = glade_xml_get_widget (psDandeData->psXML, "DrawingArea");

  // Load settings
  LoadSettingsAll (psDandeData);

  //SetDisplayPropertiesDialogue (psDandeData);

  // Automatically redraw the window children change allocation
  gtk_container_set_reallocate_redraws (GTK_CONTAINER (psWindow), TRUE);

  // Set OpenGL-capability to the drawing area widget
  gtk_widget_set_gl_capability (psDandeData->psDrawingArea, GlConfig, NULL, TRUE, GDK_GL_RGBA_TYPE);

  // Connect up relevant signals
  glade_xml_signal_autoconnect (psDandeData->psXML);

  g_signal_connect_after (G_OBJECT (psDandeData->psDrawingArea), "realize", G_CALLBACK (Realize), 
    (gpointer)psDandeData);
  g_signal_connect_after (G_OBJECT (psDandeData->psDrawingArea), "unrealize", G_CALLBACK (Unrealize), 
    (gpointer)psDandeData);



  g_signal_connect (G_OBJECT (psDandeData->psDrawingArea), "configure_event", G_CALLBACK (ConfigureEvent), 
    (gpointer)psDandeData);
  g_signal_connect (G_OBJECT (psDandeData->psDrawingArea), "expose_event", G_CALLBACK (ExposeEvent), 
    (gpointer)psDandeData);

  g_signal_connect (G_OBJECT (psDandeData->psDrawingArea), "button_press_event", 
    G_CALLBACK (ButtonPressEvent), (gpointer)psDandeData);
  g_signal_connect (G_OBJECT (psDandeData->psDrawingArea), "button_release_event", 
    G_CALLBACK (ButtonReleaseEvent), (gpointer)psDandeData);
  g_signal_connect (G_OBJECT (psDandeData->psDrawingArea), "motion_notify_event", 
    G_CALLBACK (MotionNotifyEvent), (gpointer)psDandeData);
  g_signal_connect (G_OBJECT (psDandeData->psDrawingArea), "scroll_event", 
    G_CALLBACK (ScrollEvent), (gpointer)psDandeData);

  g_signal_connect (G_OBJECT (psDandeData->psDrawingArea), "map_event", G_CALLBACK (MapEvent), 
    (gpointer)psDandeData);
  g_signal_connect (G_OBJECT (psDandeData->psDrawingArea), "unmap_event", G_CALLBACK (UnmapEvent), 
    (gpointer)psDandeData);
  g_signal_connect (G_OBJECT (psDandeData->psDrawingArea), "visibility_notify_event", 
    G_CALLBACK (VisibilityNotifyEvent), (gpointer)psDandeData);

  g_signal_connect (G_OBJECT (psWindow), "key_press_event", G_CALLBACK (KeyPressEvent), 
    (gpointer)psDandeData);
  g_signal_connect (G_OBJECT (psWindow), "key_release_event", G_CALLBACK (KeyReleaseEvent), 
    (gpointer)psDandeData);


  psWidget = glade_xml_get_widget (psDandeData->psXML, "OpenConfigure");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ConfigureDialogueOpen), (gpointer)psDandeData);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "ConfigureOK");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ConfigureDialogueOK), (gpointer)psDandeData);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Spin");
  g_signal_connect (psWidget, "toggled", G_CALLBACK (SpinVisualisation), (gpointer)psDandeData);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "PointTowards");
  g_signal_connect (psWidget, "toggled", G_CALLBACK (FocusOnNode), (gpointer)psDandeData);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Links");
  g_signal_connect (psWidget, "toggled", G_CALLBACK (ShowLinks), (gpointer)psDandeData);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Labels");
  g_signal_connect (psWidget, "toggled", G_CALLBACK (ShowLabels), (gpointer)psDandeData);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Shake");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ShakeNodes), (gpointer)psDandeData);
  psWidget = glade_xml_get_widget (psDandeData->psXML, "Clear");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ClearNodes), (gpointer)psDandeData);

	// Add the spin slider widgets
/*float * GetVariableFocusNear (VisPersist * psVisData);*/
/*float * GetVariableFocusFar (VisPersist * psVisData);*/
/*float * GetVariableFocusScaleNear (VisPersist * psVisData);*/
/*float * GetVariableFocusScaleFar (VisPersist * psVisData);*/
/*float * GetVariableFocusDarkenMax (VisPersist * psVisData);*/

  psWidget = glade_xml_get_widget (psDandeData->psXML, "SliderTable");

	pfVariable = GetVariableViewRadius (psDandeData->psVisData);
	psDandeData->psZoomSpinSlider = CreateSpinSlider ("Distance", pfVariable, 2.0, 100.0, 0.2, 10.0, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	pfVariable = GetVariableLinkLen (psDandeData->psVisData);
	CreateSpinSlider ("Link length", pfVariable, 0.0, 20.0, 0.1, 10.0, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	pfVariable = GetVariableCentring (psDandeData->psVisData);
	CreateSpinSlider ("Centring", pfVariable, 0.0, 10.0, 0.01, 10.0, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	pfVariable = GetVariableRigidity (psDandeData->psVisData);
	CreateSpinSlider ("Rigidity", pfVariable, 0.0, 0.5, 0.01, 10.0, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	pfVariable = GetVariableForce (psDandeData->psVisData);
	CreateSpinSlider ("Force", pfVariable, 0.0, 5.0, 0.01, 10.0, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	pfVariable = GetVariableResistance (psDandeData->psVisData);
	CreateSpinSlider ("Resistance", pfVariable, 0.0, 1.0, 0.01, 10.0, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	pfVariable = GetVariableLinkScalar (psDandeData->psVisData);
	CreateSpinSlider ("Dejitter", pfVariable, 0.0, 20.0, 0.01, 10.0, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	//pfVariable = GetVariableFocusNear (psDandeData->psVisData);
	//CreateSpinSlider ("Focus near", pfVariable, 0.0, 1.0, 0.001, 0.01, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	pfVariable = GetVariableFocusFar (psDandeData->psVisData);
	CreateSpinSlider ("Focal length", pfVariable, 0.0, 1.0, 0.001, 0.01, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	//pfVariable = GetVariableFocusScaleNear (psDandeData->psVisData);
	//CreateSpinSlider ("Blur near", pfVariable, 0.0, 1000.0, 10.0, 100.0, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	//pfVariable = GetVariableFocusScaleFar (psDandeData->psVisData);
	//CreateSpinSlider ("Blur far", pfVariable, 0.0, 1000.0, 10.0, 100.0, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

	//pfVariable = GetVariableFocusDarkenMax (psDandeData->psVisData);
	//CreateSpinSlider ("Darken", pfVariable, 0.0, 1.0, 0.01, 0.1, 3, GTK_TABLE (psWidget), psDandeData->psSpinSliderData);

  // Display the window
  gtk_widget_show (psWindow);

  // Initialise visualisation
  Init (psVisData);

  // Main loop
  gtk_main ();

  // Save settings
  SaveSettingsAll (psDandeData);

  // Close down the server
  VisSetServer (NULL, psVisData);
  DeleteServerPersist (psServerData);

  // Delete persistent structures
  DeleteVisPersist (psVisData);
  psDandeData->psVisData = NULL;
  DeleteDandePersist (psDandeData);

  return 0;
}

void SaveSettingsAll (DandePersist * psDandeData) {
  SettingsPersist * psSettingsData = NULL;

  psSettingsData = NewSettingsPersist ("dandelion", ".dandelion.xml");

  SettingsSaveStart (psSettingsData);

  SettingsStartTag (psSettingsData, "vis");
  SaveSettingsVis (psSettingsData, psDandeData->psVisData);
  SettingsEndTag (psSettingsData, "vis");

  SettingsSaveEnd (psSettingsData);

  DeleteSettingsPersist (psSettingsData);
}

void LoadSettingsAll (DandePersist * psDandeData) {
  SettingsPersist * psSettingsData = NULL;
  SettingsLoadParser * psLoadParser = NULL;

  psSettingsData = NewSettingsPersist ("dandelion", ".dandelion.xml");
  psLoadParser = g_new0 (SettingsLoadParser, 1);


  psLoadParser->LoadProperty = DandeLoadProperty;
  psLoadParser->LoadSectionStart = DandeLoadSectionStart;
  psLoadParser->LoadSectionEnd = DandeLoadSectionEnd;
  psLoadParser->psData = psDandeData;

  AddParser (psLoadParser, psSettingsData);
  
  SettingsLoad (psSettingsData);

  RemoveParser (psSettingsData);
  g_free (psLoadParser);
  DeleteSettingsPersist (psSettingsData);
}

void DandeLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
  // Nothing to do at this level
}

void DandeLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  DandePersist * psDandeData = (DandePersist *)psData;

  if (stricmp (szName, "vis") == 0) {
    // Move in to the vis section
    LoadSettingsStartVis (psSettingsData, psDandeData->psVisData);
  }
}

void DandeLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
  // Nothing to do at this level
}

