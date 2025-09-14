using PvlSppComm;

namespace PvlSppTest
{
    public partial class Form1 : Form
    {

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            StandardComms.Instance.Initialize();
            while (StandardComms.Instance.IsRunnningBridge == false)
                System.Threading.Thread.Sleep(1000);

        }

        private void _btnNewSession_Click(object sender, EventArgs e)
        {
            if (!StandardComms.Instance.IsRunnningBridge)
            {
                MessageBox.Show("IPC process don't runnning.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            StandardComms.Instance.Terminate();
            while (StandardComms.Instance.IsRunnningBridge)
                System.Threading.Thread.Sleep(1000);
        }
    }
}
