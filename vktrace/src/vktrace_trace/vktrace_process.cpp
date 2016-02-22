/*
 * Copyright (c) 2013, NVIDIA CORPORATION. All rights reserved.
 * Copyright (c) 2014, Valve Software. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Peter Lohrmann <peterl@valvesoftware.com>
 */

#include <string>
#include "vktrace_process.h"
#include "vktrace.h"

#if defined(PLATFORM_LINUX)
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

extern "C" {
#include "vktrace_filelike.h"
#include "vktrace_interconnect.h"
#include "vktrace_trace_packet_utils.h"
}

const unsigned long kWatchDogPollTime = 250;

#if defined(WIN32)
void SafeCloseHandle(HANDLE& _handle)
{
    if (_handle) {
        CloseHandle(_handle);
        _handle = NULL;
    }
}
#endif

// ------------------------------------------------------------------------------------------------
VKTRACE_THREAD_ROUTINE_RETURN_TYPE Process_RunWatchdogThread(LPVOID _procInfoPtr)
{
    vktrace_process_info* pProcInfo = (vktrace_process_info*)_procInfoPtr;

#if defined(WIN32)

    while (WaitForSingleObject(pProcInfo->hProcess, kWatchDogPollTime) == WAIT_TIMEOUT)
    {
        if (pProcInfo->serverRequestsTermination)
        {
            vktrace_LogVerbose("Vktrace has requested exit.");
            return 0;
        }
    }

    vktrace_LogVerbose("Child process has terminated.");

    PostThreadMessage(pProcInfo->parentThreadId, VKTRACE_WM_COMPLETE, 0, 0);
    pProcInfo->serverRequestsTermination = TRUE;
    
#elif defined(PLATFORM_LINUX)
    int status = 0;
    int options = 0;
    while (waitpid(pProcInfo->processId, &status, options) != -1)
    {
        if (WIFEXITED(status))
        {
            vktrace_LogVerbose("Child process exited.");
            break;
        }
        else if (WCOREDUMP(status))
        {
            vktrace_LogError("Child process crashed.");
            break;
        }
        else if (WIFSIGNALED(status))
            vktrace_LogVerbose("Child process was signaled.");
        else if (WIFSTOPPED(status))
            vktrace_LogVerbose("Child process was stopped.");
        else if (WIFCONTINUED(status))
            vktrace_LogVerbose("Child process was continued.");
    }
#endif
    return 0;
}

// ------------------------------------------------------------------------------------------------
VKTRACE_THREAD_ROUTINE_RETURN_TYPE Process_RunRecordTraceThread(LPVOID _threadInfo)
{
    vktrace_process_capture_trace_thread_info* pInfo = (vktrace_process_capture_trace_thread_info*)_threadInfo;

    MessageStream* pMessageStream = vktrace_MessageStream_create(TRUE, "", VKTRACE_BASE_PORT + pInfo->tracerId);
    if (pMessageStream == NULL)
    {
        vktrace_LogError("Thread_CaptureTrace() cannot create message stream.");
        return 1;
    }

    // create trace file
    pInfo->pProcessInfo->pTraceFile = vktrace_write_trace_file_header(pInfo->pProcessInfo);

    if (pInfo->pProcessInfo->pTraceFile == NULL) {
        // writing trace file generated an error, no sense in continuing.
        vktrace_LogError("Error cannot create trace file and write header.");
        vktrace_process_info_delete(pInfo->pProcessInfo);
        return 1;
    }

    FileLike* fileLikeSocket = vktrace_FileLike_create_msg(pMessageStream);
    unsigned int total_packet_count = 0;
    vktrace_trace_packet_header* pHeader = NULL;
    size_t bytes_written;

    while (pInfo->pProcessInfo->serverRequestsTermination == FALSE)
    {
        // get a packet
        //vktrace_LogDebug("Waiting for a packet...");

        // read entire packet in
        pHeader = vktrace_read_trace_packet(fileLikeSocket);
        ++total_packet_count;
        if (pHeader == NULL)
        {
            if (pMessageStream->mErrorNum == WSAECONNRESET)
            {
                vktrace_LogError("Network Connection Reset");
            }
            else
            {
                vktrace_LogError("Network Connection Failed");
            }
            break;
        }

        //vktrace_LogDebug("Received packet id: %hu", pHeader->packet_id);
        
        if (pHeader->pBody == (uintptr_t) NULL)
        {
            vktrace_LogWarning("Received empty packet body for id: %hu", pHeader->packet_id);
        }
        else
        {
            // handle special case packets
            if (pHeader->packet_id == VKTRACE_TPI_MESSAGE)
            {
                if (g_settings.print_trace_messages == TRUE)
                {
                    vktrace_trace_packet_message* pPacket = vktrace_interpret_body_as_trace_packet_message(pHeader);
                    vktrace_LogAlways("Packet %lu: Traced Message (%s): %s", pHeader->global_packet_index, vktrace_LogLevelToShortString(pPacket->type), pPacket->message);
                    vktrace_finalize_buffer_address(pHeader, (void **) &(pPacket->message));
                }
            }

            if (pHeader->packet_id == VKTRACE_TPI_MARKER_TERMINATE_PROCESS)
            {
                pInfo->pProcessInfo->serverRequestsTermination = true;
                vktrace_delete_trace_packet(&pHeader);
                vktrace_LogVerbose("Thread_CaptureTrace is exiting.");
                break;
            }

            if (pInfo->pProcessInfo->pTraceFile != NULL)
            {
                vktrace_enter_critical_section(&pInfo->pProcessInfo->traceFileCriticalSection);
                bytes_written = fwrite(pHeader, 1, (size_t)pHeader->size, pInfo->pProcessInfo->pTraceFile);
                fflush(pInfo->pProcessInfo->pTraceFile);
                vktrace_leave_critical_section(&pInfo->pProcessInfo->traceFileCriticalSection);
                if (bytes_written != pHeader->size)
                {
                    vktrace_LogError("Failed to write the packet for packet_id = %hu", pHeader->packet_id);
                }
            }
        }

        // clean up
        vktrace_delete_trace_packet(&pHeader);
    }

    VKTRACE_DELETE(fileLikeSocket);
    vktrace_MessageStream_destroy(&pMessageStream);

    return 0;
}
