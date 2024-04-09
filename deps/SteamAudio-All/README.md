# SteamAudio-All

Valve's SteamAudio, ready to compile with all dependencies as source. Note that export targets and post-build steps for the Unity/Unreal/Fmod plugins have been removed, and the library type has been set to static. 

### Building

```cmake
add_subdirectory(SteamAudio-All EXCLUDE_FROM_ALL)
# ...
target_link_libraries(your-game PRIVATE phonon)
```
