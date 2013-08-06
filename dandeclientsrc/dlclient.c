///////////////////////////////////////////////////////////////////
// Dandelion Client
// Client code for Dandelion network visualiser
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2008
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include "dlclient.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

static gboolean ClientConnectionCallback (GIOChannel * psIOChannel, GIOCondition nCondition, gpointer psData);
gboolean ClientDecodeDataReceived (ClientPersist * psClientData);
gboolean ClientRecieveCommand (ClientPersist * psClientData);
void ClientCommandQUIT (ClientPersist * psClientData);
void ClientCommandOKAY (ClientPersist * psClientData);
void ClientCommandFAIL (ClientPersist * psClientData);
void ClientCommandCHANGELINK (GString * szOldFrom, GString * szOldTo, GString * szNewFrom, GString * szNewTo, ClientPersist * psClientData);
GString * ReadParameter (GString * szBuffer, int nStart, int * pnNext);

///////////////////////////////////////////////////////////////////
// Function definitions

ClientPersist * NewClientPersist (char const * szHostname, int nPort) {
  ClientPersist * psClientData = NULL;

  psClientData = g_new0 (ClientPersist, 1);
  psClientData->szBuffer = g_string_new ("");
  psClientData->eClientState = CLIENTSTATE_COMMANDS;
  psClientData->psAsyncSocket = NULL;
  psClientData->psAsyncChannel = NULL;
  psClientData->szHostname = g_string_new (szHostname);
  psClientData->nPort = nPort;

  return psClientData;
}

void DeleteClientPersist (ClientPersist * psClientData) {
  if (psClientData->psAsyncChannel) {
    g_io_channel_shutdown (psClientData->psAsyncChannel, TRUE, NULL);
  }
  if (psClientData->psAsyncSocket) {
    gnet_tcp_socket_delete (psClientData->psAsyncSocket);
  }
  g_string_free (psClientData->szBuffer, TRUE);
  g_string_free (psClientData->szHostname, TRUE);
  g_free (psClientData);
}

gboolean CreateConnection (ClientPersist * psClientData) {
  gboolean boSuccess = TRUE;

  if (psClientData->psAsyncSocket == NULL) {
    psClientData->psAsyncSocket = gnet_tcp_socket_connect (psClientData->szHostname->str, psClientData->nPort);
    if (psClientData->psAsyncSocket) {
      psClientData->psAsyncChannel = gnet_tcp_socket_get_io_channel (psClientData->psAsyncSocket);
      g_io_add_watch (psClientData->psAsyncChannel, (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL), 
        ClientConnectionCallback, (void *)psClientData);
    }
    else {
      fprintf (stderr, "Error: Failed to connect\n");
      boSuccess = FALSE;
    }
  }

  return boSuccess;
}

static gboolean ClientConnectionCallback (GIOChannel * psIOChannel, GIOCondition nCondition, gpointer psData) {
  ClientPersist * psClientData = (ClientPersist *)psData;
  GString * szBuffer = psClientData->szBuffer;
  gboolean boReturn = TRUE;

  /* Check if the socket has been closed */
  if (nCondition & G_IO_HUP) {
      boReturn = FALSE;
  }

  /* Check for socket error */
  if (nCondition & (G_IO_ERR | G_IO_NVAL)) {
      fprintf (stderr, "Error: Socket error (%d)\n", nCondition);
      //ClientError (psClientData);
      boReturn = FALSE;
  }

  /* Check for data to be read */
  if (nCondition & G_IO_IN) {
  	GIOStatus eStatus;
    GError * peError;
    gchar szRead[1024];
    gsize nBytesRead;

    /* Read the data into our buffer */
    eStatus = g_io_channel_read_chars (psIOChannel, szRead, sizeof (szRead), & nBytesRead, & peError);

    /* Check for stdin error */
    if (eStatus == G_IO_STATUS_ERROR) {
    	if (peError) {
	      fprintf (stderr, "Error: Read error (%d: %s)\n", peError->code, peError->message);
			}
			else {
	      fprintf (stderr, "Error: Read error\n");
			}
      //ClientError (psClientData);
      boReturn = FALSE;
    }
    else if (nBytesRead == 0) {
      gnet_tcp_socket_delete (psClientData->psAsyncSocket);
      psClientData->psAsyncSocket = NULL;
      boReturn = FALSE;
    }
    else {
      g_string_append_len (szBuffer, szRead, nBytesRead);

      /* Do some stuff with the data that arrived */
      while (ClientDecodeDataReceived (psClientData)) {/* Repeat until done */};
    }
  }

  if (psClientData->psAsyncChannel == NULL) {
    boReturn = FALSE;
  }

  return boReturn;
}

