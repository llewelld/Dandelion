using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace DandeClient
{
    public partial class DandeClient : Form
    {
        /// <summary>
        /// Class that implemnts the protocol to comunicate with Dandelion.
        /// </summary>
        private DlClient dlClient;
        
        /// <summary>
        /// The name of the next node to be added using the Quick Node button. Incremented after each addition.
        /// </summary>
        int quickNodeCount;
        public DandeClient()
        {
            InitializeComponent();
            dlClient = new DlClient();
            quickNodeCount = 0;
        }

        private void bConnect_Click(object sender, EventArgs e)
        {
            if (!dlClient.Connected)
            {
                String ip = tbServerIp.Text;
                if (ip.Equals("localhost"))
                    ip = "127.0.0.1";
                try
                {
                    dlClient.Connect(ip, nudServerPort.Text);
                    EnableComponents();
                }
                catch(Exception exception)
                {
                    MessageBox.Show(exception.Message, "An error has produced",MessageBoxButtons.OK,MessageBoxIcon.Error);
                }
            }
        }

        private void EnableComponents()
        {
            bConnect.Enabled = false;
            tbServerIp.Enabled = false;
            nudServerPort.Enabled = false;

            bDisconnect.Enabled = true;
            tbNode.Enabled = true;
            bAddNode.Enabled = true;
            bRemoveNode.Enabled = true;
            bShakeNode.Enabled = true;
            tbLink1.Enabled = true;
            tbLink2.Enabled = true;
            bAddLink.Enabled = true;
            bRemoveLink.Enabled = true;
            cbBidirectedLink.Enabled = true;
            bClearAll.Enabled = true;
            bQuickNode.Enabled = true;
            bQuickLink.Enabled = true;
        }

        private void DisableComponents()
        {
            bConnect.Enabled = true;
            tbServerIp.Enabled = true;
            nudServerPort.Enabled = true;

            bDisconnect.Enabled = false;
            tbNode.Enabled = false;
            bAddNode.Enabled = false;
            bRemoveNode.Enabled = false;
            bShakeNode.Enabled = false;
            tbLink1.Enabled = false;
            tbLink2.Enabled = false;
            bAddLink.Enabled = false;
            bRemoveLink.Enabled = false;
            cbBidirectedLink.Enabled = false;
            bClearAll.Enabled = false;
            bQuickNode.Enabled = false;
            bQuickLink.Enabled = false;
        }

        private void bAddNode_Click(object sender, EventArgs e)
        {
            dlClient.AddNode(tbNode.Text);
        }

        private void bRemoveNode_Click(object sender, EventArgs e)
        {
            dlClient.SubNode(tbNode.Text);
        }

        private void bAddLink_Click(object sender, EventArgs e)
        {
            if(cbBidirectedLink.Checked)
                dlClient.AddLinkBi(tbLink1.Text, tbLink2.Text);
            else
                dlClient.AddLink(tbLink1.Text, tbLink2.Text);
        }

        private void bRemoveLink_Click(object sender, EventArgs e)
        {
            if (cbBidirectedLink.Checked)
                dlClient.SubLinkBi(tbLink1.Text, tbLink2.Text);
            else
                dlClient.SubLink(tbLink1.Text, tbLink2.Text);
        }

        private void bClearAll_Click(object sender, EventArgs e)
        {
            dlClient.Clear();
            quickNodeCount = 0;
        }

        private void bExit_Click(object sender, EventArgs e)
        {
            if(dlClient.Connected)
                dlClient.Disconnect();
            Dispose();
        }

        private void bShakeNode_Click(object sender, EventArgs e)
        { 
            Random r = new Random();
            double theta = r.NextDouble() * 2.0 * Math.PI;
            double phi = r.NextDouble() * 2.0 * Math.PI;
            double magnitude = r.NextDouble() * 5.0;
            double x = magnitude * Math.Cos(phi) * Math.Cos(theta);
            double y = magnitude * Math.Cos(phi) * Math.Sin(theta);
            double z = magnitude * Math.Sin(phi);
            dlClient.MoveNode(tbNode.Text, x, y, z);
        }

        private void bDisconnect_Click(object sender, EventArgs e)
        {
            if (dlClient.Connected)
            {
                dlClient.Disconnect();
                DisableComponents();
            }
        }

        private void bQuickNode_Click(object sender, EventArgs e)
        {
            dlClient.AddNode(quickNodeCount.ToString());
            if (quickNodeCount > 0)
            {
                int link = (int)((float)new Random().NextDouble() * quickNodeCount);
                if (cbBidirectedLink.Checked)     
                    dlClient.AddLinkBi(link.ToString(), quickNodeCount.ToString());                
                else                
                    dlClient.AddLink(link.ToString(), quickNodeCount.ToString());                
            }
            quickNodeCount++;
        }

        private void bQuickLink_Click(object sender, EventArgs e)
        {
            if (quickNodeCount > 0)
            {
                Random r = new Random();
                int linkFrom = (int)((float)r.NextDouble() * quickNodeCount);
                int linkTo = (int)((float)r.NextDouble() * quickNodeCount);

                if (cbBidirectedLink.Checked)             
                    dlClient.AddLinkBi(linkFrom.ToString() , linkTo.ToString());            
                else               
                    dlClient.AddLink(linkFrom.ToString(), linkTo.ToString());              
            }
        }
    }
}
