#!/usr/bin/env python

"""
A simple Dandelion client.
This can be used to communicate with a Dandelion server for
the purposes of visualising 3D networks.

http://www.flypig.co.uk/?to=dandelion

David Llewellyn-Jones
Liverpool John Moores University
Summer 2013

Released under an LGPL licence.
"""

import socket
import threading

class DlClient(threading.Thread):
	"""
	The main DlClient class encapsulates everything needed to
	communicate with a Dandelion server.
	
	The following is a simple example of its use.
	
	import dlclient
	
	dlclient = DlClient()
	dlclient.setHost("localhost")
	dlclient.addNode("FirstNode")
	dlclient.addNode("SecondNode")
	dlclient.addLink("MainLink", "FirstNode", "SecondNode")
	dlclient.quit()
	"""

	# Main class and thread methods

	def __init__(self):
		"""
		Class constructor.
		Initialise the class variables and start
		the network thread.
		"""
		threading.Thread.__init__(self)
		self.host = 'localhost'
		self.port = 4972
		self._size = 1024
		self._active = False
		self._buffer = []
		self._sock = None
		self._cond = threading.Condition()
		self._paused = False
		self.start()

	def __del__(self):
		"""
		Class destructor.
		Close the network connection if there is one
		in case the class is deleted.
		"""
		self.finish()

	def run(self):
		"""
		The main network thread code. This runs, waiting for
		commands to be added to the command buffer and sends
		them as they're added.
		The thread sleeps while there are no commands.
		"""
		self._active = True;
		while self._active:
			self._waitForData()

			if self._sock == None:
				try:
					self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
					self._sock.connect((self.host,self.port))
				except socket.error, (value, message):
					if self._sock:
						self._sock.close()
						self._sock = None
						print "Failed to connect: " + message
						self._buffer = []

			while len(self._buffer) > 0 and self._sock != None:
				nextCommand = self._getNextCommand()
				if nextCommand != "":
					self._sock.send(nextCommand)
					#print "Sending command: " + nextCommand
					result = self._sock.recv(self._size)
					#print "Received: " + result
					if result == "QUIT\n":
						self._active = False
					elif result != "OKAY\n":
						print "Command failure: " + nextCommand
		
		self._closeConnection()

	# Library public methods for use by other code

	def setHost(self, host):
		"""
		Set the hostname of the Dandelion server to connect to.
		"""
		self._closeConnection()
		self.host = host
		
	def setPort(self, port):
		"""
		Set the port to connect to the Dandelion server on.
		"""
		self._closeConnection()
		self.port = port

	def addNode(self, node):
		"""
		Add a node with the given name.
		
		Arguments:
		node -- the name of the node to add
		"""
		command = "ADDNODE \"" + node + "\"\n";
		self._sendCommand(command);

	def subNode(self, node):
		"""
		Remove the node with the given name.
	
		Arguments:
		node -- the name of the node to remove
		"""
		command = "SUBNODE \"" + node + "\"\n";
		self._sendCommand(command);
		
	def moveNode(self, node, x, y, z):
		"""
		Move the node with the given name in the given direction.
	
		Arguments:
		node -- the name of the node to remove
		x -- the distance to move the node along the x-axis
		y -- the distance to move the node along the y-axis
		z -- the distance to move the node along the z-axis
		"""
		command = "MOVENODE \"" + node + "\" " + str(x) + " " + str(y) + " " + str(z) + '\n';
		self._sendCommand(command);
		
	def setNodeColour(self, node, red, green, blue):
		"""
		Set the colour of the given node.
	
		Arguments:
		node -- the name of the node to colour
		red -- the red colomponent of the colour to set
		green -- the green colomponent of the colour to set
		blue -- the blue colomponent of the colour to set
		"""
		command = "SETNODECOLOUR \"" + node + "\" " + str(red) + " " + str(green) + " " + str(blue) + '\n';
		self._sendCommand(command);

	def addLink(self, nodeFrom, nodeTo):
		"""
		Add a link from one node to another.
	
		Arguments:
		nodeFrom -- the name of the node to link from
		nodeTo -- the name of the node to link to
		"""
		command = "ADDLINK \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
		self._sendCommand(command);
		
	def subLink(self, nodeFrom, nodeTo):
		"""
		Remove a link between one node and another.
	
		Arguments:
		nodeFrom -- the name of the node the link to be removed starts from
		nodeTo -- the name of the node the link to be removed ends at
		"""
		command = "SUBLINK \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
		self._sendCommand(command);
		
	def addLinkBi(self, node1, node2):
		"""
		Add a bidirectional link between the two nodes.
	
		Arguments:
		node1 -- a node to add the bidirectional link between
		node2 -- a node to add the bidirectional link between
		"""
		command = "ADDLINKBI \"" + node1 + "\" \"" + node2 + "\"\n";
		self._sendCommand(command);
		
	def subLinkBi(self, node1, node2):
		"""
		Remove a bidirectional link between the two nodes.
	
		Arguments:
		node1 -- a node to remove the bidirectional link from
		node2 -- a node to remove the bidirectional link from
		"""
		command = "SUBLINKBI \"" + node1 + "\" \"" + node2 + "\"\n";
		self._sendCommand(command);

	def addLinkNamed(self, name, nodeFrom, nodeTo):
		"""
		Add a named link from one node to another.
	
		Arguments:
		name -- the name of the link to add
		nodeFrom -- the name of the node to link from
		nodeTo -- the name of the node to link to
		"""
		command = "ADDLINK \"" + name + "\" \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
		self._sendCommand(command);

	def subLinkNamed(self, name):
		"""
		Remove a link.
	
		Arguments:
		name -- the link to remove
		"""
		command = "SUBLINK \"" + name + "\"\n";
		self._sendCommand(command);

	def addLinkBiNamed(self, name, nodeFrom, nodeTo):
		"""
		Add a bidirectional named link from one node to another.
	
		Arguments:
		name -- the name of the link to add
		nodeFrom -- the name of the node to link from
		nodeTo -- the name of the node to link to
		"""
		command = "ADDLINKBI \"" + name + "\" \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
		self._sendCommand(command);

	def setLinkColour(self, link, red, green, blue):
		"""
		Set the colour of the given link.
	
		Arguments:
		link -- the name of the link to colour
		red -- the red colomponent of the colour to set
		green -- the green colomponent of the colour to set
		blue -- the blue colomponent of the colour to set
		"""
		command = "SETLINKCOLOUR \"" + link + "\" " + str(red) + " " + str(green) + " " + str(blue) + '\n';
		self._sendCommand(command);

	def addNodePropInt(self, name, property, value):
		"""
		Add an integer property to a named node.
	
		Arguments:
		name -- the name of the node to apply the property to
		property -- name of the property to add
		value -- integer value to add
		"""
		command = "ADDNODEPROP \"" + name + "\" \"" + property + "\" int \"" + str(value) + "\"\n";
		self._sendCommand(command);

	def addNodePropFloat(self, name, property, value):
		"""
		Add a float property to a named node.
	
		Arguments:
		name -- the name of the node to apply the property to
		property -- name of the property to add
		value -- the float value to add
		"""
		command = "ADDNODEPROP \"" + name + "\" \"" + property + "\" float \"" + str(value) + "\"\n";
		self._sendCommand(command);

	def addNodePropString(self, name, property, value):
		"""
		Add a string property to a named node.
	
		Arguments:
		name -- the name of the node to apply the property to
		property -- the name of the property to add
		value -- the string value to add
		"""
		command = "ADDNODEPROP \"" + name + "\" \"" + property + "\" string \"" + value + "\"\n";
		self._sendCommand(command);

	def subNodeProp(self, name, property):
		"""
		Remove a string property from a named node.
	
		Arguments:
		name -- the name of the node to remove the property from
		property -- the name of the property to remove
		"""
		command = "SUBNODEPROP \"" + name + "\" \"" + property + "\"\n";
		self._sendCommand(command);

	def addLinkPropInt(self, name, property, value):
		"""
		Add an integer property to a named link.
	
		Arguments:
		name -- the name of the link to apply the property to
		property -- the name of the property to add
		value -- the integer value to add
		"""
		command = "ADDLINKPROP \"" + name + "\" \"" + property + "\" int \"" + str(value) + "\"\n";
		self._sendCommand(command);

	def addLinkPropFloat(self, name, property, value):
		"""
		Add a float property to a named link.
	
		Arguments:
		name -- the name of the link to apply the property to
		property -- the name of the property to add
		value -- the float value to add
		"""
		command = "ADDLINKPROP \"" + name + "\" \"" + property + "\" float \"" + str(value) + "\"\n";
		self._sendCommand(command);

	def addLinkPropString(self, name, property, value):
		"""
		Add a string property to a named link.

		Arguments:
		name -- the name of the link to apply the property to
		property -- the name of the property to add
		value -- the string value to add
		"""
		command = "ADDLINKPROP \"" + name + "\" \"" + property + "\" string \"" + value + "\"\n";
		self._sendCommand(command);

	def subLinkProp(self, name, property):
		"""
		Remove a string property from a named link.
	
		Arguments:
		name -- the name of the link to remove the property from
		property -- the name of the property to remove
		"""
		command = "SUBLINKPROP \"" + name + "\" \"" + property + "\"\n";
		self._sendCommand(command);

	def clear(self):
		"""
		Clear all nodes and links.
		"""
		command = "CLEAR\n";
		self._sendCommand(command);
		
	def quit(self):
		"""
		Finish off.
		"""
		command = "QUIT\n";
		self._sendCommand(command);
		self.finish();

	def finish(self):
		"""
		Close down the thread without sending the Quit command.
		"""
		self._active = False
		self._prod()

	# Internal private methods

	def _closeConnection(self):
		"""
		Close the connection to the server if there is one.
		For internal use. Use quit() to perform the same action cleanly.
		"""
		if self._sock != None:
			self._sock.close()
			self._sock = None

	def _waitForData(self):
		"""
		Wait for some data to be added to the buffer and 
		set the thread to sleep if there is none.
		For internal use. Commands should be added to the buffer
		using the many specific methods for this purpose.
		"""
		with self._cond:
			if len(self._buffer) <= 0:
				self._paused = True
				self._cond.wait()
				self._paused = False

	def _getNextCommand(self):
		"""
		Extract the next command from the command buffer
		so that it can be sent.
		For internal use.
		"""
		command = ""
		if len(self._buffer) > 0:
			command = self._buffer.pop(0)
		return command

	def _prod(self):
		"""
		Prod the thread so that it wakes up.
		For internal use.
		"""
		if self._paused:
			with self._cond:
				self._cond.notify()

	def _sendCommand(self, command):
		"""
		Send a the given command string.
		For internal use. To send commands the many specific
		methods for this purpose should be used.
		"""
		self._buffer.append(command)
		self._prod()


