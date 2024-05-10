// Copyright 2017 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using FMODUnity;

/// This is the main Resonance Audio class that communicates with the FMOD Unity integration. Native
/// functions of the system can only be called through this class to preserve the internal system
/// functionality.
public static class FmodResonanceAudio {
  /// Updates the room effects of the environment with given |room| properties.
  /// @note This should only be called from the main Unity thread.
  public static void UpdateAudioRoom(FmodResonanceAudioRoom room, bool roomEnabled) {
    // Update the enabled rooms list.
    if (roomEnabled) {
      if (!enabledRooms.Contains(room)) {
        enabledRooms.Add(room);
      }
    } else {
      enabledRooms.Remove(room);
    }
    // Update the current room effects to be applied.
    if (enabledRooms.Count > 0) {
      FmodResonanceAudioRoom currentRoom = enabledRooms[enabledRooms.Count - 1];
      RoomProperties roomProperties = GetRoomProperties(currentRoom);
      // Pass the room properties into a pointer.
      IntPtr roomPropertiesPtr = Marshal.AllocHGlobal(roomPropertiesSize);
      Marshal.StructureToPtr(roomProperties, roomPropertiesPtr, false);
      ListenerPlugin.setParameterData (roomPropertiesIndex, GetBytes(roomPropertiesPtr,
                                                                     roomPropertiesSize));
      Marshal.FreeHGlobal(roomPropertiesPtr);
    } else {
      // Set the room properties to a null room, which will effectively disable the room effects.
      ListenerPlugin.setParameterData (roomPropertiesIndex, GetBytes(IntPtr.Zero, 0));
    }
  }

  /// Returns whether the listener is currently inside the given |room| boundaries.
  public static bool IsListenerInsideRoom(FmodResonanceAudioRoom room) {
    // Compute the room position relative to the listener.
    FMOD.VECTOR unused;
    RuntimeManager.LowlevelSystem.get3DListenerAttributes(0, out listenerPositionFmod, out unused,
                                                          out unused, out unused);
    Vector3 listenerPosition = new Vector3(listenerPositionFmod.x, listenerPositionFmod.y,
                                           listenerPositionFmod.z);
    Vector3 relativePosition = listenerPosition - room.transform.position;
    Quaternion rotationInverse = Quaternion.Inverse(room.transform.rotation);
    // Set the size of the room as the boundary and return whether the listener is inside.
    bounds.size = Vector3.Scale(room.transform.lossyScale, room.size);
    return bounds.Contains(rotationInverse * relativePosition);
  }

  /// Maximum allowed gain value in decibels.
  public const float maxGainDb = 24.0f;

  /// Minimum allowed gain value in decibels.
  public const float minGainDb = -24.0f;

  /// Maximum allowed reverb brightness modifier value.
  public const float maxReverbBrightness = 1.0f;

  /// Minimum allowed reverb brightness modifier value.
  public const float minReverbBrightness = -1.0f;

  /// Maximum allowed reverb time modifier value.
  public const float maxReverbTime = 3.0f;

  /// Maximum allowed reflectivity multiplier of a room surface material.
  public const float maxReflectivity = 2.0f;

  [StructLayout(LayoutKind.Sequential)]
  private struct RoomProperties {
    // Center position of the room in world space.
    public float positionX;
    public float positionY;
    public float positionZ;

    // Rotation (quaternion) of the room in world space.
    public float rotationX;
    public float rotationY;
    public float rotationZ;
    public float rotationW;

    // Size of the shoebox room in world space.
    public float dimensionsX;
    public float dimensionsY;
    public float dimensionsZ;

    // Material name of each surface of the shoebox room.
    public FmodResonanceAudioRoom.SurfaceMaterial materialLeft;
    public FmodResonanceAudioRoom.SurfaceMaterial materialRight;
    public FmodResonanceAudioRoom.SurfaceMaterial materialBottom;
    public FmodResonanceAudioRoom.SurfaceMaterial materialTop;
    public FmodResonanceAudioRoom.SurfaceMaterial materialFront;
    public FmodResonanceAudioRoom.SurfaceMaterial materialBack;

    // User defined uniform scaling factor for reflectivity. This parameter has no effect when set
    // to 1.0f.
    public float reflectionScalar;

    // User defined reverb tail gain multiplier. This parameter has no effect when set to 0.0f.
    public float reverbGain;

    // Adjusts the reverberation time across all frequency bands. RT60 values are multiplied by this
    // factor. Has no effect when set to 1.0f.
    public float reverbTime;

    // Controls the slope of a line from the lowest to the highest RT60 values (increases high
    // frequency RT60s when positive, decreases when negative). Has no effect when set to 0.0f.
    public float reverbBrightness;
  };

  // Returns the FMOD Resonance Audio Listener Plugin.
  private static FMOD.DSP ListenerPlugin {
    get {
      if (!listenerPlugin.hasHandle()) {
        listenerPlugin = Initialize();
      }
      return listenerPlugin;
    }
  }

  // Converts given |db| value to its amplitude equivalent where 'dB = 20 * log10(amplitude)'.
  private static float ConvertAmplitudeFromDb (float db) {
    return Mathf.Pow(10.0f, 0.05f * db);
  }

