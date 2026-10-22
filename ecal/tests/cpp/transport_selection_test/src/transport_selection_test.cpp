#include <pubsub/transport_selection.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <vector>

namespace
{
  using eCAL::TransportLayer::eType;
  using eCAL::TransportSelection::TransportSet;
  using eCAL::TransportSelection::ComputeEligibility;
  using eCAL::TransportSelection::SelectTransport;

  const std::array<eType, 3> transports{{ eType::udp_mc, eType::shm, eType::tcp }};

  TransportSet FromMask(unsigned int mask)
  {
    std::vector<eType> enabled;
    for (std::size_t index = 0; index < transports.size(); ++index)
    {
      if ((mask & (1U << index)) != 0)
      {
        enabled.push_back(transports[index]);
      }
    }
    return TransportSet(enabled);
  }
}

TEST(TransportSelection, ConstructsSetsFromNamedTransports)
{
  const TransportSet empty;
  EXPECT_FALSE(empty.Contains(eType::shm));
  EXPECT_FALSE(empty.Contains(eType::udp_mc));
  EXPECT_FALSE(empty.Contains(eType::tcp));

  const TransportSet selected{ eType::shm, eType::tcp, eType::shm };
  EXPECT_TRUE(selected.Contains(eType::shm));
  EXPECT_FALSE(selected.Contains(eType::udp_mc));
  EXPECT_TRUE(selected.Contains(eType::tcp));

  EXPECT_THROW((TransportSet{ eType::none }), std::invalid_argument);
  EXPECT_THROW((TransportSet{ static_cast<eType>(999) }), std::invalid_argument);
}

TEST(TransportSelection, ExhaustiveCapabilitiesCompatibilityAndPreferenceOrders)
{
  for (unsigned int sender_mask = 0; sender_mask < 8; ++sender_mask)
  {
    for (unsigned int receiver_mask = 0; receiver_mask < 8; ++receiver_mask)
    {
      for (unsigned int compatibility_mask = 0; compatibility_mask < 8; ++compatibility_mask)
      {
        const auto eligible = ComputeEligibility(FromMask(sender_mask), FromMask(receiver_mask), FromMask(compatibility_mask));
        const auto expected_mask = sender_mask & receiver_mask & compatibility_mask;
        for (std::size_t index = 0; index < transports.size(); ++index)
        {
          EXPECT_EQ(eligible.Contains(transports[index]), (expected_mask & (1U << index)) != 0);
        }

        auto order = transports;
        do
        {
          for (std::size_t count = 0; count <= order.size(); ++count)
          {
            const std::vector<eType> preferences(order.begin(), order.begin() + count);
            auto expected = eType::none;
            for (const auto transport : preferences)
            {
              const auto position = std::find(transports.begin(), transports.end(), transport) - transports.begin();
              if ((expected_mask & (1U << position)) != 0)
              {
                expected = transport;
                break;
              }
            }
            EXPECT_EQ(SelectTransport(preferences, eligible), expected);
            EXPECT_EQ(SelectTransport(preferences, eligible), SelectTransport(preferences, eligible));
          }
        } while (std::next_permutation(order.begin(), order.end()));
      }
    }
  }
}

TEST(TransportSelection, DuplicatePreferencesDoNotChangeSelection)
{
  const TransportSet eligible{ eType::tcp };
  EXPECT_EQ(SelectTransport({ eType::shm, eType::shm, eType::tcp }, eligible), eType::tcp);
}

TEST(TransportSelection, OmittedTransportIsNotAnImplicitFallback)
{
  const TransportSet eligible{ eType::tcp };
  EXPECT_EQ(SelectTransport({ eType::shm }, eligible), eType::none);
}

TEST(TransportSelection, InvalidPreferenceIsRejectedEvenAfterAnEligibleEntry)
{
  const TransportSet eligible{ eType::shm, eType::udp_mc, eType::tcp };
  const auto invalid = static_cast<eType>(999);
  EXPECT_THROW(SelectTransport({ eType::shm, invalid }, eligible), std::invalid_argument);
  EXPECT_THROW(SelectTransport({ invalid, eType::shm }, eligible), std::invalid_argument);
  EXPECT_THROW(SelectTransport({ eType::none }, eligible), std::invalid_argument);
  EXPECT_THROW(SelectTransport({ invalid }, TransportSet{}), std::invalid_argument);
}