///////////////////////////////////////////////////////////////////
// DandeClient
// Simple example client for Dandelion network visualiser
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2008
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include "dlclient.h"

#include <math.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

///////////////////////////////////////////////////////////////////
// Defines

#define ARRANGESHAKESCALE (5.0)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _ProgPersist {
  GladeXML * psXML;
  ClientPersist * psClientData;
  int nQuickNodeCount;
} ProgPersist;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

ProgPersist * NewProgPersist (void);
void DeleteProgPersist (ProgPersist * psProgData);
void WarningMessage (gchar const * szMessage, GladeXML * psXML);
void ButtonClickedConnect (GtkWidget * psWidget, gpointer psData);
void ButtonClickedDisconnect (GtkWidget * psWidget, gpointer psData);
void ButtonClickedAddNode (GtkWidget * psWidget, gpointer psData);
void ButtonClickedSubNode (GtkWidget * psWidget, gpointer psData);
void ButtonClickedShakeNode (GtkWidget * psWidget, gpointer psData);
void ButtonClickedAddLink (GtkWidget * psWidget, gpointer psData);
void ButtonClickedSubLink (GtkWidget * psWidget, gpointer psData);
void ButtonClickedClear (GtkWidget * psWidget, gpointer psData);
void ButtonClickedQuickNode (GtkWidget * psWidget, gpointer psData);
void ButtonClickedQuickLink (GtkWidget * psWidget, gpointer psData);
void ButtonClickedAddProp (GtkWidget * psWidget, gpointer psData);
void ButtonClickedSubProp (GtkWidget * psWidget, gpointer psData);

///////////////////////////////////////////////////////////////////
// Function definitions

ProgPersist * NewProgPersist (void) {
  ProgPersist * psProgData = NULL;

  psProgData = g_new0 (ProgPersist, 1);
  psProgData->psClientData = NewClientPersist ("localhost", 4972);
  psProgData->nQuickNodeCount = 0;

  return psProgData;
}

void DeleteProgPersist (ProgPersist * psProgData) {
  if (psProgData->psClientData) {
    DeleteClientPersist (psProgData->psClientData);
  }

  g_free (psProgData);
}

int main (int argc, char * argv[]) {
  ProgPersist * psProgData = NULL;

  GtkWidget * psMainWindow;
  GtkWidget * psWidget;

  gtk_init (&argc, & argv);
  gnet_init ();

  psProgData = NewProgPersist ();

  psProgData->psXML = glade_xml_new (DANDEDIR "/DandeClient.glade", NULL, NULL);

  /* Connect up the signals */
  glade_xml_signal_autoconnect (psProgData->psXML);

  // We want to add some user_data to some of the callbacks, so we assign them here
  psWidget = glade_xml_get_widget (psProgData->psXML, "Connect");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedConnect), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "Disconnect");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedDisconnect), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "AddNode");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedAddNode), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "SubNode");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedSubNode), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "ShakeNode");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedShakeNode), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "AddLink");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedAddLink), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "SubLink");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedSubLink), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "Clear");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedClear), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "QuickNode");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedQuickNode), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "QuickLink");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedQuickLink), (gpointer *)(psProgData));

  psWidget = glade_xml_get_widget (psProgData->psXML, "AddProperty");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedAddProp), (gpointer *)(psProgData));
  psWidget = glade_xml_get_widget (psProgData->psXML, "SubProperty");
  g_signal_connect (psWidget, "clicked", G_CALLBACK (ButtonClickedSubProp), (gpointer *)(psProgData));

  psWidget = glade_xml_get_widget (psProgData->psXML, "PropertyType");
	gtk_combo_box_set_active (GTK_COMBO_BOX (psWidget), 2);
  psWidget = glade_xml_get_widget (psProgData->psXML, "PropertyApplyTo");
	gtk_combo_box_set_active (GTK_COMBO_BOX (psWidget), 0);

  psMainWindow = glade_xml_get_widget (psProgData->psXML, "Main");
  gtk_widget_show (psMainWindow);

  // Enter the main program loop
  gtk_main ();
  // End main program loop

  DeleteProgPersist (psProgData);

  return 0;
}

