///////////////////////////////////////////////////////////////////
// Dandelion Server
// Network listener for Dandelion networl visualiser
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2008
///////////////////////////////////////////////////////////////////

#ifndef SERVER_H
#define SERVER_H

///////////////////////////////////////////////////////////////////
// Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gnet.h>

typedef struct _ServerConnectionPersist ServerConnectionPersist;
typedef struct _ServerPersist ServerPersist;

#include "vis.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef enum {
  SERVERSTATE_INVALID = -1,

  SERVERSTATE_COMMANDS,

  SERVERSTATENUM
} SERVERSTATE;


///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

ServerPersist * NewServerPersist (int nPort, VisPersist * psVisData);
void DeleteServerPersist (ServerPersist * psServerData) ;

gboolean ServerBroadcastCommand (char * szCommand, ServerPersist * psServerData);
gboolean ServerSendCommand (char * szCommand, ServerConnectionPersist * psConnectionData);
void ServerCallback (GServer * psServer, GConn * psConn, gpointer psData);

gboolean ServerBroadcastCHANGELINK (char const * szOldFrom, char const * szOldTo, char const * szNewFrom, char const * szNewTo, ServerPersist * psServerData);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* SERVER_H */
