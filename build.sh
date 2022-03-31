#!/bin/bash

################################################################################
#                                                                              #
# This script generates build scripts, and may run make and clang-tidy.        #
#                                                                              #
# Check help with '-h|--help'.                                                 #
# The C and C++ compilers can be set to alternative compilers with the         #
# '-o|--compiler' option.                                                      #
# After running cmake, with '-m|--make', make runs.                            #
# If the '-d|--database' arg is given, it sets the RUN_DATABASE parameter ON.  #
# If the RUN_DATABASE parameter is ON, the compile commands database is        #
# filtered first:                                                              #
#   - the 'compile_commands.json' file be filtered for inc/exc commands,       #
#   - the excluded commands are saved as 'compile_commands_exc.json',          #
#   - the original file is renamed as 'compile_commands_orig.json',            #
#   - the file of included commmands is renamed as 'compile_commands.json'.    #
#   - clang-tidy configuration is dumped to 'config_clang_tidy.yaml',          #
#   - clang-tidy runs with the filtered 'compile_commands.json'.               #
# Outputs are redirected into timestamped log files, 'log_*.txt', in the build #
# directory as well.                                                           #
#                                                                              #
################################################################################

# exit at the first error
set -e

RUN_MAKE='OFF'
RUN_DATABASE='OFF'

CMAKE_BUILD_TYPE='Release'
DIR_BUILD='_build'
NUM_INST=4
DATE_TIME=$(date +"%Y-%m-%d_%H-%M-%S")
FILE_MAKE_OUTPUT="log_make_${DATE_TIME}.txt"
FILE_CLANG_TIDY_OUTPUT="log_clang_tidy_${DATE_TIME}.txt"
FILE_CLANG_TIDY_CONFIG='config_clang_tidy.yaml'

# leave empty for default values
DCMAKE_EXPORT_COMPILE_COMMANDS=   #'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'
DCMAKE_C_COMPILER=                #'-DCMAKE_C_COMPILER=/usr/bin/clang-14'
DCMAKE_CXX_COMPILER=              #'-DCMAKE_CXX_COMPILER=/usr/bin/clang++-14'

# ------------------------------------------------------------------------------

USAGE="$(basename $0) [-h|-help] [-o|compiler <C> <CXX>] [-m|--make] [-d|--database]
run cmake, and then optionally make and/or clang-tidy - where:
    -h | --help                 show this help message and exit
    -o | --compiler <C> <CXX>   set C & CXX compiler paths
    -m | --make                 run make
    -d | --database             run clang-tidy on the compilation database
"

if [[ $# -ge 1 ]]
then
    while true
    do
        case "$1" in
            -h | --help )       echo -e "${USAGE}" ; shift ; exit 0 ;;
            -o | --compiler )   if [[  $# -lt 3 ]];then echo "ERROR - missing compiler args" ; exit 1 ; fi ;
                                DCMAKE_C_COMPILER="-DCMAKE_C_COMPILER=$2" ;
                                DCMAKE_CXX_COMPILER="-DCMAKE_CXX_COMPILER=$3" ;
                                shift 3 ; if [[  $# -eq 0 ]];then break ; fi ;;
            -m | --make )       RUN_MAKE='ON' ; shift ;
                                if [[  $# -eq 0 ]];then break ; fi ;;
            -d | --database )   RUN_DATABASE='ON' ; shift ;
                                if [[  $# -eq 0 ]];then break ; fi ;;
            * )                 echo "ERROR - unknown option: '$1'" ; shift ; exit 1 ;;
        esac
    done
fi

if [[ "${RUN_DATABASE}" == 'ON' ]]
then
    CLANG_TIDY=$(which clang-tidy)
    if [[ -z ${CLANG_TIDY} ]]
    then
        echo -e "WARNING: clang-tidy is not available"
        RUN_DATABASE='OFF'
    fi
fi

echo "++ run clang-tidy on the compilation database: ${RUN_DATABASE}"

if [[ "${RUN_DATABASE}" == 'ON' ]]
then
    DCMAKE_EXPORT_COMPILE_COMMANDS='-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'
    clang-tidy --version
fi

# ------------------------------------------------------------------------------

# run cmake always
echo "++ build type: ${CMAKE_BUILD_TYPE}"
echo -e "\n++ running cmake ..."

rm -rf "${DIR_BUILD}/"
mkdir ${DIR_BUILD}
cd "${DIR_BUILD}/"

cmake .. ${DCMAKE_EXPORT_COMPILE_COMMANDS} \
         ${DCMAKE_C_COMPILER} \
         ${DCMAKE_CXX_COMPILER} \
         -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
         -DECAL_THIRDPARTY_BUILD_PROTOBUF=OFF \
         -DECAL_THIRDPARTY_BUILD_CURL=OFF  \
         -DECAL_THIRDPARTY_BUILD_HDF5=OFF

if [[ "${RUN_MAKE}" == 'ON' ]]
then
    echo -e "\n++ running make -j${NUM_INST} ...\nsee: ${FILE_MAKE_OUTPUT}"
    echo "make -j${NUM_INST}" >> ${FILE_MAKE_OUTPUT}
    time make -j${NUM_INST} |& tee -a ${FILE_MAKE_OUTPUT}
fi

if [[ "${RUN_DATABASE}" == 'ON' ]]
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

    # see: clang-tidy --help
    # see: run-clang-tidy --help
    echo -e "\n++ running clang-tidy -j${NUM_INST} ...\ncfg: ${FILE_CLANG_TIDY_CONFIG}\nsee: ${FILE_CLANG_TIDY_OUTPUT}"
    clang-tidy --dump-config > ${FILE_CLANG_TIDY_CONFIG}
    echo "run-clang-tidy -j${NUM_INST}" >> ${FILE_CLANG_TIDY_OUTPUT}
    time run-clang-tidy -j${NUM_INST} |& tee -a ${FILE_CLANG_TIDY_OUTPUT}
fi

echo -e "\n++ completed"
