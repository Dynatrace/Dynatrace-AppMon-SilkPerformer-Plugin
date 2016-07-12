
#ifndef _piWorkbenchPluginTypes_h
#define _piWorkbenchPluginTypes_h

//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------

#define TEXT_TRUE   "True"
#define TEXT_FALSE  "False"


//-----------------------------------------------------------------------------
/// Bit-mask used by Initialize, Get/Set Attributes, Get/Set Features
#define SPPI_MOD_PERFORMER             1
#define SPPI_MOD_TRUELOGEXPLORER       2
#define SPPI_MOD_PERFORMANCEEXPLORER   4
#define SPPI_MOD_ALL                  -1


typedef enum EPI_SeverityTag
{
  eSeverityError,
  eSeverityWarning,
  eSeverityInfo,

  eSeverityLast
} EPI_Severity;

typedef enum EPI_TypeTag
{
  eTypeInvalid,
  eTypeString,
  eTypeFloat,
  eTypeNumber,
  eTypeBoolean,
  eTypeLastPI
} EPI_Type;

static const char* s_TypeText[] = 
{
  "invalid",
  "string",
  "float",
  "number",
  "boolean",
  ""
};

// TestType enum 
// do not modify order of values 
// only extend enum by adding values to the end
// used for translating the the performer internal values to values all plugins understand
enum EPI_TestType
{
  ePI_TestType_Invalid = 0,

  ePI_TestType_SingleUserTest,   // single-user run from right mouse of bdf tree list
  ePI_TestType_SingleScriptTest, // single-script run from right mouse of bdf script
  ePI_TestType_TryScriptTest,    // Try Script run
  ePI_TestType_BaseLineTest,     // Base Line run
  ePI_TestType_LoadTest,         // Load Test run
  ePI_TestType_VerificationTest, // Verification Test run

  ePI_TestType_Last
};

typedef enum EPI_EventTag
{
  eEventStart,    ///< triggered when loadtest is about to be started
  eEventStarted,  ///< triggered when loadtest has been started
  eEventStop,     ///< triggered when loadtest has been stopped
  eEventKill,     ///< triggered when loadtest has been killed

  eEventUserChange, // Not supported
  eEventRampUp,     // Not supported
  eEventRampDown,   // Not supported
  eEventStep,       // Not supported
  eEventRampupMax,  // Not supported
  eEventWarmupBegin,// Not supported
  eEventWarmupEnd,  // Not supported
  eEventShutdownBegin,// Not supported
  eEventShutdownEnd,  // Not supported

} EPI_Event;

typedef enum EPI_NotificationTag
{
  eNotificationLoadPlugin,
  eNotificationUnloadPlugin,
  eNotificationInitialized,

  eNotificationLast
} EPI_Notification;

/// Icons which can be requested by callee from the plugin.
typedef enum EPI_IconTag
{
  ePIIcon_Settings,         ///< Icon used by the settings dialog for this plugin.  
  ePIIcon_WindowTab,        ///< Icon used for windows which integrate into GUI (Workbench, TrueLogExplorer)  

  ePIIcon_Last,             ///< Enum may get extended in future
} EPI_Icon;

/// Information which may be requested by Performer Applications.
enum EPI_Info
{
  ePIInfo_CurrentSessionInfo, ///< May hold Current Session Id or any other data which is needed to identify the session, is requested after a loadtest has been started.
  ePIInfo_AnalyzeImageFile,   ///< File (Bmp, gif, ...) that should be used for Analyze Icon in Overview Report (Performance Explorer). Should include path relative to PlugIn dll.

  ePIInfo_Last,             ///< Enum may get extended in future
};

/// Information which may be requested by plugin from Performer Applications.
enum EPI_PerformerInfo
{
  ePIPInfo_RegKey_Plugin,           ///< Key to registry where PlugIn Settings are stored (PI may use this to store it's attributes data)
                                    /// Data is stored per user. 

  ePIPInfo_SessionInfo,             ///< The SessionInfo retrieved from plugin when loadtest was run.

