# Using RavEngine on Android
For every platform except Android, RavEngine can create a ready-to-go IDE project which can build and run a complete game with a single click. Unfortunately, Android requires some additional manual effort to make work.

1. Download a copy of RavEngine's [SDL Android Builder](https://github.com/RavEngine/sdl-android-builder) project and place it somewhere __outside__ your game repository.
2. Symlink your game repository into the `sdl-android-builder/to-build` directory.
3. Modify your game's root `CMakeLists.txt` to change the main executable target such that:
   - It is a shared library rather than an executable, via `add_library`, and the target is named `main`.
      - you can find an example of how to do this in the HelloCube sample: https://github.com/RavEngine/HelloCube/blob/84222b9b846862321c07b016609a840637cbe589/CMakeLists.txt#L22-L30
4. Open the root `sdl-android-builder` project in Android Studio. Make any changes necessary to `app/build.gradle` such that your project is able to compile. For example, `minSDKVersion` must be set high enough that Vulkan 1.3 is enabled.
5. Press the green Run-Debug button in Android Studio. If all goes well, your game should boot up and begin running! If an exception occurs at runtime, you should get a breakpoint in Android studio.

The HelloCube example can be run on Android, if you want to compare your configuration to that one. 
