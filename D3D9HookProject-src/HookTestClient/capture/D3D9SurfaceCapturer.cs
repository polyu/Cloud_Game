using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using HookTestClient.Injection;
using System.Runtime.InteropServices;
using HookTestClient.injection;
using System.Threading;
using System.Diagnostics;

namespace HookTestClient
{
    class D3D9SurfaceCapturer
    {
        private int m_length;
        private int m_width;
        private int m_height;
        private int m_desiredFPS;
        private int m_fpscount = 0;
        private bool m_stop = false;
        private IntPtr m_targetWindow = IntPtr.Zero;
        private string m_targetWindowName = string.Empty;

        private MainForm m_parent = null;

        public event CaptureEventHandler CapturedEvent;


        public D3D9SurfaceCapturer(MainForm parent)
        {
            m_parent = parent;
            //m_parent.AddCaptureEventHandler(new CaptureEventHandler(this.queueCapturedFrame));
        }
        
        public bool Initialize(IntPtr targetWindow, string targetWindowName, int width, int height, int fps)
        {
            m_width = width;
            m_height = height;
            m_length = m_width * m_height * 4;
            m_desiredFPS = fps;
            m_targetWindow = targetWindow;
            m_targetWindowName = targetWindowName;
            
            // get working dir for loading D3D9Hook.dll
            System.Reflection.Assembly a = System.Reflection.Assembly.GetExecutingAssembly();
            string dllPath = a.Location;
            int pos = dllPath.LastIndexOf('\\');
            dllPath = dllPath.Remove(pos);
            dllPath += "\\D3D9Hook.dll";

            // inject into target process (must be a directx appliction using d3d9.dll)
            if (!ProcessInjector.DoProcessInjection(m_targetWindow, m_targetWindowName, dllPath))
            {
                return false;
            }

            // Set D3D9Hook.dll capture size
            PinvokeD3D9Hook.SetCapSize(m_width, m_height);

            return true;
        }

        public void Start()
        {
            Trace.TraceInformation("D3D9SurfaceCapturer Start");

            //tell D3D9Hook to start capturing 
            PinvokeD3D9Hook.StartCapturing();
            
            Thread th = new Thread(new ThreadStart(CaptureThread));
            th.IsBackground = true;
            th.Start();
        }

        public void Stop()
        {
            Trace.TraceInformation("D3D9SurfaceCapturer Stop");

            //tell D3D9Hook to stop capturing 
            PinvokeD3D9Hook.StopCapturing();

            m_stop = true;            
        }

        private void InvokeCaptured(Byte[] data)
        {
            CaptureEventHandler capturedHandler = CapturedEvent;
            if (capturedHandler != null)
                capturedHandler(data);
            else
                Trace.TraceInformation("capturedHandler is null");
        }

        private void CaptureThread()
        {
            m_stop = false;
            try
            {
                while (!m_stop)
                {
                    DateTime start = DateTime.Now;
                    
                    //get shared memory buffer from D3D9Hook.dll
                    Byte[] data = GetBuffer();

                    if (data != null)
                    {
                        if (data.Length <= 0)
                        {
                            Trace.TraceInformation("GetBuffer() returned null");
                        }
                        else
                        {
                            //Notify Image is ready for display in UI
                            //InvokeCaptured(data);
                            m_parent.UpdateCaptureImage(data);
                            
                            //throttle if needed based on fps
                            if (m_desiredFPS != 0)
                            {
                                if (start.Second % 5 == 1)
                                {
                                    m_fpscount++;
                                }
                                else
                                {
                                    if(m_fpscount != 0)
                                        Trace.TraceInformation("CaptureThread() FPS: " + m_fpscount);
                                    m_fpscount = 0;
                                }

                                DateTime end = DateTime.Now;
                                int millis = 1000 / m_desiredFPS;
                                if ((end - start).TotalMilliseconds < millis)
                                    Thread.Sleep(millis - (end.Millisecond % millis));
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Trace.TraceInformation(ex.ToString());
            }
        }

        // gets bytes(surface) from d3d9hook.dll
        public Byte[] GetBuffer()
        {
            try
            {
                Byte[] buffer = new Byte[m_length];
                IntPtr pdataout = Marshal.UnsafeAddrOfPinnedArrayElement(buffer, 0);
                int size = PinvokeD3D9Hook.FillBuffer(pdataout, m_length);

                return buffer;
            }
            catch (Exception ex)
            {
                Trace.TraceInformation(ex.ToString());
            }

            return null; //bad
        }
    }
}
