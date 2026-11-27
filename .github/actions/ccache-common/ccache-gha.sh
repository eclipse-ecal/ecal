#!/usr/bin/env bash
# ========================= eCAL LICENSE =================================
#
# Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================
#
# Helper functions shared by the ccache-prepare and ccache-save composite actions.
#
# Caching strategy:
#   full         Push to the default branch or support/*: the complete cache is saved and
#                older full caches of the same branch are deleted afterwards.
#   delta        Pull request: the full cache of the base branch is restored, then the
#                PR's delta on top. Only entries that are new or changed compared to the
#                base are saved, so the delta stays small and never stacks up.
#   restore-only Anything else (tags, workflow_dispatch, ...): restores the default branch
#                cache, never saves.
# In full and delta mode, nothing is saved if the job produced fewer than MIN_NEW_ENTRIES
# new results compared to the restored cache (base + delta).

set -euo pipefail
export LC_ALL=C

CCACHE_PINNED_VERSION="4.14.1"

log()  { echo "ccache-gha: $*"; }
warn() { echo "::warning::ccache-gha: $*"; }

sha256_check() { # <sha256> <file>
  if command -v sha256sum >/dev/null 2>&1; then
    echo "$1  $2" | sha256sum -c -
  else
    echo "$1  $2" | shasum -a 256 -c -
  fi
}

set_output() { echo "$1=$2" >> "${GITHUB_OUTPUT}"; }
set_env()    { echo "$1=$2" >> "${GITHUB_ENV}"; }

# Prints one line per cache entry: "<path><TAB><fingerprint>", sorted.
# Result files (*R) are content addressed by their name, so their fingerprint is "-".
# All other files (manifests, ...) can change in place and are fingerprinted by content.
list_entries() { # <cache dir>
  local dir="$1"
  [ -d "${dir}" ] || return 0
  local sha1="sha1sum"
  command -v sha1sum >/dev/null 2>&1 || sha1="shasum"
  (
    cd "${dir}"
    local filter=(-type f ! -name stats ! -name '*.lock' ! -name ccache.conf ! -name CACHEDIR.TAG ! -path './tmp/*')
    find . "${filter[@]}" -name '*R' | awk '{ print $0 "\t-" }'
    find . "${filter[@]}" ! -name '*R' -exec "${sha1}" {} + \
      | awk '{ h = $1; sub(/^[0-9a-f]+ [ *]/, ""); print $0 "\t" h }'
  ) | sort
}

count_results() { # <list file>
  grep -c $'\t-$' "$1" || true
}

# ---------------------------------------------------------------------------------------

# Installs a pinned, checksum verified ccache release into <dir> and adds it to the PATH.
cmd_install() { # <dir>
  local dir="$1" v="${CCACHE_PINNED_VERSION}" name ext sha
  case "${RUNNER_OS}-${RUNNER_ARCH}" in
    Linux-X64)   name="ccache-${v}-linux-x86_64-musl-static";  ext="tar.xz"; sha="fa1443c9fbea879c9bff6619d1284eda3102992e91831b9f6e546879972195cc" ;;
    Linux-ARM64) name="ccache-${v}-linux-aarch64-musl-static"; ext="tar.xz"; sha="265d687c6d84a96cef851b57f804b21fc8268e53bfbc468441d029a5f7000a10" ;;
    macOS-*)     name="ccache-${v}-darwin";                    ext="tar.gz"; sha="c279fa81e2e806b4d9e64d4a06cb569a84ef54151feef796e5d4f4e95a464563" ;;
    Windows-X64) name="ccache-${v}-windows-x86_64";            ext="zip";    sha="6219f3865ca59aec41ee4b678df171d5d35855ecb2b6dbbbd20690b3a68af7b4" ;;
    *) echo "::error::ccache-gha: no pinned ccache build for ${RUNNER_OS}-${RUNNER_ARCH}"; exit 1 ;;
  esac

  local archive="${RUNNER_TEMP}/${name}.${ext}"
  curl -fsSL --retry 3 -o "${archive}" "https://github.com/ccache/ccache/releases/download/v${v}/${name}.${ext}"
  sha256_check "${sha}" "${archive}"

  mkdir -p "${dir}"
  if [ "${ext}" = "zip" ]; then
    if command -v unzip >/dev/null 2>&1; then
      unzip -q -o -j "${archive}" "${name}/ccache.exe" -d "${dir}"
    else
      7z e -y -o"${dir}" "${archive}" "${name}/ccache.exe" >/dev/null
    fi
  else
    tar -xf "${archive}" -C "${dir}" --strip-components=1 "${name}/ccache"
  fi
  rm -f "${archive}"

  echo "${dir}" >> "${GITHUB_PATH}"
  set_output bin-dir "${dir}"
  "${dir}/ccache" --version | head -n 1
}

