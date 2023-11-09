/*
Copyright (c) 2022 CrazedoutSoft / Fredrik Roos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <stdio.h>
#include <tchar.h>
#include "../../include/win/wwebs.h"

#define BUFSIZE 4096

void WriteToPipe(char* buffer, int len);
BOOL ReadFromPipe(char* buffer, int len);
int CreateChildProcess(http_response* response, const char* exe_ptr);

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

void exec_cgi_win(http_response* response, const char* exe_ptr) {

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HANDLE hStdin;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) printf("StdoutRd CreatePipe\n");
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) printf("Stdout SetHandleInformation\n");
    if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) printf("Stdin CreatePipe\n");
    if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) printf("Stdin SetHandleInformation\n");

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = g_hChildStd_OUT_Wr;
    si.hStdOutput = g_hChildStd_OUT_Wr;
    si.hStdInput = g_hChildStd_IN_Rd;
    si.dwFlags |= STARTF_USESTDHANDLES;

      if(CreateChildProcess(response, exe_ptr)==-1){
        send_internal_error(response);
        return;
      }

        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        if (hStdin == INVALID_HANDLE_VALUE){
            ExitProcess(1);
        }

        char buffer[] = "Fredrik Rossan Roos Win32";
        char header[] = "HTTP/1.1 200 OK\nConnection: Close\n";
        char resv[4096];
        if(response->request->post_data!=NULL){
          WriteToPipe(&buffer[0], strlen(&buffer[0]));
        }
        socket_write(response->request, &header[0], strlen(&header[0]));
        if(ReadFromPipe(&resv[0],4096)){
          socket_write(response->request, &resv[0], strlen(&resv[0]));
        }else{
          send_internal_error(response);
        }

}

int CreateChildProcess(http_response* response, const char* exe_ptr) {

    TCHAR szCmdline[0];
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;
    char *file_path = (char*)malloc(4096);
    char* executable = (char*)malloc(1024);
    char* argv[128];
    int error=0;

    sprintf(file_path,"%s%s%s",
                    &response->request->virtual_path[0],&response->request->path[0],&response->request->file[0]);

    char* env = (char*)malloc(8192);
  	int n = 0;
  	int l = 0;
  	int i = 0;

  	while((response->envp[n])!=NULL){
      strcat(env, response->envp[n]);
      strcat(env, "|");
  		n++;
  	}
    l = strlen(env);
    env[l+1]='\0';
    for(i=0;i<l;i++){
      if(env[i]=='|') env[i]='\0';
    }

    if(strstr(exe_ptr,"jgazm")!=NULL){
  	  strcpy(executable, exe_ptr);
  	  argv[0]=(char*)malloc(strlen(file_path)+1);
   	  strcpy(argv[0],"-f");
  	  argv[1]=(char*)malloc(strlen(file_path)+1);
   	  strcpy(argv[1],file_path);
  	  argv[2]=NULL;
  	}else if(strcmp(exe_ptr,"[shell]")!=0){
  	  strcpy(executable, exe_ptr);
  	  argv[0]=(char*)malloc(strlen(file_path)+1);
   	  strcpy(argv[0],file_path);
  	  argv[1]=NULL;
  	}else{
  	  strcpy(executable, file_path);
  	  argv[0]=(char*)malloc(strlen(&file_path[0]));
   	  strcpy(argv[0],&file_path[0]);
  	  argv[1]=NULL;
  	}


    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcess(executable,
        szCmdline,
        NULL,
        NULL,
        TRUE,
        0,
        env,
        NULL,
        &siStartInfo,
        &piProcInfo);

    if (!bSuccess){
        printf("Error CreateProcess: %s\n", executable);
        error=-1;
    }
    else {
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }
    free(env);
    free(file_path);
    free(executable);
    n=0;
    while(1){
      if(argv[n]==NULL) break;
      free(argv[n++]);
    }

    return error;
}


void WriteToPipe(char* buffer, int len)
{
    DWORD dwWritten;
    BOOL bSuccess = FALSE;
    bSuccess = WriteFile(g_hChildStd_IN_Wr, buffer, len, &dwWritten, NULL);
    if (!bSuccess) printf("StdInWr Cannot write into child process.\n");
    CloseHandle(g_hChildStd_IN_Wr);
}

BOOL ReadFromPipe(char* buffer, int len) {
    DWORD dwRead;
    BOOL bSuccess = FALSE;
    bSuccess = ReadFile(g_hChildStd_OUT_Rd, buffer, len, &dwRead, NULL);
    if(bSuccess){
      buffer[dwRead] = '\0';
    }
    return bSuccess;
}
