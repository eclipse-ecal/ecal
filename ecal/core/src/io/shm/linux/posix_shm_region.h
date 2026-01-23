#pragma once

#include <functional>
#include <string>
#include <type_traits>

namespace eCAL::posix
{
  struct ShmRegion
  {
    void* addr = nullptr;
    size_t size = 0;
    std::string name;
    bool owner = false;   // true if we performed first-time init
  };

  using InitFn = std::function<bool(void* mapped_addr)>;

  ShmRegion open_or_create_mapped_region(
      std::string shm_name,
      size_t size,
      const InitFn init_fn);

  void close_region(const ShmRegion& r);
  int  unlink_region(const std::string& name);


  template <typename T>
  struct ShmTypedRegion
  {
    ShmRegion region;

    explicit operator bool() const { return region.addr != nullptr; }
    T* ptr() const {return static_cast<T*>(region.addr); }
    bool owner() const { return region.owner; }
    const std::string& name() const { return region.name; }
  };

  template <typename T, typename Init>
  ShmTypedRegion<T> open_or_create_typed_mapped_region(std::string name, Init&& init)
  {
    static_assert(std::is_standard_layout_v<T>,
      "T should be standard-layout for shared memory compatibility");

    ShmTypedRegion<T> out;
    out.region = open_or_create_mapped_region(
      std::move(name),
      sizeof(T),
      [fn = std::forward<Init>(init)](void* mem) -> bool
      {
        return fn(static_cast<T*>(mem));
      });

    return out;
  }
}
