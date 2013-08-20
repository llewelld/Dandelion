#!/usr/bin/env python

"""
A simple Dandelion client.
"""

import dlclient

dlclient = dlclient.DlClient()

#dlclient.start()

for node in range(0,200):
	dlclient.addNodePropFloat(str(node),"new", node)

#dlclient.addNodePropString("46", "test", "test");

dlclient.quit()



