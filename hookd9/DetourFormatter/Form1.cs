using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;

namespace DetourFormatter
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            List<string> output = new List<string>();

            var lines = textBox1.Lines;
            foreach (var p in lines)
            {
                output.Add(Transform(p));
            }

            File.WriteAllLines("output.txt", output.ToArray());
            Process.Start("output.txt");
        }

        private string Transform(string p)
        {
            p = p.Trim();
            if (p == "" || p.StartsWith("/*"))
                return p;

            int prebracket = p.IndexOf('(');
            int postbracket = p.IndexOf(')');
            string typeMethod = p.Substring(prebracket + 1, postbracket - prebracket - 1);
            string methodName = typeMethod.Split(',').Last().Trim();

            int prebracket2 = p.LastIndexOf('(');
            int postbracket2 = p.LastIndexOf(')');
            string paramsWrapper = p.Substring(prebracket2 + 1, postbracket2 - prebracket2 - 1);

            string generatedParams = null;

            if (paramsWrapper.StartsWith("THIS_"))
            {
                generatedParams = "";

                foreach (var s in paramsWrapper.Substring(6).Split(','))
                {
                    generatedParams += s.Split(' ').Last().Trim('*') + ", ";
                }

                generatedParams = generatedParams.Substring(0, generatedParams.Length - 2);
            }
            else if (paramsWrapper.StartsWith("THIS"))
            {
                generatedParams = "";
            }
            else
            {
                throw new Exception("Unknown?");
            }

            var final = p.Substring(prebracket2, postbracket2 - prebracket2 + 1) + " { /*TODO*/ return m_ptr->" + methodName + "(" + generatedParams + "); }";

            if (p.StartsWith("STDMETHOD_"))
                final = string.Format("COM_METHOD({0}, {1})", typeMethod.Split(',').Select(z => z.Trim()).ToArray()) + final;
            else if (p.StartsWith("STDMETHOD"))
                final = string.Format("COM_METHOD(HRESULT, {0})", methodName) + final;
            else
                throw new Exception("Unknown??");

            return final;
        }
    }
}
