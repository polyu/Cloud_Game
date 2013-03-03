using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Security.Principal;
using System.Collections;

namespace HookTestClient.injection
{
    public class SecurityPrivilege
    {
        [StructLayout(LayoutKind.Sequential, Pack = 4)]
        private struct LUID_AND_ATTRIBUTES
        {
            public long Luid;
            public int Attributes;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 4)]
        private struct TOKEN_PRIVILEGES
        {
            public int PrivilegeCount;
            public LUID_AND_ATTRIBUTES Privilege1;
        }

        [DllImport("advapi32.dll")]
        private static extern bool OpenProcessToken(IntPtr ProcessHandle, int DesiredAccess, ref IntPtr TokenHandle);

        [DllImport("advapi32.dll", CharSet = System.Runtime.InteropServices.CharSet.Auto)]
        private static extern bool LookupPrivilegeValue(string lpSystemName, string lpName, ref long lpLuid);

        [DllImport("advapi32.dll")]
        private static extern bool AdjustTokenPrivileges(IntPtr TokenHandle, bool DisableAllPrivileges, ref TOKEN_PRIVILEGES NewState, int BufferLength, IntPtr PreviousState, IntPtr ReturnLength);

        private const int TOKEN_QUERY = 8;
        private const int TOKEN_ADJUST_PRIVILEGES = 32;
        private const string SE_DEBUG_NAME = "SeDebugPrivilege";
        private const string SE_SECURITY_NAME = "SeSecurityPrivilege";
        private const int SE_PRIVILEGE_ENABLED = 2;

        public static bool SetPrivileges()
        {
            IntPtr hProc;
            IntPtr hToken;
            long luid_Security;
            TOKEN_PRIVILEGES tp = new TOKEN_PRIVILEGES();

            // get the process token
            hProc = Process.GetCurrentProcess().Handle;
            hToken = IntPtr.Zero;
            if (!(OpenProcessToken(hProc, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, ref hToken)))
            {
                return false;
            }

            // lookup the ID for the privilege we want to enable
            luid_Security = 0;
            //if (!(LookupPrivilegeValue(null, SE_SECURITY_NAME, ref luid_Security)))
            if (!(LookupPrivilegeValue(null, SE_DEBUG_NAME, ref luid_Security)))
            {
                return false;
            }

            tp.PrivilegeCount = 1;
            tp.Privilege1.Luid = luid_Security;
            tp.Privilege1.Attributes = SE_PRIVILEGE_ENABLED;

            // enable the privilege
            if (!(AdjustTokenPrivileges(hToken, false, ref tp, 0, IntPtr.Zero, IntPtr.Zero)))
            {
                return false;
            }
            return true;
        }
    }
}
