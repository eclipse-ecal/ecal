#include <io/ecal_memfile_naming.h>

#include <sstream>
#include <algorithm>

namespace eCAL
{
  namespace memfile
  {
    std::string BuildMemFileName(const std::string& base_name_, const std::chrono::time_point<std::chrono::steady_clock>& time_point)
    {
      std::string mfile_name(base_name_);
      std::stringstream out;
      out << mfile_name << "_" << std::chrono::duration_cast<std::chrono::microseconds>(time_point.time_since_epoch()).count();
      mfile_name = out.str();

      // replace all '\\' and '/' to '_'
      std::replace(mfile_name.begin(), mfile_name.end(), '\\', '_');
      std::replace(mfile_name.begin(), mfile_name.end(), '/', '_');

      // append "_shm" for debugging purposes
      mfile_name += "_shm";

      return mfile_name;
    }
  }
}