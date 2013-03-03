using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Security;

namespace HookTestClient.injection
{
    class PinvokeD3D9Hook
    {
        [DllImport("D3D9Hook.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        public static extern void StartCapturing();

        [DllImport("D3D9Hook.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        public static extern void StopCapturing();

        [DllImport("D3D9Hook.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        public static extern int HostHooked();

        [DllImport("D3D9Hook.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetCapSize(int width, int height);

        [DllImport("D3D9Hook.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        public static extern int FreeResources(IntPtr address);

        //[DllImport("D3D9Hook.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        //public static extern void Binder();

        [DllImport("D3D9Hook.dll", CallingConvention = CallingConvention.Cdecl), SuppressUnmanagedCodeSecurity]
        public static extern int FillBuffer(IntPtr ptr, int size);
    }
}