  // Converts given |position| and |rotation| from Unity space to audio space.
  private static void ConvertAudioTransformFromUnity (ref Vector3 position,
    ref Quaternion rotation) {
    // Compose the transformation matrix.
    Matrix4x4 transformMatrix = Matrix4x4.TRS(position, rotation, Vector3.one);
    // Convert the transformation matrix from left-handed to right-handed.
    transformMatrix = flipZ * transformMatrix * flipZ;
    // Update |position| and |rotation| respectively.
    position = transformMatrix.GetColumn(3);
    rotation = Quaternion.LookRotation(transformMatrix.GetColumn(2), transformMatrix.GetColumn(1));
  }

  // Returns a byte array of |length| created from |ptr|.
  private static byte[] GetBytes(IntPtr ptr, int length) {
    if (ptr != IntPtr.Zero) {
      byte[] byteArray = new byte[length];
      Marshal.Copy(ptr, byteArray, 0, length);
      return byteArray;
    }
    // Return an empty array if the pointer is null.
    return new byte[1];
  }

  // Returns room properties of the given |room|.
  private static RoomProperties GetRoomProperties(FmodResonanceAudioRoom room) {
    RoomProperties roomProperties;
    Vector3 position = room.transform.position;
    Quaternion rotation = room.transform.rotation;
    Vector3 scale = Vector3.Scale(room.transform.lossyScale, room.size);
    ConvertAudioTransformFromUnity(ref position, ref rotation);
    roomProperties.positionX = position.x;
    roomProperties.positionY = position.y;
    roomProperties.positionZ = position.z;
    roomProperties.rotationX = rotation.x;
    roomProperties.rotationY = rotation.y;
    roomProperties.rotationZ = rotation.z;
    roomProperties.rotationW = rotation.w;
    roomProperties.dimensionsX = scale.x;
    roomProperties.dimensionsY = scale.y;
    roomProperties.dimensionsZ = scale.z;
    roomProperties.materialLeft = room.leftWall;
    roomProperties.materialRight = room.rightWall;
    roomProperties.materialBottom = room.floor;
    roomProperties.materialTop = room.ceiling;
    roomProperties.materialFront = room.frontWall;
    roomProperties.materialBack = room.backWall;
    roomProperties.reverbGain = ConvertAmplitudeFromDb(room.reverbGainDb);
    roomProperties.reverbTime = room.reverbTime;
    roomProperties.reverbBrightness = room.reverbBrightness;
    roomProperties.reflectionScalar = room.reflectivity;
    return roomProperties;
  }

  // Initializes and returns the FMOD Resonance Audio Listener Plugin.
  private static FMOD.DSP Initialize() {
    // Search through all busses on in banks.
    int numBanks = 0;
    FMOD.DSP dsp = new FMOD.DSP();
    FMOD.Studio.Bank[] banks = null;
    RuntimeManager.StudioSystem.getBankCount(out numBanks);
    RuntimeManager.StudioSystem.getBankList(out banks);
    for (int currentBank = 0; currentBank < numBanks; ++currentBank) {
      int numBusses = 0;
      FMOD.Studio.Bus[] busses = null;
      banks[currentBank].getBusCount(out numBusses);
      banks[currentBank].getBusList(out busses);
      RuntimeManager.StudioSystem.flushCommands();
      for (int currentBus = 0; currentBus < numBusses; ++currentBus) {
        // Make sure the channel group of the current bus is assigned properly.
        string busPath = null;
        busses[currentBus].getPath(out busPath);
        RuntimeManager.StudioSystem.getBus(busPath, out busses[currentBus]);
        RuntimeManager.StudioSystem.flushCommands();
        FMOD.ChannelGroup channelGroup;
        busses[currentBus].getChannelGroup(out channelGroup);
        RuntimeManager.StudioSystem.flushCommands();
        if (channelGroup.hasHandle()) {
          int numDsps = 0;
          channelGroup.getNumDSPs(out numDsps);
          for (int currentDsp = 0; currentDsp < numDsps; ++currentDsp) {
            channelGroup.getDSP(currentDsp, out dsp);
            string dspNameSb;
            int unusedInt = 0;
            uint unusedUint = 0;
            dsp.getInfo(out dspNameSb, out unusedUint, out unusedInt, out unusedInt, out unusedInt);
            if (dspNameSb.ToString().Equals(listenerPluginName) && dsp.hasHandle()) {
              return dsp;
            }
          }
        }
      }
    }
    Debug.LogError(listenerPluginName + " not found in the FMOD project.");
    return dsp;
  }

  // Right-handed to left-handed matrix converter (and vice versa).
  private static readonly Matrix4x4 flipZ = Matrix4x4.Scale(new Vector3(1, 1, -1));

  // Get a handle to the Resonance Audio Listener FMOD Plugin.
  private static readonly string listenerPluginName = "Resonance Audio Listener";

  // Size of |RoomProperties| struct in bytes.
  private static readonly int roomPropertiesSize = Marshal.SizeOf(typeof(RoomProperties));

  // Plugin data parameter index for the room properties.
  private static readonly int roomPropertiesIndex = 1;

  // Boundaries instance to be used in room detection logic.
  private static Bounds bounds = new Bounds(Vector3.zero, Vector3.zero);

  // Container to store the currently active rooms in the scene.
  private static List<FmodResonanceAudioRoom> enabledRooms = new List<FmodResonanceAudioRoom>();

  // Current listener position.
  private static FMOD.VECTOR listenerPositionFmod = new FMOD.VECTOR();

  // FMOD Resonance Audio Listener Plugin.
  private static FMOD.DSP listenerPlugin;
}
