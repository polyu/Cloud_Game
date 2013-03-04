using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace InterceptionGUI
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void m_picMain_DoubleClick(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "exe files (*.exe)|*.exe|All files (*.*)|*.*";
            ofd.Multiselect = false;
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                String file = ofd.FileName;
                String[] pathSplit = file.Split(new char[] { '\\' });
                String path = "";
                for (int i = 0; i < pathSplit.Length - 1; i++)
                    path += pathSplit[i] + "\\";

                TextWriter tw = new StreamWriter(path + "\\intercept.txt");
                tw.WriteLine(file);
                tw.WriteLine(path);
                tw.Close();

                String runpath = Application.ExecutablePath;
                int endIndex = runpath.LastIndexOf('\\');
                runpath = runpath.Substring(0, endIndex + 1);

                File.Copy(runpath + "Injector.exe", path + "Injector.exe", true);
                File.Copy(runpath + "dxhook.dll", path + "dxhook.dll", true);
                File.Copy(runpath + "White.psh", path + "White.psh", true);

                System.Diagnostics.Process.Start(path + "Injector.exe");

                Application.Exit();
            }
        }
    }
}