void ButtonClickedConnect (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gchar const * szHostname;
  gint nPort;
  gboolean boSuccess = FALSE;

  psWidget = glade_xml_get_widget (psProgData->psXML, "Address");
  szHostname = gtk_entry_get_text (GTK_ENTRY (psWidget));

  psWidget = glade_xml_get_widget (psProgData->psXML, "Port");
  nPort = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (psWidget));

  g_string_assign (psProgData->psClientData->szHostname, szHostname);
  psProgData->psClientData->nPort = nPort;

  boSuccess = CreateConnection (psProgData->psClientData);

  if (!boSuccess) {
    WarningMessage ("Failed to connect", psProgData->psXML);
  }
}

void ButtonClickedDisconnect (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gboolean boSuccess = FALSE;

  //DeleteClientPersist (psProgData->psClientData);
  //psProgData->psClientData = NULL;

  boSuccess = Quit (psProgData->psClientData);

  if (!boSuccess) {
    WarningMessage ("Failed to send disconnect request", psProgData->psXML);
  }
}

void ButtonClickedAddNode (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gchar const * szNode;
  gboolean boSuccess = FALSE;

  psWidget = glade_xml_get_widget (psProgData->psXML, "NodeName");
  szNode = gtk_entry_get_text (GTK_ENTRY (psWidget));

  boSuccess = AddNode (szNode, psProgData->psClientData);

  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
}

void ButtonClickedSubNode (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gchar const * szNode;
  gboolean boSuccess = FALSE;

  psWidget = glade_xml_get_widget (psProgData->psXML, "NodeName");
  szNode = gtk_entry_get_text (GTK_ENTRY (psWidget));

  boSuccess = SubNode (szNode, psProgData->psClientData);

  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
}

void ButtonClickedShakeNode (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gchar const * szNode;
  gboolean boSuccess = FALSE;
  double fTheta;
  double fPhi;
  double fMagnitude;
  double fX;
  double fY;
  double fZ;

  fTheta = ((double)rand () / (double)RAND_MAX) * 2.0 * M_PI;
  fPhi = ((double)rand () / (double)RAND_MAX) * 2.0 * M_PI;
  fMagnitude = ((double)rand () / (double)RAND_MAX) * ARRANGESHAKESCALE;
  fX = fMagnitude * cos (fPhi) * cos (fTheta);
  fY = fMagnitude * cos (fPhi) * sin (fTheta);
  fZ = fMagnitude * sin (fPhi);
  
  psWidget = glade_xml_get_widget (psProgData->psXML, "NodeName");
  szNode = gtk_entry_get_text (GTK_ENTRY (psWidget));

  boSuccess = MoveNode (szNode, fX, fY, fZ, psProgData->psClientData);

  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
}

void ButtonClickedAddLink (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gchar const * szName;
  gchar const * szNodeFrom;
  gchar const * szNodeTo;
  gboolean boSuccess = FALSE;
  gboolean boBidirected;
  gboolean boNamed;

  psWidget = glade_xml_get_widget (psProgData->psXML, "LinkName");
  szName = gtk_entry_get_text (GTK_ENTRY (psWidget));
  psWidget = glade_xml_get_widget (psProgData->psXML, "NodeFrom");
  szNodeFrom = gtk_entry_get_text (GTK_ENTRY (psWidget));
  psWidget = glade_xml_get_widget (psProgData->psXML, "NodeTo");
  szNodeTo = gtk_entry_get_text (GTK_ENTRY (psWidget));
  psWidget = glade_xml_get_widget (psProgData->psXML, "Bidirected");
  boBidirected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidget));

	boNamed = (strlen (szName) > 0);

  if (boBidirected)  {
  	if (boNamed) {
	    boSuccess = AddLinkBiNamed (szName, szNodeFrom, szNodeTo, psProgData->psClientData);
	  }
	  else {
	    boSuccess = AddLinkBi (szNodeFrom, szNodeTo, psProgData->psClientData);
	  }
  }
  else {
  	if (boNamed) {
	    boSuccess = AddLinkNamed (szName, szNodeFrom, szNodeTo, psProgData->psClientData);
	  }
	  else {
	    boSuccess = AddLink (szNodeFrom, szNodeTo, psProgData->psClientData);
	  }
  }

  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
}

