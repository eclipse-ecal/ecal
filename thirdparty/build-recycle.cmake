add_subdirectory(thirdparty/recycle EXCLUDE_FROM_ALL)
set_property(TARGET recycle PROPERTY FOLDER lib/recycle)
add_library(steinwurf::recycle ALIAS recycle)