# dds_image

A C++11 single-header DDS (DirectDraw Surface) image library,
with no dependencies.
It also includes utilities to work with Vulkan, as well
as optional support for C++17 and C++20 specific features.
You can quickly add this as a submodule and add the header files,
or import it as a subdirectory in CMake.

### Example

```cpp
#include <dds.hpp>

dds::Image image;
dds::readFile("example.dds", &image);
```

On MSVC you might need to build with `/Zc:__cplusplus` for the built-in C++17 features to work.
You can then optionally also define `DDS_USE_STD_FILESYSTEM`, which will make `dds::readFile` accept
a `std::filesystem::path` instead.

Each mipmap can be accessed through `image.mipmaps`. A mipmap is a `dds::span` which represents a
contiguous block of memory within the image data that represents the specific mipmap. All mipmaps
use the same format and the mipmaps are arranged in order of size in the vector, meaning the
largest mipmap is `image.mipmaps.front()` and every next mipmap is exactly 50% smaller.

### Vulkan Usage

If you want to use the built-in Vulkan features, you **have** to include
the Vulkan header before you import this library.
To create the VkImage and VkImageView you can use the following two functions.
Note that you still need to set some fields yourself, most notably `VkImageCreateInfo::usage`,
`VkImageCreateInfo::samples` and `VkImageViewCreateInfo::image`.

```cpp
#include <vulkan/vulkan.h> // Before!
#include <dds.hpp>
```
```cpp
// Optional, for own usage.
VkImageFormat imageFormat = dds::getVulkanFormat(image.format, image.supportsAlpha);

// Will automatically fill VkImageCreateInfo::format with a separate call to dds::getVulkanFormat.
VkImageCreateInfo imageCreateInfo = dds::getVulkanImageCreateInfo(&image);
VkImageViewCreateInfo imageViewCreateInfo = dds::getVulkanImageViewCreateInfo(&image);
```

### License

The library itself is licensed under MIT.