void ButtonClickedSubLink (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gchar const * szName;
  gchar const * szNodeFrom;
  gchar const * szNodeTo;
  gboolean boSuccess = FALSE;
  gboolean boBidirected;
  gboolean boNamed;

  psWidget = glade_xml_get_widget (psProgData->psXML, "LinkName");
  szName = gtk_entry_get_text (GTK_ENTRY (psWidget));
  psWidget = glade_xml_get_widget (psProgData->psXML, "NodeFrom");
  szNodeFrom = gtk_entry_get_text (GTK_ENTRY (psWidget));
  psWidget = glade_xml_get_widget (psProgData->psXML, "NodeTo");
  szNodeTo = gtk_entry_get_text (GTK_ENTRY (psWidget));
  psWidget = glade_xml_get_widget (psProgData->psXML, "Bidirected");
  boBidirected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidget));

	boNamed = (strlen (szName) > 0);

  if (boBidirected)  {
  	if (boNamed) {
	    boSuccess = SubLinkNamed (szName, psProgData->psClientData);
	  }
	  else {
	    boSuccess = SubLinkBi (szNodeFrom, szNodeTo, psProgData->psClientData);
	  }
  }
  else {
  	if (boNamed) {
	    boSuccess = SubLinkNamed (szName, psProgData->psClientData);
	  }
	  else {
	    boSuccess = SubLink (szNodeFrom, szNodeTo, psProgData->psClientData);
	  }
  }

  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
}

void ButtonClickedClear (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gboolean boSuccess = FALSE;

  boSuccess = Clear (psProgData->psClientData);
  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
  else {
    psProgData->nQuickNodeCount = 0;
  }
}

void ButtonClickedQuickNode (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gboolean boSuccess = FALSE;
  GString * szNewNode;
  GString * szLinkNode;
  gboolean boBidirected;

  psWidget = glade_xml_get_widget (psProgData->psXML, "Bidirected");
  boBidirected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidget));

  szNewNode = g_string_new ("");
  szLinkNode = g_string_new ("");
  g_string_printf (szNewNode, "%d", psProgData->nQuickNodeCount);
  if (psProgData->nQuickNodeCount > 0) {
    g_string_printf (szLinkNode, "%d", (rand() % psProgData->nQuickNodeCount));
  }

  boSuccess = AddNode (szNewNode->str, psProgData->psClientData);
  if ((boSuccess) && (psProgData->nQuickNodeCount > 0)) {
    if (boBidirected) {
      boSuccess = AddLinkBi (szLinkNode->str, szNewNode->str, psProgData->psClientData);
    }
    else {
      boSuccess = AddLink (szLinkNode->str, szNewNode->str, psProgData->psClientData);
    }
  }

  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
  g_string_free (szNewNode, TRUE);
  g_string_free (szLinkNode, TRUE);

  psProgData->nQuickNodeCount++;
}

void ButtonClickedQuickLink (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gboolean boSuccess = FALSE;
  GString * szNodeFrom;
  GString * szNodeTo;
  gboolean boBidirected;

  if (psProgData->nQuickNodeCount > 0) {
    psWidget = glade_xml_get_widget (psProgData->psXML, "Bidirected");
    boBidirected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidget));

    szNodeFrom = g_string_new ("");
    szNodeTo = g_string_new ("");
    g_string_printf (szNodeFrom, "%d", (rand() % psProgData->nQuickNodeCount));
    g_string_printf (szNodeTo, "%d", (rand() % psProgData->nQuickNodeCount));

    if (boBidirected) {
      boSuccess = AddLinkBi (szNodeFrom->str, szNodeTo->str, psProgData->psClientData);
    }
    else {
      boSuccess = AddLink (szNodeFrom->str, szNodeTo->str, psProgData->psClientData);
    }
    g_string_free (szNodeFrom, TRUE);
    g_string_free (szNodeTo, TRUE);
  }

  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
}

