using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace DandeClient
{
    public class DlClient
    {
        private StreamWriter swSender;
        private StreamReader srReceiver;
        private TcpClient tcpServer;
        private bool connected;

        public TcpClient TcpServer
        {
            get { return tcpServer; }
            set { tcpServer = value; }
        }

        public bool Connected
        {
            get { return connected; }
            set { connected = value; }
        }

        public StreamWriter SwSender
        {
            get { return swSender; }
            set { swSender = value; }
        }

        public StreamReader SrReceiver
        {
            get { return srReceiver; }
            set { srReceiver = value; }
        }

        /// <summary>
        /// Connect to Dandelion
        /// </summary>
        /// <param name="ip">The host ip of the Dandelion server</param>
        /// <param name="port">The port being used by the Dadelion server</param>
        public void Connect(String ip, String port)
        {
            try
            {
                TcpServer = new TcpClient();
                TcpServer.Connect(IPAddress.Parse(ip), int.Parse(port));
                Connected = true;
                SwSender = new StreamWriter(tcpServer.GetStream());
                SrReceiver = new StreamReader(tcpServer.GetStream());
            }
            catch (Exception exception)
            {
                throw new Exception("An exception has been produced. Check that the values of the connection are correct and that the server is online.", exception);
            }
        }

        /// <summary>
        /// Disconnect from Dandelion
        /// </summary>
        public void Disconnect()
        {
            Connected = false;
            SwSender.Close();
            SrReceiver.Close();
            TcpServer.Close();
        }

        /// <summary>
        /// Method that send a command to the server.
        /// Wait for the response, if the response is OKAY, then returns true, otherwise return false.
        /// </summary>
        /// <param name="message">Command to send</param>
        /// <returns>Result of sending the command</returns>
        private bool SendMessage(String message)
        {
            if (!Connected) return false;
            SwSender.WriteLine(message + "\n");
            SwSender.Flush();
            if (SrReceiver.ReadLine().Equals("OKAY"))
                return true;
            return false;
        }

        /// <summary>
        /// Add a node with the given name
        /// </summary>
        /// <param name="node">The name of the node to add</param>
        /// <returns>Return true if the node was added</returns>
        public bool AddNode(String node)
        {
            return SendMessage("ADDNODE " + node);
        }

        /// <summary>
        /// Remove the node with the given name
        /// </summary>
        /// <param name="node">The name of the node to remove</param>
        /// <returns>Return true if the node was removed</returns>
        public bool SubNode(String node)
        {
            return SendMessage("SUBNODE " + node);
        }

        /// <summary>
        /// Move the node with the given name in the given direction
        /// </summary>
        /// <param name="node">The name of the node to remove</param>
        /// <param name="x">The distance to move the node along the x-axis</param>
        /// <param name="y">The distance to move the node along the y-axis</param>
        /// <param name="z">The distance to move the node along the z-axis</param>
        /// <returns>Return true if the node was moved</returns>
        public bool MoveNode(String node, double x, double y, double z)
        {
            return SendMessage("MOVENODE " + node + " " + x + " " + y + " " + z);
        }

        /// <summary>
        /// Set the colour of the given node
        /// </summary>
        /// <param name="node">The name of the node to set colour of</param>
        /// <param name="red">The level of red between 0.0 and 1.0</param>
        /// <param name="green">The level of green between 0.0 and 1.0</param>
        /// <param name="blue">The level of blue between 0.0 and 1.0</param>
        /// <returns>Return true if the node colour was set</returns>
        public bool SetNodeColour(String node, double red, double green, double blue)
        {
            return SendMessage("SETNODECOLOUR " + node + " " + red + " " + green + " " + blue);
        }

        /// <summary>
        /// Add a link from one node to another
        /// </summary>
        /// <param name="nodeFrom">The name of the node to link from</param>
        /// <param name="nodeTo">The name of the node to link to</param>
        /// <returns>Return true if the link was added</returns>
        public bool AddLink(String nodeFrom, String nodeTo)
        {
            return SendMessage("ADDLINK " + nodeFrom + " " + nodeTo);
        }

        /// <summary>
        /// Remove a link between one node and another
        /// </summary>
        /// <param name="nodeFrom">The name of the node the link to be removed starts from</param>
        /// <param name="nodeTo">The name of the node the link to be removed ends at</param>
        /// <returns>Return true if the link was removed</returns>
        public bool SubLink(String nodeFrom, String nodeTo)
        {
            return SendMessage("SUBLINK " + nodeFrom + " " + nodeTo);
        }

        /// <summary>
        /// Add a bidirectional link between the two nodes
        /// </summary>
        /// <param name="node1">Node to add the bidirectional link between</param>
        /// <param name="node2">Node to add the bidirectional link between</param>
        /// <returns>Return true if the link was added</returns>
        public bool AddLinkBi(String node1, String node2)
        {
            return SendMessage("ADDLINKBI " + node1 + " " + node2);
        }

        /// <summary>
        /// Remove a bidirectional link between the two nodes
        /// </summary>
        /// <param name="node1">Node to remove the bidirectional link from</param>
        /// <param name="node2">Node to remove the bidirectional link from</param>
        /// <returns>Return true if the bidirectional link was deleted</returns>
        public bool SubLinkBi(String node1, String node2)
        {
            return SendMessage("SUBLINKBI " + node1 + " " + node2);
        }

        /// <summary>
        /// Add a named link from one node to another
        /// </summary>
        /// <param name="name">The name of link to add</param>
        /// <param name="nodeFrom">The name of the node to link from</param>
        /// <param name="nodeTo">The name of the node to link to</param>
        /// <returns>Return true if the link was added</returns>
        public bool AddLinkNamed(String name, String nodeFrom, String nodeTo)
        {
            return SendMessage("ADDLINK " + name + " " + nodeFrom + " " + nodeTo);
        }

        /// <summary>
        /// Remove a named link
        /// </summary>
        /// <param name="name">The name of the link to be removed</param>
        /// <returns>Return true if the link was removed</returns>
        public bool SubLinkNamed(String name)
        {
            return SendMessage("SUBLINK " + name);
        }

        /// <summary>
        /// Add a named bidirectional link from one node to another
        /// </summary>
        /// <param name="name">The name of link to add</param>
        /// <param name="node1">Node to add the named bidirectional link between</param>
        /// <param name="node2">Node to add the named bidirectional link between</param>
        /// <returns>Return true if the link was added</returns>
        public bool AddLinkBiNamed(String name, String node1, String node2)
        {
            return SendMessage("ADDLINKBI " + name + " " + node1 + " " + node2);
        }

        /// <summary>
        /// Remove a named bidirectional link
        /// </summary>
        /// <param name="name">The name of the bidirectional link to be removed</param>
        /// <returns>Return true if the link was removed</returns>
        public bool SubLinkBiNamed(String name)
        {
            return SendMessage("SUBLINKBI " + name);
        }

        /// <summary>
        /// Set the colour of the given link
        /// </summary>
        /// <param name="link">The name of the link to set colour of</param>
        /// <param name="red">The level of red between 0.0 and 1.0</param>
        /// <param name="green">The level of green between 0.0 and 1.0</param>
        /// <param name="blue">The level of blue between 0.0 and 1.0</param>
        /// <returns>Return true if the node colour was set</returns>
        public bool SetLinkColour(String link, double red, double green, double blue)
        {
            return SendMessage("SETLINKCOLOUR " + link + " " + red + " " + green + " " + blue);
        }

        /// <summary>
        /// Add an integer property to a node
        /// </summary>
        /// <param name="node">The name of the node to set the property for</param>
        /// <param name="property">The name of the property to set</param>
        /// <param name="value">The integer value of the property to set</param>
        /// <returns>Return true if the node property was set</returns>
        public bool SetNodePropertyInt(String node, String property, int value)
        {
            return SendMessage("ADDNODEPROP " + node + " " + property + " int " + value);
        }

        /// <summary>
        /// Add a float property to a node
        /// </summary>
        /// <param name="node">The name of the node to set the property for</param>
        /// <param name="property">The name of the property to set</param>
        /// <param name="value">The float value of the property to set</param>
        /// <returns>Return true if the node property was set</returns>
        public bool SetNodePropertyFloat(String node, String property, float value)
        {
            return SendMessage("ADDNODEPROP " + node + " " + property + " float " + value);
        }

        /// <summary>
        /// Add a string property to a node
        /// </summary>
        /// <param name="node">The name of the node to set the property for</param>
        /// <param name="property">The name of the property to set</param>
        /// <param name="value">The string value of the property to set</param>
        /// <returns>Return true if the node property was set</returns>
        public bool SetNodePropertyString(String node, String property, String value)
        {
            return SendMessage("ADDNODEPROP " + node + " " + property + " string " + value);
        }

        /// <summary>
        /// Remove a property from a node
        /// </summary>
        /// <param name="node">The name of the node to remove the property from</param>
        /// <param name="property">The name of the property to remove</param>
        /// <returns>Return true if the node property was removed</returns>
        public bool SubNodeProperty(String node, String property)
        {
            return SendMessage("SUBNODEPROP " + node + " " + property);
        }

        /// <summary>
        /// Add an integer property to a link
        /// </summary>
        /// <param name="link">The name of the link to set the property for</param>
        /// <param name="property">The name of the property to set</param>
        /// <param name="value">The integer value of the property to set</param>
        /// <returns>Return true if the link property was set</returns>
        public bool SetLinkPropertyInt(String link, String property, int value)
        {
            return SendMessage("ADDLINKPROP " + link + " " + property + " int " + value);
        }

        /// <summary>
        /// Add a float property to a link
        /// </summary>
        /// <param name="link">The name of the link to set the property for</param>
        /// <param name="property">The name of the property to set</param>
        /// <param name="value">The float value of the property to set</param>
        /// <returns>Return true if the link property was set</returns>
        public bool SetLinkPropertyFloat(String link, String property, float value)
        {
            return SendMessage("ADDLINKPROP " + link + " " + property + " float " + value);
        }

        /// <summary>
        /// Add a string property to a link
        /// </summary>
        /// <param name="link">The name of the link to set the property for</param>
        /// <param name="property">The name of the property to set</param>
        /// <param name="value">The string value of the property to set</param>
        /// <returns>Return true if the link property was set</returns>
        public bool SetLinkPropertyString(String link, String property, String value)
        {
            return SendMessage("ADDLINKPROP " + link + " " + property + " string " + value);
        }

        /// <summary>
        /// Remove a property from a link
        /// </summary>
        /// <param name="link">The name of the link to remove the property from</param>
        /// <param name="property">The name of the property to remove</param>
        /// <returns>Return true if the link property was removed</returns>
        public bool SubLinkProperty(String link, String property)
        {
            return SendMessage("SUBLINKPROP " + link + " " + property);
        }

        /// <summary>
        /// Clear all nodes and links
        /// </summary>
        /// <returns>Return true if all elements were delted</returns>
        public bool Clear()
        {
            return SendMessage("CLEAR");
        }

        /// <summary>
        /// Finish off
        /// </summary>
        /// <returns>Return true if the finish off was made</returns>
        public bool Quit()
        {
            return SendMessage("QUIT");
        }
    }
}
