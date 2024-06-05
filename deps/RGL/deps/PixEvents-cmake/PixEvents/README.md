- [PixEvents](#pixevents)
  - [Usage](#usage)
    - [Recommendations for game/application developers](#recommendations-for-gameapplication-developers)
    - [Recommendations for developer tool authors](#recommendations-for-developer-tool-authors)
  - [Why use PIX events?](#why-use-pix-events)
  - [How do PIX events work?](#how-do-pix-events-work)
- [Building and Running](#building-and-running)
- [Contributing](#contributing)
- [Component Overview](#component-overview)
- [Microsoft Open Source Code of Conduct](#microsoft-open-source-code-of-conduct)
- [Trademarks](#trademarks)

# PixEvents
PIX events can be used by developers to instrument their game or application, labeling regions of work or important occurrences in the application's CPU or GPU workloads. Adding this instrumentation during development can make it far easier to use developer tools like [PIX](https://devblogs.microsoft.com/pix/).

This repository contains:
- The PIX event headers (pix3.h and friends) that define the PIX event APIs.
- The source code for WinPixEventRuntime.dll, which contains key PIX event functionality.
- The code necessary to decode PIX event blobs into strings and colors.
- Miscellaneous associated code, such as unit tests.

<br>

## Usage
Installation, usage instructions, and an API reference can be found at the [WinPixEventRuntime devblog](https://devblogs.microsoft.com/pix/winpixeventruntime/).

### Recommendations for game/application developers
There are two good options:
1. Use the [WinPixEventRuntime NuGet package](https://www.nuget.org/packages/WinPixEventRuntime). This conveniently combines the PIX event headers and prebuilt WinPixEventRuntime.dll binaries into one package. Note that the .nupkg file can be renamed to a .zip file, allowing you to easily extract its contents. Also note that, as of March 2024, the NuGet package is distributed under the MIT license.
2. Build WinPixEventRuntime.dll yourself. This may be more convenient for some users. If you do this, then we strongly recommend that you build and use WinPixEventRuntime.dll instead of statically linking the full WinPixEventRuntime.lib (and all of its functionality) into each of your binaries. This will avoid ETW problems that may occur if separate binaries within your process each emit PIX events.

### Recommendations for developer tool authors
If you would like to decode PIX events in your tool, then we strongly recommend that you use the PIX events decoder in this repository instead of creating your own decoder. This will give you a reliable decoder that supports all versions of the PIX event blob format. See the [How do PIX events work?](#how-do-pix-events-work) section below for more info about the PIX event blob format.

<br>

## Why use PIX events?
PIX events offer several significant advantages over simpler solutions, such as the similar (but deprecated) APIs in the old pix.h. For example:

1) **Performance:** PIX events are designed to be as efficient as possible, offering helpful developer annotations while minimizing impact on the application's performance. For example:
    * We do not sprintf strings inside the game/application process. The sprintf parameters are encoded into the PIX event blob, and the sprintf'ing is done by the analysis tool like PIX.
    * We have micro-optimized the PIX event APIs to make the blob encoding as fast as possible on a wide range of hardware.
    * When emitting PIX events via ETW, we batch the events into blocks to reduce the ETW overhead.
2. **CPU and GPU annotations**: the PIX event APIs such as `PIXBeginEvent(...)` have multiple overloads that allow you to annotate work on both your CPU and GPU timelines. The GPU timeline overloads take a GPU context (e.g. an `ID3D12GraphicsCommandList*`) while the CPU timeline overloads do not.
3. **ETW support**: GPU PIX events can be interpreted by API-level tools, including API capture/replay tools like PIX's GPU Captures or tools such as Microsoft's [DRED](https://learn.microsoft.com/en-us/windows/win32/direct3d12/use-dred). CPU and GPU PIX events can be interpreted by tools that process ETW events, such as PIX's Timing Captures.
4. **Timestamps**: PIX events include CPU timestamps (via `QueryPerformanceCounter())`). This allows performance tools to show helpful information, such as correlating when a PIX event is recorded on the CPU with its GPU execution time.

<br>

## How do PIX events work?

*The implementation of `void PIXBeginEvent(CONTEXT* context, UINT64 color, STR formatString, ARGS... args)` in [`include/PIXEvents.h`](include/PIXEvents.h) will be a helpful reference in this section.*

When the application calls `PIXBeginEvent()` with a GPU context (e.g. an `ID3D12GraphicsCommandList*`), the following happens:
1. The implementation checks to see if a tool (e.g. PIX Timing Captures) has turned on the PIX events ETW provider. If yes, then the PIX event runtime will encode the incoming PIX event information into the current "block" of PIX events that are due to be emitted by ETW. If the block is full, then the block will be emitted via ETW.
2. After that, the implementation will pass the encoded PIX event blob into `ID3D12GraphicsCommandList::BeginEvent()` (or equivalent API). This means that API tools such as PIX's GPU Captures can intercept the blob and save it into the capture file. It also means that D3D features like DRED can access the event blobs.

The exact behavior depends on whether `PIX_USE_GPU_MARKERS_V2` was defined before the application included `pix3.h`. In 2023, the PIX team changed the format of the PIX event blob *(for the first time in 10 years!)* to enable future functionality. Step 1 above will always use the new blob format. If `PIX_USE_GPU_MARKERS_V2` is set, then Step 2 will use the new blob format too. However, if `PIX_USE_GPU_MARKERS_V2` isn't set, then Step 2 will use the old legacy blob format. This preserves compatibility with older tools that aren't aware of the new blob format. By default, `PIX_USE_GPU_MARKERS_V2` is not currently defined, but the PIX team will change this default in the future. If third party products use the PIX event decoder available in this repository, then they will automatically support both the old and the new PIX event blob layout.

When the application calls `PIXBeginEvent()` without a GPU context parameter, Step 1 above still happens but Step 2 does not.

<br>

# Building and Running

**Requirements:**
- Visual Studio 2022 with v143 build tools *(optional: v143 arm64 tools)*

**Instructions:**
- Open `/pixEvents.sln` in VS2022 and build.
- The projects under the `test/` directory are the unit tests built on Google Test. These can be run through the Visual Studio Test Explorer window or as a standalone .exe.

<br>

# Contributing

This project is open to contributions! We are particularly interested in bug fixes, performance optimizations, and contributions that add or simplify the ability to use PIX events in languages other than C++.

We are certainly open to other contributions too, such as new PIX event-style APIs or even PIX event overloads for other graphics APIs. These kinds of contributions would require careful collaboration with with Microsoft before they could be accepted though. If you are interested in making a contribution like this then please reach out to us.

This project doesn't follow any particular style guide. However, if you are submitting changes then please match the style of the surrounding code.

<br>

# Component Overview

| Directory | Description |
| ----------- | ----------- |
| `decoder/`      | Contains code tht can decode a PIX event blob. We expect `TryDecodePIXBeginEventOrPIXSetMarkerBlob(...)` in [`/decoder/include/PixEventDecoder.h`](/decoder/include/PixEventDecoder.h) to be the most commonly used function for tools other than PIX.     |
| `include/`   | Contains pix3.h and associated headers that define APIs such as PIXBeginEvent(). These are the headers that are distributed with the WinPixEventRuntime nuget package.  |
| `pipelines/`   | Contains `.yml` files used by Microsoft to build PixEvents in Azure DevOps.  |
| `runtime/dll/`   | Creates the various .dll files that are shipped in the WinPixEventRuntime.       |
| `runtime/lib/`   | Creates a static lib that contains WinPixEventRuntime functionality, which is linked into the DLLs in the `runtime/dll/` directory. We recommend using the .DLLs in your own project and not the static lib. See [](#recommended-usage) above for details.        |
| `shared/`   | Contains a header file that is shared across various components. |
| `test/`   | Contains unit tests for most PixEvents functionality.  |
| `third_party/` | Contains third party code used in this repo, e.g. Google Test and Windows Implementation Library (WIL).     |

<br>

# Microsoft Open Source Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). Resources:

- [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/)
- [Microsoft Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
- Contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with questions or concerns

<br/>

# Trademarks
This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft trademarks or logos is subject to and must follow Microsoft’s Trademark & Brand Guidelines. Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship. Any use of third-party trademarks or logos are subject to those third-party’s policies.