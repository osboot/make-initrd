# Testing overview

This directory contains two layers of tests:

1. **Shell-based functional tests** under `testing/tests/`.
2. **Integration/boot tests** driven by `testing/testing-*` scripts plus kickstart configs in `testing/*.cfg`.

The notes below focus on what a developer needs to add a new test without describing each existing one.

## 1) Shell test suite (`testing/tests/`)

### Runner
- The runner is `testing/tests/run`.
- It accepts one or more test suite directories as arguments. Example usage from repo root:

```sh
./testing/tests/run testing/tests
```

- For each suite directory, it walks all subdirectories named `ts*` (e.g. `ts0010-put-file`).
- It executes each test’s `run` script and captures **stdout+stderr** into `ts*/output`.
- It compares the output with `ts*/expect` and prints a diff on mismatch.
- On success, `output` is removed. If `expect.in` exists, it is expanded into
  `expect` before compare (currently unused in tree, but supported by the runner).
- If the test exits with code `127`, the test is **skipped** and `output` is removed.
- If a file named `known-failure` exists in the test directory, failures do not fail the overall suite.

### Test directory layout
A typical test directory looks like:

```
testing/tests/tsNNNN-some-name/
  run        # executable script
  expect     # exact expected output
  data*      # optional input data files (varies per test)
  src/       # optional sources to build (varies per test)
```

Patterns in existing tests:
- **`put-file` tests** use helper functions in `testing/tests/put-file-sh-functions` and often build small sources in a local `src/` directory.
- **`sort-services` tests** are data-driven; the `run` script calls `tools/sort-services` over `data*` files and compares to `expect`.
- **`kernel-compare` tests** run `tools/kernel-compare` and compare the collected output to `expect`.

### Adding a new shell test
1. Choose the next `tsNNNN-name` directory name under `testing/tests/`.
2. Create an executable `run` script. Keep it hermetic, use `set -e`/`-u` as appropriate, and write
   all output to stdout/stderr (the runner captures it).
3. Create an `expect` file with the **exact** expected output. A common workflow:

```sh
# From repo root
cd testing/tests/tsNNNN-name
./run > expect
```

4. If the test depends on external tools (e.g. `tools/*`), ensure those tools are built/available before running the suite.
5. If you need a permanent known failure, add an empty `known-failure` file in the test directory.

Notes:
- The runner does not order tests by number; it uses shell glob order, so keep naming consistent.
- Use relative paths inside tests when possible, and prefer `$cwd` patterns shown in existing tests.

## 2) Integration/boot tests (`testing/testing-*` and `testing/*.cfg`)

These are higher-level tests that build images and boot them under QEMU for different distros (e.g.
Fedora, Ubuntu, Gentoo). The driver scripts are:

- `testing/testing-altlinux`
- `testing/testing-fedora`
- `testing/testing-gentoo`
- `testing/testing-gentoo_musl`
- `testing/testing-ubuntu`

Common helpers and inputs:
- `testing/sh-functions` contains shared logic (QEMU setup, kickstart generation, status tracking).
- `testing/packages-*` define per-distro package sets.
- Kickstart fragments:
  - `testing/ks-*-sysimage.cfg`
  - `testing/ks-*-initrd.cfg`
  - `testing/ks-*-done.cfg`
  - `testing/ks-sysimage.cfg` / `testing/ks-initrd.cfg` (fallbacks)
- Test case configs like `testing/test-root-*.cfg` define partition layouts and parameters.

### How test case configs are wired
When you pass a test name to a `testing/testing-*` script, it reads `testing/<TESTNAME>.cfg` and
also any `# param ...` lines inside it. Example header:

```
# param KICKSTART_DISKS=4
# param BOOT_DISKS=4
# param BOOT_CMDLINE="..."
```

`testing/sh-functions:prepare_testsuite` combines the kickstart fragments and the test case config
into `testing/cache/<vendor>/<test>/ks.cfg`, then runs the requested steps (image build, sysimage
pack, kickstart, boot, etc.). Logs go under `testing/logs/<vendor>/<test>` and status is tracked
under `testing/status/`.

### Adding a new integration test
1. Add a new kickstart test config in `testing/` (e.g. `testing/test-root-myfeature.cfg`).
2. Include any `# param` lines needed for the test (disks, cmdline, etc.).
3. Run the appropriate distro script with your test name and steps, for example:

```sh
./testing/testing-fedora test-root-myfeature create-images build-sources build-sysimage build-kickstart run-boot
```

4. Inspect `testing/logs/<vendor>/<test>` and `testing/status/` for results.

## Quick reference
- Shell suite runner: `testing/tests/run`
- Shell tests live under: `testing/tests/ts*`
- Integration test drivers: `testing/testing-*`
- Integration test cases: `testing/test-root-*.cfg`
- Shared helper functions: `testing/sh-functions`
