# Contributing to sfizz

Thanks for considering contributing!
There is actually many things anyone can do, programming-related or music-related.

## Using the software for your projects

One of the best and probably simplest thing to do is simply to use sfizz in a number of situations.
There is only so much we can test as far as sfz libraries go, or differents hosts for the plugin versions.
If during the course of your usage you find out that sfizz does not behave as it should with respect to what the SFZ spec dictates, or what other players do, please file in a bug reports.
If there is some feature that would be fantastic for you, feel free to post a feature requests in the issues.
This will help us track and remember everything.
Refer to (#filing-bugs-and-feature-requests) for more precision.

### Filing bugs and feature requests

When filing a bug, try to be as precise as possible.
Obviously bugs that are easily reproducible are easier to solve.

- If sfizz fails to play a library correctly, please give us a link to the library and ideally an example of what you except. It helps **tremendously** if you can provide a *minimal* example in the form of a simple custom sfz library that pinpoints the problem exactly, along with instruction or midi files to render.
- If the plugins crashes, please try to reproduce and detail everything you did up to the crash, as well as the host and sfz library. It can be really helpful to provide session files, again with a minimal example.
- If the behavior of sfizz is clumsy and annoying, and the user experience feels subpar, please tell us and try to propose some solutions on how it could be improved. As developers, we are viewing and using sfizz in specific ways which may actually be quite far from what users would like, but we're all ears :)

For feature requests, provide details about the expected behavior and possibly example sfz files that will benefit from the new feature.
Note that we aim at implementing the sfz spec as a whole, and possibly some extensions by e.g. ARIA so normally all sfz features should come at some point.

### Using the C or C++ library

Sfizz is liberally licensed and you can freely use it in any project under the term of its license and its dependencies.
We provide a somewhat simple C and C++ API that covers our use cases within plugins mostly, and should be useful.
The API is documented over at the [api] page.
You can find example of using the C API in the `lv2` directory which contains sfizz's LV2 plugin.
You can find example of using the C++ API in the `vst3` directory which contains sfizz's VST3 plugin.
In both cases sfizz is built in the plugins as a static library.
The project [sfizz-render] illustrates how one might use sfizz as a shared library through a `make`- and `pkg-config`-based process.
Other projects that bundle or use sfizz are:

- (https://github.com/sfztools/beatbox): a live drum machine

If you use the library, maybe you will have specific needs and requests that could be useful to you.
Be sure to post an issue for this so we can improve the API, especially because we have to queue API changes so it stays a bit stable over time.

## Developing for sfizz

We welcome all code contributions to sfizz, whatever your coding level.
If there is a specific feature you would like, and you are willing to put in the time (and have some form of initial background in modern C++ code), we can mentor you and provide breadcrumbs through the codebase.
Sfizz is still very much alpha, and we're refactoring large parts of the code on a regular basis.
We try to keep the public API documented, and most of the internal APIs are also documented, but this can be in flux from time to time.
If you want to add things we encourage you to ask around through an issue, on Discord, IRC, or by email, so that we may be able to guide you and discuss about how and what to implement.

### Handling the repository and its submodules

Sfizz uses a number of submodules, namely abseil and the VST sdk.
These submodules are purely consumed by sfizz, and the maintainers will manage updating them when needed.
As such, they should *not* pose too much issues.
They are all pinned to specific branches or commits in their original repository.

To clone the sfizz repository, the recommended way is to do
```bash
git clone https://github.com/sfztools/sfizz.git --recursive
# Other possibility
git clone https://github.com/sfztools/sfizz.git
cd sfizz
git submodule update --init
```

If after some time you happen to see something like this:
```
❯ git status
On branch develop
Your branch is up to date with 'origin/develop'.

Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git checkout -- <file>..." to discard changes in working directory)

	modified:   external/abseil-cpp (new commits)

no changes added to commit (use "git add" and/or "git commit -a")
```
Then it means that the abseil submodule *on your disk* differs from the one expected by sfizz.
This usually happens if i) you modified one of abseil's files, in which case the repository will be dirty, or ii) sfizz updated abseil's and pinned it to another commit.
Assuming you were not adding relevant changes to abseil in order to push them upstream, you can usually resolve this issue through
```bash
git submodule update
```
**All changes in the submodule will be lost**.
If you were in fact modifying abseil for some reason, then consider either pushing your changes upstream or find an alternative.
Same goes for the VST SDK.

### Building sfizz from source

For details on the dependencies and platform-specific information for building sfizz, please refer to the [build] page.
Note that for development you will probably want all dependencies, which means using debian's lingo:

- `cmake`
- `g++` or `clang++`
- `clang-format`
- `libjack-jackd2-dev`
- `libsndfile-dev`

We also recommend building [benchmark] from source and installing it as a static library.
You will also want to configure using the `-DSFIZZ_TESTS=ON` flag and `-DSFIZZ_BENCHMARKS=ON` flag to be able to run the tests and possibly the micro-benchmarks.

### Forking and creating pull requests

We do development on the git `develop` branch, and distribute using the `master` branch.
We mostly avoid directly committing to the `develop` branch and go through pull requests and issues for development-related discussion and choices.
Try to have public and private discussion about development, and if such discussion happen on IRC or Discord, try to sum them up in a pull request or an issue that we can then refer to.
Obviously, light technical questions can be asked freely in private, but the goal is to open the discussion for everyone to see.

To start contributing, we encourage you to fork the main sfizz repository on your personal account and clone it locally on your machine
In the clone of your fork, and add the `sfztools` repository as a remote as follows
```bash
# Clone your own fork in a specific directory
git clone https://github.com/MY_GITHUB_NAME/sfizz.git sfizz-fork
cd sfizz-fork
# Add the upstream sfztools remote
git remote add upstream https://github.com/sfztools/sfizz.git
```
Once done, the contribution process is as follows:

1. Fetch the latest changes from the `upstream` remote, in particular the `develop` branch.
2. Checkout the `upstream/develop` branch, and immediately checkout to a new local branch to start adding changes
3. Add commits to your local branch
4. Push the local branch to your own fork
5. Go back to Github web interface and create a pull request

In command language, this translates to
```bash
# Fetch the latest status from upstrea
git fetch upstream
# Checkout the upstream develop branch
git checkout upstream/develop
# Create a new local branch
git checkout -b my-new-contribution
# Do things as usual for git: git add, git commit
# ...
# Once ready, push the branch to your fork. Github will tell you where to go for a pull request from there.
git push --set-upstream origin my-new-contribution
```

### Adding files to the build system

We use CMake for the build system.
For the main sfizz library, the main file is `src/CMakeLists.txt`.
In particular if you add a new `.cpp` file to sfizz, you should add it to the `SFIZZ_SOURCES` list in this file so it gets compiled and linked with the rest of the library.

### Coding style and guidelines

We have continuous integration targets down to gcc 4.9, so the code has to be fully C++11 compliant.
Sadly this means some bonuses from C++14 are not available, although `absl` compensates many things from a standard library point of view.
All modern C++ guidelines apply, as well as rules related to audio programming:

- Raw pointers are non-owning observers; use `unique_ptr` and `shared_ptr` for owning references, and `absl::optional` for nullable/optional/maybe values returned from functions.
- Use move semantics when appropriate.
- Try to use [RAII] wrappers and write them if necessary.
- Avoid allocating memory, calling functions that throw, using system calls, or any launching any blocking operation in the audio thread; for sfizz, that means anything that is called at some point from `Synth::renderBlock` and `Synth::noteOn`, `noteOff`, ... and all midi events.
- We have a helper class that helps us to track allocation in the form of the `sfz::Buffer` class and the `sfz::AudioBuffer` class, so use it for heap memory in lieu of naked `new` or `unique_ptr<T[]>` when relevant.
- Most things in the [CppCoreGuidelines] are good to take, although be careful as many proposed things may break the *no allocation* rule.

We also have a `clang-tidy` CI step that may annoy you from time to time.

We have a `clang-format` coding style in that is loosely based on the WebKit style.
We encourage you to use it, as well as `git-clang-format` to format your PR.
If possible, find a plugin for your text editor than handles `.editorconfig` files. Most editors have one.
Some notes :

- Class and type names start by a capital letter and follow the `PascalCase`, function names and variables in the C++ part follow the `camelCase`, and function names and variables in the C part follows a `snake_case` to blend with the surroundings.
- Try to keep names meaningful, and code clear if possible.
- Comment your classes APIs, in a terse and clear way. Comments inside the code are usually not needed *if the code is written clearly with meaningful names*, but if you feel that it's needed please do.
- You can use `_` in front of private variables, but we do not have a consistent naming for these yet.

All these rules are subject to change upon discussion and integration.

### Testing your changes

To build and run the tests, you need to have configured sfizz with the `-DSFIZZ_TESTS=ON` flag.
You can then use the `make sfizz_tests` command from your build directory.
The tests are located in `tests/sfizz_tests` from the same build directory.

Note that these tests mostly cover the parsing and observable invariants of sfizz, as well as all helper methods as much as possible.
The processing and rendering is usually harder to test as it produces alot of data.
We are in the process of making a testing platform using rendered wave files, but it is not yet here.
If you write anything testable however, we encourage you to write the tests.
We use the [Catch] library for testing, and you can check out the files in `tests/` for examples on how to add some.

To actually try your changes, you can use the `clients/sfizz_jack` client to instantiate a JACK application, or use `jalv` for a similar purpose by launching from your build directory
```bash
env LV2_PATH=$PWD jalv.gtk3 http://sfztools.github.io/sfizz
```
Similarly, you can launch e.g. Ardour with suitable reference to your version of the sfizz LV2 plugin with
```bash
env LV2_PATH=$PWD Ardour6
```

### Running micro-benchmarks

For micro-optimization purposes we have a number of benchmarks written in `benchmarks/`, build against the [benchmark] library.
If you wonder about the performance of something, feel free to write some too and add them to the build system.
We tend to name the benchmarks with the `bm_` prefix.
It can be as simple as adding a line
```cmake
sfizz_add_benchmark(bm_wavfile BM_wavfile.cpp)
```
to the `benchmarks/CMakeLists.txt` file but you might need to link to other libraries depending on what you are benchmarking.
The `CMakeLists.txt` file contains example that you can copy and try.

### Running in-use benchmarks

We have logging facilities for the processing inside of sfizz, that we use to track how the additions impact the rendering speed.
For now the usage is a bit hand-made, as it requires the [sfizz-render] program and manually pre-loading the development `.so` library.
The logger outputs CSV files for the rendering and file-loading processes.
The file `scripts/performance_report.py` is made to process and produce a nice interactive report from this data.

[build]: https://sfz.tools/sfizz/development/build/
[benchmark]: https://github.com/google/benchmark
[sfizz-render]: https://github.com/sfztools/sfizz-render
[GOVERNANCE]: GOVERNANCE.md
[api]: https://sfz.tools/sfizz/api/
[RAII]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-resource
[CppCoreGuidelines]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
[Catch]: https://github.com/catchorg/Catch2
