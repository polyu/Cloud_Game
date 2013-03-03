using System; 
using System.Diagnostics; 
using System.Runtime.InteropServices;
using System.Threading;

//This is used to actually inject the specific DLL into the selected process 

namespace HookTestClient.InjectorOne
{
    public static class InjectorOne 
    {
        public static bool InjectDLL(IntPtr hProcess, String strDLLName)
        {
            IntPtr bytesout;

            // Length of string containing the DLL file name +1 byte padding 
            Int32 LenWrite = strDLLName.Length + 1;

            // Allocate memory within the virtual address space of the target process 
            IntPtr AllocMem = (IntPtr)WINAPI.VirtualAllocEx(hProcess, (IntPtr)null, (uint)LenWrite, 0x1000, 0x40);

            // Write DLL file name to allocated memory in target process 
            WINAPI.WriteProcessMemory(hProcess, AllocMem, strDLLName, (UIntPtr)LenWrite, out bytesout);

            // Function pointer "Injector" 
            UIntPtr Injector = (UIntPtr)WINAPI.GetProcAddress(WINAPI.GetModuleHandle("kernel32.dll"), "LoadLibraryA");

            if (Injector == null)
            {
                throw new Exception("GetProcAddress failed!\n\n Call tech support.");
            }

            // Create thread in target process, and store handle in hThread 
            IntPtr hThread = (IntPtr)WINAPI.CreateRemoteThread(hProcess, (IntPtr)null, 0, Injector, AllocMem, 0, out bytesout);

            // Make sure thread handle is valid 
            if (hThread == null)
            {
                //incorrect thread handle ... return failed
                throw new Exception("CreateRemoteThread failed!\n\n Call tech support.");
            }

            // Time-out is 10 seconds... 
            int Result = WINAPI.WaitForSingleObject(hThread, 10 * 1000);

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

            // Sleep thread so as not to crash host
            Thread.Sleep(5000); 

            // Clear up allocated space ( Allocmem ) 
            WINAPI.VirtualFreeEx(hProcess, AllocMem, (UIntPtr)0, 0x8000);

            // Make sure thread handle is valid before closing... prevents crashes. 
            if (hThread != null)
            {
                //Close thread in target process 
                WINAPI.CloseHandle(hThread);
            }

            return true;
        }
        
        public static class WINAPI
        {
            [DllImport("kernel32")]
            public static extern IntPtr CreateRemoteThread(
              IntPtr hProcess,
              IntPtr lpThreadAttributes,
              uint dwStackSize,
              UIntPtr lpStartAddress, // raw Pointer into remote process
              IntPtr lpParameter,
              uint dwCreationFlags,
              out IntPtr lpThreadId
            );

            [DllImport("kernel32.dll")]
            public static extern IntPtr OpenProcess(
                UInt32 dwDesiredAccess,
                Int32 bInheritHandle,
                Int32 dwProcessId
                );

            [DllImport("kernel32.dll")]
            public static extern Int32 CloseHandle(
            IntPtr hObject
            );

            [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
            public static extern bool VirtualFreeEx(
                IntPtr hProcess,
                IntPtr lpAddress,
                UIntPtr dwSize,
                uint dwFreeType
                );

            [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true)]
            public static extern UIntPtr GetProcAddress(
                IntPtr hModule,
                string procName
                );

            [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
            public static extern IntPtr VirtualAllocEx(
                IntPtr hProcess,
                IntPtr lpAddress,
                uint dwSize,
                uint flAllocationType,
                uint flProtect
                );

            [DllImport("kernel32.dll")]
            public static extern bool WriteProcessMemory(
                IntPtr hProcess,
                IntPtr lpBaseAddress,
                string lpBuffer,
                UIntPtr nSize,
                out IntPtr lpNumberOfBytesWritten
            );

            [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
            public static extern IntPtr GetModuleHandle(
                string lpModuleName
                );

            [DllImport("kernel32", SetLastError = true, ExactSpelling = true)]
            public static extern Int32 WaitForSingleObject(
                IntPtr handle,
                Int32 milliseconds
                );

            public static Int32 GetProcessId(String proc)
            {
                Process[] ProcList;
                ProcList = Process.GetProcessesByName(proc);
                return ProcList[0].Id;
            }         
        }
    } 
} 