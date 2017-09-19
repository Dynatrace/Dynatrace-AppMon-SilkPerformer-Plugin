//-----------------------------------------------------------------------------
#ifndef _PIdynaTraceDiagnostics_h
#define _PIdynaTraceDiagnostics_h
//-----------------------------------------------------------------------------
#include <apr_global_mutex.h>
#include "pnapi/String.h"
#include <vector>

// Global Defines
#define DTD_APPNAME "dtdclient"

#ifdef _DEBUG
#define DLOG(x) Log(eSeverityInfo, x, 0)
#else
#define DLOG(x) (void) 0;
#endif

#ifndef ULONG_MAX
  #define ULONG_MAX ((ULONG)-1)
#endif

#if _MSC_VER >= 1300    // for VC 7.0
  // from ATL 7.0 sources
  #ifndef _delayimp_h
  extern "C" IMAGE_DOS_HEADER __ImageBase;
  #endif
#endif

// global extern deffinitions
extern apr_global_mutex_t *mutex;

using namespace pnapi;

struct page_type;
typedef struct page_type page;

struct page_type {
	String pageName;
	ULONG uid;
	String dtHeader;
	String dtApiBDString;
	String dtUrlAbs;
	std::vector<page> subpages;
};

typedef struct loadtestdata_type {
	String sessionID;
} loadtestdata;


// Function prototypes --- helpers

/// \addtogroup PluginHelpers 
/// Helper functions for the plugin
/// @{

void Log (EPI_Severity eSeverity, LPCSTR pszLogMsg, ULONG uCode);

void TextLog(LPCSTR pszLogMsg);

void getVersionInfo(HMODULE hmodule);

int getWaitTime(void);

String getSystemProfile(void);

int getClientPort(void);

TContent *renderSimpleContentPage(String strDTHeader, String strApi, LPCTSTR lpszUrlAbs, LPCTSTR lpszPageName, page sSubPageInfo);

void renderSubPage(TContent *pageContent, page pg, BOOL linkPage);

TContent *renderComplexPage(page pg);

/// Checks, if the current Loadtest is really a loadtest. Returns FALSE, if OnEvent-StartLoadtest was called
/// when starting a TryScript or FindBasline Functionality in Silkperformer.
/// This function makes use of the GetInfo callback function
///
/// \return TRUE, if this is a real loadtest, FALSE in case of TryScript or FindBaseLine a.s.o.
BOOL checkIsLoadTest(void);

/// Wrapper Function for the GetInfo Callback function. Safely wraps the TFNInfo callback.
/// always returns ruSize=0 and 0 if the Callback is not registered yet
///
/// \see {TFNInfo} for more information
int GetInfo(HPILTCONTEXT hLt, EPI_PerformerInfo eInformation, char *pszBuffer, unsigned long &ruSize);

/// checks Registry entry for "IsSessionRecording" and Provide activation state. If Value is not found,
/// the default is returned (TRUE)
///
/// \return TRUE, if SessionRecording should be started, FALSE otherwise
BOOL checkIsSessionRecording(void);

/// Reads the Project-Name and the Loadtestname from the GetInfo callback function.
/// Is intended to be called when PI_OnEvent is called with "LoadTestStart" - Event
/// Concats the Projectname with an underbar ("_") if both elements are present.
/// Otherwise the present element is returned. If nothing can be read - an empty string is returned.
///
/// \param [in] param Provided with PI_OnEvent - for future use
/// \return the Projectname and the LoadtestName concatinated with an underbar
String getRecordSessionName(void *param);

/// @}
#endif
