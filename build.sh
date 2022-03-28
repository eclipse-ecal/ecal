#!/bin/bash

################################################################################
#                                                                              #
# This script runs the build and optionally clang-tidy.                        #
#                                                                              #
# Check help with '-h|--help'.                                                 #
# If '-c|--clang-tidy' arg is given, it sets the run-clang-tidy parameter ON.  #
# The default value of the run-clang-tidy parameter is OFF.                    #
# If the run-clang-tidy parameter is ON:                                       #
#   - the 'compile_commands.json' file be filtered for inc/exc source files,   #
#   - the excluded commands will be saved as 'compile_commands_exc.json',      #
#   - the original file will be renamed as 'compile_commands_orig.json',       #
#   - build type will be set as 'Debug',                                       #
#   - clang-tidy configuration is dumped to 'config_clang_tidy.yaml',          #
#   - clang-tidy runs with the filtered 'compile_commands.json'.               #
# The config file to set excluded dirs is 'excludes_clang_tidy.json'.          #
# The C and C++ compilers can be set to alternative compilers with the         #
# '-o|--compiler' option.                                                      #
#                                                                              #
################################################################################

# exit at the first error
set -e

RUN_CLANG_TIDY='OFF'
CMAKE_BUILD_TYPE='Release'
DIR_BUILD='_build'
NUM_INST=4
DATE_TIME=$(date +"%Y-%m-%d_%H-%M-%S")
FILE_MAKE_OUTPUT="log_make_${DATE_TIME}.txt"
FILE_CLANG_TIDY_OUTPUT="log_clang_tidy_${DATE_TIME}.txt"
FILE_CLANG_TIDY_CONFIG='config_clang_tidy.yaml'
# leave empty for the default compiler
DCMAKE_C_COMPILER=    #"-DCMAKE_C_COMPILER=/usr/bin/clang-14"
DCMAKE_CXX_COMPILER=  #"-DCMAKE_CXX_COMPILER=/usr/bin/clang++-14"

# ------------------------------------------------------------------------------

USAGE="$(basename $0) [-h|-help] [-c|--clang-tidy] [-o|compiler C CXX]
run the build scripts - where:
    -h | --help             show this help message and exit
    -c | --clang-tidy       launch clang-tidy
    -o | --compiler C CXX   set C & CXX compiler paths
"

if [[ $# -ge 1 ]]
then
    while true
    do
        case "$1" in
            -h | --help )       echo -e "${USAGE}" ; shift ; exit 0 ;;
            -c | --clang-tidy ) RUN_CLANG_TIDY='ON' ; shift ;
                                if [[  $# -eq 0 ]];then break ; fi ;;
            -o | --compiler )   if [[  $# -lt 3 ]];then echo "ERROR - missing compiler args"; exit 1 ; fi ;
                                DCMAKE_C_COMPILER="-DCMAKE_C_COMPILER=$2" ;
                                DCMAKE_CXX_COMPILER="-DCMAKE_CXX_COMPILER=$3" ;
                                shift 3 ; if [[  $# -eq 0 ]];then break ; fi ;;
            * )                 echo "ERROR - unknown option: '$1'" ; shift ; exit 1 ;;
        esac
    done
fi

if [[ "${RUN_CLANG_TIDY}" == 'ON' ]]
then
    CLANG_TIDY=$(which clang-tidy)
    if [[ -z ${CLANG_TIDY} ]]
    then
        echo -e "WARNING: clang-tidy is not available"
        RUN_CLANG_TIDY='OFF'
    fi
fi

# ------------------------------------------------------------------------------

echo "++ clang-tidy: ${RUN_CLANG_TIDY}"
if [[ "${RUN_CLANG_TIDY}" == 'ON' ]]
then
    CMAKE_BUILD_TYPE='Debug'
    clang-tidy --version
fi
echo "++ build type: ${CMAKE_BUILD_TYPE}"
echo -e "\n++ running cmake ..."

rm -rf "${DIR_BUILD}/"
mkdir ${DIR_BUILD}
cd "${DIR_BUILD}/"

cmake .. ${DCMAKE_C_COMPILER} \
         ${DCMAKE_CXX_COMPILER} \
         -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
         -DECAL_THIRDPARTY_BUILD_PROTOBUF=OFF \
         -DECAL_THIRDPARTY_BUILD_CURL=OFF  \
         -DECAL_THIRDPARTY_BUILD_HDF5=OFF

echo -e "\n++ running make ...\nsee: ${FILE_MAKE_OUTPUT}"
time make -j${NUM_INST} &> ${FILE_MAKE_OUTPUT}

if [[ "${RUN_CLANG_TIDY}" == 'ON' ]]
then
    echo -e "\n++ filtering the compile commands ..."
    cd ..
    echo "excluded directories:"
    cat excludes_clang_tidy.json
    python3 filter_clang_tidy.py
    cd "${DIR_BUILD}/"
    # use the included commands as compile commands
    mv compile_commands.json compile_commands_orig.json
    mv compile_commands_inc.json compile_commands.json

    #see: clang-tidy --help
    #see: run-clang-tidy --help
    echo -e "\n++ running clang-tidy ...\ncfg: ${FILE_CLANG_TIDY_CONFIG}\nsee: ${FILE_CLANG_TIDY_OUTPUT}"
    clang-tidy --dump-config > ${FILE_CLANG_TIDY_CONFIG}
    time run-clang-tidy -j${NUM_INST} &> ${FILE_CLANG_TIDY_OUTPUT}
fi

echo -e "\n++ completed"
