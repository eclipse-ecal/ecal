# Cross-compilation testing suite

## Quickstart

```
# Build an example arm64 Ubuntu sysroot containing some 
# pre-built dependencies replicating potential user situation.
make build-target-sysroot

# Create a generic C++ debian build container
make build-host

# Go into container with shell
make run-host

# Cross-compile eCAL.
make ecal
```