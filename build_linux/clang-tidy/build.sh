#!/bin/bash

################################################################################
#                                                                              #
# This script first runs cmake, and later may run make, and/or clang-tidy      #
# either on the compilation database or on selected files.                     #
#                                                                              #
# Check help with '-h|--help'.                                                 #
# The build path, which is relative to the current script's directory, can be  #
# given with the '-b|--build' option.                                          #
# The C and C++ compilers can be set to alternative compilers with the         #
# '-o|--compiler' option.                                                      #
# With the '-m|--make' option cmake and make runs.                             #
# With the '-a|--already' option cmake and make does not run, the build is     #
# already completed.                                                           #
# If not building now or not already built, still 'protoc' will be called to   #
# to enable running clang-tidy.                                                #
# With the '-i|--filter' option the paths to the filtering Python application  #
# and to its JSON configuration can be given.                                  #
# If the '-d|--database' option is given, it sets the RUN_DATABASE parameter   #
# ON.                                                                          #
# If the '-f|--files' option is given, the rest of the parameters are read as  #
# a list of files and the RUN_FILES parameter is set as ON.                    #
# If the RUN_DATABASE or RUN_FILES option is ON:                               #
#   - the 'compile_commands.json' file is filtered for inc/exc commands,       #
#   - the excluded commands are saved as 'compile_commands_exc.json',          #
#   - the original file is renamed as 'compile_commands_orig.json',            #
#   - the file of included commmands is renamed as 'compile_commands.json',    #
#   - clang-tidy configuration is dumped to a YAML file,                       #
#   - if required protobuf will be called.                                     #
# If the RUN_DATABASE option is ON:                                            #
#   - clang-tidy runs with the filtered 'compile_commands.json' database.      #
# If the RUN_FILES option is ON:                                               #
#   - file extensions are checked for relevance,                               #
#   - file paths are checked for excluded directories,                         #
#   - after these checks the files are analyzed by clang-tidy one-by-one.      #
# Outputs are redirected into timestamped log files, 'log_*.txt', in the build #
# directory as well.                                                           #
#                                                                              #
################################################################################

# exit at the first error
set -e

RUN_MAKE='OFF'
RUN_DATABASE='OFF'
RUN_FILES='OFF'
GEN_FILES='ON'

EXT_PATTERN='^(cpp|cppm|ixx|cxx|cc|c|h|hh|hxx|hpp)$'
PATH_BUILD='../../_build'
DIR_BUILD=
DIR_SCRIPT=
DIR_ROOT=
PATH_FILTER='filter_clang_tidy.py'
PATH_EXC_CONFIG='excludes_clang_tidy.json'
CMAKE_BUILD_TYPE='Release'
NUM_INST=4
DATE_TIME=$(date +"%Y-%m-%d_%H-%M-%S")
FILE_MAKE_OUTPUT="make_log_${DATE_TIME}.txt"
FILE_CLANG_TIDY_OUTPUT="clang_tidy_log_${DATE_TIME}.txt"
FILE_CLANG_TIDY_CONFIG='config_clang_tidy.yml'
# optionally specify with version number, for example: 'clang-tidy-14'
CLANG_TIDY='clang-tidy'
SEP_1=$(printf "%0.s=" {1..80})
SEP_2=$(printf "%0.s-" {1..80})
declare -a FILE_LIST
declare -a EXC_LIST

# leave empty for default values
DCMAKE_EXPORT_COMPILE_COMMANDS=   #'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'
DCMAKE_C_COMPILER=                #'-DCMAKE_C_COMPILER=/usr/bin/clang-14'
DCMAKE_CXX_COMPILER=              #'-DCMAKE_CXX_COMPILER=/usr/bin/clang++-14'

# ------------------------------------------------------------------------------

