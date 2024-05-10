/*
Copyright 2018 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_WIN_APP_H_
#define RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_WIN_APP_H_

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be
                                            // explicit

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdisp.h>   // MFC Automation classes
#include <afxodlgs.h>  // MFC OLE dialog classes
#include <afxole.h>    // MFC OLE classes
#endif                 // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>  // MFC ODBC database classes
#endif              // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>  // MFC DAO database classes
#endif               // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>  // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>  // MFC support for Windows Common Controls
#endif               // _AFX_NO_AFXCMN_SUPPORT

// Windows app that encapsulates the Authoring plugins for the VrAudio Wwise
// integration.
class WwiseWinApp : public CWinApp {
 public:
  WwiseWinApp() = default;

  // Initializes WwiseRendererApp.
  virtual BOOL InitInstance();

  DECLARE_MESSAGE_MAP()
};

#endif  // RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_WIN_APP_H_