void ButtonClickedAddProp (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gchar const * szName;
  gchar const * szProperty;
  gint nType;
  gchar const * szValue;
  int nValue = 0;
  float fValue = 0.0f;
  gboolean boSuccess = FALSE;
  gint nApplyTo;

  psWidget = glade_xml_get_widget (psProgData->psXML, "PropertyApplyTo");
	nApplyTo = gtk_combo_box_get_active (GTK_COMBO_BOX (psWidget));

	if (nApplyTo == 0) {
	  psWidget = glade_xml_get_widget (psProgData->psXML, "NodeName");
	}
	else {
	  psWidget = glade_xml_get_widget (psProgData->psXML, "LinkName");
	}
  szName = gtk_entry_get_text (GTK_ENTRY (psWidget));

  psWidget = glade_xml_get_widget (psProgData->psXML, "PropertyName");
  szProperty = gtk_entry_get_text (GTK_ENTRY (psWidget));

  psWidget = glade_xml_get_widget (psProgData->psXML, "PropertyType");
	nType = gtk_combo_box_get_active (GTK_COMBO_BOX (psWidget));

  psWidget = glade_xml_get_widget (psProgData->psXML, "PropertyValue");
  szValue = gtk_entry_get_text (GTK_ENTRY (psWidget));

	switch (nType) {
	case 0:
		sscanf (szValue, "%d", & nValue);
		if (nApplyTo == 0) {
		  boSuccess = AddNodePropInt (szName, szProperty, nValue, psProgData->psClientData);
		}
		else {
		  boSuccess = AddLinkPropInt (szName, szProperty, nValue, psProgData->psClientData);
		}
		break;
	case 1:
		sscanf (szValue, "%f", & fValue);
		if (nApplyTo == 0) {
		  boSuccess = AddNodePropFloat (szName, szProperty, fValue, psProgData->psClientData);
		}
		else {
		  boSuccess = AddLinkPropFloat (szName, szProperty, fValue, psProgData->psClientData);
		}
		break;
	case 2:
		if (nApplyTo == 0) {
		  boSuccess = AddNodePropString (szName, szProperty, szValue, psProgData->psClientData);
		}
		else {
		  boSuccess = AddLinkPropString (szName, szProperty, szValue, psProgData->psClientData);
		}
		break;
	default:
		// Do nothing
		break;
	}

  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
}

void ButtonClickedSubProp (GtkWidget * psWidget, gpointer psData) {
  ProgPersist * psProgData = (ProgPersist *)psData;
  gchar const * szName;
  gchar const * szProperty;
  gboolean boSuccess = FALSE;
  gint nApplyTo;

  psWidget = glade_xml_get_widget (psProgData->psXML, "PropertyApplyTo");
	nApplyTo = gtk_combo_box_get_active (GTK_COMBO_BOX (psWidget));

	if (nApplyTo == 0) {
	  psWidget = glade_xml_get_widget (psProgData->psXML, "NodeName");
	}
	else {
	  psWidget = glade_xml_get_widget (psProgData->psXML, "LinkName");
	}
  szName = gtk_entry_get_text (GTK_ENTRY (psWidget));

  psWidget = glade_xml_get_widget (psProgData->psXML, "PropertyName");
  szProperty = gtk_entry_get_text (GTK_ENTRY (psWidget));

	if (nApplyTo == 0) {
	  boSuccess = SubNodeProp (szName, szProperty, psProgData->psClientData);
	}
	else {
	  boSuccess = SubLinkProp (szName, szProperty, psProgData->psClientData);
	}

  if (!boSuccess) {
    WarningMessage ("Failed to send message", psProgData->psXML);
  }
}

void WarningMessage (gchar const * szMessage, GladeXML * psXML) {
  GtkWidget * psMessage;
  GtkWidget * psMainWindow;

  // Announce the warning
  psMainWindow = glade_xml_get_widget (psXML, "Main");
  psMessage = gtk_message_dialog_new (GTK_WINDOW (psMainWindow), GTK_DIALOG_DESTROY_WITH_PARENT, 
    GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "%s", szMessage);
  gtk_dialog_run (GTK_DIALOG (psMessage));
  gtk_widget_destroy (psMessage);
}


