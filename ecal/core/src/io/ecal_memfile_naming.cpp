#include <io/ecal_memfile_naming.h>

#include <sstream>
#include <algorithm>

namespace eCAL
{
  namespace memfile
  {
    std::string BuildMemFileName(const std::string& base_name_, const std::chrono::time_point<std::chrono::steady_clock>& time_point)
    {
      std::stringstream out;

      size_t hash = std::hash<std::string>()(base_name_ + std::to_string(time_point.time_since_epoch().count()));
      uint32_t compressed_hash = static_cast<uint32_t>(hash);

      out << "ecal_" << std::hex << compressed_hash;

      return out.str();
    }
  }
}