# Determines the caching mode and cache keys, and exports the ccache environment.
# Inputs (env): KEY_ROOT, CACHE_DIR, MIN_NEW_ENTRIES, DEFAULT_BRANCH, PR_NUMBER
cmd_setup() {
  local mode base
  if [ "${GITHUB_EVENT_NAME}" = "push" ] \
     && { [ "${GITHUB_REF}" = "refs/heads/${DEFAULT_BRANCH}" ] || [[ "${GITHUB_REF}" == refs/heads/support/* ]]; }; then
    mode="full";  base="${GITHUB_REF_NAME}"
  elif [ "${GITHUB_EVENT_NAME}" = "pull_request" ]; then
    mode="delta"; base="${GITHUB_BASE_REF}"
  else
    mode="restore-only"; base="${DEFAULT_BRANCH}"
  fi

  local suffix="${GITHUB_RUN_ID}-${GITHUB_RUN_ATTEMPT}"
  local full_prefix="${KEY_ROOT}-ccache-full-${base}-"
  local default_prefix="${KEY_ROOT}-ccache-full-${DEFAULT_BRANCH}-"
  local delta_prefix="${KEY_ROOT}-ccache-delta-pr${PR_NUMBER:-none}-"
  local state_dir="${RUNNER_TEMP}/ccache-gha"
  local delta_dir="${RUNNER_TEMP}/ccache-delta"

  local save_prefix=""
  case "${mode}" in
    full)  save_prefix="${full_prefix}" ;;
    delta) save_prefix="${delta_prefix}" ;;
  esac

  mkdir -p "${state_dir}" "${CACHE_DIR}"

  set_output mode           "${mode}"
  set_output full-key       "${full_prefix}${suffix}"
  set_output full-prefix    "${full_prefix}"
  set_output default-prefix "${default_prefix}"
  set_output delta-key      "${delta_prefix}${suffix}"
  set_output delta-prefix   "${delta_prefix}"
  set_output delta-path     "${delta_dir}"

  # Consumed by ccache-save
  set_env CCACHE_GHA_MODE        "${mode}"
  set_env CCACHE_GHA_SAVE_PREFIX "${save_prefix}"
  set_env CCACHE_GHA_SAVE_KEY    "${save_prefix}${suffix}"
  set_env CCACHE_GHA_CACHE_PATH  "${CACHE_DIR}"
  set_env CCACHE_GHA_DELTA_PATH  "${delta_dir}"
  set_env CCACHE_GHA_STATE_DIR   "${state_dir}"
  set_env CCACHE_GHA_MIN_NEW     "${MIN_NEW_ENTRIES}"

  # ccache / CMake environment for the build
  set_env CCACHE_DIR                  "${CACHE_DIR}"
  set_env CCACHE_BASEDIR              "${GITHUB_WORKSPACE}"
  set_env CCACHE_LOGFILE              "${RUNNER_TEMP}/ccache.log"
  set_env CCACHE_JOB_START            "$(date +%s)"
  set_env CMAKE_C_COMPILER_LAUNCHER   "ccache"
  set_env CMAKE_CXX_COMPILER_LAUNCHER "ccache"

  log "mode=${mode} base=${base} save-prefix=${save_prefix:-<none>}"
}

# Stores the settings in the cache's own ccache.conf, so they also apply to ccache
# running inside containers that mount the cache directory.
# Inputs (env): CACHE_DIR, MAX_SIZE
cmd_configure() {
  mkdir -p "${CACHE_DIR}"
  local setting
  for setting in "max_size=${MAX_SIZE}" "compression=true" "compression_level=6" "compiler_check=content"; do
    ccache -d "${CACHE_DIR}" -o "${setting}"
  done
}

# Records the entries of the restored base cache, so ccache-save can compute what changed.
# Inputs (env): CACHE_DIR
cmd_snapshot() {
  local state_dir="${RUNNER_TEMP}/ccache-gha"
  mkdir -p "${state_dir}"
  list_entries "${CACHE_DIR}" > "${state_dir}/base.list"
  log "restored base cache: $(count_results "${state_dir}/base.list") results, $(wc -l < "${state_dir}/base.list" | tr -d ' ') files"
}

# Merges a restored delta into the cache directory.
# Inputs (env): CACHE_DIR, DELTA_DIR
cmd_apply_delta() {
  if [ ! -d "${DELTA_DIR}" ] || [ -z "$(ls -A "${DELTA_DIR}" 2>/dev/null)" ]; then
    log "no delta restored"
    return 0
  fi
  cp -a "${DELTA_DIR}/." "${CACHE_DIR}/"
  log "applied delta: $(find "${DELTA_DIR}" -type f | wc -l | tr -d ' ') files"
  rm -rf "${DELTA_DIR}"
  # Recalculate size and file counters after adding files behind ccache's back
  ccache -d "${CACHE_DIR}" --cleanup >/dev/null
  list_entries "${CACHE_DIR}" > "${RUNNER_TEMP}/ccache-gha/restored.list"
}

# Builds running in containers (e.g. manylinux) write the mounted cache as root.
cmd_fix_ownership() {
  [ "${RUNNER_OS}" = "Linux" ] || return 0
  if [ -n "$(find "${CCACHE_GHA_CACHE_PATH}" ! -user "$(id -u)" -print -quit 2>/dev/null)" ]; then
    log "fixing ownership of ${CCACHE_GHA_CACHE_PATH}"
    sudo chown -R "$(id -u):$(id -g)" "${CCACHE_GHA_CACHE_PATH}"
  fi
}

# Keeps only entries used (hit or written) during this job.
cmd_evict() {
  local age=3600
  if [ -n "${CCACHE_JOB_START:-}" ]; then
    age=$(( $(date +%s) - CCACHE_JOB_START + 300 ))
  fi
  ccache --evict-older-than "${age}s"
}

cmd_stats() {
  ccache --show-stats --verbose
  ccache --zero-stats
  if [ -f "${CCACHE_LOGFILE:-}" ]; then
    echo "Most frequent reasons for uncacheable calls:"
    grep -hiE "unsupported|not supported|too hard" "${CCACHE_LOGFILE}" \
      | sed -E 's/^\[[^]]*\] *//' \
      | grep -vE "^(Command line|Source file|Object file|Detected input file|Writing to)" \
      | sort | uniq -c | sort -rn | head -n 20 || true
    rm -f "${CCACHE_LOGFILE}"
  fi
}

# Compares the cache with the restored state and decides whether saving is worthwhile:
# at least MIN_NEW_ENTRIES new results compared to what was restored (base + delta).
# In delta mode, stages all entries that are new or changed compared to the base.
cmd_diff() {
  local mode="${CCACHE_GHA_MODE}"
  local state_dir="${CCACHE_GHA_STATE_DIR}"
  local base_list="${state_dir}/base.list"
  local restored_list="${state_dir}/restored.list"
  local current_list="${state_dir}/current.list"
  local changed_list="${state_dir}/changed.list"

  [ -f "${base_list}" ] || : > "${base_list}"
  [ -f "${restored_list}" ] || restored_list="${base_list}"
  list_entries "${CCACHE_GHA_CACHE_PATH}" > "${current_list}"
  comm -13 "${base_list}" "${current_list}" > "${changed_list}"

  local new_results changed_files
  new_results=$(comm -13 "${restored_list}" "${current_list}" | grep -c $'\t-$' || true)
  changed_files=$(wc -l < "${changed_list}" | tr -d ' ')
  log "${new_results} new results in this job, ${changed_files} files differ from the base cache"

  local save="true"
  if [ "${mode}" = "restore-only" ]; then
    log "mode restore-only: not saving"
    save="false"
  elif [ "${new_results}" -lt "${CCACHE_GHA_MIN_NEW}" ]; then
    log "fewer than ${CCACHE_GHA_MIN_NEW} new results: not saving"
    save="false"
  fi

  local path="${CCACHE_GHA_CACHE_PATH}"
  if [ "${mode}" = "delta" ]; then
    path="${CCACHE_GHA_DELTA_PATH}"
    rm -rf "${path}"
    mkdir -p "${path}"
    if [ "${save}" = "true" ]; then
      ( cd "${CCACHE_GHA_CACHE_PATH}" && cut -f1 "${changed_list}" | tar -cf - -T - ) | tar -xf - -C "${path}"
      log "delta size: $(du -sh "${path}" | cut -f1)"
    fi
  fi

  set_output save "${save}"
  set_output path "${path}"
}

# Deletes older caches with the same prefix on the current ref, keeping the one just saved.
# actions/cache/save only warns if the upload fails, so nothing is deleted unless the new
# cache is actually listed on this ref.
# Inputs (env): GH_TOKEN
cmd_cleanup() {
  local entries id key found="false" old_ids=()
  if ! entries=$(gh cache list --repo "${GITHUB_REPOSITORY}" --ref "${GITHUB_REF}" --key "${CCACHE_GHA_SAVE_PREFIX}" \
                   --limit 100 --json id,key --jq '.[] | "\(.id)\t\(.key)"'); then
    warn "could not list old caches (missing 'actions: write' permission?)"
    return 0
  fi
  while IFS=$'\t' read -r id key; do
    [ -n "${id}" ] || continue
    if [ "${key}" = "${CCACHE_GHA_SAVE_KEY}" ]; then
      found="true"
    else
      old_ids+=("${id}")
    fi
  done <<< "${entries}"
  if [ "${found}" != "true" ]; then
    warn "saved cache ${CCACHE_GHA_SAVE_KEY} not found on ${GITHUB_REF}, keeping older caches"
    return 0
  fi
  for id in ${old_ids[@]+"${old_ids[@]}"}; do
    if gh cache delete "${id}" --repo "${GITHUB_REPOSITORY}" >/dev/null; then
      log "deleted old cache ${id}"
    else
      warn "could not delete old cache ${id}"
    fi
  done
}

command="${1:-}"
shift || true
case "${command}" in
  install)       cmd_install "$@" ;;
  setup)         cmd_setup ;;
  configure)     cmd_configure ;;
  snapshot)      cmd_snapshot ;;
  apply-delta)   cmd_apply_delta ;;
  fix-ownership) cmd_fix_ownership ;;
  evict)         cmd_evict ;;
  stats)         cmd_stats ;;
  diff)          cmd_diff ;;
  cleanup)       cmd_cleanup ;;
  *) echo "Usage: $0 <install|setup|configure|snapshot|apply-delta|fix-ownership|evict|stats|diff|cleanup>" >&2; exit 2 ;;
esac
