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
            _lsvList = new ListView();
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
            // _lsvList
            // 
            _lsvList.Location = new Point(172, 29);
            _lsvList.Name = "_lsvList";
            _lsvList.Size = new Size(1143, 409);
            _lsvList.TabIndex = 1;
            _lsvList.UseCompatibleStateImageBehavior = false;
            _lsvList.View = View.Details;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(10F, 25F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(1490, 450);
            Controls.Add(_lsvList);
            Controls.Add(_btnNewSession);
            Name = "Form1";
            Text = "Form1";
            FormClosed += Form1_FormClosed;
            Load += Form1_Load;
            ResumeLayout(false);
        }

        #endregion

        private Button _btnNewSession;
        private ListView _lsvList;
    }
}
