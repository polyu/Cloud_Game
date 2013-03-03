using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace HookTestClient.Injection
{
    class Interop
    {
        [DllImport("USER32.DLL")]
        public static extern IntPtr FindWindow(
        string lpClassName,
        string lpWindowName);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern uint GetWindowThreadProcessId(
        IntPtr hWnd,
        out uint lpdwProcessId);

        [DllImport("kernel32")]
        public static extern IntPtr CreateRemoteThread(
          IntPtr hProcess,
          IntPtr lpThreadAttributes,
          uint dwStackSize,
          UIntPtr lpStartAddress, // raw Pointer into remote process 
          IntPtr lpParameter,
          uint dwCreationFlags,
          out IntPtr lpThreadId);

        [Flags]
        public enum ProcessAccessFlags : uint
        {
            Test = 0x1F0FFF,
            All = 0x001F0FFF,
            Terminate = 0x00000001,
            CreateThread = 0x00000002,
            VMOperation = 0x00000008,
            VMRead = 0x00000010,
            VMWrite = 0x00000020,
            DupHandle = 0x00000040,
            SetInformation = 0x00000200,
            QueryInformation = 0x00000400,
            Synchronize = 0x00100000
        }

        [DllImport("kernel32.dll")]
        public static extern IntPtr OpenProcess(ProcessAccessFlags dwDesiredAccess,
            [MarshalAs(UnmanagedType.Bool)] bool bInheritHandle,
           uint dwProcessId);

        [DllImport("kernel32.dll")]
        public static extern Int32 CloseHandle(
        IntPtr hObject);

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        static extern bool VirtualFreeEx(
            IntPtr hProcess,
            IntPtr lpAddress,
            UIntPtr dwSize,
            uint dwFreeType);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true)]
        public static extern UIntPtr GetProcAddress(
            IntPtr hModule,
            string procName);

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        public static extern IntPtr VirtualAllocEx(
            IntPtr hProcess,
            IntPtr lpAddress,
            uint dwSize,
            uint flAllocationType,
            uint flProtect);

        [DllImport("kernel32.dll")]
        public static extern bool WriteProcessMemory(
            IntPtr hProcess,
            IntPtr lpBaseAddress,
            string lpBuffer,
            UIntPtr nSize,
            out IntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr GetModuleHandle(
            string lpModuleName);

        [DllImport("kernel32", SetLastError = true, ExactSpelling = true)]
        public static extern Int32 WaitForSingleObject(
            IntPtr handle,
            Int32 milliseconds);
    }
}
