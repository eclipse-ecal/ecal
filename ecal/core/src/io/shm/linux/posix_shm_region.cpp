#include "io/shm/linux/posix_shm_region.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <mutex>
#include <variant>


namespace eCAL::posix
{
  namespace detail
  {
    struct Fd
    {
      int fd = -1;
      ~Fd() { if (fd != -1) ::close(fd); }
      Fd() = default;
      explicit Fd(int f) : fd(f) {}
      Fd(const Fd&) = delete;
      Fd& operator=(const Fd&) = delete;
      Fd(Fd&& o) noexcept : fd(o.fd) { o.fd = -1; }
      Fd& operator=(Fd&& o) noexcept { if (this != &o) { if (fd!=-1) ::close(fd); fd=o.fd; o.fd=-1; } return *this; }
      explicit operator bool() const { return fd != -1; }
    };


    enum class LockResult {
      LockSuccess = 0,
      FlockUnsupported = 1,
      FcntlUnsupported = 2,
      OtherLockingError = 3,
    };

    inline bool NotSupportedError(int e) noexcept
    {
#ifdef ENOTSUP
      if (e == ENOTSUP) return true;
#endif
#ifdef EOPNOTSUPP
      if (e == EOPNOTSUPP) return true;
#endif
      return false;
    }


    [[nodiscard]] LockResult LockFlock(int fd) {
      if (::flock(fd, LOCK_EX) == 0)
         return LockResult::LockSuccess;

      const int error = errno;

      if (NotSupportedError(error))
        return LockResult::FlockUnsupported;
      return LockResult::OtherLockingError;
    }

    void UnlockFlock(int fd) {
      ::flock(fd, LOCK_UN);
    }

    [[nodiscard]] LockResult LockFcntl(int fd) {
      struct flock fl{};
      fl.l_type   = F_WRLCK;
      fl.l_whence = SEEK_SET;
      fl.l_start  = 0;
      fl.l_len    = 0; // 0 => to EOF (whole file)

      const int cmd = F_SETLKW;
      if (::fcntl(fd, cmd, &fl) == 0)
        return LockResult::LockSuccess;

      const int error = errno;
      if (NotSupportedError(error))
        return LockResult::FcntlUnsupported;
      return LockResult::OtherLockingError;
    }

    void UnlockFcntl(int fd) {
      struct flock fl{};
      fl.l_type   = F_UNLCK;
      fl.l_whence = SEEK_SET;
      fl.l_start  = 0;
      fl.l_len    = 0;

      (void)::fcntl(fd, F_SETLK, &fl);
    }


    class ExclusiveFileLock
    {
      int fd = -1;
      bool locked = false;
      void (*unlock_function)(int) = nullptr;

    public:
      explicit ExclusiveFileLock(const Fd& f)
       : fd(f.fd)
      {
        if (fd < 0) return;

        const auto flock_result = LockFlock(fd);

        if (flock_result == LockResult::LockSuccess) {
          locked = true;
          unlock_function = &UnlockFlock;
          return;
        }

        if (flock_result == LockResult::OtherLockingError)
          return;

        const auto fcntl_result = LockFcntl(fd);

        if (fcntl_result == LockResult::LockSuccess) {
          locked = true;
          unlock_function = &UnlockFcntl;
        }
      }

      ~ExclusiveFileLock()
      {
        if (locked)
          unlock_function(fd);
      }

      ExclusiveFileLock(const ExclusiveFileLock&) = delete;
      ExclusiveFileLock& operator=(const ExclusiveFileLock&) = delete;
      ExclusiveFileLock(ExclusiveFileLock&&) = delete;
      ExclusiveFileLock& operator=(ExclusiveFileLock&&) = delete;

      explicit operator bool() const { return locked; }
    };

    inline int shm_open_create_or_open(const std::string& name)
    {
      const int previous_umask = ::umask(000);

      int fd = ::shm_open(name.c_str(),
                        O_RDWR | O_CREAT | O_EXCL,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

      if (fd == -1 && errno == EEXIST)
        fd = ::shm_open(name.c_str(),
                      O_RDWR,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

      ::umask(previous_umask);
      return fd;
    }

    inline void* map_shared(const Fd& fd, size_t size)
    {
      void* addr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd.fd, 0);
      return (addr == MAP_FAILED) ? nullptr : addr;
    }
  }


  std::string normalize_shm_name(std::string name)
  {
    if (name.empty()) return name;
    if (name.front() != '/') name.insert(name.begin(), '/');
    return name;
  }

  ShmRegion open_or_create_mapped_region(
      std::string shm_name,
      size_t size,
      const InitFn& init_fn)
  {
    ShmRegion out;
    out.name = normalize_shm_name(std::move(shm_name));
    out.size = size;

    const detail::Fd fd{ detail::shm_open_create_or_open(out.name)};
    if (!fd)
    {
      ::perror("shm_open");
      return out;
    }

    const detail::ExclusiveFileLock lock{fd};
    if (!lock)
    {
      ::perror("flock");
      // we continue even without holding a lock
      // under very rare conditions that may lead to data races
    }

    struct stat st{};
    if (::fstat(fd.fd, &st) == -1)
    {
      ::perror("fstat");
      return out;
    }

    const bool needs_init = (st.st_size == 0);

    if (needs_init)
    {
      if (::ftruncate(fd.fd, static_cast<off_t>(size)) == -1)
      {
        ::perror("ftruncate");
        return out;
      }
    }
    else
    {
      // Defensive: if an older version exists with wrong size, you can either fail
      // or handle versioning here. For now, fail hard to avoid UB.
      if (static_cast<size_t>(st.st_size) < size)
      {
        // Don’t resize existing objects implicitly — that can corrupt other processes.
        errno = EINVAL;
        ::perror("file size too small");
        return out;
      }
    }

    out.addr = detail::map_shared(fd, size);
    if (out.addr == nullptr)
    {
      ::perror("mmap");
      return out;
    }

    // One-time init while still under LOCK_EX, so nobody observes partially-initialized state
    if (needs_init)
    {
      if (!init_fn(out.addr))
      {
        ::munmap(out.addr, size);
         out.addr = nullptr;
        return out;
      }
      out.owner = true;
    }
 
    return out;
  }

  void close_region(ShmRegion& region)
  {
    if (region.addr != nullptr && region.size != 0)
    {
      ::munmap(region.addr, region.size);
    }
    region.addr = nullptr;
    region.size = 0;
  }

  int unlink_region(const ShmRegion& region)
  {
    return ::shm_unlink(region.name.c_str());
  }

} // namespace ecal::posix