USAGE="$(basename "$0") [-h|-help] [-b|--build <build>] [-c|compiler <C> <CXX>] [-m|--make] [-i|--filter <app> <cfg>] [-d|--database] [-a|--already] [-f|--files <files...>]
run cmake, and then optionally make and/or clang-tidy - where:
    -h | --help                 show this help message and exit
    -b | --build <build>        build path relative to this script, default: '${PATH_BUILD}'
    -c | --compiler <C> <CXX>   set C & CXX compiler paths
    -m | --make                 run make
    -i | --filter <app> <cfg>   filtering app & its config rel. to this script or abs. paths, default: '${PATH_FILTER}' '${PATH_EXC_CONFIG}'
    -d | --database             run clang-tidy on the compilation database
    -a | --already              already built, all machine-generated codes are available
    -f | --files <files...>     run clang-tidy on the given files (remaining args)
"

if [[ $# -ge 1 ]]
then
    while true
    do
        case "$1" in
            -h | --help )       echo -e "${USAGE}" ; shift ; exit 0 ;;
            -b | --build )      if [[  $# -lt 2 ]];then echo "ERROR - missing build path arg" ; exit 1 ; fi ;
                                PATH_BUILD="$2" ; shift 2 ;
                                if [[  $# -eq 0 ]];then break ; fi ;;
            -c | --compiler )   if [[  $# -lt 3 ]];then echo "ERROR - missing compiler args" ; exit 1 ; fi ;
                                DCMAKE_C_COMPILER="-DCMAKE_C_COMPILER=$2" ;
                                DCMAKE_CXX_COMPILER="-DCMAKE_CXX_COMPILER=$3" ;
                                shift 3 ; if [[  $# -eq 0 ]];then break ; fi ;;
            -m | --make )       RUN_MAKE='ON' ; shift ;
                                if [[  $# -eq 0 ]];then break ; fi ;;
            -i | --filter )     if [[  $# -lt 3 ]];then echo "ERROR - missing filtering path arg" ; exit 1 ; fi ;
                                PATH_FILTER="$2" ; PATH_EXC_CONFIG="$3" ;
                                shift 3 ; if [[  $# -eq 0 ]];then break ; fi ;;
            -d | --database )   RUN_DATABASE='ON' ; shift ;
                                if [[  $# -eq 0 ]];then break ; fi ;;
            -a | --already )    GEN_FILES='OFF' ; shift ;
                                if [[  $# -eq 0 ]];then break ; fi ;;
            -f | --files )      RUN_FILES='ON' ; shift ;
                                if [[  $# -eq 0 ]];then echo "WARNING - missing file list" ; exit 0 ; fi ;
                                FILE_LIST=("$@") ; break ;;
            * )                 echo "ERROR - unknown option: '$1'" ; shift ; exit 1 ;;
        esac
    done
fi

# ------------------------------------------------------------------------------

find_root_dir() {
    local path=''

    while true
    do
        if [[ -d '.git' ]]
        then
            path=$(pwd)
            break
        else
            cd ..
        fi
    done

    echo "${path}"
}

check_args() {
    # cd to script's directory
    cd "${0%/*}"
    DIR_SCRIPT=$(pwd)

    # detect relative path by removing the final '/' and all trailing chars '*'
    local dir_rel=${PATH_BUILD%/*}
    local root_dir_git
    local path_clang_tidy

    DIR_BUILD_NAME=${PATH_BUILD#"${dir_rel}"/}
    cd "${dir_rel}"
    DIR_BUILD_ROOT=$(pwd)
    DIR_BUILD="${DIR_BUILD_ROOT}/${DIR_BUILD_NAME}"

    # find the root directory where '.git/' resides
    cd "${DIR_SCRIPT}"
    root_dir_git=$(find_root_dir)
    cd "${root_dir_git}"
    DIR_ROOT=$(pwd)

    if [[ "${RUN_DATABASE}" == 'ON' || "${RUN_FILES}" == 'ON' ]]
    then
        set +e
        path_clang_tidy=$(command -v ${CLANG_TIDY})
        set -e
        if [[ -z ${path_clang_tidy} ]]
        then
            echo -e "FATAL: ${CLANG_TIDY} is not available"
            exit 1
        fi
    fi

    if [[ "${RUN_DATABASE}" == 'ON' && "${RUN_FILES}" == 'ON' ]]
    then
        RUN_DATABASE='OFF'
    fi

    echo "++ run clang-tidy on the compilation database: ${RUN_DATABASE}"
    echo "++ run clang-tidy on the given files         : ${RUN_FILES}"

    if [[ "${RUN_DATABASE}" == 'ON' || "${RUN_FILES}" == 'ON' ]]
    then
        DCMAKE_EXPORT_COMPILE_COMMANDS='-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'
        ( set -x ; ${CLANG_TIDY} --version )
    fi
}

run_cmake() {
    if [[ "${GEN_FILES}" == 'ON' ]]
    then
        echo "++ build type: ${CMAKE_BUILD_TYPE}"
        echo -e "\n++ running cmake ..."

        rm -rf "${DIR_BUILD:?}/"
        mkdir -p "${DIR_BUILD}"
        cd "${DIR_BUILD}"

        cmake "${DIR_ROOT}" "${DCMAKE_EXPORT_COMPILE_COMMANDS}" \
                            "${DCMAKE_C_COMPILER}" \
                            "${DCMAKE_CXX_COMPILER}" \
                            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" \
                            -DECAL_THIRDPARTY_BUILD_PROTOBUF=OFF \
                            -DECAL_THIRDPARTY_BUILD_CURL=OFF  \
                            -DECAL_THIRDPARTY_BUILD_HDF5=OFF
    fi
}

run_make() {
    if [[ "${RUN_MAKE}" == 'ON' ]]
    then
        GEN_FILES='OFF'
        cd "${DIR_BUILD}"
        local cmd="cmake --build . -- -j${NUM_INST}"
        echo -e "\n++ ${cmd}\nsee: ${FILE_MAKE_OUTPUT}"
        echo "${cmd}" >> "${FILE_MAKE_OUTPUT}"
        time ${cmd} |& tee -a "${FILE_MAKE_OUTPUT}"
    fi
}

# searched for in the output of 'make --debug=b all'
run_protoc() {
    echo -e "\n++ running protoc ..."
    make -f ecal/pb/CMakeFiles/pb.dir/build.make ecal/pb/CMakeFiles/pb.dir/depend
    make -f samples/cpp/measurement/measurement_read/CMakeFiles/measurement_read.dir/build.make samples/cpp/measurement/measurement_read/CMakeFiles/measurement_read.dir/depend
    make -f samples/cpp/measurement/measurement_write/CMakeFiles/measurement_write.dir/build.make samples/cpp/measurement/measurement_write/CMakeFiles/measurement_write.dir/depend
    make -f samples/cpp/multiple/multiple_rec_cb/CMakeFiles/multiple_rec_cb.dir/build.make samples/cpp/multiple/multiple_rec_cb/CMakeFiles/multiple_rec_cb.dir/depend
    make -f samples/cpp/multiple/multiple_snd/CMakeFiles/multiple_snd.dir/build.make samples/cpp/multiple/multiple_snd/CMakeFiles/multiple_snd.dir/depend
    make -f samples/cpp/person/person_rec/CMakeFiles/person_rec.dir/build.make samples/cpp/person/person_rec/CMakeFiles/person_rec.dir/depend
    make -f samples/cpp/person/person_rec_events/CMakeFiles/person_rec_events.dir/build.make samples/cpp/person/person_rec_events/CMakeFiles/person_rec_events.dir/depend
    make -f samples/cpp/person/person_rec_lambda_in_class/CMakeFiles/person_rec_lambda_in_class.dir/build.make samples/cpp/person/person_rec_lambda_in_class/CMakeFiles/person_rec_lambda_in_class.dir/depend
    make -f samples/cpp/person/person_snd/CMakeFiles/person_snd.dir/build.make samples/cpp/person/person_snd/CMakeFiles/person_snd.dir/depend
    make -f samples/cpp/person/person_snd_dyn/CMakeFiles/person_snd_dyn.dir/build.make samples/cpp/person/person_snd_dyn/CMakeFiles/person_snd_dyn.dir/depend
    make -f samples/cpp/person/person_snd_events/CMakeFiles/person_snd_events.dir/build.make samples/cpp/person/person_snd_events/CMakeFiles/person_snd_events.dir/depend
    make -f samples/cpp/person/person_snd_inproc/CMakeFiles/person_snd_inproc.dir/build.make samples/cpp/person/person_snd_inproc/CMakeFiles/person_snd_inproc.dir/depend
    make -f samples/cpp/person/person_snd_multicast/CMakeFiles/person_snd_multicast.dir/build.make samples/cpp/person/person_snd_multicast/CMakeFiles/person_snd_multicast.dir/depend
    make -f samples/cpp/person/person_snd_tcp/CMakeFiles/person_snd_tcp.dir/build.make samples/cpp/person/person_snd_tcp/CMakeFiles/person_snd_tcp.dir/depend
    make -f samples/cpp/services/math_client/CMakeFiles/math_client.dir/build.make samples/cpp/services/math_client/CMakeFiles/math_client.dir/depend
    make -f samples/cpp/services/math_server/CMakeFiles/math_server.dir/build.make samples/cpp/services/math_server/CMakeFiles/math_server.dir/depend
    make -f samples/cpp/services/ping_client/CMakeFiles/ping_client.dir/build.make samples/cpp/services/ping_client/CMakeFiles/ping_client.dir/depend
    make -f samples/cpp/services/ping_server/CMakeFiles/ping_server.dir/build.make samples/cpp/services/ping_server/CMakeFiles/ping_server.dir/depend
}

filter_compile_commands() {
    if [[ "${RUN_DATABASE}" == 'ON' || "${RUN_FILES}" == 'ON' ]]
    then
        echo -e "\n++ filtering the compile commands ..."
        cd "${DIR_SCRIPT}"
        echo "excluded directories:"
        cat "$PATH_EXC_CONFIG"
        python3 "${PATH_FILTER}" --build "${DIR_BUILD}"

        cp -a "${DIR_ROOT}/.clang-tidy" "${DIR_BUILD}"
        cd "${DIR_BUILD}"
        ${CLANG_TIDY} --dump-config > ${FILE_CLANG_TIDY_CONFIG}

        # use the included commands as compile commands
        mv compile_commands.json compile_commands_orig.json
        mv compile_commands_inc.json compile_commands.json

        # for protobuf generated source & header files, protoc is fast
        # if built with this script (-m) or built already (-a), then there is no need to call protoc
        if [[ "${GEN_FILES}" == 'ON' ]]
        then
            run_protoc
        fi
    fi
}

run_clang_tidy_on_database() {
    if [[ "${RUN_DATABASE}" == 'ON' ]]
    then
        # see: run-clang-tidy --help
        local cmd="run-${CLANG_TIDY} -j${NUM_INST}"
        echo -e "\n++ ${cmd}\ncfg: ${FILE_CLANG_TIDY_CONFIG}\nsee: ${FILE_CLANG_TIDY_OUTPUT}"
        echo "${cmd}" >> "${FILE_CLANG_TIDY_OUTPUT}"
        time ${cmd} |& tee -a "${FILE_CLANG_TIDY_OUTPUT}"
    fi
}

# ASSUMPTION: JSON is well-formed and pretty-printed.
# if empty:
#     []
# else (for example):
#     [
#         ["a", "b"],
#         ["c"]
#     ]
read_config_basic() {
    local line_count
    local line_trim
    local line_no_quotes
    local line_no_brackets
    local subdir

    cd "${DIR_SCRIPT}"
    if [[ -f "$PATH_EXC_CONFIG" ]]
    then
        line_count=$(wc -l < "$PATH_EXC_CONFIG")
        if [[ "${line_count}" -gt "1" ]]
        then
            declare -a exc_dirs
            while IFS= read -r line
            do
                exc_dirs[${#exc_dirs[@]}]="$line"
            done < "$PATH_EXC_CONFIG"
            # eliminate the first and the last element, '[' and ']' respectively
            unset 'exc_dirs[0]'
            unset 'exc_dirs[-1]'
            for exc_dir in "${exc_dirs[@]}"
            do
                line_trim=$(echo "${exc_dir}" | tr -d '[:space:]')
                line_no_quotes=$(echo "${line_trim}" | tr -d '"')
                line_no_brackets=$(echo "${line_no_quotes}" | tr -d '[]')
                declare -a dirs
                IFS=',' read -ar dirs <<< "${line_no_brackets}"
                subdir=$(printf "/%s" "${dirs[@]}")
                EXC_LIST[${#EXC_LIST[@]}]="${subdir}/"
            done
        else
            echo "WARNING: JSON for exclusion list is empty"
        fi
    else
        echo "FATAL: config file does not exist"
        exit 1
    fi
}

# jq can parse valid JSONs, pretty-print is not required.
read_config() {
    local path_jq
    local subdir

    cd "${DIR_SCRIPT}"
    set +e
    path_jq=$(command -v jq)
    set -e
    if [[ -z ${path_jq} ]]
    then
        echo -e "WARNING: 'jq' is not available, fallig back to the simple Bash JSON parser"
        read_config_basic
    else
        if [[ -f "$PATH_EXC_CONFIG" ]]
        then
            for row in $(jq -c '.[]' < "${PATH_EXC_CONFIG}")
            do
                subdir=''
                for dir in $(echo "${row}" | jq -r '.[]')
                do
                    subdir="${subdir}/${dir}"
                done
                EXC_LIST[${#EXC_LIST[@]}]="${subdir}/"
            done
        else
            echo "FATAL: config file does not exist"
            exit 1
        fi
    fi
}

is_excluded() {
    local path="$1"
    for subdir in "${EXC_LIST[@]}"
    do
        if [[ "${path}" =~ .*"${subdir}".* ]]
        then
            true
            return
        fi
    done
    false
    return
}

run_clang_tidy_on_files() {
    if [[ "${RUN_FILES}" == 'ON' ]]
    then
        local file_abs
        local file_char
        local file_base
        local file_name
        local file_ext

        read_config

        cd "${DIR_BUILD}"
        # see: clang-tidy --help
        echo -e "\n++ analyzing source files by clang-tidy ..."
        printf -- "-- %s\n" "${FILE_LIST[@]}"
        echo

        for file in "${FILE_LIST[@]}"
        do
            file_abs="${file}"
            file_char="${file_abs:0:1}"

            if [[ ${file_char} != '/' ]]
            then
                file_abs="${DIR_ROOT}/${file}"
            fi

            file_base=$(basename "${file_abs}")
            file_name="${file_base%.*}"
            file_ext="${file_base##*.}"

            echo "${SEP_1}"
            echo -e "-- ${file}\n   ${file_name} . ${file_ext}"

            # existing?
            if [[ -f "${file_abs}" ]]
            then
                # if 1. the file is by its extension a C++-related one
                #    2. its path does not contain any excluded directories
                # then analyze it by clang-tidy
                if [[ "${file_ext}" =~ $EXT_PATTERN ]]
                then
                    if is_excluded "${file_abs}"
                    then
                        echo "## excluded"
                    else
                        # if 'compile_commands.json' is unreachable, clang-tidy gives the following error:
                        # "Error while trying to load a compilation database: ..."
                        local cmd="${CLANG_TIDY} -p . ${file_abs}"
                        echo "## analyzing"
                        echo "${SEP_2}"
                        echo "${cmd}" |& tee -a "${FILE_CLANG_TIDY_OUTPUT}"
                        ${cmd} |& tee -a "${FILE_CLANG_TIDY_OUTPUT}"
                        echo "${SEP_2}" >> "${FILE_CLANG_TIDY_OUTPUT}"
                    fi
                else
                    echo "## skipping"
                fi
            else
                echo "## not existing"
            fi
        done
        echo "${SEP_1}"
    fi
}

# ------------------------------------------------------------------------------

check_args
run_cmake
run_make
filter_compile_commands
run_clang_tidy_on_database
run_clang_tidy_on_files

# for GitHub actions
# https://docs.github.com/en/actions/using-workflows/workflow-commands-for-github-actions
echo "::set-output name=timestamp::${DATE_TIME}"

echo -e "\n++ completed"
