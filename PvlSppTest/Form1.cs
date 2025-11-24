using PvlSppComm;
using System.ComponentModel;

namespace PvlSppTest
{
    public partial class Form1 : Form
    {
        StandardComms _standardComms;

        class SpiSessionInfo : ListViewItem
        {
            public required PvlSppComm.SpiInstanceComms spi;

            public override string ToString()
            {
                return "No Spi Session";
            }

        }

        public Form1()
        {
            _standardComms = new();
            _standardComms.OnReciveNewSession += (s) =>
            {
                BeginInvoke(() =>
                {
                    var spiInfo = new SpiSessionInfo() { spi = s };
                    _lsvList.Items.Add(spiInfo);

                } );
            };
            InitializeComponent();
            


        }


        
        private void Form1_Load(object sender, EventArgs e)
        {
            _standardComms.StartupServer();
            while (_standardComms.IsRunning == false)
                System.Threading.Thread.Sleep(1000);

        }
        
        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            base.OnFormClosed(e);
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
            _standardComms.SendNewSession();
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
        }
    }
}
