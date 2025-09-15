using PvlSppComm;

namespace PvlSppTest
{
    public partial class Form1 : Form
    {
        StandardComms _standardComms = new();
        public Form1()
        {
            InitializeComponent();
        }



        private void Form1_Load(object sender, EventArgs e)
        {
            _standardComms.Initialize();
            while (_standardComms.IsRunning == false)
                System.Threading.Thread.Sleep(1000);

            _standardComms.Initialize();
        }

        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
            _standardComms.Terminate();
            while (_standardComms.IsRunning)
                System.Threading.Thread.Sleep(1000);
        }

        private void _btnNewSession_Click(object sender, EventArgs e)
        {
            if (!_standardComms.IsRunning)
            {
                MessageBox.Show("IPC process don't runnning.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
        }
    }
}
