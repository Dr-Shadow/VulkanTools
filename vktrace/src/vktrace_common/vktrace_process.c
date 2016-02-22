/**************************************************************************
 *
 * Copyright 2014-2016 Valve Corporation
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Peter Lohrmann <peterl@valvesoftware.com>
 * Author: David Pinedo <david@lunarg.com>
 **************************************************************************/
#include "vktrace_process.h"


BOOL vktrace_process_spawn(vktrace_process_info* pInfo)
{
    assert(pInfo != NULL);

#if defined(WIN32)
    {
    unsigned long processCreateFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;
    char fullExePath[_MAX_PATH];
    PROCESS_INFORMATION processInformation;
    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);

    memset(&processInformation, 0, sizeof(PROCESS_INFORMATION));
    memset(fullExePath, 0, sizeof(char)*_MAX_PATH);
    fullExePath[0] = 0;

    SetLastError(0);
    SearchPath(NULL, pInfo->exeName, ".exe", ARRAYSIZE(fullExePath), fullExePath, NULL);

    if (!CreateProcess(fullExePath, pInfo->fullProcessCmdLine, NULL, NULL, TRUE,
        processCreateFlags, NULL, pInfo->workingDirectory,
        &si, &processInformation))
    {
        vktrace_LogError("Failed to inject ourselves into target process--couldn't spawn '%s'.", fullExePath);
        return FALSE;
    }

    pInfo->hProcess = processInformation.hProcess;
    pInfo->hThread = processInformation.hThread;
    pInfo->processId = processInformation.dwProcessId;
    // TODO : Do we need to do anything with processInformation.dwThreadId?
    }
#elif defined(PLATFORM_LINUX)
    pInfo->processId = fork();
    if (pInfo->processId == -1)
    {
        vktrace_LogError("Failed to spawn process.");
        return FALSE;
    }
    else if (pInfo->processId == 0)
    {
        // Inside new process
        char *args[128];
        const char delim[] = " \t";
        unsigned int idx;

        // Change process name so the the tracer DLLs will behave as expected when loaded.
        // NOTE: Must be 15 characters or less.
        const char * tmpProcName = "vktraceChildProcess";
        prctl(PR_SET_NAME, (unsigned long)tmpProcName, 0, 0, 0);

        // Change working directory
        if (chdir(pInfo->workingDirectory) == -1)
        {
            vktrace_LogError("Failed to set working directory.");
        }

        args[0] = pInfo->exeName;
        args[127] = NULL;
        idx = 1;
        args[idx] = strtok(pInfo->processArgs, delim);
        while ( args[idx] != NULL && idx < 128)
        {
            idx++;
            args[idx] = strtok(NULL, delim);
        }
        vktrace_LogDebug("exec process=%s argc=%u\n", pInfo->exeName, idx);
#if 0  //uncoment to print out list of env vars
        char *env = environ[0];
        idx = 0;
        while (env && strlen(env)) {
            if (strstr(env, "VK") || strstr(env, "LD"))
                vktrace_LogDebug("env[%d] = %s", idx++, env);
            else
                idx++;
            env = environ[idx];
        }
#endif
        if (execv(pInfo->exeName, args) < 0)
        {
            vktrace_LogError("Failed to spawn process.");
            return FALSE;
        }
    }
#endif

    return TRUE;
}

void vktrace_process_info_delete(vktrace_process_info* pInfo)
{
    if (pInfo->pCaptureThreads != NULL)
    {
        vktrace_platform_delete_thread(&(pInfo->pCaptureThreads[0].recordingThread));
        VKTRACE_DELETE(pInfo->pCaptureThreads);
    }

#ifdef WIN32
    vktrace_platform_delete_thread(&(pInfo->watchdogThread));
#endif

    VKTRACE_DELETE(pInfo->traceFilename);
    VKTRACE_DELETE(pInfo->workingDirectory);
    VKTRACE_DELETE(pInfo->processArgs);
    VKTRACE_DELETE(pInfo->fullProcessCmdLine);
    VKTRACE_DELETE(pInfo->exeName);

    if (pInfo->pTraceFile != NULL)
    {
        fclose(pInfo->pTraceFile);
    }
    vktrace_delete_critical_section(&(pInfo->traceFileCriticalSection));
}
