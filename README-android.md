# Using RavEngine on Android
For every platform except Android, RavEngine can create a ready-to-go IDE project which can build and run a complete game with a single click. Unfortunately, Android requires some additional manual effort to make work. 

1. Download a copy of the [SDL Android Builder](https://github.com/Ravbug/sdl-android-builder) project and place it somewhere outside your game repository.
2. Symlink your game repository into the `sdl-android-builder/to-build` directory.
3. Modify your game's root `CMakeLists.txt` to change the main executable target such that:
    - It is a shared library rather than an executable, via `add_library`
        - you can find an example of how to do this in the HelloCube sample: https://github.com/RavEngine/HelloCube/blob/84222b9b846862321c07b016609a840637cbe589/CMakeLists.txt#L22-L30
    - It is named `main`
4. Open the root `sdl-android-builder` project in Android Studio. Make any changes necessary to `app/build.gradle` such that your project is able to compile. For example, `minSDKVersion` must be set high enough that Vulkan 1.3 is enabled.
5. Press the build button. If all goes well, your project should compile successfully. But don't run it yet! If the build fails, do not proceed to the next step until all compile errors have been resolved.
6. Locate the `main.rvedata` file produced by the build in the previous step. 
    - It is generally located in `sdl-android-builder/app/.cxx/<Debug, Release, etc>/<some hash>/<architecture>/main.rvedata`
7. copy or symlink `main.rvedata` to `sdl-android-builder/app/src/main/assets`. Create the `assets` directory if it does not exist.
    - You will need to copy it again if you make any changes to `main.rvedata` (such as adding, removing, or changing any assets)
8. Follow the steps outlined on https://developer.android.com/ndk/guides/graphics/validation-layer to add a copy of the Vulkan validation layers to your project.
9. Press the green Run-Debug button in Android Studio. If all goes well, your game should boot up and begin running! If an exception occurs at runtime, you should get a breakpoint in Android studio.

It is highly recommended that you develop your game on any other platform first, and compile for Android only when you absolutely need it running there. The HelloCube example can be run on Android, if you want to compare your configuration to that one. 
