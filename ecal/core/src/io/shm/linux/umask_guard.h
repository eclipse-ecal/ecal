#pragma once

#include <sys/stat.h>

#include <mutex>

namespace eCAL::io::shm
{
  std::mutex& GetUmaskCreationMutex();

  class ScopedUmaskRestore
  {
  public:
    explicit ScopedUmaskRestore(mode_t temporary_umask)
      : previous_umask_(::umask(temporary_umask))
    {}

    ~ScopedUmaskRestore()
    {
      ::umask(previous_umask_);
    }

    ScopedUmaskRestore(const ScopedUmaskRestore&) = delete;
    ScopedUmaskRestore& operator=(const ScopedUmaskRestore&) = delete;
    ScopedUmaskRestore(ScopedUmaskRestore&&) = delete;
    ScopedUmaskRestore& operator=(ScopedUmaskRestore&&) = delete;

  private:
    mode_t previous_umask_;
  };
}
