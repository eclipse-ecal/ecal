/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

/**
 * @brief  UDP initialization
**/

#include <ecal/ecal_os.h>

#include <stdio.h>
#include <atomic>

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_socket.h"
#endif /* ECAL_OS_WINDOWS */

static std::atomic<int> g_socket_init_refcnt(0);

namespace eCAL
{
  namespace Net
  {
    int Initialize()
    {
      g_socket_init_refcnt++;
      if(g_socket_init_refcnt == 1)
      {
#ifdef ECAL_OS_WINDOWS
        WORD wVersionRequested = MAKEWORD(2, 2);

        WSADATA wsaData;
        int err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0)
        {
          /* Tell the user that we could not find a usable */
          /* Winsock DLL.                                  */
          printf("WSAStartup failed with error: %d\n", err);
          return(-1);
        }

        /* Confirm that the WinSock DLL supports 2.2.*/
        /* Note that if the DLL supports versions greater    */
        /* than 2.2 in addition to 2.2, it will still return */
        /* 2.2 in wVersion since that is the version we      */
        /* requested.                                        */

        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
        {
          /* Tell the user that we could not find a usable */
          /* WinSock DLL.                                  */
          printf("Could not find a usable version of Winsock.dll\n");
          WSACleanup();
        }
#endif /* ECAL_OS_WINDOWS */
      }
      return(0);
    }

    int Finalize()
    {
      if(g_socket_init_refcnt == 0) return(0);

      g_socket_init_refcnt--;
      if(g_socket_init_refcnt == 0)
      {
#ifdef ECAL_OS_WINDOWS
        WSACleanup();
#endif /* ECAL_OS_WINDOWS */
      }

      return(0);
    }
  }
}
