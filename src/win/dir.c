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

#include <windows.h>
#include <stdio.h>
#define MAX_ALLOCX		4096

int list_dir(char* dir, char* buffer){

   WIN32_FIND_DATA ffd;
   HANDLE hFind = INVALID_HANDLE_VALUE;

   int r = 0;
   char* fold = (char*)malloc(MAX_ALLOCX);
   char* tmp = (char*)malloc(1024);
   char* back = (char*)malloc(128);
   char* reg = (char*)malloc(MAX_ALLOCX);

   strcat(dir,"*");

   memset(tmp,0,1024);
   memset(back,0,128);
   memset(buffer,0,65536);
   memset(fold,0,MAX_ALLOCX);
   memset(reg,0,MAX_ALLOCX);

   hFind = FindFirstFile(dir, &ffd);

  if (INVALID_HANDLE_VALUE == hFind)
  {
    printf("Error: %s", buffer);
    return 0;
  }
  do
     {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
           if(strlen(&ffd.cFileName[0])<2) continue;
           if(strcmp(&ffd.cFileName[0],"..")==0){
             sprintf(tmp,"<a href=\"%s\"><img src=\"/res/back.gif\" /> </a><a href=\"%s\">%s</a><br/>\n", &ffd.cFileName[0],&ffd.cFileName[0],&ffd.cFileName[0]);
             strcat(back,tmp);
           }else{
             sprintf(tmp,"<a href=\"%s/\"><img src=\"/res/folder.gif\" /> </a><a href=\"%s/\">%s</a><br/>\n", &ffd.cFileName[0],&ffd.cFileName[0],&ffd.cFileName[0]);
             strcat(fold,tmp);
           }
         }
         else {
           sprintf(tmp,"<a href=\"%s\"><img src=\"/res/text.gif\" /> </a><a href=\"%s\">%s</a><br/>\n", &ffd.cFileName[0],&ffd.cFileName[0],&ffd.cFileName[0]);
	         strcat(reg,tmp);
        }
     }
     while ((r=FindNextFile(hFind, &ffd)) != 0);
     sprintf(buffer, "%s\n%s\n%s\n", back,fold, reg);
     FindClose(hFind);
     free(back);
     free(tmp);
     free(fold);
     free(reg);
     return 1;
 }
