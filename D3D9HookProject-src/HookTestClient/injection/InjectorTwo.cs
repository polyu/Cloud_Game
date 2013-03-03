using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Threading;

namespace HookTestClient.InjectorTwo
{
    class InjectorTwo
    {
        Exception injectException;

        [DllImport("kernel32", SetLastError = true, ExactSpelling = true)]
        internal static extern Int32 WaitForSingleObject(
            IntPtr handle,
            Int32 milliseconds);

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        static extern bool VirtualFreeEx(
            IntPtr hProcess,
            IntPtr lpAddress,
            UIntPtr dwSize,
            uint dwFreeType);


        public static bool DoInject(
          Process pToBeInjected,
          string sDllPath,
          out string sError)
        {
            IntPtr hwnd;
            if (!CRT(pToBeInjected, sDllPath, out sError, out hwnd)) //CreateRemoteThread 
            {
                //close the handle, since the method wasn't able to get to that 
                if (hwnd != (IntPtr)0)
                {
                    WINAPI.CloseHandle(hwnd);
                }
                return false;
            }
            
            return true;
        }

        private static bool CRT(
          Process pToBeInjected,
          string sDllPath,
          out string sError,
          out IntPtr hwnd)
        {
            sError = String.Empty; //in case we encounter no errors 
                       
            IntPtr hndProc = WINAPI.OpenProcess((0x2 | 0x8 | 0x10 | 0x20 | 0x400), 1, (uint)pToBeInjected.Id);

            hwnd = hndProc;

            if (hndProc == (IntPtr)0)
            {
                sError = "Unable to attatch to process.\n";
                sError += "Error code: " + Marshal.GetLastWin32Error();
                return false;
            }

            IntPtr lpLLAddress = WINAPI.GetProcAddress(
              WINAPI.GetModuleHandle("kernel32.dll"),
              "LoadLibraryA");

            if (lpLLAddress == (IntPtr)0)
            {
                sError = "Unable to find address of \"LoadLibraryA\".\n";
                sError += "Error code: " + Marshal.GetLastWin32Error();
                return false;
            }

            IntPtr lpAddress = WINAPI.VirtualAllocEx(
              hndProc,
              (IntPtr)null,
              (IntPtr)sDllPath.Length,
                //520 bytes should be enough 
              (uint)WINAPI.VAE_Enums.AllocationType.MEM_COMMIT |
              (uint)WINAPI.VAE_Enums.AllocationType.MEM_RESERVE,
              (uint)WINAPI.VAE_Enums.ProtectionConstants.PAGE_EXECUTE_READWRITE);

            if (lpAddress == (IntPtr)0)
            {
                    sError = "Unable to allocate memory to target process.\n";
                    sError += "Error code: " + Marshal.GetLastWin32Error();
                    return false;
            }

            byte[] bytes = CalcBytes(sDllPath);
            IntPtr ipTmp;

            WINAPI.WriteProcessMemory(
              hndProc,
              lpAddress,
              bytes,
              (uint)bytes.Length,
              out ipTmp);

            if (Marshal.GetLastWin32Error() != 0)
            {
                sError = "Unable to write memory to process.\n";
                sError += "Error code: " + Marshal.GetLastWin32Error();
                return false;
            }

            IntPtr hThread = WINAPI.CreateRemoteThread(
              hndProc,
              (IntPtr)null,
              (IntPtr)0,
              lpLLAddress,
              lpAddress,
              0,
              (IntPtr)null);

            if (hThread == (IntPtr)0)
            {
                sError = "Unable to load dll into memory.";
                sError += "Error code: " + Marshal.GetLastWin32Error();
                return false;
            }

            //added so it won't crash while thread is initializing
            // Time-out is 10 seconds... 
            int Result = WaitForSingleObject(hThread, 10 * 1000);

            // Check whether thread timed out... 
            if (Result == 0x00000080L || Result == 0x00000102L || Result == 0xFFFFFFFF)
            {
                // Make sure thread handle is valid before closing... prevents crashes. 
                if (hThread != null)
                {
                    //Close thread in target process 
                    WINAPI.CloseHandle(hThread);
                }

                /* Thread timed out... */
                throw new Exception("WaitForSingleObject failed!\n\n Call tech support.");
            }

            // Sleep thread for so as not to crash host
            Thread.Sleep(1000);

            // Clear up allocated space ( Allocmem ) 
            VirtualFreeEx(hndProc, lpAddress, (UIntPtr)0, 0x8000);

            // Make sure thread handle is valid before closing... prevents crashes. 
            if (hThread != null)
            {
                //Close thread in target process 
                WINAPI.CloseHandle(hThread);
            }
            
            Trace.TraceInformation("Injecter thinks all is OK");
            return true;
        }


        private static byte[] CalcBytes(string sToConvert)
        {
            byte[] bRet = Encoding.ASCII.GetBytes(sToConvert);
            return bRet;
        }
        
        private static class WINAPI
        {
            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern IntPtr OpenProcess(
              UInt32 dwDesiredAccess,
              Int32 bInheritHandle,
              UInt32 dwProcessId);

            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern Int32 CloseHandle(
              IntPtr hObject);

            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern IntPtr GetProcAddress(
              IntPtr hModule,
              string lpProcName);

            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern IntPtr GetModuleHandle(
              string lpModuleName);

            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern IntPtr VirtualAllocEx(
              IntPtr hProcess,
              IntPtr lpAddress,
              IntPtr dwSize,
              uint flAllocationType,
              uint flProtect);

            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern Int32 WriteProcessMemory(
              IntPtr hProcess,
              IntPtr lpBaseAddress,
              byte[] buffer,
              uint size,
              out IntPtr lpNumberOfBytesWritten);

            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern IntPtr CreateRemoteThread(
              IntPtr hProcess,
              IntPtr lpThreadAttribute,
              IntPtr dwStackSize,
              IntPtr lpStartAddress,
              IntPtr lpParameter,
              uint dwCreationFlags,
              IntPtr lpThreadId);



            public static class VAE_Enums
            {
                public enum AllocationType
                {
                    MEM_COMMIT = 0x1000,
                    MEM_RESERVE = 0x2000,
                    MEM_RESET = 0x80000,
                }

                 public enum ProtectionConstants
                {
                    PAGE_EXECUTE = 0X10,
                    PAGE_EXECUTE_READ = 0X20,
                    PAGE_EXECUTE_READWRITE = 0X40,
                    PAGE_EXECUTE_WRITECOPY = 0X80,
                    PAGE_NOACCESS = 0X01
                }
            }
        }
    }
}
    

