using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using HookTestClient.injection;
using System.Drawing.Imaging;

namespace HookTestClient
{
    public partial class MainForm : Form
    {
        static readonly int GWL_STYLE = -16;
        static readonly ulong WS_VISIBLE = 0x10000000L;
        static readonly ulong WS_BORDER = 0x00800000L;
        static readonly ulong TARGETWINDOW = WS_BORDER | WS_VISIBLE;
        
        [DllImport("user32.dll")]
        static extern ulong GetWindowLongA(IntPtr hWnd, int nIndex);

        [DllImport("user32.dll")]
        static extern int EnumWindows(EnumWindowsCallback lpEnumFunc, int lParam);
        delegate bool EnumWindowsCallback(IntPtr hwnd, int lParam);

        [DllImport("user32.dll")]
        public static extern void GetWindowText(IntPtr hWnd, StringBuilder lpString, int nMaxCount);
        
        internal class Window
        {
            public string Title;
            public IntPtr Handle;

            public override string ToString()
            {
                return Title;
            }
        }
        
        private D3D9SurfaceCapturer m_d3d9Capturer = null;
        private List<Window> m_windows;
        private Window m_selectedWindow;
        
        public MainForm()
        {
            InitializeComponent();

            m_d3d9Capturer = new D3D9SurfaceCapturer(this);
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            //get active windows
            GetWindows();
        }

        public void AddCaptureEventHandler(CaptureEventHandler handler)
        {
            m_d3d9Capturer.CapturedEvent += handler;
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            this.Cursor = Cursors.WaitCursor;

            if (!m_d3d9Capturer.Initialize(m_selectedWindow.Handle, m_selectedWindow.Title, 720, 480, 25))
            {
                MessageBox.Show("DLL injection failed.");
                this.Cursor = Cursors.Default;
                return;
            }

            m_d3d9Capturer.Start();
            
            this.buttonStart.Enabled = false;
            this.buttonStop.Enabled = true;

            this.Cursor = Cursors.Default;
             
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            this.Cursor = Cursors.WaitCursor;
            
            m_d3d9Capturer.Stop();
            
            this.buttonStart.Enabled = true;
            this.buttonStop.Enabled = false;

            this.Cursor = Cursors.Default;
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            GetWindows();            
        }

        private void lstWindows_SelectedIndexChanged(object sender, EventArgs e)
        {
            m_selectedWindow = (Window)lstWindows.SelectedItem;
            this.buttonStart.Enabled = true;
        }

        private void GetWindows()
        {
            m_windows = new List<Window>();
            EnumWindows(Callback, 0);

            lstWindows.Items.Clear();
            foreach (Window w in m_windows)
                lstWindows.Items.Add(w);
        }

        private bool Callback(IntPtr hwnd, int lParam)
        {
            if (this.Handle != hwnd && (GetWindowLongA(hwnd, GWL_STYLE) & TARGETWINDOW) == TARGETWINDOW)
            {
                StringBuilder sb = new StringBuilder(100);
                GetWindowText(hwnd, sb, sb.Capacity);

                Window t = new Window();
                t.Handle = hwnd;
                t.Title = sb.ToString();
                m_windows.Add(t);
            }

            return true;
        }

        public void UpdateCaptureImage(Byte[] rawBytes)
        {
            if (this.InvokeRequired)
            {
                BeginInvoke(new MethodInvoker(delegate() { UpdateCaptureImage(rawBytes); }));
            }
            else
            {
                this.pictureBoxCapture.Image = CreateScaledImage(rawBytes);
                this.pictureBoxCapture.Refresh();
            }
        }

        //creates a scaled image that fits the PictureBox size
        private Bitmap CreateScaledImage(Byte[] rawBytes)
        {
            //create a real bitmap
            int stride = 0;
            Bitmap bmp = CreateDIB(rawBytes, 720, 480, ref stride);
            
            //use image control size
            int w = pictureBoxCapture.Width;
            int h = pictureBoxCapture.Height;

            Bitmap scaled = null;
            scaled = new Bitmap(w, h, PixelFormat.Format24bppRgb);

            Bitmap clonedBmp = bmp.Clone(new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height),
                PixelFormat.Format24bppRgb);

            using (Graphics g = Graphics.FromImage((Image)scaled))
                g.DrawImage(clonedBmp, 0, 0, w, h);
            bmp.Dispose();
            clonedBmp.Dispose();

            return scaled;
        }

        // Create a bitmap from raw bytes
        private Bitmap CreateDIB(Byte[] data, int width, int height, ref int stride)
        {
            Bitmap bmp = new Bitmap(width, height, PixelFormat.Format32bppArgb);

            BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, width, height),
                                              ImageLockMode.WriteOnly,
                                              PixelFormat.Format32bppArgb);

            IntPtr safePtr = bmpData.Scan0;
            stride = width * height * 4;
            Marshal.Copy(data, 0, safePtr, stride);
            bmp.UnlockBits(bmpData);            

            return bmp;
        }        
    }
}
