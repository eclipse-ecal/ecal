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

"""Helper for the ccache-prepare and ccache-save composite actions.

Caching strategy:
  full          Push to the default branch or support/*: the complete cache is saved and
                older full caches of the same branch are deleted afterwards.
  delta         Pull request: the full cache of the base branch is restored, then the
                PR's delta on top. Only entries that are new or changed compared to the
                base are saved, so the delta stays small and never stacks up.
  restore-only  Anything else (tags, workflow_dispatch, ...): restores the default
                branch cache, never saves.

In full and delta mode, nothing is saved if the job produced fewer than MIN_NEW_ENTRIES
new cache entries compared to the restored cache (base + delta).

Only the Python standard library is used. External tools: ccache, and gh for cleanup.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import io
import json
import os
import re
import shutil
import subprocess
import sys
import tarfile
import time
import urllib.request
import zipfile
from pathlib import Path
from typing import Dict, List, Optional

CCACHE_VERSION = "4.14.1"

# (archive name, sha256) of the pinned ccache release per runner platform
CCACHE_DOWNLOADS = {
    ("Linux", "X64"): (
        f"ccache-{CCACHE_VERSION}-linux-x86_64-musl-static.tar.xz",
        "fa1443c9fbea879c9bff6619d1284eda3102992e91831b9f6e546879972195cc",
    ),
    ("Linux", "ARM64"): (
        f"ccache-{CCACHE_VERSION}-linux-aarch64-musl-static.tar.xz",
        "265d687c6d84a96cef851b57f804b21fc8268e53bfbc468441d029a5f7000a10",
    ),
    ("macOS", "X64"): (
        f"ccache-{CCACHE_VERSION}-darwin.tar.gz",
        "c279fa81e2e806b4d9e64d4a06cb569a84ef54151feef796e5d4f4e95a464563",
    ),
    ("macOS", "ARM64"): (
        f"ccache-{CCACHE_VERSION}-darwin.tar.gz",
        "c279fa81e2e806b4d9e64d4a06cb569a84ef54151feef796e5d4f4e95a464563",
    ),
    ("Windows", "X64"): (
        f"ccache-{CCACHE_VERSION}-windows-x86_64.zip",
        "6219f3865ca59aec41ee4b678df171d5d35855ecb2b6dbbbd20690b3a68af7b4",
    ),
}

# Files and directories in the cache directory that are not cache entries
NON_ENTRY_DIRS = {"tmp", "lock"}
NON_ENTRY_FILES = {"stats", "ccache.conf", "CACHEDIR.TAG"}

# Snapshot of a cache directory: relative path (with "/") -> sha1 of the content
Snapshot = Dict[str, str]


def log(message: str) -> None:
    print(f"ccache-gha: {message}", flush=True)


def warn(message: str) -> None:
    print(f"::warning::ccache-gha: {message}", flush=True)


def env(name: str, default: Optional[str] = None) -> str:
    value = os.environ.get(name, default)
    if value is None:
        sys.exit(f"ccache-gha: environment variable {name} is not set")
    return value


def append_line(variable: str, line: str) -> None:
    """Appends a line to a GitHub Actions command file (GITHUB_OUTPUT, GITHUB_ENV, GITHUB_PATH)."""
    with open(env(variable), "a", encoding="utf-8") as f:
        f.write(f"{line}\n")


def set_output(name: str, value: str) -> None:
    append_line("GITHUB_OUTPUT", f"{name}={value}")


def set_env(name: str, value: str) -> None:
    append_line("GITHUB_ENV", f"{name}={value}")


def run(*args: str, check: bool = True, capture: bool = False) -> subprocess.CompletedProcess:
    return subprocess.run(args, check=check, text=True, stdout=subprocess.PIPE if capture else None)


def state_dir() -> Path:
    return Path(env("RUNNER_TEMP")) / "ccache-gha"


# ---------------------------------------------------------------------------------------
# Cache snapshots


def is_entry(relative: Path) -> bool:
    """Since ccache 4.10, entries are named by their key only (no R / M type suffix)."""
    if relative.parts[0] in NON_ENTRY_DIRS:
        return False
    name = relative.name
    return not (name in NON_ENTRY_FILES or name.startswith(".") or name.endswith(".lock"))


def file_sha1(path: Path) -> str:
    sha1 = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            sha1.update(chunk)
    return sha1.hexdigest()


def take_snapshot(cache_dir: Path) -> Snapshot:
    """Fingerprints all entries by content, as manifests can change in place."""
    snapshot: Snapshot = {}
    if not cache_dir.is_dir():
        return snapshot
    for root, _, files in os.walk(cache_dir):
        for name in files:
            path = Path(root) / name
            relative = path.relative_to(cache_dir)
            if is_entry(relative):
                snapshot[relative.as_posix()] = file_sha1(path)
    return snapshot


def save_snapshot(snapshot: Snapshot, name: str) -> None:
    state_dir().mkdir(parents=True, exist_ok=True)
    (state_dir() / name).write_text(json.dumps(snapshot), encoding="utf-8")


def load_snapshot(name: str) -> Optional[Snapshot]:
    path = state_dir() / name
    if not path.is_file():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def copy_entries(source_dir: Path, target_dir: Path, entries: List[str]) -> None:
    for entry in entries:
        target = target_dir / entry
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source_dir / entry, target)


# ---------------------------------------------------------------------------------------
# Commands


def cmd_install(args: argparse.Namespace) -> None:
    """Installs the pinned, checksum verified ccache release and adds it to the PATH."""
    platform = (env("RUNNER_OS"), env("RUNNER_ARCH"))
    if platform not in CCACHE_DOWNLOADS:
        sys.exit(f"::error::ccache-gha: no pinned ccache build for {platform[0]}-{platform[1]}")
    archive_name, sha256 = CCACHE_DOWNLOADS[platform]
    url = f"https://github.com/ccache/ccache/releases/download/v{CCACHE_VERSION}/{archive_name}"

    for attempt in range(3):
        try:
            with urllib.request.urlopen(url, timeout=60) as response:
                data = response.read()
            break
        except OSError as error:
            if attempt == 2:
                raise
            warn(f"download failed ({error}), retrying")
            time.sleep(5)

    actual = hashlib.sha256(data).hexdigest()
    if actual != sha256:
        sys.exit(f"::error::ccache-gha: checksum mismatch for {archive_name}: {actual}")

    bin_dir = Path(args.bin_dir)
    bin_dir.mkdir(parents=True, exist_ok=True)
    top_dir = archive_name.rsplit(".tar", 1)[0].rsplit(".zip", 1)[0]
    if archive_name.endswith(".zip"):
        executable = bin_dir / "ccache.exe"
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            executable.write_bytes(archive.read(f"{top_dir}/ccache.exe"))
    else:
        executable = bin_dir / "ccache"
        with tarfile.open(fileobj=io.BytesIO(data)) as archive:
            member = archive.extractfile(f"{top_dir}/ccache")
            assert member is not None
            executable.write_bytes(member.read())
        executable.chmod(0o755)

    append_line("GITHUB_PATH", str(bin_dir))
    set_output("bin-dir", str(bin_dir))
    run(str(executable), "--print-version")


def determine_mode(event: str, ref: str, default_branch: str) -> str:
    if event == "push" and (ref == f"refs/heads/{default_branch}" or ref.startswith("refs/heads/support/")):
        return "full"
    if event == "pull_request":
        return "delta"
    return "restore-only"


def cmd_setup(_: argparse.Namespace) -> None:
    """Determines the caching mode and cache keys, and exports the ccache environment."""
    key_root = env("KEY_ROOT")
    cache_dir = env("CACHE_DIR")
    default_branch = env("DEFAULT_BRANCH")
    mode = determine_mode(env("GITHUB_EVENT_NAME"), env("GITHUB_REF"), default_branch)
    base = {"full": env("GITHUB_REF_NAME"), "delta": env("GITHUB_BASE_REF", "")}.get(mode, default_branch)

    suffix = f"{env('GITHUB_RUN_ID')}-{env('GITHUB_RUN_ATTEMPT')}"
    full_prefix = f"{key_root}-ccache-full-{base}-"
    delta_prefix = f"{key_root}-ccache-delta-pr{os.environ.get('PR_NUMBER') or 'none'}-"
    save_prefix = {"full": full_prefix, "delta": delta_prefix}.get(mode, "")
    runner_temp = Path(env("RUNNER_TEMP"))
    delta_dir = runner_temp / "ccache-delta"

    state_dir().mkdir(parents=True, exist_ok=True)
    Path(cache_dir).mkdir(parents=True, exist_ok=True)

    set_output("mode", mode)
    set_output("full-key", full_prefix + suffix)
    set_output("full-prefix", full_prefix)
    set_output("default-prefix", f"{key_root}-ccache-full-{default_branch}-")
    set_output("delta-key", delta_prefix + suffix)
    set_output("delta-prefix", delta_prefix)
    set_output("delta-path", str(delta_dir))

    # Consumed by ccache-save
    set_env("CCACHE_GHA_MODE", mode)
    set_env("CCACHE_GHA_SAVE_PREFIX", save_prefix)
    set_env("CCACHE_GHA_SAVE_KEY", save_prefix + suffix if save_prefix else "")
    set_env("CCACHE_GHA_CACHE_PATH", cache_dir)
    set_env("CCACHE_GHA_DELTA_PATH", str(delta_dir))
    set_env("CCACHE_GHA_MIN_NEW", env("MIN_NEW_ENTRIES"))

    # ccache / CMake environment for the build
    set_env("CCACHE_DIR", cache_dir)
    set_env("CCACHE_BASEDIR", env("GITHUB_WORKSPACE"))
    set_env("CCACHE_LOGFILE", str(runner_temp / "ccache.log"))
    set_env("CCACHE_JOB_START", str(int(time.time())))
    set_env("CMAKE_C_COMPILER_LAUNCHER", "ccache")
    set_env("CMAKE_CXX_COMPILER_LAUNCHER", "ccache")

    log(f"mode={mode} base={base} save-prefix={save_prefix or '<none>'}")


def cmd_configure(_: argparse.Namespace) -> None:
    """Stores the settings in the cache's own ccache.conf, so they also apply to ccache
    running inside containers that mount the cache directory."""
    cache_dir = env("CACHE_DIR")
    Path(cache_dir).mkdir(parents=True, exist_ok=True)
    for setting in (f"max_size={env('MAX_SIZE')}", "compression=true", "compression_level=6", "compiler_check=content"):
        run("ccache", "-d", cache_dir, "-o", setting)


def cmd_snapshot(_: argparse.Namespace) -> None:
    """Records the entries of the restored base cache, so ccache-save can compute what changed."""
    snapshot = take_snapshot(Path(env("CACHE_DIR")))
    save_snapshot(snapshot, "base.json")
    log(f"restored base cache: {len(snapshot)} entries")


def cmd_apply_delta(_: argparse.Namespace) -> None:
    """Merges a restored delta into the cache directory."""
    cache_dir = Path(env("CACHE_DIR"))
    delta_dir = Path(env("DELTA_DIR"))
    files = [p for p in delta_dir.rglob("*") if p.is_file()] if delta_dir.is_dir() else []
    if not files:
        log("no delta restored")
        return
    copy_entries(delta_dir, cache_dir, [p.relative_to(delta_dir).as_posix() for p in files])
    shutil.rmtree(delta_dir)
    log(f"applied delta: {len(files)} entries")
    # Recalculate size and file counters after adding files behind ccache's back
    run("ccache", "-d", str(cache_dir), "--cleanup", capture=True)
    save_snapshot(take_snapshot(cache_dir), "restored.json")


def cmd_fix_ownership(_: argparse.Namespace) -> None:
    """Builds running in containers (e.g. manylinux) write the mounted cache as root."""
    if env("RUNNER_OS") != "Linux":
        return
    cache_dir = env("CCACHE_GHA_CACHE_PATH")
    uid = os.getuid()  # type: ignore[attr-defined]  # Linux only
    for root, dirs, files in os.walk(cache_dir):
        if any(os.lstat(os.path.join(root, name)).st_uid != uid for name in dirs + files):
            log(f"fixing ownership of {cache_dir}")
            run("sudo", "chown", "-R", f"{uid}:{os.getgid()}", cache_dir)  # type: ignore[attr-defined]
            return


def cmd_evict(_: argparse.Namespace) -> None:
    """Keeps only entries used (hit or written) during this job."""
    job_start = os.environ.get("CCACHE_JOB_START")
    age = int(time.time()) - int(job_start) + 300 if job_start else 3600
    run("ccache", "--evict-older-than", f"{age}s")


def uncacheable_reasons(log_text: str, limit: int = 20) -> List[tuple]:
    """Most frequent reasons for uncacheable calls in a ccache log."""
    ignored = ("Command line", "Source file", "Object file", "Detected input file", "Writing to")
    reasons: collections.Counter = collections.Counter()
    for line in log_text.splitlines():
        message = re.sub(r"^\[[^]]*\]\s*", "", line)
        if re.search(r"unsupported|not supported|too hard", message, re.IGNORECASE) and not message.startswith(ignored):
            reasons[message] += 1
    return reasons.most_common(limit)


def cmd_stats(_: argparse.Namespace) -> None:
    run("ccache", "--show-stats", "--verbose")
    run("ccache", "--zero-stats")
    log_file = Path(os.environ.get("CCACHE_LOGFILE", ""))
    if log_file.is_file():
        print("Most frequent reasons for uncacheable calls:")
        for reason, count in uncacheable_reasons(log_file.read_text(encoding="utf-8", errors="replace")):
            print(f"{count:7} {reason}")
        log_file.unlink()


def decide_save(mode: str, base: Snapshot, restored: Snapshot, current: Snapshot, min_new: int) -> tuple:
    """Returns (save, entries that differ from the base)."""
    changed = sorted(entry for entry, sha1 in current.items() if base.get(entry) != sha1)
    new_entries = sum(1 for entry in current if entry not in restored)
    log(f"{new_entries} new entries in this job, {len(changed)} entries differ from the base cache")
    if mode == "restore-only":
        log("mode restore-only: not saving")
        return False, changed
    if new_entries < min_new:
        log(f"fewer than {min_new} new entries: not saving")
        return False, changed
    return True, changed


def cmd_diff(_: argparse.Namespace) -> None:
    """Decides whether saving is worthwhile. In delta mode, stages the entries that are
    new or changed compared to the base for saving."""
    mode = env("CCACHE_GHA_MODE")
    cache_dir = Path(env("CCACHE_GHA_CACHE_PATH"))
    base = load_snapshot("base.json") or {}
    restored = load_snapshot("restored.json") or base
    save, changed = decide_save(mode, base, restored, take_snapshot(cache_dir), int(env("CCACHE_GHA_MIN_NEW")))

    # actions/cache derives the cache version from the literal path string, so the path must be
    # passed on exactly as it was used for restoring (Path() would normalize "/" to "\" on Windows)
    save_path = env("CCACHE_GHA_CACHE_PATH")
    if mode == "delta":
        save_path = env("CCACHE_GHA_DELTA_PATH")
        delta_dir = Path(save_path)
        shutil.rmtree(delta_dir, ignore_errors=True)
        delta_dir.mkdir(parents=True)
        if save:
            copy_entries(cache_dir, delta_dir, changed)
            size = sum(f.stat().st_size for f in delta_dir.rglob("*") if f.is_file())
            log(f"delta size: {size / 1e6:.1f} MB")

    set_output("save", "true" if save else "false")
    set_output("path", save_path)


def superseded_cache_ids(caches: List[dict], saved_key: str) -> Optional[List[int]]:
    """IDs of caches to delete, or None if the saved cache is not listed.

    actions/cache/save only warns if the upload fails, so nothing must be deleted unless
    the new cache actually exists."""
    if not any(cache["key"] == saved_key for cache in caches):
        return None
    return [cache["id"] for cache in caches if cache["key"] != saved_key]


def cmd_cleanup(_: argparse.Namespace) -> None:
    """Deletes older caches with the same prefix on the current ref, keeping the one just saved."""
    repo, ref = env("GITHUB_REPOSITORY"), env("GITHUB_REF")
    saved_key = env("CCACHE_GHA_SAVE_KEY")
    listing = run("gh", "cache", "list", "--repo", repo, "--ref", ref, "--key", env("CCACHE_GHA_SAVE_PREFIX"),
                  "--limit", "100", "--json", "id,key", check=False, capture=True)
    if listing.returncode != 0:
        warn("could not list old caches (missing 'actions: write' permission?)")
        return
    ids = superseded_cache_ids(json.loads(listing.stdout or "[]"), saved_key)
    if ids is None:
        warn(f"saved cache {saved_key} not found on {ref}, keeping older caches")
        return
    for cache_id in ids:
        if run("gh", "cache", "delete", str(cache_id), "--repo", repo, check=False, capture=True).returncode == 0:
            log(f"deleted old cache {cache_id}")
        else:
            warn(f"could not delete old cache {cache_id}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    install = commands.add_parser("install", help="install the pinned ccache release")
    install.add_argument("bin_dir")
    install.set_defaults(func=cmd_install)
    for name, func in (("setup", cmd_setup), ("configure", cmd_configure), ("snapshot", cmd_snapshot),
                       ("apply-delta", cmd_apply_delta), ("fix-ownership", cmd_fix_ownership),
                       ("evict", cmd_evict), ("stats", cmd_stats), ("diff", cmd_diff), ("cleanup", cmd_cleanup)):
        commands.add_parser(name).set_defaults(func=func)
    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
