namespace PvlSppTest
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            _btnNewSession = new Button();
            _lstSessions = new ListBox();
            SuspendLayout();
            // 
            // _btnNewSession
            // 
            _btnNewSession.Location = new Point(15, 29);
            _btnNewSession.Name = "_btnNewSession";
            _btnNewSession.Size = new Size(151, 55);
            _btnNewSession.TabIndex = 0;
            _btnNewSession.Text = "NewSession";
            _btnNewSession.UseVisualStyleBackColor = true;
            _btnNewSession.Click += _btnNewSession_Click;
            // 
            // _lstSessions
            // 
            _lstSessions.FormattingEnabled = true;
            _lstSessions.ItemHeight = 25;
            _lstSessions.Location = new Point(194, 44);
            _lstSessions.Name = "_lstSessions";
            _lstSessions.Size = new Size(373, 304);
            _lstSessions.TabIndex = 1;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(10F, 25F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(_lstSessions);
            Controls.Add(_btnNewSession);
            Name = "Form1";
            Text = "Form1";
            FormClosed += Form1_FormClosed;
            Load += Form1_Load;
            ResumeLayout(false);
        }

        #endregion

        private Button _btnNewSession;
        private ListBox _lstSessions;
    }
}
