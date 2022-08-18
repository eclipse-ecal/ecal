#pragma once

#include <ifaddrs.h>
#include <net/if.h>
#include <vector>
#include <iostream>


namespace eCAL
{
  inline static std::vector<int> get_interface_index_list()
  {
    std::vector<int> interface_index_list;
    ifaddrs* ifa;
    ifaddrs* ifap;

    // get a list of network interfaces
    getifaddrs(&ifap);

    // create a list of network interaces indexes
    for (ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
      if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_PACKET)
      {
        int index = if_nametoindex(ifa->ifa_name);
        if (index)
        {
          interface_index_list.push_back(index);
        }
      }
    }

    freeifaddrs(ifap);

    return interface_index_list;
  }

  inline static bool set_socket_mcast_group_option(int socket, const char* ipaddr_, int option)
  {
    // set the multicast socket option on all interfaces
    for (int iface : get_interface_index_list())
    {
      group_req group_req;
      sockaddr_in *group;

      memset(&group_req, 0, sizeof(group_req));
      group_req.gr_interface = iface;
      group = reinterpret_cast<sockaddr_in*>(&group_req.gr_group);
      group->sin_family = AF_INET;
      group->sin_addr.s_addr = inet_addr(ipaddr_);
      group->sin_port = 0;

      int rc = setsockopt(socket, IPPROTO_IP, option, &group_req, sizeof(group_source_req));
      if (rc != 0)
      {
        std::cerr << "setsockopt failed. Unable to set multicast group option: " << strerror(errno) << std::endl;
        return(false);
      }
    }

    return(true);
  }
}