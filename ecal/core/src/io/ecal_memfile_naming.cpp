#include <io/ecal_memfile_naming.h>

#include <sstream>
#include <random>

namespace eCAL
{
  namespace memfile
  {

    std::string BuildRandomMemFileName()
    {
      static std::random_device rd;
      static std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

      std::stringstream out;
      out << "ecal_" << std::hex << dist(rd);

      return out.str();
    }
  }
}