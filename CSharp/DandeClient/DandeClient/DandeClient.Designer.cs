namespace DandeClient
{
    partial class DandeClient
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DandeClient));
            this.lServer = new System.Windows.Forms.Label();
            this.lNode = new System.Windows.Forms.Label();
            this.lLink = new System.Windows.Forms.Label();
            this.tbServerIp = new System.Windows.Forms.TextBox();
            this.nudServerPort = new System.Windows.Forms.NumericUpDown();
            this.bConnect = new System.Windows.Forms.Button();
            this.bDisconnect = new System.Windows.Forms.Button();
            this.tbNode = new System.Windows.Forms.TextBox();
            this.bAddNode = new System.Windows.Forms.Button();
            this.bRemoveNode = new System.Windows.Forms.Button();
            this.bShakeNode = new System.Windows.Forms.Button();
            this.tbLink1 = new System.Windows.Forms.TextBox();
            this.tbLink2 = new System.Windows.Forms.TextBox();
            this.bRemoveLink = new System.Windows.Forms.Button();
            this.bAddLink = new System.Windows.Forms.Button();
            this.cbBidirectedLink = new System.Windows.Forms.CheckBox();
            this.bClearAll = new System.Windows.Forms.Button();
            this.bQuickNode = new System.Windows.Forms.Button();
            this.bQuickLink = new System.Windows.Forms.Button();
            this.bExit = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.nudServerPort)).BeginInit();
            this.SuspendLayout();
            // 
            // lServer
            // 
            this.lServer.AutoSize = true;
            this.lServer.Location = new System.Drawing.Point(0, 9);
            this.lServer.Name = "lServer";
            this.lServer.Size = new System.Drawing.Size(38, 13);
            this.lServer.TabIndex = 0;
            this.lServer.Text = "Server";
            // 
            // lNode
            // 
            this.lNode.AutoSize = true;
            this.lNode.Location = new System.Drawing.Point(0, 35);
            this.lNode.Name = "lNode";
            this.lNode.Size = new System.Drawing.Size(33, 13);
            this.lNode.TabIndex = 1;
            this.lNode.Text = "Node";
            // 
            // lLink
            // 
            this.lLink.AutoSize = true;
            this.lLink.Location = new System.Drawing.Point(0, 62);
            this.lLink.Name = "lLink";
            this.lLink.Size = new System.Drawing.Size(27, 13);
            this.lLink.TabIndex = 2;
            this.lLink.Text = "Link";
            // 
            // tbServerIp
            // 
            this.tbServerIp.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbServerIp.Location = new System.Drawing.Point(40, 6);
            this.tbServerIp.Name = "tbServerIp";
            this.tbServerIp.Size = new System.Drawing.Size(219, 20);
            this.tbServerIp.TabIndex = 3;
            this.tbServerIp.Text = "localhost";
            // 
            // nudServerPort
            // 
            this.nudServerPort.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.nudServerPort.Location = new System.Drawing.Point(265, 6);
            this.nudServerPort.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.nudServerPort.Minimum = new decimal(new int[] {
            1024,
            0,
            0,
            0});
            this.nudServerPort.Name = "nudServerPort";
            this.nudServerPort.Size = new System.Drawing.Size(113, 20);
            this.nudServerPort.TabIndex = 4;
            this.nudServerPort.Value = new decimal(new int[] {
            4972,
            0,
            0,
            0});
            // 
            // bConnect
            // 
            this.bConnect.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.bConnect.Location = new System.Drawing.Point(383, 4);
            this.bConnect.Name = "bConnect";
            this.bConnect.Size = new System.Drawing.Size(61, 23);
            this.bConnect.TabIndex = 5;
            this.bConnect.Text = "&Connect";
            this.bConnect.UseVisualStyleBackColor = true;
            this.bConnect.Click += new System.EventHandler(this.bConnect_Click);
            // 
            // bDisconnect
            // 
            this.bDisconnect.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.bDisconnect.Enabled = false;
            this.bDisconnect.Location = new System.Drawing.Point(447, 4);
            this.bDisconnect.Name = "bDisconnect";
            this.bDisconnect.Size = new System.Drawing.Size(77, 23);
            this.bDisconnect.TabIndex = 6;
            this.bDisconnect.Text = "D&isconnect";
            this.bDisconnect.UseVisualStyleBackColor = true;
            this.bDisconnect.Click += new System.EventHandler(this.bDisconnect_Click);
            // 
            // tbNode
            // 
            this.tbNode.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbNode.Enabled = false;
            this.tbNode.Location = new System.Drawing.Point(40, 32);
            this.tbNode.Name = "tbNode";
            this.tbNode.Size = new System.Drawing.Size(338, 20);
            this.tbNode.TabIndex = 7;
            // 
            // bAddNode
            // 
            this.bAddNode.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.bAddNode.Enabled = false;
            this.bAddNode.Location = new System.Drawing.Point(383, 30);
            this.bAddNode.Name = "bAddNode";
            this.bAddNode.Size = new System.Drawing.Size(34, 23);
            this.bAddNode.TabIndex = 8;
            this.bAddNode.Text = "&Add";
            this.bAddNode.UseVisualStyleBackColor = true;
            this.bAddNode.Click += new System.EventHandler(this.bAddNode_Click);
            // 
            // bRemoveNode
            // 
            this.bRemoveNode.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.bRemoveNode.Enabled = false;
            this.bRemoveNode.Location = new System.Drawing.Point(420, 30);
            this.bRemoveNode.Name = "bRemoveNode";
            this.bRemoveNode.Size = new System.Drawing.Size(55, 23);
            this.bRemoveNode.TabIndex = 9;
            this.bRemoveNode.Text = "&Remove";
            this.bRemoveNode.UseVisualStyleBackColor = true;
            this.bRemoveNode.Click += new System.EventHandler(this.bRemoveNode_Click);
            // 
            // bShakeNode
            // 
            this.bShakeNode.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.bShakeNode.Enabled = false;
            this.bShakeNode.Location = new System.Drawing.Point(478, 30);
            this.bShakeNode.Name = "bShakeNode";
            this.bShakeNode.Size = new System.Drawing.Size(46, 23);
            this.bShakeNode.TabIndex = 10;
            this.bShakeNode.Text = "&Shake";
            this.bShakeNode.UseVisualStyleBackColor = true;
            this.bShakeNode.Click += new System.EventHandler(this.bShakeNode_Click);
            // 
            // tbLink1
            // 
            this.tbLink1.Enabled = false;
            this.tbLink1.Location = new System.Drawing.Point(40, 59);
            this.tbLink1.Name = "tbLink1";
            this.tbLink1.Size = new System.Drawing.Size(153, 20);
            this.tbLink1.TabIndex = 11;
            // 
            // tbLink2
            // 
            this.tbLink2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbLink2.Enabled = false;
            this.tbLink2.Location = new System.Drawing.Point(198, 59);
            this.tbLink2.Name = "tbLink2";
            this.tbLink2.Size = new System.Drawing.Size(153, 20);
            this.tbLink2.TabIndex = 12;
            // 
            // bRemoveLink
            // 
            this.bRemoveLink.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.bRemoveLink.Enabled = false;
            this.bRemoveLink.Location = new System.Drawing.Point(393, 57);
            this.bRemoveLink.Name = "bRemoveLink";
            this.bRemoveLink.Size = new System.Drawing.Size(55, 23);
            this.bRemoveLink.TabIndex = 14;
            this.bRemoveLink.Text = "R&emove";
            this.bRemoveLink.UseVisualStyleBackColor = true;
            this.bRemoveLink.Click += new System.EventHandler(this.bRemoveLink_Click);
            // 
            // bAddLink
            // 
            this.bAddLink.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.bAddLink.Enabled = false;
            this.bAddLink.Location = new System.Drawing.Point(356, 57);
            this.bAddLink.Name = "bAddLink";
            this.bAddLink.Size = new System.Drawing.Size(34, 23);
            this.bAddLink.TabIndex = 13;
            this.bAddLink.Text = "A&dd";
            this.bAddLink.UseVisualStyleBackColor = true;
            this.bAddLink.Click += new System.EventHandler(this.bAddLink_Click);
            // 
            // cbBidirectedLink
            // 
            this.cbBidirectedLink.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cbBidirectedLink.AutoSize = true;
            this.cbBidirectedLink.Enabled = false;
            this.cbBidirectedLink.Location = new System.Drawing.Point(451, 61);
            this.cbBidirectedLink.Name = "cbBidirectedLink";
            this.cbBidirectedLink.Size = new System.Drawing.Size(73, 17);
            this.cbBidirectedLink.TabIndex = 15;
            this.cbBidirectedLink.Text = "Bidirected";
            this.cbBidirectedLink.UseVisualStyleBackColor = true;
            // 
            // bClearAll
            // 
            this.bClearAll.Enabled = false;
            this.bClearAll.Location = new System.Drawing.Point(3, 85);
            this.bClearAll.Name = "bClearAll";
            this.bClearAll.Size = new System.Drawing.Size(54, 23);
            this.bClearAll.TabIndex = 16;
            this.bClearAll.Text = "C&lear All";
            this.bClearAll.UseVisualStyleBackColor = true;
            this.bClearAll.Click += new System.EventHandler(this.bClearAll_Click);
            // 
            // bQuickNode
            // 
            this.bQuickNode.Enabled = false;
            this.bQuickNode.Location = new System.Drawing.Point(63, 85);
            this.bQuickNode.Name = "bQuickNode";
            this.bQuickNode.Size = new System.Drawing.Size(72, 23);
            this.bQuickNode.TabIndex = 17;
            this.bQuickNode.Text = "Quick &Node";
            this.bQuickNode.UseVisualStyleBackColor = true;
            this.bQuickNode.Click += new System.EventHandler(this.bQuickNode_Click);
            // 
            // bQuickLink
            // 
            this.bQuickLink.Enabled = false;
            this.bQuickLink.Location = new System.Drawing.Point(141, 85);
            this.bQuickLink.Name = "bQuickLink";
            this.bQuickLink.Size = new System.Drawing.Size(66, 23);
            this.bQuickLink.TabIndex = 18;
            this.bQuickLink.Text = "Quick Lin&k";
            this.bQuickLink.UseVisualStyleBackColor = true;
            this.bQuickLink.Click += new System.EventHandler(this.bQuickLink_Click);
            // 
            // bExit
            // 
            this.bExit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.bExit.Location = new System.Drawing.Point(483, 85);
            this.bExit.Name = "bExit";
            this.bExit.Size = new System.Drawing.Size(41, 23);
            this.bExit.TabIndex = 19;
            this.bExit.Text = "E&xit";
            this.bExit.UseVisualStyleBackColor = true;
            this.bExit.Click += new System.EventHandler(this.bExit_Click);
            // 
            // DandeClient
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(526, 114);
            this.Controls.Add(this.bExit);
            this.Controls.Add(this.bQuickLink);
            this.Controls.Add(this.bQuickNode);
            this.Controls.Add(this.bClearAll);
            this.Controls.Add(this.cbBidirectedLink);
            this.Controls.Add(this.bRemoveLink);
            this.Controls.Add(this.bAddLink);
            this.Controls.Add(this.tbLink2);
            this.Controls.Add(this.tbLink1);
            this.Controls.Add(this.bShakeNode);
            this.Controls.Add(this.bRemoveNode);
            this.Controls.Add(this.bAddNode);
            this.Controls.Add(this.tbNode);
            this.Controls.Add(this.bDisconnect);
            this.Controls.Add(this.bConnect);
            this.Controls.Add(this.nudServerPort);
            this.Controls.Add(this.tbServerIp);
            this.Controls.Add(this.lLink);
            this.Controls.Add(this.lNode);
            this.Controls.Add(this.lServer);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "DandeClient";
            this.Text = "Dandelion Test Client";
            ((System.ComponentModel.ISupportInitialize)(this.nudServerPort)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lServer;
        private System.Windows.Forms.Label lNode;
        private System.Windows.Forms.Label lLink;
        private System.Windows.Forms.TextBox tbServerIp;
        private System.Windows.Forms.NumericUpDown nudServerPort;
        private System.Windows.Forms.Button bConnect;
        private System.Windows.Forms.Button bDisconnect;
        private System.Windows.Forms.TextBox tbNode;
        private System.Windows.Forms.Button bAddNode;
        private System.Windows.Forms.Button bRemoveNode;
        private System.Windows.Forms.Button bShakeNode;
        private System.Windows.Forms.TextBox tbLink1;
        private System.Windows.Forms.TextBox tbLink2;
        private System.Windows.Forms.Button bRemoveLink;
        private System.Windows.Forms.Button bAddLink;
        private System.Windows.Forms.CheckBox cbBidirectedLink;
        private System.Windows.Forms.Button bClearAll;
        private System.Windows.Forms.Button bQuickNode;
        private System.Windows.Forms.Button bQuickLink;
        private System.Windows.Forms.Button bExit;
    }
}

