/*
***********************************************************************
README.TXT file 
***********************************************************************
Copyright © 2011 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.

AMD is granting you permission to use this software and documentation (if any) (collectively, the “Materials”) pursuant to the terms and conditions of the Software Development Kit License Agreement included with the Materials.  This header does NOT give you permission to use the Materials or any rights under AMD’s intellectual property.  Your use of any portion of these Materials shall constitute your acceptance of the terms and conditions of such Software Development Kit License Agreement.  If you do not agree to the terms and conditions of the Software Development Kit License Agreement, you do not have permission to use any portion of these Materials.  If you do not have a copy of the Software Development Kit License Agreement, contact your AMD representative for a copy.

CONFIDENTIALITY:  The Materials and all other related information, identified as confidential and provided to you by AMD shall be kept confidential in accordance with the terms and conditions of the Software Development Kit License Agreement.  If no Software Development Kit License Agreement exists between you and AMD, you agree to keep these Materials confidential and to not disclose it to any third party.

LIMITATION OF LIABILITY: THE MATERIALS ARE PROVIDED “AS IS” WITHOUT ANY EXPRESS OR IMPLIED WARRANTY OF ANY KIND INCLUDING WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT OF THIRD-PARTY INTELLECTUAL PROPERTY, TITLE, OR FITNESS FOR ANY PARTICULAR PURPOSE, OR THOSE ARISING FROM CUSTOM OF TRADE OR COURSE OF USAGE.  
FOR CLARIFICATION, THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE MATERIALS REMAINS WITH YOU.  AMD DOES NOT WARRANT, GUARANTEE, OR MAKE ANY REPRESENTATIONS AS TO THE CORRECTNESS, ACCURACY, COMPLETENESS, QUALITY, OR RELIABILITY OF THE MATERIALS.  AMD DOES NOT WARRANT THAT OPERATION OF THE MATERIALS WILL BE UNINTERRUPTED OR ERROR-FREE.  YOU ARE RESPONSIBLE FOR DETERMINING THE APPROPRIATENESS OF USING THE SOFTWARE AND ASSUME ALL RISKS ASSOCIATED WITH THE USE OF THE MATERIALS, INCLUDING BUT NOT LIMITED TO THE RISKS OF PROGRAM ERRORS, DAMAGE TO OR LOSS OF DATA, PROGRAMS OR EQUIPMENT, AND UNAVAILABILITY OR INTERRUPTION OF OPERATIONS.  Some jurisdictions do not allow for the exclusion or limitation of implied warranties, so the above limitations or exclusions may not apply to You.
IN NO EVENT SHALL AMD OR ITS DIRECTORS, OFFICERS, EMPLOYEES AND AGENTS, ITS SUPPLIERS OR ITS LICENSORS BE LIABLE TO YOU OR ANY THIRD PARTIES IN RECEIPT OF THE MATERIALS UNDER ANY THEORY OF LIABILITY, WHETHER EQUITABLE, LEGAL OR COMMON LAW ACTION ARISING HEREUNDER FOR CONTRACT, STRICT LIABILITY, INDEMNITY, TORT (INCLUDING NEGLIGENCE), OR OTHERWISE FOR DAMAGES WHICH, IN THE AGGREGATE EXCEED TEN DOLLARS ($10.00).  IN NO EVENT SHALL AMD BE LIABLE FOR ANY CONSEQUENTIAL, INCIDENTAL, PUNITIVE OR SPECIAL DAMAGES, INCLUDING, BUT NOT LIMITED TO LOSS OF PROFITS, BUSINESS INTERRUPTION, OR LOSS OF INFORMATION ARISING OUT OF THE USE OF OR INABILITY TO USE THE MATERIALS, EVEN IF AMD HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  BY USING THE MATERIALS WITHOUT CHARGE, YOU ACCEPT THIS ALLOCATION OF RISK.  Because some jurisdictions prohibit the exclusion or limitation of liability for consequential or incidental damages, the above limitation may not apply to you.

AMD does not assume any responsibility for any errors which may appear in the Materials or any other related information provided to you by AMD, or result from use of the Materials or any related information.  

You agree that you will not reverse engineer or decompile the Materials, in whole or in part, except as allowed by applicable law.

NO SUPPORT OBLIGATION: AMD is not obligated to furnish, support, or make any further information, software, technical information, know-how, or show-how available to you.  Additionally, AMD retains the right to modify the Materials at any time, without notice, and is not obligated to provide such modified Materials to you.

U.S. GOVERNMENT RESTRICTED RIGHTS: The Materials are provided with "RESTRICTED RIGHTS." Use, duplication, or disclosure by the Government is subject to the restrictions as set forth in FAR 52.227-14 and DFAR252.227-7013, et seq., or its successor.  Use of the Materials by the Government constitutes acknowledgement of AMD's proprietary rights in them.

EXPORT RESTRICTIONS: You shall adhere to all applicable U.S., European, and other export laws, including but not limited to the U.S. Export Administration Regulations (“EAR”), (15 C.F.R. Sections 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify that, except pursuant to a license granted by the United States Department of Commerce Bureau of Industry and Security or as otherwise permitted pursuant to a License Exception under the EAR, you will not (1) export, re-export or release to a national of a country in Country Groups D:1, E:1 or E:2 any restricted technology, software, or source code it receives from AMD, or (2) export to Country Groups D:1, E:1 or E:2 the direct product of such technology or software, if such foreign produced direct product is subject to national security controls as identified on the Commerce Control List (currently found in Supplement 1 to Part 774 of EAR).  For the most current Country Group listings, or for additional information about the EAR or Your obligations under those regulations, please refer to the U.S. Bureau of Industry and Security’s website at http://www.bis.doc.gov/.
*/
#ifndef _ATIDX9STEREO_H_
#define _ATIDX9STEREO_H_

