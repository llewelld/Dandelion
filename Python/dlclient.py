#!/usr/bin/env python

"""
A simple Dandelion client.
"""

import socket
import threading

class DlClient(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
		self.host = 'localhost'
		self.port = 4972
		self.size = 1024
		self.active = False
		self.buffer = []
		self.sock = None
		self.cond = threading.Condition()
		self.paused = False
		self.start()

	def __del__(self):
		self.finish()

	def closeConnection(self):
		if self.sock != None:
			self.sock.close()
			self.sock = None

	def setHost(self, host):
		closeConnection()
		self.host = host
		
	def setPort(self, port):
		closeConnection()
		self.port = port

	def waitForData(self):
		with self.cond:
			if len(self.buffer) <= 0:
				self.paused = True
				self.cond.wait()
				self.paused = False

	def run(self):
		self.active = True;
		while self.active:
			self.waitForData()

			if self.sock == None:
				try:
					self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
					self.sock.connect((self.host,self.port))
				except socket.error, (value, message):
					if self.sock:
						self.sock.close()
						self.sock = None
						print "Failed to connect: " + message
						self.buffer = []

			while len(self.buffer) > 0 and self.sock != None:
				nextCommand = self.getNextCommand()
				if nextCommand != "":
					self.sock.send(nextCommand)
					#print "Sending command: " + nextCommand
					result = self.sock.recv(self.size)
					#print "Received: " + result
					if result == "QUIT\n":
						self.active = False
					elif result != "OKAY\n":
						print "Command failure: " + nextCommand
		
		self.closeConnection()

	def getNextCommand(self):
		command = ""
		if len(self.buffer) > 0:
			command = self.buffer.pop(0)
		return command

	def prod(self):
		if self.paused:
			with self.cond:
				self.cond.notify()

	def sendCommand(self, command):
		self.buffer.append(command)
		self.prod()

	def addNode(self, node):
		"""
		Add a node with the given name.
		
		Arguments:
		node -- the name of the node to add
		"""
		command = "ADDNODE \"" + node + "\"\n";
		self.sendCommand(command);

	def subNode(self, node):
		"""
		Remove the node with the given name.
	
		Arguments:
		node -- the name of the node to remove
		"""
		command = "SUBNODE \"" + node + "\"\n";
		self.sendCommand(command);
		
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
		self.sendCommand(command);
		
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
		self.sendCommand(command);

	def addLink(self, nodeFrom, nodeTo):
		"""
		Add a link from one node to another.
	
		Arguments:
		nodeFrom -- the name of the node to link from
		nodeTo -- the name of the node to link to
		"""
		command = "ADDLINK \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
		self.sendCommand(command);
		
	def subLink(self, nodeFrom, nodeTo):
		"""
		Remove a link between one node and another.
	
		Arguments:
		nodeFrom -- the name of the node the link to be removed starts from
		nodeTo -- the name of the node the link to be removed ends at
		"""
		command = "SUBLINK \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
		self.sendCommand(command);
		
	def addLinkBi(self, node1, node2):
		"""
		Add a bidirectional link between the two nodes.
	
		Arguments:
		node1 -- a node to add the bidirectional link between
		node2 -- a node to add the bidirectional link between
		"""
		command = "ADDLINKBI \"" + node1 + "\" \"" + node2 + "\"\n";
		self.sendCommand(command);
		
	def subLinkBi(self, node1, node2):
		"""
		Remove a bidirectional link between the two nodes.
	
		Arguments:
		node1 -- a node to remove the bidirectional link from
		node2 -- a node to remove the bidirectional link from
		"""
		command = "SUBLINKBI \"" + node1 + "\" \"" + node2 + "\"\n";
		self.sendCommand(command);

	def addLinkNamed(self, name, nodeFrom, nodeTo):
		"""
		Add a named link from one node to another.
	
		Arguments:
		name -- the name of the link to add
		nodeFrom -- the name of the node to link from
		nodeTo -- the name of the node to link to
		"""
		command = "ADDLINK \"" + name + "\" \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
		self.sendCommand(command);

	def subLinkNamed(self, name):
		"""
		Remove a link.
	
		Arguments:
		name -- the link to remove
		"""
		command = "SUBLINK \"" + name + "\"\n";
		self.sendCommand(command);

	def addLinkBiNamed(self, name, nodeFrom, nodeTo):
		"""
		Add a bidirectional named link from one node to another.
	
		Arguments:
		name -- the name of the link to add
		nodeFrom -- the name of the node to link from
		nodeTo -- the name of the node to link to
		"""
		command = "ADDLINKBI \"" + name + "\" \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
		self.sendCommand(command);

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
		self.sendCommand(command);

	def addNodePropInt(self, name, property, value):
		"""
		Add an integer property to a named node.
	
		Arguments:
		name -- the name of the node to apply the property to
		property -- name of the property to add
		value -- integer value to add
		"""
		command = "ADDNODEPROP \"" + name + "\" \"" + property + "\" int \"" + str(value) + "\"\n";
		self.sendCommand(command);

	def addNodePropFloat(self, name, property, value):
		"""
		Add a float property to a named node.
	
		Arguments:
		name -- the name of the node to apply the property to
		property -- name of the property to add
		value -- the float value to add
		"""
		command = "ADDNODEPROP \"" + name + "\" \"" + property + "\" float \"" + str(value) + "\"\n";
		self.sendCommand(command);

	def addNodePropString(self, name, property, value):
		"""
		Add a string property to a named node.
	
		Arguments:
		name -- the name of the node to apply the property to
		property -- the name of the property to add
		value -- the string value to add
		"""
		command = "ADDNODEPROP \"" + name + "\" \"" + property + "\" string \"" + value + "\"\n";
		self.sendCommand(command);

	def subNodeProp(self, name, property):
		"""
		Remove a string property from a named node.
	
		Arguments:
		name -- the name of the node to remove the property from
		property -- the name of the property to remove
		"""
		command = "SUBNODEPROP \"" + name + "\" \"" + property + "\"\n";
		self.sendCommand(command);

	def addLinkPropInt(self, name, property, value):
		"""
		Add an integer property to a named link.
	
		Arguments:
		name -- the name of the link to apply the property to
		property -- the name of the property to add
		value -- the integer value to add
		"""
		command = "ADDLINKPROP \"" + name + "\" \"" + property + "\" int \"" + str(value) + "\"\n";
		self.sendCommand(command);

	def addLinkPropFloat(self, name, property, value):
		"""
		Add a float property to a named link.
	
		Arguments:
		name -- the name of the link to apply the property to
		property -- the name of the property to add
		value -- the float value to add
		"""
		command = "ADDLINKPROP \"" + name + "\" \"" + property + "\" float \"" + str(value) + "\"\n";
		self.sendCommand(command);

	def addLinkPropString(self, name, property, value):
		"""
		Add a string property to a named link.

		Arguments:
		name -- the name of the link to apply the property to
		property -- the name of the property to add
		value -- the string value to add
		"""
		command = "ADDLINKPROP \"" + name + "\" \"" + property + "\" string \"" + value + "\"\n";
		self.sendCommand(command);

	def subLinkProp(self, name, property):
		"""
		Remove a string property from a named link.
	
		Arguments:
		name -- the name of the link to remove the property from
		property -- the name of the property to remove
		"""
		command = "SUBLINKPROP \"" + name + "\" \"" + property + "\"\n";
		self.sendCommand(command);

	def clear(self):
		"""
		Clear all nodes and links.
		"""
		command = "CLEAR\n";
		self.sendCommand(command);
		
	def quit(self):
		"""
		Finish off.
		"""
		command = "QUIT\n";
		self.sendCommand(command);
		self.finish();

	def finish(self):
		self.active = False
		self.prod()


