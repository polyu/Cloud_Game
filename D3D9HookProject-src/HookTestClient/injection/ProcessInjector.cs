using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using HookTestClient.InjectorOne;
using HookTestClient.InjectorTwo;
using HookTestClient.injection;

// http://www.codeproject.com/KB/vista-security/SubvertingVistaUAC.aspx

namespace HookTestClient.Injection
{
    class ProcessInjector
    {
        //wrapper that tries both injection routines if one or the other fails
        public static bool DoProcessInjection(IntPtr targetWindow, string targetWindowName, string dllpath)
        {
            //set SE_DEBUG_NAME privilege
            SecurityPrivilege.SetPrivileges();
            
            //if one attempt fails try the other
            if (!InjectOne(targetWindow, targetWindowName, dllpath))
            {
                if (!InjectTwo(targetWindow, targetWindowName, dllpath))
                {
                    return false;
                }
            }

            return true;
        }

        private static bool InjectOne(IntPtr targetWindow, string targetWindowName, string dllpath)
        {
            UInt32 ProcID;

            // Find Process ID
            Interop.GetWindowThreadProcessId(targetWindow, out ProcID);

            if (ProcID <= 0)
            {
                Trace.TraceError("ERROR: GetWindowThreadProcessId FAILED!");
                return false;
            }

            // Find Process Handle
            IntPtr ProcessHandle = Interop.OpenProcess(Interop.ProcessAccessFlags.Test, true, ProcID);
            if (ProcessHandle == IntPtr.Zero)
            {
                Trace.TraceError("ERROR: OpenProcess FAILED!");
                return false;
            }

            // Inject into process
            try
            {                
                if (!HookTestClient.InjectorOne.InjectorOne.InjectDLL(ProcessHandle, dllpath))
                {
                    Trace.TraceError("ERROR: InjectDLL() FAILED!");
                    return false;
                }
            }
            catch (Exception ex)
            {
                Trace.TraceError("ERROR: InjectDLL() - " + ex.ToString());
                return false;
            }

            Trace.TraceInformation("INFO: InjectOne SUCCEEDED");

            return true;
        }

        private static bool InjectTwo(IntPtr targetWindow, string targetWindowName, string dllpath)
        {
            //get target process and name
            int procID = 0;
            Process targetReadProcess = null;
            Process[] procList = Process.GetProcesses();

            for (int i = 0; i < procList.Length; i++)
            {
                if (procList[i].MainWindowTitle.Equals(targetWindowName))
                {
                    procID = procList[i].Id;
                    targetReadProcess = procList[i];
                    break;
                }
            }

            if (procID >= 0)
            {
                string err;
                if (!HookTestClient.InjectorTwo.InjectorTwo.DoInject(targetReadProcess, dllpath, out err))
                {
                    Trace.TraceError("ERROR: DoInject() - " + err);
                    return false;
                }
            }
            else
            {
                Trace.TraceError("ERROR: Could not find procID");
                return false;
            }

            Trace.TraceInformation("INFO: InjectTwo SUCCEEDED");

            return true;
        }
    }
}
