///////////////////////////////////////////////////////////////////
// Dandelion Client
// Client code for Dandelion network visualiser
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2008
///////////////////////////////////////////////////////////////////

#ifndef CLIENT_H
#define CLIENT_H

///////////////////////////////////////////////////////////////////
// Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gnet.h>

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef enum {
  CLIENTSTATE_INVALID = -1,

  CLIENTSTATE_COMMANDS,

  CLIENTSTATENUM
} CLIENTSTATE;

typedef struct _ClientPersist ClientPersist;

struct _ClientPersist {
  GString * szHostname;
  int nPort;
  GTcpSocket * psAsyncSocket;
  GIOChannel * psAsyncChannel;
  CLIENTSTATE eClientState;
  GString * szBuffer;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

ClientPersist * NewClientPersist (char const * szHostname, int nPort);
void DeleteClientPersist (ClientPersist * psClientData);
gboolean CreateConnection (ClientPersist * psClientData);
gboolean ClientSendCommand (char * szCommand, ClientPersist * psClientData);

gboolean AddNode (char const * szNode, ClientPersist * psClientData);
gboolean SubNode (char const * szNode, ClientPersist * psClientData);
gboolean MoveNode (char const * szNode, double fX, double fY, double fZ, ClientPersist * psClientData);
gboolean SetNodeColour (char const * szNode, double fRed, double fGreen, double fBlue, ClientPersist * psClientData);
gboolean AddLink (char const * szNodeFrom, char const * szNodeTo, ClientPersist * psClientData);
gboolean SubLink (char const * szNodeFrom, char const * szNodeTo, ClientPersist * psClientData);
gboolean AddLinkBi (char const * szNode1, char const * szNode2, ClientPersist * psClientData);
gboolean SubLinkBi (char const * szNode1, char const * szNode2, ClientPersist * psClientData);
gboolean AddLinkNamed (char const * szName, char const * szNodeFrom, char const * szNodeTo, ClientPersist * psClientData);
gboolean SubLinkNamed (char const * szName, ClientPersist * psClientData);
gboolean AddLinkBiNamed (char const * szName, char const * szNode1, char const * szNode2, ClientPersist * psClientData);

gboolean AddNodePropInt (char const * szNode, char const * szProperty, int nValue, ClientPersist * psClientData);
gboolean AddNodePropFloat (char const * szNode, char const * szProperty, float fValue, ClientPersist * psClientData);
gboolean AddNodePropString (char const * szNode, char const * szProperty, char const * szValue, ClientPersist * psClientData);
gboolean SubNodeProp (char const * szNode, char const * szProperty, ClientPersist * psClientData);

gboolean AddLinkPropInt (char const * szLink, char const * szProperty, int nValue, ClientPersist * psClientData);
gboolean AddLinkPropFloat (char const * szLink, char const * szProperty, float fValue, ClientPersist * psClientData);
gboolean AddLinkPropString (char const * szLink, char const * szProperty, char const * szValue, ClientPersist * psClientData);
gboolean SubLinkProp (char const * szLink, char const * szProperty, ClientPersist * psClientData);

gboolean Clear (ClientPersist * psClientData);
gboolean Quit (ClientPersist * psClientData);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* CLIENT_H */

