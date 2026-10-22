#pragma once

#include <ecal/config/transport_layer.h>

#include <initializer_list>
#include <stdexcept>
#include <vector>

namespace eCAL
{
  namespace TransportSelection
  {
    class TransportSet
    {
    public:
      TransportSet(std::initializer_list<TransportLayer::eType> transports = {})
        : TransportSet(std::vector<TransportLayer::eType>(transports))
      {
      }

      explicit TransportSet(const std::vector<TransportLayer::eType>& transports)
      {
        for (const auto transport : transports)
        {
          switch (transport)
          {
          case TransportLayer::eType::shm:    shm = true; break;
          case TransportLayer::eType::udp_mc: udp_mc = true; break;
          case TransportLayer::eType::tcp:    tcp = true; break;
          default: throw std::invalid_argument("Expected SHM, UDP, or TCP transport");
          }
        }
      }

      bool Contains(TransportLayer::eType transport) const
      {
        switch (transport)
        {
        case TransportLayer::eType::shm:    return shm;
        case TransportLayer::eType::udp_mc: return udp_mc;
        case TransportLayer::eType::tcp:    return tcp;
        default: throw std::invalid_argument("Expected SHM, UDP, or TCP transport");
        }
      }

    private:
      bool shm = false;
      bool udp_mc = false;
      bool tcp = false;
    };

    inline TransportSet ComputeEligibility(const TransportSet& sender_capabilities,
                                          const TransportSet& receiver_capabilities,
                                          const TransportSet& pair_compatibility)
    {
      std::vector<TransportLayer::eType> eligible;
      for (const auto transport : { TransportLayer::eType::shm, TransportLayer::eType::udp_mc, TransportLayer::eType::tcp })
      {
        if (sender_capabilities.Contains(transport)
          && receiver_capabilities.Contains(transport)
          && pair_compatibility.Contains(transport))
        {
          eligible.push_back(transport);
        }
      }
      return TransportSet(eligible);
    }

    inline TransportLayer::eType SelectTransport(const std::vector<TransportLayer::eType>& preferences,
                                                const TransportSet& eligible)
    {
      for (const auto transport : preferences)
      {
        static_cast<void>(eligible.Contains(transport));
      }

      for (const auto transport : preferences)
      {
        if (eligible.Contains(transport))
        {
          return transport;
        }
      }
      return TransportLayer::eType::none;
    }
  }
}