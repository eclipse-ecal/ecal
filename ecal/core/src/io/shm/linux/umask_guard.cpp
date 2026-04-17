#include "io/shm/linux/umask_guard.h"

namespace eCAL::io::shm::linux
{
  std::mutex& GetUmaskCreationMutex()
  {
    static std::mutex umask_creation_mutex;
    return umask_creation_mutex;
  }
}
