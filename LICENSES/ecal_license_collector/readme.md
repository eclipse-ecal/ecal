# License Collector

These python files are a set of scripts that collect license information from various eCAL thirdparty dependencies and copies them to a common directory.

## Scripts

- `update_licenses_dirs.py`

    This script fills the `licenses` and `licenses_python_binding` directories in the repo root. For that, it goes through the `thirdparty` directory, where each thirdparty dependency publishes information about itself in a python script.

    Whenever a thirdparty dependency has been added / modified / removed, this script needs to be run again.

- `grab_external_license_files.py`

    This script does not need to be executed regularly, only when specific external thirdparty dependencies change.

    As not all thirdparty dependencies have license information available from a submodule, this script grabs them from other places and copies them to the `thirdparty/external` directory.

    - Qt: from a source directory (check the script to change the path)
    - Pcap++: From the CMake-fetched _deps directory (CMake eCAL on Windows to fetch Pcap++)

## Sbom structure

Each thirdparty component needs to publish information about itself for being collected by the scripts. The `sbom.py` scripts are auto detected, so they only need to exist.

For adding new dependencies, it is easiest to copy and modify the sbom.py

```
thirdparty
│
├─ dependency1
│  └─ sbom.py   << Information about dependency 1
│
├─ dependency2
│  └─ sbom.py   << Information about dependency 2 
│
├─ external
│  ├─ sbom.py   << Information about dependenies that are not submodules
│  └─ license_files  << dir with copies of external license files
│
└─ ecal_license_utils.py  << utility functions for querying dependency information
```