gboolean ClientSendCommand (char * szCommand, ClientPersist * psClientData) {
  GIOError eError;
  gsize nBytesWritten;
  gsize nBytesToSend;
  gboolean boSuccess = TRUE;

  printf ("Send command: %s", szCommand);

  if (psClientData->psAsyncSocket == NULL) {
    boSuccess = CreateConnection (psClientData);
  }

  if (boSuccess) {
    nBytesToSend = strlen (szCommand);
    eError = gnet_io_channel_writen (psClientData->psAsyncChannel, szCommand, nBytesToSend, & nBytesWritten);
    if ((eError != G_IO_ERROR_NONE) || (nBytesWritten != nBytesToSend)) {
      boSuccess = FALSE;
    }
  }

  return boSuccess;
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

gboolean ClientDecodeDataReceived (ClientPersist * psClientData) {
  gboolean boMore = FALSE;

  switch (psClientData->eClientState) {
  case CLIENTSTATE_COMMANDS:
    boMore = ClientRecieveCommand (psClientData);
    break;
  default:
    break;
  }

  return boMore;
}

gboolean ClientRecieveCommand (ClientPersist * psClientData) {
  GString * szBuffer = psClientData->szBuffer;
  gboolean boMore = FALSE;
  gchar * pcCommandEnd;
  int nCommandLen;
  GString * szCommand = NULL;
  GString * szParameter1 = NULL;
  GString * szParameter2 = NULL;
  GString * szParameter3 = NULL;
  GString * szParameter4 = NULL;
  int nNextParameter;

  pcCommandEnd = strchr (szBuffer->str, '\n');

  if (pcCommandEnd) {
    // The length does not include the newline
    nCommandLen = pcCommandEnd - szBuffer->str;

    szCommand = ReadParameter (szBuffer, 0, & nNextParameter);
    szParameter1 = ReadParameter (szBuffer, nNextParameter, & nNextParameter);
    szParameter2 = ReadParameter (szBuffer, nNextParameter, & nNextParameter);
    szParameter3 = ReadParameter (szBuffer, nNextParameter, & nNextParameter);
    szParameter4 = ReadParameter (szBuffer, nNextParameter, & nNextParameter);

    if (szCommand) {
      if (strcmp (szCommand->str, "OKAY") == 0) {
        // OKAY
        ClientCommandOKAY (psClientData);
      }
      else if (strcmp (szCommand->str, "FAIL") == 0) {
        // FAIL
        ClientCommandFAIL (psClientData);
      }
      else if (strcmp (szCommand->str, "QUIT") == 0) {
        // QUIT
        ClientCommandQUIT (psClientData);
      }
      else if (strcmp (szCommand->str, "CHANGELINK") == 0) {
        // CHANGELINK
        ClientCommandCHANGELINK (szParameter1, szParameter2, szParameter3, szParameter4, psClientData);
      }
      else {
        printf ("Unknown command: %s\n", szCommand->str);
      }
    }

    // Free the strings
    if (szCommand) {
      g_string_free (szCommand, TRUE);
    }
    if (szParameter1) {
      g_string_free (szParameter1, TRUE);
    }
    if (szParameter2) {
      g_string_free (szParameter2, TRUE);
    }
    if (szParameter3) {
      g_string_free (szParameter3, TRUE);
    }
    if (szParameter4) {
      g_string_free (szParameter4, TRUE);
    }

    // Remove the command
    g_string_erase (szBuffer, 0, nCommandLen + 1);

    if (szBuffer->len > 0) {
      boMore = TRUE;
    }
  }

  return boMore;
}

void ClientCommandQUIT (ClientPersist * psClientData) {
  // QUIT
  // Okay to quit
  //Close (psClientData);
  if (psClientData->psAsyncChannel) {
    g_io_channel_shutdown (psClientData->psAsyncChannel, TRUE, NULL);
  }
  psClientData->psAsyncChannel = NULL;
  if (psClientData->psAsyncSocket) {
    gnet_tcp_socket_delete (psClientData->psAsyncSocket);
  }
  psClientData->psAsyncSocket = NULL;
}

void ClientCommandOKAY (ClientPersist * psClientData) {
  // OKAY
  // Success!
  printf ("Success!\n");
  //Success (psClientData);
  //ClientSendCommand ("QUIT\n", psClientData);
}

void ClientCommandFAIL (ClientPersist * psClientData) {
  // FAIL
  // Failure :(
  printf ("Failure :(\n");
  //Failure (psClientData);
  //ClientSendCommand ("QUIT\n", psClientData);
}

void ClientCommandCHANGELINK (GString * szOldFrom, GString * szOldTo, GString * szNewFrom, GString * szNewTo, ClientPersist * psClientData) {
  // CHANGELINK <OldFrom> <OldTo> <NewFrom> <NewTo>
  printf ("Change link %s %s %s %s\n", szOldFrom->str, szOldTo->str, szNewFrom->str, szNewTo->str);
  ClientSendCommand ("OKAY\n", psClientData);
}

gboolean AddNode (char const * szNode, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDNODE %s\n", szNode);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean SubNode (char const * szNode, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "SUBNODE %s\n", szNode);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean MoveNode (char const * szNode, double fX, double fY, double fZ, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "MOVENODE %s %f %f %f\n", szNode, fX, fY, fZ);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean SetNodeColour (char const * szNode, double fRed, double fGreen, double fBlue, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "SETNODECOLOUR %s %f %f %f\n", szNode, fRed, fGreen, fBlue);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddLink (char const * szNodeFrom, char const * szNodeTo, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNodeFrom) > 0) && (strlen (szNodeTo) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDLINK %s %s\n", szNodeFrom, szNodeTo);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean SubLink (char const * szNodeFrom, char const * szNodeTo, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNodeFrom) > 0) && (strlen (szNodeTo) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "SUBLINK %s %s\n", szNodeFrom, szNodeTo);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddLinkBi (char const * szNode1, char const * szNode2, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode1) > 0) && (strlen (szNode2) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDLINKBI %s %s\n", szNode1, szNode2);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean SubLinkBi (char const * szNode1, char const * szNode2, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode1) > 0) && (strlen (szNode2) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "SUBLINKBI %s %s\n", szNode1, szNode2);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddLinkNamed (char const * szName, char const * szNodeFrom, char const * szNodeTo, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNodeFrom) > 0) && (strlen (szNodeTo) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDLINK %s %s %s\n", szName, szNodeFrom, szNodeTo);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean SubLinkNamed (char const * szName, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szName) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "SUBLINK %s\n", szName);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddLinkBiNamed (char const * szName, char const * szNode1, char const * szNode2, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode1) > 0) && (strlen (szNode2) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDLINKBI %s %s %s\n", szName, szNode1, szNode2);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean SetLinkColour (char const * szLink, double fRed, double fGreen, double fBlue, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szLink) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "SETLINKCOLOUR %s %f %f %f\n", szLink, fRed, fGreen, fBlue);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddNodePropInt (char const * szNode, char const * szProperty, int nValue, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode) > 0) && (strlen (szProperty) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDNODEPROP %s %s \"%s\" %d\n", szNode, szProperty, "int", nValue);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddNodePropFloat (char const * szNode, char const * szProperty, float fValue, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode) > 0) && (strlen (szProperty) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDNODEPROP %s %s \"%s\" %f\n", szNode, szProperty, "float", fValue);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddNodePropString (char const * szNode, char const * szProperty, char const * szValue, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode) > 0) && (strlen (szProperty) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDNODEPROP %s %s %s \"%s\"\n", szNode, szProperty, "string", szValue);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean SubNodeProp (char const * szNode, char const * szProperty, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szNode) > 0) && (strlen (szProperty) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "SUBNODEPROP %s %s\n", szNode, szProperty);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddLinkPropInt (char const * szLink, char const * szProperty, int nValue, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szLink) > 0) && (strlen (szProperty) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDLINKPROP %s %s %s %d\n", szLink, szProperty, "int", nValue);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddLinkPropFloat (char const * szLink, char const * szProperty, float fValue, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szLink) > 0) && (strlen (szProperty) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDLINKPROP %s %s %s %f\n", szLink, szProperty, "float", fValue);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean AddLinkPropString (char const * szLink, char const * szProperty, char const * szValue, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szLink) > 0) && (strlen (szProperty) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "ADDLINKPROP %s %s %s \"%s\"\n", szLink, szProperty, "string", szValue);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean SubLinkProp (char const * szLink, char const * szProperty, ClientPersist * psClientData) {
  GString * szCommand;
  gboolean boSuccess = FALSE;

  if ((psClientData) && (strlen (szLink) > 0) && (strlen (szProperty) > 0)) {
    szCommand = g_string_new ("");
    g_string_printf (szCommand, "SUBLINKPROP %s %s\n", szLink, szProperty);
    boSuccess = ClientSendCommand (szCommand->str, psClientData);
    g_string_free (szCommand, TRUE);
  }

  return boSuccess;
}

gboolean Clear (ClientPersist * psClientData) {
  gboolean boSuccess = FALSE;

  if (psClientData) {
    boSuccess = ClientSendCommand ("CLEAR\n", psClientData);
  }

  return boSuccess;
}

gboolean Quit (ClientPersist * psClientData) {
  gboolean boSuccess = FALSE;

  if (psClientData) {
    boSuccess = ClientSendCommand ("QUIT\n", psClientData);
  }

  return boSuccess;
}


