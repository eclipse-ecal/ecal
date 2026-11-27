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

"""Tests for ccache_gha.py. Run with: python -m unittest discover .github/actions/ccache-common"""

import os
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import ccache_gha


def write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content)


class CacheTestCase(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = Path(tempfile.mkdtemp())
        self.cache = self.tmp / "cache"
        self.delta = self.tmp / "ccache-delta"
        self.output = self.tmp / "output"
        self.environment = mock.patch.dict(os.environ, {
            "RUNNER_TEMP": str(self.tmp),
            "GITHUB_OUTPUT": str(self.output),
            "CCACHE_GHA_CACHE_PATH": str(self.cache),
            "CCACHE_GHA_DELTA_PATH": str(self.delta),
            "CCACHE_GHA_MIN_NEW": "2",
            "CACHE_DIR": str(self.cache),
            "DELTA_DIR": str(self.delta),
        })
        self.environment.start()
        # ccache is not needed for the tested logic
        self.run_patch = mock.patch.object(ccache_gha, "run")
        self.run_patch.start()

    def tearDown(self) -> None:
        self.run_patch.stop()
        self.environment.stop()

    def outputs(self) -> dict:
        lines = self.output.read_text().splitlines() if self.output.exists() else []
        return dict(line.split("=", 1) for line in lines)

    def diff(self, mode: str) -> dict:
        self.output.unlink(missing_ok=True)
        with mock.patch.dict(os.environ, {"CCACHE_GHA_MODE": mode}):
            ccache_gha.cmd_diff(None)
        return self.outputs()

    def delta_entries(self) -> list:
        return sorted(p.relative_to(self.delta).as_posix() for p in self.delta.rglob("*") if p.is_file())


class SnapshotTest(CacheTestCase):
    def test_only_cache_entries_are_listed(self) -> None:
        write(self.cache / "a" / "b" / "1f2e3d", "result")
        for name in ("a/stats", "a/b/stats", "ccache.conf", "CACHEDIR.TAG", "lock/auto_cleanup",
                     "tmp/x.tmp", "tmp/.cleaned", "a/b/1f2e3d.lock"):
            write(self.cache / name, "x")
        self.assertEqual(list(ccache_gha.take_snapshot(self.cache)), ["a/b/1f2e3d"])


class DeltaTest(CacheTestCase):
    def test_pull_request_delta_lifecycle(self) -> None:
        # Base cache from master with one entry
        write(self.cache / "a" / "b" / "base", "base")
        ccache_gha.cmd_snapshot(None)

        # Build adds two entries and updates nothing else: delta contains only the new ones
        write(self.cache / "a" / "c" / "new1", "1")
        write(self.cache / "b" / "0" / "new2", "2")
        self.assertEqual(self.diff("delta")["save"], "true")
        self.assertEqual(self.delta_entries(), ["a/c/new1", "b/0/new2"])

        # Next run: fresh base cache, delta applied on top
        for p in self.cache.rglob("*"):
            if p.is_file() and p.name != "base":
                p.unlink()
        ccache_gha.cmd_snapshot(None)
        ccache_gha.cmd_apply_delta(None)
        self.assertFalse(self.delta.exists())
        self.assertEqual(sorted(ccache_gha.take_snapshot(self.cache)), ["a/b/base", "a/c/new1", "b/0/new2"])

        # A changed manifest alone is not worth saving
        write(self.cache / "a" / "c" / "new1", "1-updated")
        self.assertEqual(self.diff("delta")["save"], "false")

        # Two more new entries: the delta contains everything that differs from the base,
        # including the updated entry, so it replaces the previous delta
        write(self.cache / "c" / "d" / "new3", "3")
        write(self.cache / "c" / "e" / "new4", "4")
        self.assertEqual(self.diff("delta")["save"], "true")
        self.assertEqual(self.delta_entries(), ["a/c/new1", "b/0/new2", "c/d/new3", "c/e/new4"])
        self.assertEqual((self.delta / "a" / "c" / "new1").read_text(), "1-updated")

    def test_full_mode_saves_cache_directory(self) -> None:
        ccache_gha.cmd_snapshot(None)
        write(self.cache / "a" / "b" / "new1", "1")
        write(self.cache / "a" / "b" / "new2", "2")
        outputs = self.diff("full")
        self.assertEqual(outputs["save"], "true")
        self.assertEqual(outputs["path"], str(self.cache))

    def test_restore_only_never_saves(self) -> None:
        ccache_gha.cmd_snapshot(None)
        for i in range(5):
            write(self.cache / "a" / f"new{i}", str(i))
        self.assertEqual(self.diff("restore-only")["save"], "false")


class ModeTest(unittest.TestCase):
    def test_determine_mode(self) -> None:
        cases = [
            ("push", "refs/heads/master", "full"),
            ("push", "refs/heads/support/v6.1", "full"),
            ("push", "refs/heads/feature/x", "restore-only"),
            ("push", "refs/tags/v6.1.0", "restore-only"),
            ("pull_request", "refs/pull/7/merge", "delta"),
            ("workflow_dispatch", "refs/heads/master", "restore-only"),
        ]
        for event, ref, expected in cases:
            with self.subTest(event=event, ref=ref):
                self.assertEqual(ccache_gha.determine_mode(event, ref, "master"), expected)


class CleanupTest(unittest.TestCase):
    def test_superseded_caches(self) -> None:
        caches = [{"id": 1, "key": "k-1"}, {"id": 2, "key": "k-2"}, {"id": 3, "key": "k-0"}]
        self.assertEqual(ccache_gha.superseded_cache_ids(caches, "k-2"), [1, 3])

    def test_nothing_deleted_if_saved_cache_is_missing(self) -> None:
        self.assertIsNone(ccache_gha.superseded_cache_ids([{"id": 1, "key": "k-1"}], "k-2"))
        self.assertIsNone(ccache_gha.superseded_cache_ids([], "k-2"))

    def test_cleanup_deletes_only_superseded(self) -> None:
        listing = subprocess.CompletedProcess([], 0, stdout='[{"id": 1, "key": "k-1"}, {"id": 2, "key": "k-2"}]')
        deleted = subprocess.CompletedProcess([], 0, stdout="")
        environment = {"GITHUB_REPOSITORY": "o/r", "GITHUB_REF": "refs/heads/master",
                       "CCACHE_GHA_SAVE_KEY": "k-2", "CCACHE_GHA_SAVE_PREFIX": "k-"}
        with mock.patch.dict(os.environ, environment), \
             mock.patch.object(ccache_gha, "run", side_effect=[listing, deleted]) as run:
            ccache_gha.cmd_cleanup(None)
        self.assertEqual(run.call_args_list[1].args[:4], ("gh", "cache", "delete", "1"))


class StatsTest(unittest.TestCase):
    def test_uncacheable_reasons(self) -> None:
        text = "\n".join([
            "[2026-11-27T10:00:00.000000 1234] Compiler option /Zi is unsupported",
            "[2026-11-27T10:00:00.000000 1235] Compiler option /Zi is unsupported",
            "[2026-11-27T10:00:00.000000 1236] Command line: cl /Zi unsupported_field.cc",
            "[2026-11-27T10:00:00.000000 1237] Non-UTF-8 source code unsupported in preprocessor mode",
        ])
        self.assertEqual(ccache_gha.uncacheable_reasons(text), [
            ("Compiler option /Zi is unsupported", 2),
            ("Non-UTF-8 source code unsupported in preprocessor mode", 1),
        ])


if __name__ == "__main__":
    unittest.main()