  ePIPInfo_ProjectName,             ///< Project Name of the Loadtest identified by hLT parameter of GetInfo. Value for Current running loadtest in Performer can be retrieved by providing NULL for hLT.
  ePIPInfo_ProjectDescription,      ///< Project Description of the Loadtest identified by hLT parameter of GetInfo. Value for Current running loadtest in Performer can be retrieved by providing NULL for hLT.
  ePIPInfo_ProjectType,             ///< Project Type (Number) of the Loadtest identified by hLT parameter of GetInfo. Value for Current running loadtest in Performer can be retrieved by providing NULL for hLT.
  ePIPInfo_ProjectTypeText,         ///< Project Type Text of the Loadtest identified by hLT parameter of GetInfo. Value for Current running loadtest in Performer can be retrieved by providing NULL for hLT.
  ePIPInfo_LoadtestName,			///< Name of the Loadtest identified by hLT parameter of GetInfo. Value for Current running loadtest in Performer can be retrieved by providing NULL for hLT.
  ePIPInfo_LoadtestDescription,     ///< Loadtest Description of the Loadtest identified by hLT parameter of GetInfo. Value for Current running loadtest in Performer can be retrieved by providing NULL for hLT.
  ePIPInfo_LoadtestType,            ///< Loadtest Type (EPI_TestType) of the Loadtest identified by hLT parameter of GetInfo. Value for Current running loadtest in Performer can be retrieved by providing NULL for hLT.
  ePIPInfo_LoadtestTypeText,        ///< Loadtest Type Text of the Loadtest identified by hLT parameter of GetInfo. Value for Current running loadtest in Performer can be retrieved by providing NULL for hLT.
  ePIPInfo_ResultsDirectory,        ///< Resultsdirectory of the Loadtest identified by hLT parameter of GetInfo. Value for Current running loadtest in Performer can be retrieved by providing NULL for hLT.
  ePIPInfo_Last,                    ///< Enum may get extended in future
};


typedef void (*TMessageLogCallback)(void* pClient, EPI_Severity eSeverity, const char* sMessage, unsigned long uCode);

/// \defgroup TrueLog   TrueLog Explorer
/// Interface for functions used by TrueLog Explorer.
/// @{

/// Internal Plugin structure which may store/cache data for one loadtest.
typedef void* HPILOADTEST;

/// Handle to Performer loadtest Context
/// Used for retrieving information in context to a given loadtest
typedef void* HPILTCONTEXT;


/// structure for returning data to the callee.
/// Used to provide html data to the callee.
/// Following links are supported inside the html:
/// - Home.html\n
///   Application will return to default html for current selection.
///   \code
///     <a href="Home.html"/>Home</a> 
///   \endcode
///   
/// - Content.html?pszContext\n
///   Application will call GetContent() using parmString for pszContext to get extra content for display.
///   \code 
///     <a href="Content.html?Details"/>Details</a> 
///   \endcode
///   
/// - File.html?pszRelFile\n
///   Application will embed the file <PlugIn-Directory>/images/logo.gif into the html.
///   \code 
///     <img href="File.html?images/logo.gif"/>
///   \endcode
///   
/// - Action.html?pszContext\n
///   Application will call IndicateAction providing pszContext on the dll.
///   \code 
///     <a href="Action.html?CmdBeep">Beep</a>
///   \endcode
///   
/// - Node.html?NodeId\n
///   Application will select the node with the given id.
///   The id is provided by functions like:\n
///     - PI_BeginPage()
///     - PI_AddDocument()
///     - PI_GetContent()
///     .
///   \code 
///     <a href="Node.html?8745361">Document1</a>
///   \endcode
///
/// \see PI_EndPage(), PI_AddDocument(), PI_GetContent()
struct TContent{
  const char* pszContentType; ///< e.g. "text/html" or "text"
  const char* pszContent;
  unsigned long lLength;
};

/// Callback function type for selecting nodes
/// \param[in] pContext provided when registering the callback function
/// \param[in] uId id of node to select
/// \see PI_SetFNSelectNode()
typedef void (*TFNSelectNode)(void* pContext, unsigned long uId);

/// Callback function type for requesting information from application
/// \param[in] pContext   provided when registering the callback function
/// \param[in] hLt        provided when initializing a loadtest #PI_InitializeLoadtest. 
///                       Only used for loadtest context information.
///                       When retrieving information from Performer about current running 
///                       loadtest, this parameter has to be null.
/// \param[in] eInformation indicates which information is required
/// \param[in] pszBuffer buffer where application should store the result
/// \param[in,out] ruSize size of buffer where application should store result. 
///                       If application needs more memory, then FALSE is returned and ruSize is 
///                       adopted to needed size.
/// \retval FALSE if error occurred, or buffer is too small. (Error is indicated by setting ruSize to 0)
/// \see PI_SetFNInfo(), EPI_PerformerInfo
typedef int (*TFNInfo )(void* pContext, HPILTCONTEXT hLt, EPI_PerformerInfo eInformation, char* pszBuffer, unsigned long& ruSize);


/// @}

//-----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//-----------------------------------------------------------------------------

#endif

