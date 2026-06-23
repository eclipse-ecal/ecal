/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2026 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * ========================= eCAL LICENSE =================================
*/

#include "ecal_event.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
  const std::string ping_event_name("ping");
  const std::string pong_event_name("pong");
  const long wait_timeout_ms(5000);

  int ParseIterations(int argc, char** argv)
  {
    if (argc < 2) return 1000000;
    return std::atoi(argv[1]);
  }

  void DrainEvent(const eCAL::EventHandleT& event_)
  {
    while (eCAL::gWaitForEvent(event_, 0))
    {
    }
  }
}

int main(int argc, char** argv)
{
  const int iterations = ParseIterations(argc, argv);

  eCAL::EventHandleT ping_event;
  eCAL::EventHandleT pong_event;

  if (!eCAL::gOpenNamedEvent(&ping_event, ping_event_name, false))
  {
    std::cerr << "Failed to open ping event." << std::endl;
    return 1;
  }

  if (!eCAL::gOpenNamedEvent(&pong_event, pong_event_name, false))
  {
    std::cerr << "Failed to open pong event." << std::endl;
    eCAL::gCloseEvent(ping_event);
    return 1;
  }

  DrainEvent(ping_event);
  DrainEvent(pong_event);

  std::cout << "pong app ready. Iterations: " << iterations << std::endl;

  const auto start_time = std::chrono::steady_clock::now();

  for (int i = 0; i < iterations; ++i)
  {
    if (!eCAL::gWaitForEvent(ping_event, wait_timeout_ms))
    {
      std::cerr << "Timeout waiting for ping at iteration " << i << "." << std::endl;
      eCAL::gCloseEvent(pong_event);
      eCAL::gCloseEvent(ping_event);
      return 2;
    }

    if (!eCAL::gSetEvent(pong_event))
    {
      std::cerr << "Failed to set pong at iteration " << i << "." << std::endl;
      eCAL::gCloseEvent(pong_event);
      eCAL::gCloseEvent(ping_event);
      return 1;
    }

    if (((i + 1) % 100000) == 0)
    {
      std::cout << "Completed " << (i + 1) << " ping-pong cycles." << std::endl;
    }
  }

  const auto stop_time = std::chrono::steady_clock::now();
  const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();
  const double cycles_per_second = elapsed_ms > 0 ? static_cast<double>(iterations) * 1000.0 / static_cast<double>(elapsed_ms) : 0.0;

  std::cout << "Finished without detected loss." << std::endl;
  std::cout << "Elapsed: " << elapsed_ms << " ms, rate: " << cycles_per_second << " cycles/s." << std::endl;

  eCAL::gCloseEvent(pong_event);
  eCAL::gCloseEvent(ping_event);

  return 0;
}