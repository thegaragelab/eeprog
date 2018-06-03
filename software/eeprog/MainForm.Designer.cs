namespace eeprog
{
  partial class MainForm
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
      this.groupBox1 = new System.Windows.Forms.GroupBox();
      this.m_lstChipType = new System.Windows.Forms.ComboBox();
      this.label3 = new System.Windows.Forms.Label();
      this.m_lstAddressSize = new System.Windows.Forms.ComboBox();
      this.label6 = new System.Windows.Forms.Label();
      this.m_lstCapacity = new System.Windows.Forms.ComboBox();
      this.label5 = new System.Windows.Forms.Label();
      this.m_lstPageSize = new System.Windows.Forms.ComboBox();
      this.label4 = new System.Windows.Forms.Label();
      this.groupBox2 = new System.Windows.Forms.GroupBox();
      this.m_progress = new System.Windows.Forms.ProgressBar();
      this.m_btnWrite = new System.Windows.Forms.Button();
      this.m_btnRead = new System.Windows.Forms.Button();
      this.m_lstEEPROM = new System.Windows.Forms.ComboBox();
      this.label2 = new System.Windows.Forms.Label();
      this.m_lstPort = new System.Windows.Forms.ComboBox();
      this.label1 = new System.Windows.Forms.Label();
      this.tabControl1 = new System.Windows.Forms.TabControl();
      this.tabMessages = new System.Windows.Forms.TabPage();
      this.m_txtMessages = new System.Windows.Forms.TextBox();
      this.tabComms = new System.Windows.Forms.TabPage();
      this.m_txtComms = new System.Windows.Forms.TextBox();
      this.groupBox1.SuspendLayout();
      this.groupBox2.SuspendLayout();
      this.tabControl1.SuspendLayout();
      this.tabMessages.SuspendLayout();
      this.tabComms.SuspendLayout();
      this.SuspendLayout();
      // 
      // groupBox1
      // 
      this.groupBox1.Controls.Add(this.m_lstChipType);
      this.groupBox1.Controls.Add(this.label3);
      this.groupBox1.Controls.Add(this.m_lstAddressSize);
      this.groupBox1.Controls.Add(this.label6);
      this.groupBox1.Controls.Add(this.m_lstCapacity);
      this.groupBox1.Controls.Add(this.label5);
      this.groupBox1.Controls.Add(this.m_lstPageSize);
      this.groupBox1.Controls.Add(this.label4);
      this.groupBox1.Location = new System.Drawing.Point(12, 12);
      this.groupBox1.Name = "groupBox1";
      this.groupBox1.Size = new System.Drawing.Size(216, 134);
      this.groupBox1.TabIndex = 6;
      this.groupBox1.TabStop = false;
      this.groupBox1.Text = "EEPROM Info";
      // 
      // m_lstChipType
      // 
      this.m_lstChipType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
      this.m_lstChipType.Enabled = false;
      this.m_lstChipType.FormattingEnabled = true;
      this.m_lstChipType.Location = new System.Drawing.Point(84, 19);
      this.m_lstChipType.Name = "m_lstChipType";
      this.m_lstChipType.Size = new System.Drawing.Size(121, 21);
      this.m_lstChipType.TabIndex = 11;
      this.m_lstChipType.SelectedIndexChanged += new System.EventHandler(this.OnChipTypeChanged);
      // 
      // label3
      // 
      this.label3.AutoSize = true;
      this.label3.Location = new System.Drawing.Point(10, 22);
      this.label3.Name = "label3";
      this.label3.Size = new System.Drawing.Size(31, 13);
      this.label3.TabIndex = 10;
      this.label3.Text = "Type";
      // 
      // m_lstAddressSize
      // 
      this.m_lstAddressSize.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
      this.m_lstAddressSize.Enabled = false;
      this.m_lstAddressSize.FormattingEnabled = true;
      this.m_lstAddressSize.Location = new System.Drawing.Point(84, 100);
      this.m_lstAddressSize.Name = "m_lstAddressSize";
      this.m_lstAddressSize.Size = new System.Drawing.Size(121, 21);
      this.m_lstAddressSize.TabIndex = 9;
      this.m_lstAddressSize.SelectedIndexChanged += new System.EventHandler(this.OnAddressSizeChanged);
      // 
      // label6
      // 
      this.label6.AutoSize = true;
      this.label6.Location = new System.Drawing.Point(10, 103);
      this.label6.Name = "label6";
      this.label6.Size = new System.Drawing.Size(68, 13);
      this.label6.TabIndex = 8;
      this.label6.Text = "Address Size";
      // 
      // m_lstCapacity
      // 
      this.m_lstCapacity.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
      this.m_lstCapacity.Enabled = false;
      this.m_lstCapacity.FormattingEnabled = true;
      this.m_lstCapacity.Location = new System.Drawing.Point(84, 73);
      this.m_lstCapacity.Name = "m_lstCapacity";
      this.m_lstCapacity.Size = new System.Drawing.Size(121, 21);
      this.m_lstCapacity.TabIndex = 7;
      this.m_lstCapacity.SelectedIndexChanged += new System.EventHandler(this.OnCapacityChanged);
      // 
      // label5
      // 
      this.label5.AutoSize = true;
      this.label5.Location = new System.Drawing.Point(10, 76);
      this.label5.Name = "label5";
      this.label5.Size = new System.Drawing.Size(48, 13);
      this.label5.TabIndex = 6;
      this.label5.Text = "Capacity";
      // 
      // m_lstPageSize
      // 
      this.m_lstPageSize.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
      this.m_lstPageSize.Enabled = false;
      this.m_lstPageSize.FormattingEnabled = true;
      this.m_lstPageSize.Location = new System.Drawing.Point(84, 46);
      this.m_lstPageSize.Name = "m_lstPageSize";
      this.m_lstPageSize.Size = new System.Drawing.Size(121, 21);
      this.m_lstPageSize.TabIndex = 5;
      this.m_lstPageSize.SelectedIndexChanged += new System.EventHandler(this.OnPageSizeChanged);
      // 
      // label4
      // 
      this.label4.AutoSize = true;
      this.label4.Location = new System.Drawing.Point(10, 49);
      this.label4.Name = "label4";
      this.label4.Size = new System.Drawing.Size(55, 13);
      this.label4.TabIndex = 4;
      this.label4.Text = "Page Size";
      // 
      // groupBox2
      // 
      this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
      this.groupBox2.Controls.Add(this.m_progress);
      this.groupBox2.Controls.Add(this.m_btnWrite);
      this.groupBox2.Controls.Add(this.m_btnRead);
      this.groupBox2.Controls.Add(this.m_lstEEPROM);
      this.groupBox2.Controls.Add(this.label2);
      this.groupBox2.Controls.Add(this.m_lstPort);
      this.groupBox2.Controls.Add(this.label1);
      this.groupBox2.Location = new System.Drawing.Point(234, 13);
      this.groupBox2.Name = "groupBox2";
      this.groupBox2.Size = new System.Drawing.Size(266, 133);
      this.groupBox2.TabIndex = 8;
      this.groupBox2.TabStop = false;
      this.groupBox2.Text = "Activity";
      // 
      // m_progress
      // 
      this.m_progress.Location = new System.Drawing.Point(17, 104);
      this.m_progress.Name = "m_progress";
      this.m_progress.Size = new System.Drawing.Size(239, 23);
      this.m_progress.TabIndex = 15;
      // 
      // m_btnWrite
      // 
      this.m_btnWrite.Location = new System.Drawing.Point(181, 72);
      this.m_btnWrite.Name = "m_btnWrite";
      this.m_btnWrite.Size = new System.Drawing.Size(75, 23);
      this.m_btnWrite.TabIndex = 13;
      this.m_btnWrite.Text = "Write";
      this.m_btnWrite.UseVisualStyleBackColor = true;
      this.m_btnWrite.Click += new System.EventHandler(this.OnWriteClick);
      // 
      // m_btnRead
      // 
      this.m_btnRead.Location = new System.Drawing.Point(97, 72);
      this.m_btnRead.Name = "m_btnRead";
      this.m_btnRead.Size = new System.Drawing.Size(75, 23);
      this.m_btnRead.TabIndex = 12;
      this.m_btnRead.Text = "Read";
      this.m_btnRead.UseVisualStyleBackColor = true;
      this.m_btnRead.Click += new System.EventHandler(this.OnReadClick);
      // 
      // m_lstEEPROM
      // 
      this.m_lstEEPROM.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
      this.m_lstEEPROM.FormattingEnabled = true;
      this.m_lstEEPROM.Location = new System.Drawing.Point(100, 45);
      this.m_lstEEPROM.Name = "m_lstEEPROM";
      this.m_lstEEPROM.Size = new System.Drawing.Size(156, 21);
      this.m_lstEEPROM.TabIndex = 11;
      this.m_lstEEPROM.SelectedIndexChanged += new System.EventHandler(this.OnEEPROMChanged);
      // 
      // label2
      // 
      this.label2.AutoSize = true;
      this.label2.Location = new System.Drawing.Point(14, 48);
      this.label2.Name = "label2";
      this.label2.Size = new System.Drawing.Size(80, 13);
      this.label2.TabIndex = 10;
      this.label2.Text = "EEPROM Type";
      // 
      // m_lstPort
      // 
      this.m_lstPort.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
      this.m_lstPort.FormattingEnabled = true;
      this.m_lstPort.Location = new System.Drawing.Point(100, 18);
      this.m_lstPort.Name = "m_lstPort";
      this.m_lstPort.Size = new System.Drawing.Size(156, 21);
      this.m_lstPort.TabIndex = 9;
      // 
      // label1
      // 
      this.label1.AutoSize = true;
      this.label1.Location = new System.Drawing.Point(14, 21);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(53, 13);
      this.label1.TabIndex = 8;
      this.label1.Text = "COM Port";
      // 
      // tabControl1
      // 
      this.tabControl1.Controls.Add(this.tabMessages);
      this.tabControl1.Controls.Add(this.tabComms);
      this.tabControl1.Dock = System.Windows.Forms.DockStyle.Bottom;
      this.tabControl1.Location = new System.Drawing.Point(0, 152);
      this.tabControl1.Name = "tabControl1";
      this.tabControl1.SelectedIndex = 0;
      this.tabControl1.Size = new System.Drawing.Size(509, 183);
      this.tabControl1.TabIndex = 9;
      // 
      // tabMessages
      // 
      this.tabMessages.Controls.Add(this.m_txtMessages);
      this.tabMessages.Location = new System.Drawing.Point(4, 22);
      this.tabMessages.Name = "tabMessages";
      this.tabMessages.Padding = new System.Windows.Forms.Padding(3);
      this.tabMessages.Size = new System.Drawing.Size(501, 157);
      this.tabMessages.TabIndex = 0;
      this.tabMessages.Text = "Messages";
      this.tabMessages.UseVisualStyleBackColor = true;
      // 
      // m_txtMessages
      // 
      this.m_txtMessages.Dock = System.Windows.Forms.DockStyle.Fill;
      this.m_txtMessages.Location = new System.Drawing.Point(3, 3);
      this.m_txtMessages.Multiline = true;
      this.m_txtMessages.Name = "m_txtMessages";
      this.m_txtMessages.ReadOnly = true;
      this.m_txtMessages.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
      this.m_txtMessages.Size = new System.Drawing.Size(495, 151);
      this.m_txtMessages.TabIndex = 0;
      // 
      // tabComms
      // 
      this.tabComms.Controls.Add(this.m_txtComms);
      this.tabComms.Location = new System.Drawing.Point(4, 22);
      this.tabComms.Name = "tabComms";
      this.tabComms.Padding = new System.Windows.Forms.Padding(3);
      this.tabComms.Size = new System.Drawing.Size(501, 157);
      this.tabComms.TabIndex = 1;
      this.tabComms.Text = "Communications";
      this.tabComms.UseVisualStyleBackColor = true;
      // 
      // m_txtComms
      // 
      this.m_txtComms.Dock = System.Windows.Forms.DockStyle.Fill;
      this.m_txtComms.Location = new System.Drawing.Point(3, 3);
      this.m_txtComms.Multiline = true;
      this.m_txtComms.Name = "m_txtComms";
      this.m_txtComms.ReadOnly = true;
      this.m_txtComms.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
      this.m_txtComms.Size = new System.Drawing.Size(495, 151);
      this.m_txtComms.TabIndex = 1;
      // 
      // MainForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(509, 335);
      this.Controls.Add(this.tabControl1);
      this.Controls.Add(this.groupBox2);
      this.Controls.Add(this.groupBox1);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
      this.MaximizeBox = false;
      this.Name = "MainForm";
      this.Text = "EEProg - The Garage Lab";
      this.groupBox1.ResumeLayout(false);
      this.groupBox1.PerformLayout();
      this.groupBox2.ResumeLayout(false);
      this.groupBox2.PerformLayout();
      this.tabControl1.ResumeLayout(false);
      this.tabMessages.ResumeLayout(false);
      this.tabMessages.PerformLayout();
      this.tabComms.ResumeLayout(false);
      this.tabComms.PerformLayout();
      this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.GroupBox groupBox1;
    private System.Windows.Forms.ComboBox m_lstAddressSize;
    private System.Windows.Forms.Label label6;
    private System.Windows.Forms.ComboBox m_lstCapacity;
    private System.Windows.Forms.Label label5;
    private System.Windows.Forms.ComboBox m_lstPageSize;
    private System.Windows.Forms.Label label4;
    private System.Windows.Forms.ComboBox m_lstChipType;
    private System.Windows.Forms.Label label3;
    private System.Windows.Forms.GroupBox groupBox2;
    private System.Windows.Forms.ProgressBar m_progress;
    private System.Windows.Forms.Button m_btnWrite;
    private System.Windows.Forms.Button m_btnRead;
    private System.Windows.Forms.ComboBox m_lstEEPROM;
    private System.Windows.Forms.Label label2;
    private System.Windows.Forms.ComboBox m_lstPort;
    private System.Windows.Forms.Label label1;
    private System.Windows.Forms.TabControl tabControl1;
    private System.Windows.Forms.TabPage tabMessages;
    private System.Windows.Forms.TextBox m_txtMessages;
    private System.Windows.Forms.TabPage tabComms;
    private System.Windows.Forms.TextBox m_txtComms;
  }
}

