#!/usr/bin/python3 -i
#
# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
# Copyright 2023 RasterGrid Kft.
#
# SPDX-License-Identifier: Apache-2.0

class PlatformGuardHelper():
    """Used to elide platform guards together, so redundant #endif then #ifdefs are removed
    Note - be sure to call addGuard(None) when done to add a trailing #endif if needed
    """
    def __init__(self):
        self.currentGuard = None

    def addGuard(self, guard):
        out = []
        if self.currentGuard != guard:
            if self.currentGuard != None:
                out.append(f'#endif  // {self.currentGuard}\n')
            if guard != None:
                out.append(f'#ifdef {guard}\n')
            self.currentGuard = guard
        return out
