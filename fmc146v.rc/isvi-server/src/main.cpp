// main.cpp : Defines the entry point for the console application.
//
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "gipcy.h"
#include "isvi_server.h"

#include "printf.h"

//------------------------------------------------------------------------------

int g_isDebugInfo = 1;

int main(int argc, char *argv[]) {

  if ((argc > 3) || (argc < 2)) {
    printf("please specify directory and port, exiting..\n");
    exit(1);
  }
  // path
  size_t path = (strnlen(argv[1], 257));
  if ((!path) || (path >= 257)) {
    printf("invalid path\n");
    exit(1);
  }
  // port
  char *pCh;
  unsigned int port_input = strtoul(argv[2], &pCh, 10);
  /*if((argc == 3) && strstr(argv[2], "-v"))
      g_isDebugInfo = 1;
*/

  Printf("ISVI SERVER STARTED\n");
  printf("port : %d, directory: %s", port_input, argv[1]);

  chdir(argv[1]);

  IPC_init();
  IPC_initKeyboard();

  {
    unsigned port = port_input;
    isvi_server server(port);
    server.start();
    while (!IPC_kbhit())
      ;
  }

  IPC_delay(1000);
  IPC_cleanupKeyboard();
  IPC_cleanup();

  Printf("ISVI SERVER FINISHED\n");

  return 0;
}

//------------------------------------------------------------------------------
