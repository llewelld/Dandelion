/*
 * DlClient.java
 *
 * Created on 19 July 2008
 */

package uk.co.flypig.dandelion;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.Socket;
import java.net.UnknownHostException;

/**
 * Dandelion client code, for sending messages to Dandelion
 * @author flypig
 */
public class DlClient implements Runnable {
  private String buffer;
  private Thread network;
  private String address;
  private int port;
  private Socket dandeSocket = null;
  private BufferedReader in = null;
  private boolean active;
  
  /**
   * Constructor
   * @param address the host address of the Dandelion server
   * @param port the port being used by the Dadelion server
   */
  public DlClient (String address, int port) {
    super ();
    this.address = address;
    this.port = port;
    buffer = "";
    active = false;

    // Start the network thread
    network = new Thread (this, "DandeClient");
    // We're off
    network.start ();
  }

  /**
   * Destructor
   * @throws java.lang.Throwable the Exception raised by this method
   */
  protected void finalize() throws Throwable
  {
    active = false;
    super.finalize ();
  }   

  /**
   * Add a node with the given name
   * @param node the name of the node to add
   */
  public synchronized void addNode (String node) {
    buffer += "ADDNODE \"" + node + "\"\n";
    notify ();
  }

  /**
   * Remove the node with the given name
   * @param node the name of the node to remove
   */
  public synchronized void subNode (String node) {
    buffer += "SUBNODE \"" + node + "\"\n";
    notify ();
  }
  
  /**
   * Move the node with the given name in the given direction
   * @param node the name of the node to remove
   * @param x the distance to move the node along the x-axis
   * @param y the distance to move the node along the y-axis
   * @param z the distance to move the node along the z-axis
   */
  public synchronized void moveNode (String node, double x, double y, double z) {
    buffer += "MOVENODE \"" + node + "\" " + x + " " + y + " " + z + '\n';
    notify ();
  }
  
  /**
   * Set the colour of the given node
   * @param node the name of the node to colour
   * @param red the red colomponent of the colour to set
   * @param green the green colomponent of the colour to set
   * @param blue the blue colomponent of the colour to set
   */
  public synchronized void setNodeColour (String node, double red, double green, double blue) {
    buffer += "SETNODECOLOUR \"" + node + "\" " + red + " " + green + " " + blue + '\n';
    notify ();
  }

  /**
   * Add a link from one node to another
   * @param nodeFrom the name of the node to link from
   * @param nodeTo the name of the node to link to
   */
  public synchronized void addLink (String nodeFrom, String nodeTo) {
    buffer += "ADDLINK \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
    notify ();
  }
  
  /**
   * Remove a link between one node and another
   * @param nodeFrom the name of the node the link to be removed starts from
   * @param nodeTo the name of the node the link to be removed ends at
   */
  public synchronized void subLink (String nodeFrom, String nodeTo) {
    buffer += "SUBLINK \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
    notify ();
  }
  
  /**
   * Add a bidirectional link between the two nodes
   * @param node1 a node to add the bidirectional link between
   * @param node2 a node to add the bidirectional link between
   */
  public synchronized void addLinkBi (String node1, String node2) {
    buffer += "ADDLINKBI \"" + node1 + "\" \"" + node2 + "\"\n";
    notify ();
  }
  
  /**
   * Remove a bidirectional link between the two nodes
   * @param node1 a node to remove the bidirectional link from
   * @param node2 a node to remove the bidirectional link from
   */
  public synchronized void subLinkBi (String node1, String node2) {
    buffer += "SUBLINKBI \"" + node1 + "\" \"" + node2 + "\"\n";
    notify ();
  }

  /**
   * Add a named link from one node to another
   * @param name of the link to add
   * @param nodeFrom the name of the node to link from
   * @param nodeTo the name of the node to link to
   */
  public synchronized void addLinkNamed (String name, String nodeFrom, String nodeTo) {
    buffer += "ADDLINK \"" + name + "\" \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
    notify ();
  }

  /**
   * Remove a link
   * @param name link to remove
   */
  public synchronized void subLinkNamed (String name) {
    buffer += "SUBLINK \"" + name + "\"\n";
    notify ();
  }

  /**
   * Add a bidirectional named link from one node to another
   * @param name of the link to add
   * @param nodeFrom the name of the node to link from
   * @param nodeTo the name of the node to link to
   */
  public synchronized void addLinkBiNamed (String name, String nodeFrom, String nodeTo) {
    buffer += "ADDLINKBI \"" + name + "\" \"" + nodeFrom + "\" \"" + nodeTo + "\"\n";
    notify ();
  }

  /**
   * Set the colour of the given link
   * @param link the name of the link to colour
   * @param red the red colomponent of the colour to set
   * @param green the green colomponent of the colour to set
   * @param blue the blue colomponent of the colour to set
   */
  public synchronized void setLinkColour (String link, double red, double green, double blue) {
    buffer += "SETLINKCOLOUR \"" + link + "\" " + red + " " + green + " " + blue + '\n';
    notify ();
  }

  /**
   * Add an integer property to a named node
   * @param name of the node to apply the property to
   * @param property name of the property to add
   * @param value integer value to add
   */
  public synchronized void addNodePropInt (String name, String property, int value) {
    buffer += "ADDNODEPROP \"" + name + "\" \"" + property + "\" int \"" + value + "\"\n";
    notify ();
  }

