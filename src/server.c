///////////////////////////////////////////////////////////////////
// Dandelion Server
// Network listener for Dandelion networl visualiser
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2008
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include "server.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

struct _ServerPersist {
  GServer * psServer;
  VisPersist * psVisData;
  GSList * psConnections;
};

struct _ServerConnectionPersist {
  ServerPersist * psServerData;
  GString * szBuffer;
  GConn * psConn;
  SERVERSTATE eServerState;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

static void ServerConnectionCallback (GConn * psConn, GConnEvent * psEvent, gpointer psData);
void DeleteConnection (gpointer data, gpointer user_data);
gboolean ServerDecodeDataReceived (ServerConnectionPersist * psConnectionData);
GString * ReadParameter (GString * szBuffer, int nStart, int * pnNext);
gboolean ServerRecieveCommand (ServerConnectionPersist * psConnectionData);
void ServerCommandADDNODE (GString * szName, ServerConnectionPersist * psConnectionData);
void ServerCommandSUBNODE (GString * szName, ServerConnectionPersist * psConnectionData);
void ServerCommandMOVENODE (GString * szName, GString * szX, GString * szY, GString * szZ, ServerConnectionPersist * psConnectionData);
void ServerCommandSETNODECOLOUR (GString * szName, GString * szRed, GString * szGreen, GString * szBlue, ServerConnectionPersist * psConnectionData);
void ServerCommandADDLINK (GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData);
void ServerCommandADDLINKNamed (GString * szName, GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData);
void ServerCommandSUBLINK (GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData);
void ServerCommandSUBLINKNamed (GString * szName, ServerConnectionPersist * psConnectionData);
void ServerCommandADDLINKBI (GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData);
void ServerCommandADDLINKBINamed (GString * szName, GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData);
void ServerCommandSUBLINKBI (GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData);
void ServerCommandSETLINKCOLOUR (GString * szName, GString * szRed, GString * szGreen, GString * szBlue, ServerConnectionPersist * psConnectionData);
void ServerCommandCLEAR (ServerConnectionPersist * psConnectionData);
void ServerCommandQUIT (ServerConnectionPersist * psConnectionData);

void ServerCommandADDNODEPROP (GString * szNode, GString * szProperty, GString * szType, GString * szValue, ServerConnectionPersist * psConnectionData);
void ServerCommandSUBNODEPROP (GString * szNode, GString * szProperty, ServerConnectionPersist * psConnectionData);
void ServerCommandADDLINKPROP (GString * szLink, GString * szProperty, GString * szType, GString * szValue, ServerConnectionPersist * psConnectionData);
void ServerCommandSUBLINKPROP (GString * szLink, GString * szProperty, ServerConnectionPersist * psConnectionData);

///////////////////////////////////////////////////////////////////
// Function definitions

ServerPersist * NewServerPersist (int nPort, VisPersist * psVisData) {
  ServerPersist * psServerData;

  psServerData = g_new0 (ServerPersist, 1);
  psServerData->psVisData = psVisData;
  psServerData->psConnections = NULL;

  // Start up the server
  psServerData->psServer = gnet_server_new (NULL, nPort, ServerCallback, psServerData);
  if (!psServerData->psServer) {
    fprintf (stderr, "Error: Could not start server\n");
    exit (EXIT_FAILURE);
  }
  //printf ("Started server\n");

  return psServerData;
}

void DeleteServerPersist (ServerPersist * psServerData) {
  if (psServerData->psServer) {
    gnet_server_delete (psServerData->psServer);
    //printf ("Ended server\n");
  }

  // Free up all of the connections
  g_slist_foreach (psServerData->psConnections, DeleteConnection, NULL);
  g_slist_free (psServerData->psConnections);
  psServerData->psConnections = NULL;

  g_free (psServerData);
}

void DeleteConnection (gpointer data, gpointer user_data) {
  // Callback for freeing up connections
  ServerConnectionPersist * psConnectionData = (ServerConnectionPersist *)data;

  if (psConnectionData->psConn) {
    gnet_conn_unref (psConnectionData->psConn);
    psConnectionData->psConn = NULL;
    g_free (psConnectionData);
  }
}

void ServerCallback (GServer * psServer, GConn * psConn, gpointer psData) {
  ServerPersist * psServerData = (ServerPersist *)psData;
  ServerConnectionPersist * psConnectionData;

  if (psConn) {
    psConnectionData = g_new0 (ServerConnectionPersist, 1);
    psConnectionData->psServerData = psServerData;
    psConnectionData->szBuffer = g_string_new ("");
    psConnectionData->psConn = psConn;
    psConnectionData->eServerState = SERVERSTATE_COMMANDS;
    
    // Add the connection to the server's linked list
    psServerData->psConnections = g_slist_prepend (psServerData->psConnections, psConnectionData);

    gnet_conn_set_callback (psConn, ServerConnectionCallback, psConnectionData);
    gnet_conn_set_watch_error (psConn, TRUE);
    gnet_conn_read (psConn);
  }
  else {
    /* Error */
    fprintf (stderr, "Error: Server error\n");
    gnet_server_delete (psServer);
    exit (EXIT_FAILURE);
  }
}

static void ServerConnectionCallback (GConn * psConn, GConnEvent * psEvent, gpointer psData) {
  ServerConnectionPersist * psConnectionData = (ServerConnectionPersist *)psData;
  GString * szBuffer = psConnectionData->szBuffer;

  switch (psEvent->type) {
  case GNET_CONN_READ:
    g_string_append_len (szBuffer, psEvent->buffer, psEvent->length);

    /* Do some stuff with the data that arrived */
    while (ServerDecodeDataReceived (psConnectionData)) {/* Repeat until done */};

    gnet_conn_read (psConn);
    break;
  case GNET_CONN_WRITE:
    ; /* Do nothing */
    break;
  case GNET_CONN_ERROR:
    //ServerError (psConnectionData);
    /* Fall through */
  case GNET_CONN_CLOSE:
  case GNET_CONN_TIMEOUT:
    // Remove the connection from the server's linked list
    psConnectionData->psServerData->psConnections = g_slist_remove_all (psConnectionData->psServerData->psConnections, psConnectionData);
    g_string_free (psConnectionData->szBuffer, TRUE);
    g_free (psConnectionData);
    gnet_conn_unref (psConn);
    fprintf (stderr, "Connection closed\n");
    break;
  default:
    g_assert_not_reached ();
  }
}

gboolean ServerSendCommand (char * szCommand, ServerConnectionPersist * psConnectionData) {

  //printf ("Send command: %s", szCommand);

  gnet_conn_write (psConnectionData->psConn, szCommand, strlen (szCommand));

  return TRUE;
}

gboolean ServerBroadcastCommand (char * szCommand, ServerPersist * psServerData) {
  gboolean boResult;
  ServerConnectionPersist * psConnectionData;
  GSList * psConnection;

  boResult = TRUE;
  if (psServerData) {
    // Cycle through all of the connections sending the message for each
    psConnection = psServerData->psConnections;
    while (psConnection) {
      psConnectionData = (ServerConnectionPersist *)(psConnection->data);
      boResult &= ServerSendCommand (szCommand, psConnectionData);
      psConnection = g_slist_next (psConnection);
    }
  }

  // Return TRUE only if all of the messages were successfully sent
  return boResult;
}

gboolean ServerDecodeDataReceived (ServerConnectionPersist * psConnectionData) {
  gboolean boMore = FALSE;

  switch (psConnectionData->eServerState) {
  case SERVERSTATE_COMMANDS:
    boMore = ServerRecieveCommand (psConnectionData);
    break;
  default:
    break;
  }

  return boMore;
}

GString * ReadParameter (GString * szBuffer, int nStart, int * pnNext) {
  GString * szParameter = NULL;
  int nEnd;
  char cFind;

  if (pnNext) {
    * pnNext = nStart;
  }

  if (szBuffer && (nStart < szBuffer->len)) {
    // Find the next non-space
    while ((nStart < szBuffer->len) && (szBuffer->str[nStart] == ' ')) {
      nStart++;
    }

    if ((nStart < szBuffer->len) && (szBuffer->str[nStart] != '\n')) {
      // Check in case it's a quote
      if (szBuffer->str[nStart] == '"') {
        // We need to find the next quote
        cFind = '\"';
        // Move past the quote
        nStart++;
      }
      else {
        // We need to find the next space
        cFind = ' ';
      }      
      nEnd = nStart;

      // Find the end
      while ((nEnd < szBuffer->len) && (szBuffer->str[nEnd] != cFind) && (szBuffer->str[nEnd] != '\n')) {
        nEnd++;
      }

      if (nStart < nEnd) {
        // Take a copy of the parameter
        szParameter = g_string_sized_new (nEnd - nStart);
        g_string_insert_len (szParameter, 0, szBuffer->str + nStart, nEnd - nStart);

        if (cFind == '\"') {
          // We don't want the quote at the end
          nEnd++;
        }

        if (pnNext) {
          * pnNext = nEnd;
        }
      }
    }
  }

  return szParameter;
}

#define COMMAND_PARAMS	(4)

gboolean ServerRecieveCommand (ServerConnectionPersist * psConnectionData) {
  GString * szBuffer = psConnectionData->szBuffer;
  gboolean boMore = FALSE;
  gchar * pcCommandEnd;
  int nCommandLen;
  GString * szCommand = NULL;
  GString * szParameter[COMMAND_PARAMS];
  int nNextParameter;
  int nParam;
  int nParamNum;

  pcCommandEnd = strchr (szBuffer->str, '\n');

  if (pcCommandEnd) {
    // The length does not include the newline
    nCommandLen = pcCommandEnd - szBuffer->str;

    szCommand = ReadParameter (szBuffer, 0, & nNextParameter);
    nParamNum = 0;
		for (nParam = 0; ((nParam == nParamNum) && (nParam < COMMAND_PARAMS)); nParam++) {
	    szParameter[nParam] = ReadParameter (szBuffer, nNextParameter, & nNextParameter);
	    if (szParameter[nParam]) {
		    nParamNum++;
		  }
		}

    if (szCommand) {
      if (strcmp (szCommand->str, "ADDNODE") == 0) {
        // ADDNODE <name>
        ServerCommandADDNODE (szParameter[0], psConnectionData);
      }
      else if (strcmp (szCommand->str, "SUBNODE") == 0) {
        // SUBNODE <name>
        ServerCommandSUBNODE (szParameter[0], psConnectionData);
      }
      else if (strcmp (szCommand->str, "MOVENODE") == 0) {
        // MOVENODE <name> <x> <y> <z>
        ServerCommandMOVENODE (szParameter[0], szParameter[1], szParameter[2], szParameter[3], psConnectionData);
      }
      else if (strcmp (szCommand->str, "SETNODECOLOUR") == 0) {
        // SETNODECOLOUR <name> <red> <green> <blue>
        ServerCommandSETNODECOLOUR (szParameter[0], szParameter[1], szParameter[2], szParameter[3], psConnectionData);
      }
      else if (strcmp (szCommand->str, "ADDLINK") == 0) {
        // ADDLINK [Name] <From> <To>
        if (nParamNum > 2) {
	        ServerCommandADDLINKNamed (szParameter[0], szParameter[1], szParameter[2], psConnectionData);
        }
        else {
	        ServerCommandADDLINK (szParameter[0], szParameter[1], psConnectionData);
        }
      }
      else if (strcmp (szCommand->str, "SUBLINK") == 0) {
        // SUBLINK [Name]|(<From> <To>)
        if (nParamNum == 1) {
					ServerCommandSUBLINKNamed (szParameter[0], psConnectionData);
	      }
	      else {
	        ServerCommandSUBLINK (szParameter[0], szParameter[1], psConnectionData);
	      }
      }
      else if (strcmp (szCommand->str, "ADDLINKBI") == 0) {
        // ADDLINKBI [Name] <From> <To>
        if (nParamNum > 2) {
	        ServerCommandADDLINKBINamed (szParameter[0], szParameter[1], szParameter[2], psConnectionData);
	      }
	      else {
	        ServerCommandADDLINKBI (szParameter[0], szParameter[1], psConnectionData);
	      }
      }
      else if (strcmp (szCommand->str, "SUBLINKBI") == 0) {
        // SUBLINKBI [Name]|(<From> <To>)
				if (nParamNum == 1) {
	        ServerCommandSUBLINKNamed (szParameter[0], psConnectionData);
				}
				else {
	        ServerCommandSUBLINKBI (szParameter[0], szParameter[1], psConnectionData);
				}
      }
      else if (strcmp (szCommand->str, "SETLINKCOLOUR") == 0) {
        // SETLINKCOLOUR <name> <red> <green> <blue>
        ServerCommandSETLINKCOLOUR (szParameter[0], szParameter[1], szParameter[2], szParameter[3], psConnectionData);
      }
      else if (strcmp (szCommand->str, "ADDNODEPROP") == 0) {
			  // ADDNODEPROP <Node> <Property> <Type> <Value>
				ServerCommandADDNODEPROP (szParameter[0], szParameter[1], szParameter[2], szParameter[3], psConnectionData);
      }
      else if (strcmp (szCommand->str, "SUBNODEPROP") == 0) {
			  // SUBNODEPROP <Node> <Property>
				ServerCommandSUBNODEPROP (szParameter[0], szParameter[1], psConnectionData);
      }
      else if (strcmp (szCommand->str, "ADDLINKPROP") == 0) {
			  // ADDLINKPROP <Link> <Property> <Type> <Value>
				ServerCommandADDLINKPROP (szParameter[0], szParameter[1], szParameter[2], szParameter[3], psConnectionData);
      }
      else if (strcmp (szCommand->str, "SUBLINKPROP") == 0) {
			  // SUBLINKPROP <Node> <Property>
				ServerCommandSUBLINKPROP (szParameter[0], szParameter[1], psConnectionData);
      }
      else if (strcmp (szCommand->str, "CLEAR") == 0) {
        // CLEAR
        ServerCommandCLEAR (psConnectionData);
      }
      else if (strcmp (szCommand->str, "QUIT") == 0) {
        // QUIT
        ServerCommandQUIT (psConnectionData);
      }
      else if (strcmp (szCommand->str, "OKAY") == 0) {
        // OKAY
        // Do nothing
      }
      else if (strcmp (szCommand->str, "FAIL") == 0) {
        // FAIL
        // Do nothing
      }
      else {
        // Unknown command
        ServerSendCommand ("FAIL\n", psConnectionData);
      }
    }

    // Free the strings
    if (szCommand) {
      g_string_free (szCommand, TRUE);
    }
    for (nParam = 0; nParam < nParamNum; nParam++) {
    	if (szParameter[nParam]) {
	      g_string_free (szParameter[nParam], TRUE);
	    }
    }

    // Remove the command
    g_string_erase (szBuffer, 0, nCommandLen + 1);

    if (szBuffer->len > 0) {
      boMore = TRUE;
    }
  }

  return boMore;
}

void ServerCommandADDNODE (GString * szName, ServerConnectionPersist * psConnectionData) {
  // ADDNODE <name>
  // Add a node to the network visualisation
  AddNode (szName->str, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandSUBNODE (GString * szName, ServerConnectionPersist * psConnectionData) {
  // SUBNODE <name>
  // Remove the node from the network visualisation
  RemoveNode (szName->str, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandMOVENODE (GString * szName, GString * szX, GString * szY, GString * szZ, ServerConnectionPersist * psConnectionData) {
  double fX;
  double fY;
  double fZ;

  // MOVENODE <name> <x> <y> <z>
  // Move the node in the given direction in the network visualisation
  fX = atof (szX->str);
  fY = atof (szY->str);
  fZ = atof (szZ->str);
  MoveNode (szName->str, fX, fY, fZ, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandSETNODECOLOUR (GString * szName, GString * szRed, GString * szGreen, GString * szBlue, ServerConnectionPersist * psConnectionData) {
  float fRed;
  float fGreen;
  float fBlue;

  // SETNODECOLOUR <name> <red> <green> <blue>
  // Set the node colour
  fRed = atof (szRed->str);
  fGreen = atof (szGreen->str);
  fBlue = atof (szBlue->str);
  SetNodeColour (szName->str, fRed, fGreen, fBlue, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandADDLINK (GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData) {
  // ADDLINK <From> <To>
  // Add a link to the network visualisation
  AddLink ("", szNodeFrom->str, szNodeTo->str, FALSE, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandADDLINKNamed (GString * szName, GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData) {
  // ADDLINK <Name> <From> <To>
  // Add a named link to the network visualisation
  AddLink (szName->str, szNodeFrom->str, szNodeTo->str, FALSE, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandSUBLINK (GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData) {
	TLink * psLink;
  // SUBLINK <From> <To>
  // Remove the link from the network visualisation
  psLink = FindLinkNodesNamed (szNodeFrom->str, szNodeTo->str, psConnectionData->psServerData->psVisData);
  RemoveLink (psLink, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandSUBLINKBI (GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData) {
	TLink * psLink;
  // SUBLINK <From> <To>
  // Remove the link from the network visualisation
  psLink = FindLinkNodesNamed (szNodeFrom->str, szNodeTo->str, psConnectionData->psServerData->psVisData);
  RemoveLink (psLink, psConnectionData->psServerData->psVisData);
  psLink = FindLinkNodesNamed (szNodeTo->str, szNodeFrom->str, psConnectionData->psServerData->psVisData);
  RemoveLink (psLink, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandSUBLINKNamed (GString * szName, ServerConnectionPersist * psConnectionData) {
	TLink * psLink;
  // SUBLINK <Name>
  // Remove the named link from the network visualisation
  psLink = FindLinkNamed (szName->str, psConnectionData->psServerData->psVisData);
  RemoveLink (psLink, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandADDLINKBI (GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData) {
  // ADDLINKBI <From> <To>
  // Add a bidirectional link to the network visualisation
  AddLink ("", szNodeFrom->str, szNodeTo->str, TRUE, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandADDLINKBINamed (GString * szName, GString * szNodeFrom, GString * szNodeTo, ServerConnectionPersist * psConnectionData) {
  // ADDLINKBI <Name> <From> <To>
  // Add a named bidirectional link to the network visualisation
  AddLink (szName->str, szNodeFrom->str, szNodeTo->str, TRUE, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandSETLINKCOLOUR (GString * szName, GString * szRed, GString * szGreen, GString * szBlue, ServerConnectionPersist * psConnectionData) {
  float fRed;
  float fGreen;
  float fBlue;

  // SETLINKCOLOUR <name> <red> <green> <blue>
  // Set the link colour
  fRed = atof (szRed->str);
  fGreen = atof (szGreen->str);
  fBlue = atof (szBlue->str);
  SetLinkColourNamed (szName->str, fRed, fGreen, fBlue, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandCLEAR (ServerConnectionPersist * psConnectionData) {
  // CLEAR
  // Remove all nodes and linsk from the network visualisation
  RemoveAll (psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandQUIT (ServerConnectionPersist * psConnectionData) {
  // QUIT
  // Okay to quit
  ServerSendCommand ("QUIT\n", psConnectionData);
}

void ServerCommandADDNODEPROP (GString * szNode, GString * szProperty, GString * szType, GString * szValue, ServerConnectionPersist * psConnectionData) {
  // ADDNODEPROP <Node> <Property> <Type> <Value>
  // Add a property to a node
  SetNodeProperty (szNode->str, szProperty->str, szType->str, szValue->str, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandSUBNODEPROP (GString * szNode, GString * szProperty, ServerConnectionPersist * psConnectionData) {
  // SUBNODEPROP <Node> <Property>
  // Remove a property from a node
  SubNodeProperty (szNode->str, szProperty->str, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandADDLINKPROP (GString * szLink, GString * szProperty, GString * szType, GString * szValue, ServerConnectionPersist * psConnectionData) {
  // ADDLINKPROP <Link> <Property> <Type> <Value>
  // Add a property to a link
  SetLinkProperty (szLink->str, szProperty->str, szType->str, szValue->str, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

void ServerCommandSUBLINKPROP (GString * szLink, GString * szProperty, ServerConnectionPersist * psConnectionData) {
  // SUBLINKPROP <Node> <Property>
  // Remove a property from a link
  SubLinkProperty (szLink->str, szProperty->str, psConnectionData->psServerData->psVisData);
  ServerSendCommand ("OKAY\n", psConnectionData);
}

gboolean ServerBroadcastCHANGELINK (char const * szOldFrom, char const * szOldTo, char const * szNewFrom, char const * szNewTo, ServerPersist * psServerData) {
  GString * szCommand;
  gboolean boResult;

  // CHANGELINK <OldFrom> <OldTo> <NewFrom> <NewTo>
  // A link has been moved to a new node via the network visualisation
  szCommand = g_string_new ("");
  g_string_printf (szCommand, "CHANGELINK \"%s\" \"%s\" \"%s\" \"%s\"\n", szOldFrom, szOldTo, szNewFrom, szNewTo);

  boResult = ServerBroadcastCommand (szCommand->str, psServerData);

  g_string_free (szCommand, TRUE);
  
  return boResult;
}



