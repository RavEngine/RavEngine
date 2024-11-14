# DDS
This is a simple library for loading `.dds` textures. It supports DXT1, DXT3 & DXT5 compressed formats and most of the uncompressed formats. It loads the data into a RGBA byte array.
It supports 3D textures.

Example:
```c
dds_image_t image = dds_load_from_file("texture.dds");

// use image->pixels, image->header.width, image->header.width, image->header.depth

dds_image_free(image);

```