  /**
   * Add a float property to a named node
   * @param name of the node to apply the property to
   * @param property name of the property to add
   * @param value float value to add
   */
  public synchronized void addNodePropFloat (String name, String property, double value) {
    buffer += "ADDNODEPROP \"" + name + "\" \"" + property + "\" float \"" + value + "\"\n";
    notify ();
  }

  /**
   * Add a string property to a named node
   * @param name of the node to apply the property to
   * @param property name of the property to add
   * @param value string value to add
   */
  public synchronized void addNodePropString (String name, String property, String value) {
    buffer += "ADDNODEPROP \"" + name + "\" \"" + property + "\" string \"" + value + "\"\n";
    notify ();
  }

  /**
   * Remove a string property from a named node
   * @param name of the node to remove the property from
   * @param property name of the property to remove
   */
  public synchronized void subNodeProp (String name, String property) {
    buffer += "SUBNODEPROP \"" + name + "\" \"" + property + "\"\n";
    notify ();
  }

  /**
   * Add an integer property to a named link
   * @param name of the link to apply the property to
   * @param property name of the property to add
   * @param value integer value to add
   */
  public synchronized void addLinkPropInt (String name, String property, int value) {
    buffer += "ADDLINKPROP \"" + name + "\" \"" + property + "\" int \"" + value + "\"\n";
    notify ();
  }

  /**
   * Add a float property to a named link
   * @param name of the link to apply the property to
   * @param property name of the property to add
   * @param value float value to add
   */
  public synchronized void addLinkPropFloat (String name, String property, double value) {
    buffer += "ADDLINKPROP \"" + name + "\" \"" + property + "\" float \"" + value + "\"\n";
    notify ();
  }

  /**
   * Add a string property to a named link
   * @param name of the link to apply the property to
   * @param property name of the property to add
   * @param value string value to add
   */
  public synchronized void addLinkPropString (String name, String property, String value) {
    buffer += "ADDLINKPROP \"" + name + "\" \"" + property + "\" string \"" + value + "\"\n";
    notify ();
  }

  /**
   * Remove a string property from a named link
   * @param name of the link to remove the property from
   * @param property name of the property to remove
   */
  public synchronized void subLinkProp (String name, String property) {
    buffer += "SUBLINKPROP \"" + name + "\" \"" + property + "\"\n";
    notify ();
  }

  /**
   * Clear all nodes and links
   */
  public synchronized void clear () {
    buffer += "CLEAR\n";
    notify ();
  }
  
  /**
   * Finish off
   */
  public synchronized void quit () {
    buffer += "QUIT\n";
    notify ();
    finish ();
  }

  /**
   * Shut down the running thread, if it is
   */
  public void finish () {
    active = false;
  }

  /**
   * The network thread that actually does the work
   */
  public void run () {
    active = true;

    while (active) {
      // Wait for something to add data to the buffer and notify us
      waitForData ();
      
      // Open the connection if it's not already
      if (dandeSocket == null) {
        try {
          dandeSocket = new Socket (address, port);
          //System.out.println ("Created socket");
          in = new BufferedReader (new InputStreamReader (dandeSocket.getInputStream()));
        }
        catch (UnknownHostException e) {
          System.err.println ("Unknown host " + address);
          dandeSocket = null;
        }
        catch (IOException e) {
          System.err.println ("Failed to get I/O for connection to " + address);
          dandeSocket = null;
          System.exit (1);
        }
      }

      // Send any data in the buffer a line at a time
      while ((buffer.length () > 0) && (dandeSocket != null)) {
        try {
            byte[] nextCommand = removeLineFromBuffer ();
                                                dandeSocket.getOutputStream ().write (nextCommand);
            String result = in.readLine ();
                                                if (result.equalsIgnoreCase ("QUIT")) {
                                                    active = false;
                                                }
                                                else if (!result.equalsIgnoreCase ("OKAY")) {
              System.err.println ("Command failure: " + nextCommand);              
            }
            //System.out.println ("Returned: " + result);
        }
        catch (IOException e) {
          System.err.println ("Failed to get I/O for connection to " + address);
          System.exit (1);
        }
      }
    }

    /**
     * Close the socket
     */
    try {
      in.close ();
      dandeSocket.close ();
    } catch (IOException e) {
      e.printStackTrace ();
    }
    dandeSocket = null;
  }

  /**
   * Wait until we get a notify
   */
  private synchronized void waitForData () {
    try {
      if (buffer.length () <= 0) {
        wait ();
      }
    }
    catch (InterruptedException e) {
      e.printStackTrace ();
    }
  }

  /**
   * Remove the first line from the buffer
   * @throws java.io.UnsupportedEncodingException if UTF-8 is not supported
   * @return byte buffer of first command in String buffer
   */
  private synchronized byte[] removeLineFromBuffer () throws UnsupportedEncodingException {
    int lineEnd = buffer.indexOf ('\n') + 1;
    String firstLine = buffer.substring (0, lineEnd);
    buffer = buffer.substring (lineEnd);

    return firstLine.getBytes ("UTF-8");
  }
}