#define ATI_STEREO_VERSION_MAJOR 0
#define ATI_STEREO_VERSION_MINOR 3

// @todo Need to handle packing to guarantee correct layouts

// This include file contains all the DX9 Stereo Display extension definitions
// (structures, enums, constants) shared between the driver and the application

#pragma pack(push, 4)

#define FOURCC_AQBS        MAKEFOURCC('A','Q','B','S')

// SetSrcEye/SetDstEye Parameters
#define ATI_STEREO_LEFTEYE   0
#define ATI_STEREO_RIGHTEYE  1

// This is the enum for all the commands that can be sent to the driver in the
// surface communication packet

typedef enum _ATIDX9STEREOCOMMAND
{
    ATI_STEREO_GETVERSIONDATA            = 0,    // Return version data structure
    ATI_STEREO_ENABLESTEREO              = 1,    // Enable stereo
    ATI_STEREO_ENABLELEFTONLY            = 2,    // Enable stereo but only display left eye
    ATI_STEREO_ENABLERIGHTONLY           = 3,    // Enable stereo but only display right eye
    ATI_STEREO_ENABLESTEREOSWAPPED       = 4,    // Enable stereo but swap left and right eyes
    ATI_STEREO_GETLINEOFFSET             = 5,    // Return the line offset from end of left eye to beginning of right eye.
    ATI_STEREO_GETDISPLAYMODES           = 6,    // Return an array of all the supported stereo display modes in a TBD format
    ATI_STEREO_SETSRCEYE                 = 7,    // Sets the source eye for blts and surface copies (left/right eye specified in dwParams)
    ATI_STEREO_SETDSTEYE                 = 8,    // Sets the destination eye for blts and surface copies (left/right eye specified in dwParams)
    ATI_STEREO_ENABLEPERSURFAA           = 9,    // Create independent AA buffers for all multi-sample render targets (excluding the flip chain)
    ATI_STEREO_ENABLEPRIMARYAA           = 10,   // Enable AA for primaries when multi-sample fields in present params are set and stereo is enabled.
    ATI_STEREO_COMMANDMAX                = 11,   // Largest command enum.
    ATI_STEREO_FORCEDWORD                = 0xffffffff
} ATIDX9STEREOCOMMAND;

// Note: The following API calls are affected by SETSRCEYE and SETDSTEYE: Clear, StretchRect, GetBackBuffer, GetFrontBufferData, UpdateSurface


// Structure used to send commands and get data from the driver through the
// FOURCC_AQBS surface.  When a FOURCC_AQBS surface is created and locked,
// a pointer to this structure is returned.  If properly filled in, it will
// process the appropriate command when the surface is unlocked

typedef struct _ATIDX9STEREOCOMMPACKET
{
    DWORD               dwSignature;        // Signature to indicate to the driver that the app is sending a command
    DWORD               dwSize;             // Size of this structure.  Passed to the app on lock
    ATIDX9STEREOCOMMAND stereoCommand;      // Command given to the driver
    HRESULT             *pResult;           // Pointer to buffer where error code will be written to. D3D_OK if successful
    DWORD               dwOutBufferSize;    // Size of optional buffer to place outgoing data into (in bytes).  Must be set if data is returned
    BYTE                *pOutBuffer;        // Pointer to buffer for outgoing data. (lineoffset, displaymodes, etc)
    DWORD               dwInBufferSize;     // Size of optional buffer to place incoming parameters
    BYTE                *pInBuffer;         // Pointer to buffer for incoming data (SetSrcEye, SetDstEye, etc)
} ATIDX9STEREOCOMMPACKET;


typedef struct _ATIDX9STEREOVERSION
{
    DWORD   dwSize;             // Size of this structure
    DWORD   dwVersionMajor;     // Major Version
    DWORD   dwVersionMinor;     // Minor Version
    DWORD   dwMaxCommand;       // Max command enum
    DWORD   dwCaps;             // Stereo Caps (not implemented yet)
    DWORD   dwReserved[11];
} ATIDX9STEREOVERSION;

typedef struct _ATIDX9GETDISPLAYMODES
{
    UINT              dwNumModes;     // Number of stereo modes available.
    D3DDISPLAYMODE*   pStereoModes;   // List containing stereo mode details for all the modes.
} ATIDX9GETDISPLAYMODES;

#pragma pack(pop)

#endif // _ATIDX9STEREO_H_