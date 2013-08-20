#!/usr/bin/env python

"""
A simple user interface to demonstrate the Dandelion client code.
This can be used to control a Dandelion 3D network visualisation
server

http://www.flypig.co.uk/?to=dandelion

David Llewellyn-Jones
Liverpool John Moores University
Summer 2013

Released under an LGPL licence.

In order to run, this requires wxPython to be installed: http://wxpython.org/
On Ubunutu, this can be installed from the repository by typing something like:
sudo apt-get install python-wxgtk2.8
"""

import wx
import dlclient
import random

class DandeClientFrame(wx.Frame):
	"""
	Main DandeClient window, derived from wxPython frame.
	"""

	def __init__(self, parent, id, title):
		"""
		Class constructor. Sets up all of the window widgets.
		"""

		# Set up code for quick node/link addition
		self.quickCount = 0

		# Call the wx.Frame initialiser to create the frame
		wx.Frame.__init__(self, parent, id, title)

		# Panel to add widgets to
		panel = wx.Panel(self, -1)
		self.panel = panel

		# Create the widgets for the controls
		labelServer = wx.StaticText(panel, -1, "Server")
		self.serverName = wx.TextCtrl(panel, -1, "localhost")
		self.serverPort = wx.SpinCtrl(panel, -1)
		self.serverPort.SetRange(0, (2**16)-1)
		self.serverPort.SetValue(4972)
		self.serverConnect = wx.Button(panel, -1, "Connect")
		self.serverDisconnect = wx.Button(panel, -1, "Disconnect")

		labelNode = wx.StaticText(panel, -1, "Node")
		self.nodeName = wx.TextCtrl(panel, -1, "")
		self.nodeAdd = wx.Button(panel, -1, "Add")
		self.nodeRemove = wx.Button(panel, -1, "Remove")

		labelLink = wx.StaticText(panel, -1, "Link")
		self.linkName = wx.TextCtrl(panel, -1, "")
		self.linkFrom = wx.TextCtrl(panel, -1, "")
		self.linkTo = wx.TextCtrl(panel, -1, "")
		self.linkBi = wx.CheckBox(panel, -1, "Bidirected")
		self.linkAdd = wx.Button(panel, -1, "Add")
		self.linkRemove = wx.Button(panel, -1, "Remove")

		labelProperty = wx.StaticText(panel, -1, "Property")
		self.propertyName = wx.TextCtrl(panel, -1, "")
		self.propertyType = wx.Choice(panel, -1, choices=("int", "float", "string"))
		self.propertyType.SetSelection(2)
		self.propertyValue = wx.TextCtrl(panel, -1, "")
		self.propertyNodeLink = wx.Choice(panel, -1, choices=("Node", "Link"))
		self.propertyAdd = wx.Button(panel, -1, "Add")
		self.propertyRemove = wx.Button(panel, -1, "Remove")

		self.clear = wx.Button(panel, -1, "Clear")
		self.quickNode = wx.Button(panel, -1, "Quick Node")
		self.quickLink = wx.Button(panel, -1, "Quick Link")
		self.quit = wx.Button(panel, -1, "Quit")

		# Attach event listeners for when buttons are pressed
		self.serverConnect.Bind(wx.EVT_BUTTON, self.OnButtonConnect)
		self.serverDisconnect.Bind(wx.EVT_BUTTON, self.OnButtonDisconnect)
		self.nodeAdd.Bind(wx.EVT_BUTTON, self.OnButtonNodeAdd)
		self.nodeRemove.Bind(wx.EVT_BUTTON, self.OnButtonNodeRemove)
		self.linkAdd.Bind(wx.EVT_BUTTON, self.OnButtonLinkAdd)
		self.linkRemove.Bind(wx.EVT_BUTTON, self.OnButtonLinkRemove)
		self.propertyAdd.Bind(wx.EVT_BUTTON, self.OnButtonPropertyAdd)
		self.propertyRemove.Bind(wx.EVT_BUTTON, self.OnButtonPropertyRemove)
		self.clear.Bind(wx.EVT_BUTTON, self.OnButtonClear)
		self.quickNode.Bind(wx.EVT_BUTTON, self.OnButtonQuickNode)
		self.quickLink.Bind(wx.EVT_BUTTON, self.OnButtonQuickLink)
		self.quit.Bind(wx.EVT_BUTTON, self.OnButtonQuit)
		self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroy)

		# Use a gridbag sizer to arrange the widgets
		sizer = wx.GridBagSizer(5, 5)

		# Add the widgets in the appropriate places
		# Row 1
		sizer.Add(labelServer, (0, 0))
		sizer.Add(self.serverName, (0, 1), (1, 2), (wx.EXPAND | wx.ALL))
		sizer.Add(self.serverPort, (0, 3), (1, 2), (wx.EXPAND | wx.ALL))
		sizer.Add(self.serverConnect, (0, 5))
		sizer.Add(self.serverDisconnect, (0, 6))

		# Row 2
		sizer.Add(labelNode, (1, 0))
		sizer.Add(self.nodeName, (1, 1), (1, 4), (wx.EXPAND | wx.ALL))
		sizer.Add(self.nodeAdd, (1, 5))
		sizer.Add(self.nodeRemove, (1, 6))

		# Row 3
		sizer.Add(labelLink, (2, 0))
		sizer.Add(self.linkName, (2, 1))
		sizer.Add(self.linkFrom, (2, 2))
		sizer.Add(self.linkTo, (2, 3))
		sizer.Add(self.linkBi, (2, 4))
		sizer.Add(self.linkAdd, (2, 5))
		sizer.Add(self.linkRemove, (2, 6))

		# Row 4
		sizer.Add(labelProperty, (3, 0))
		sizer.Add(self.propertyName, (3, 1))
		sizer.Add(self.propertyType, (3, 2))
		sizer.Add(self.propertyValue, (3, 3))
		sizer.Add(self.propertyNodeLink, (3, 4))
		sizer.Add(self.propertyAdd, (3, 5))
		sizer.Add(self.propertyRemove, (3, 6))

		# Row 5
		sizer.Add(self.clear, (4, 1))
		sizer.Add(self.quickNode, (4, 2))
		sizer.Add(self.quickLink, (4, 3))
		sizer.Add(self.quit, (4, 6))

		# Add a border
		border = wx.BoxSizer()
		border.Add(sizer, 0, wx.ALL, 15)
		panel.SetSizerAndFit(border)
		self.Fit()
		
		# Create the dlclient object
		self.dlclient = dlclient.DlClient()

	def OnButtonConnect(self, event):
		host = self.serverName.GetValue()
		port = self.serverPort.GetValue()
		self.dlclient.setHost(host)
		self.dlclient.setPort(port)
		event.Skip()

	def OnButtonDisconnect(self, event):
		self.dlclient.quit()
		event.Skip()

	def OnButtonNodeAdd(self, event):
		name = self.nodeName.GetValue()
		self.dlclient.addNode(name)
		event.Skip()

	def OnButtonNodeRemove(self, event):
		name = self.nodeName.GetValue()
		self.dlclient.subNode(name)
		event.Skip()

	def OnButtonLinkAdd(self, event):
		name = self.linkName.GetValue()
		nodeFrom = self.linkFrom.GetValue()
		nodeTo = self.linkTo.GetValue()
		bidirected = self.linkBi.GetSelection()
		if bidirected:
			self.dlclient.addLinkBi(name, nodeFrom, nodeTo)
		else:
			self.dlclient.addLink(name, nodeFrom, nodeTo)
		event.Skip()

	def OnButtonLinkRemove(self, event):
		name = self.linkName.GetValue()
		nodeFrom = self.linkFrom.GetValue()
		nodeTo = self.linkTo.GetValue()
		bidirected = self.linkBi.GetSelection()
		if bidirected:
			self.dlclient.subLinkBi(name, nodeFrom, nodeTo)
		else:
			self.dlclient.subLink(name, nodeFrom, nodeTo)

		event.Skip()

	def OnButtonPropertyAdd(self, event):
		nodeLink = self.propertyNodeLink.GetSelection()
		propertyName = self.propertyName.GetValue()
		propertyType = self.propertyType.GetSelection()
		propertyValue = self.propertyValue.GetValue()
		if nodeLink == 0:
			# Attach property to node
			node = self.nodeName.GetValue()
			if propertyType == 0:
				# Property of type int
				self.dlclient.addNodePropInt(node, propertyName, int(propertyValue))
			elif propertyType == 1:
				# Property of type float
				self.dlclient.addNodePropFloat(node, propertyName, float(propertyValue))
			elif propertyType == 2:
				# Property of type string
				self.dlclient.addNodePropString(node, propertyName, propertyValue)
		elif nodeLink == 1:
			# Attach property to link
			link = self.linkName.GetValue()
			if propertyType == 0:
				# Property of type int
				self.dlclient.addLiniPropInt(link, propertyName, int(propertyValue))
			elif propertyType == 1:
				# Property of type float
				self.dlclient.addLinkPropFloat(link, propertyName, float(propertyValue))
			elif propertyType == 2:
				# Property of type string
				self.dlclient.addLinkPropString(link, propertyName, propertyValue)
		event.Skip()

	def OnButtonPropertyRemove(self, event):
		nodeLink = self.propertyNodeLink.GetSelection()
		propertyName = self.propertyName.GetValue()
		if nodeLink == 0:
			# Remove property from node
			node = self.nodeName.GetValue()
			self.dlclient.subNodeProp(node, propertyName)
		elif nodeLink == 1:
			# Remove property from link
			link = self.linkName.GetValue()
			self.dlclient.subLinkProp(link, propertyName)
		event.Skip()

	def OnButtonClear(self, event):
		self.dlclient.clear()
		self.quickCount = 0
		event.Skip()

	def OnButtonQuickNode(self, event):
		bidirected = self.linkBi.GetValue()
		self.dlclient.addNode(str(self.quickCount))

		if self.quickCount > 0:
			linkNode = str(random.randint(0, (self.quickCount - 1)))
			if bidirected:
				self.dlclient.addLink(str(self.quickCount), linkNode)
			else:
				self.dlclient.addLinkBi(str(self.quickCount), linkNode)
		self.quickCount += 1
		event.Skip()

	def OnButtonQuickLink(self, event):
		bidirected = self.linkBi.GetValue()

		if self.quickCount > 1:
			linkFrom = str(random.randint(0, (self.quickCount - 1)))
			linkTo = str(random.randint(0, (self.quickCount - 1)))
			if bidirected:
				self.dlclient.addLink(linkFrom, linkTo)
			else:
				self.dlclient.addLinkBi(linkFrom, linkTo)
		event.Skip()

	def OnButtonQuit(self, event):
		wx.CallAfter(self.Destroy)
		event.Skip()

	def OnDestroy(self, event):
		self.dlclient.quit()
		event.Skip()

class DandeClient(wx.App):
	"""
	Application class for actually running the wxWidget application.
	"""
	def OnInit(self):
		"""
		Called by wxPython to start up the application.
		"""

		# Create and show the window
		frame = DandeClientFrame(None, -1, "Dandelion Test Client")
		frame.Show(True)

		# This is the main window
		self.SetTopWindow(frame)

		return True

"""
Run the application.
"""

application = DandeClient(0)
application.MainLoop()

