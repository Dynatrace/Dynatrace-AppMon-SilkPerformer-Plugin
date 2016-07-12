// ----------------------------------------------------------------------------
/// \file piWorkbenchPluginInterface.h
// copyright: (c) Segue Software Inc., 2006
// ----------------------------------------------------------------------------
//   content: 
// ----------------------------------------------------------------------------
// history: 
// (HP 2006-03-20) :  removed dependencies of corporate internal datatypes 
//                    (e.g.: modified TString to LPCSTR ANSI)
// ----------------------------------------------------------------------------

#ifndef _piWorkbenchPluginInterface_h
#define _piWorkbenchPluginInterface_h

//-----------------------------------------------------------------------------

#include <windows.h>
#include "piWorkbenchPluginTypes.h"

#ifdef WIN32
  #ifdef _USRDLL
    #define PI_DLL_SPEC __declspec(dllexport)
  #else
    #define PI_DLL_SPEC __declspec(dllimport)
  #endif
#else
  #define PI_DLL_SPEC
#endif


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// General

/// \addtogroup General 
/// Interface for general information.
/// @{

/// Provides the name of this plugin.
/// Used by the settings page of the main application.
/// \param [in] uModule Bit-Mask to indicate for which module the callee needs the name
///             - #SPPI_MOD_PERFORMER
///             - #SPPI_MOD_TRUELOGEXPLORER
///             - #SPPI_MOD_PERFORMANCEEXPLORER
/// \return name of the plugin.
PI_DLL_SPEC LPCSTR PI_GetName                     (ULONG uModule);

/// Provides a description for this plugin.
/// Used to get description text for settings dialog.
/// \param [in] uModule Bit-Mask to indicate for which module the callee 
///                     needs the description
///             - #SPPI_MOD_PERFORMER
///             - #SPPI_MOD_TRUELOGEXPLORER
///             - #SPPI_MOD_PERFORMANCEEXPLORER
/// \return description of the plugin.
PI_DLL_SPEC LPCSTR PI_GetDescription              (ULONG uModule);

/// Provides the version for this plugin.
/// Used by the settings page of the main application.
/// \return version as string of the plugin.
PI_DLL_SPEC LPCSTR PI_GetVersion                  ();     

/// Provides icons for this plugin.
/// Used to get icons for settings page or detail page in TrueLogExplorer.
/// \param [in] uModule Bit-Mask to indicate for which module the callee 
///                     needs the description
///             - #SPPI_MOD_PERFORMER
///             - #SPPI_MOD_TRUELOGEXPLORER
///             - #SPPI_MOD_PERFORMANCEEXPLORER
/// \param [in] eIcon indicate type of icon requested.
/// \return handle to requested icon or NULL if not available.
/// \Note This function is optional and may not be supported by every plugin. 
/// However it is highly recommended that each plugin implements this function.
/// Otherwise default or no icons may be used in performer applications.
PI_DLL_SPEC HICON PI_GetIcon (ULONG uModule, EPI_Icon eIcon);     

/// Provides Information for callee.
/// Used for several purposes, see documentation of EPI_Info for details.
/// \param [in] uModule Bit-Mask to indicate for which module the callee needs information
///             - #SPPI_MOD_PERFORMER
///             - #SPPI_MOD_TRUELOGEXPLORER
///             - #SPPI_MOD_PERFORMANCEEXPLORER
/// \param [in]     hLT (for general information this may be NULL) only set when loadtest dependend data is needed.
/// \param [in]     ePIInfo indicates which type of information is required
/// \param [in]     pszContext may point to additional information (usage depends on EPI_Info)
/// \param [in]     pszBuffer buffer where plugin will store results
/// \param [in,out] ruSize size of buffer where plugin will store results (Plugin will return required size if needed)
/// \retval FALSE if error occurred. Use GetLastErrorMessage to retrieve 
///               error message.
/// \Todo to be verified
/// \Note 
/// General function for retrieving information from the plugin.
/// \see EPI_Info
PI_DLL_SPEC BOOL  PI_GetInfo (ULONG  uModule, HPILOADTEST hLT, EPI_Info ePIInfo, LPCSTR pszContext, LPSTR pszBuffer, ULONG&  ruSize);

/// Sets the callback function for requesting information from the application.
/// \param[in] fnInfo pointer to function which should be called.
/// \param[in] pContext extra parameter which needs to be provided when calling that function.
/// \Note This function is optional and may not be supported by every plugin.
/// \see TFNInfo()
PI_DLL_SPEC 
void PI_SetFNInfo (TFNInfo fnInfo, void* pContext);
/// Forward to PI_SetFNInfo
PI_DLL_SPEC 
void PI_SetFnInfo (TFNInfo fnInfo, void* pContext);

/// Provides information for the last error.
/// Used by callee to provide extra error information to the user.
/// \return error message.
PI_DLL_SPEC LPCSTR PI_GetLastErrorMessage         ();

/// @}

//-----------------------------------------------------------------------------
/// \addtogroup Persistence Settings Persistence
/// Interface for saving and loading settings.
/// @{

/// Save the current configuration (Attributes settings)
/// Will be called by the main application to force storage of current settings.
/// \retval FALSE if error occurred. Use GetLastErrorMessage to retrieve 
///               error message.
PI_DLL_SPEC BOOL PI_Save                          ();

/// Load the settings and initialize internal structures.
/// Will be called by the main application to initialize the plugin.
/// \param [in] uModule Bit-Mask to indicate for which modules the plugin has 
///                     to be initialized.
///             Any combination of:
///             - #SPPI_MOD_PERFORMER
///             - #SPPI_MOD_TRUELOGEXPLORER
///             - #SPPI_MOD_PERFORMANCEEXPLORER
/// \retval FALSE if error occurred. Use GetLastErrorMessage to retrieve error message.
PI_DLL_SPEC BOOL PI_Initialize                    (ULONG uModule);

/// Call this member before freeing the library.
/// This will DeInitialize the internal structures of the plugin.
/// \Note This function is optional and may not be supported by every plugin.
PI_DLL_SPEC void PI_DeInitialize                  ();
/// @}

//-----------------------------------------------------------------------------
// Attributes

/// \addtogroup Attributes   Attributes
/// Attributes interface of plugins.
/// Functions for iterating over attributes and retrieving Attributes values, 
/// description or setting their values.
/// Used by settings pages of the callee.
/// @{

/// Get name of first attribute.
/// retrieves the name of the first attribute and initializes internal "iterator".
/// \param [in] uModule Bit-Mask to indicate for which modules the callee 
///                     wants to iterate the attributes list
///             Any combination of: 
///             - #SPPI_MOD_PERFORMER
///             - #SPPI_MOD_TRUELOGEXPLORER
///             - #SPPI_MOD_PERFORMANCEEXPLORER
/// \param [in, out] sName if null than the function will return required size in ruSize.
/// \param [in, out] ruSize callee has to provide available size in Buffer, 
///                         function will return used or needed memory in this parameter.
/// \param [out] eType specifies which SilkPerformer module uses the attribute.
/// \retval  TRUE   if successful
/// \retval  FALSE  if no attributes are available or error occurred. 
///                 If no attribute available ruSize will be set to 0.
PI_DLL_SPEC BOOL PI_GetFirstPluginAttribute       (ULONG uModule, LPSTR sName,   ULONG& ruSize, EPI_Type& eType);

/// Get name of next attribute.
/// retrieves the name of the next attribute and moves internal "iterator" forward.
/// \param [in, out] sName if null than the function will return required size in ruSize.
/// \param [in, out] ruSize callee has to provide available size in Buffer, 
///                         function will return used or needed memory in this parameter.
/// \param [out] eType specifies which SilkPerformer module uses the attribute.
/// \retval  TRUE   if successful
/// \retval  FALSE  if no more attributes are available or error occurred. 
///                 If no more attributes available, ruSize will be set to 0.
PI_DLL_SPEC BOOL PI_GetNextPluginAttribute        (LPSTR sName,   ULONG& ruSize, EPI_Type& eType);

/// Get description of specific attribute.
/// retrieves the description of a specific attribute.
/// \param [in, out] sDescription if null than the function will return 
///                               required size in ruSize.
/// \param [in]      uValueSize available size in buffer.
/// \param [in, out] ruSize callee has to provide available size in Buffer, 
///                         function will return used or needed memory in 
///                         this parameter.
/// \retval  TRUE   if successful
/// \retval  FALSE  on error. Call GetLastErrorMessage for details. 
///                 If memory too small than ruSize will be > uValueSize.
PI_DLL_SPEC BOOL PI_GetPluginAttributeDescription (LPCSTR csName, LPSTR sDescription, ULONG  uValueSize, ULONG& ruSize);

/// Get string value of specific attribute.
/// retrieves the string value of a specific attribute.
/// \param [in, out] sValue if null than the function will return 
///                         required size in ruSize.
/// \param [in]      uValueSize available size in sValue.
/// \param [in, out] ruSize callee has to provide available size in Buffer, 
///                         function will return used or needed memory in 
///                         this parameter.
/// \param [in, out] sDefValue if null than the function will return required 
///                            size in ruDefSize.
/// \param [in]      uDefValueSize available size in sDefValue.
/// \param [in, out] ruDefSize callee has to provide available size in Buffer, 
///                            function will return used or needed memory in 
///                            this parameter.
/// \retval  TRUE   if successful
/// \retval  FALSE  on error. Call GetLastErrorMessage for details. 
///                 If memory too small than either ruSize will be > uValueSize 
///                 or ruDefSize > uDefValueSize.
PI_DLL_SPEC BOOL PI_GetPluginAttributeString      (LPCSTR csName, LPSTR sValue, ULONG uValueSize, ULONG& ruSize, LPSTR sDefValue, ULONG uDefValueSize, ULONG& ruDefSize);

PI_DLL_SPEC BOOL PI_GetPluginAttributeNumber      (LPCSTR csName, LONG&   riVal, LONG&    riDefVal);                                                                         
PI_DLL_SPEC BOOL PI_GetPluginAttributeBoolean     (LPCSTR csName, BOOL&   rbVal, BOOL&    rbDefVal);                                    
PI_DLL_SPEC BOOL PI_GetPluginAttributeFloat       (LPCSTR csName, DOUBLE& rfVal, DOUBLE&  rfDefVal);

PI_DLL_SPEC BOOL PI_SetPluginAttributeString      (LPCSTR csName, LPCSTR  csValue);
PI_DLL_SPEC BOOL PI_SetPluginAttributeNumber      (LPCSTR csName, LONG    iVal);
PI_DLL_SPEC BOOL PI_SetPluginAttributeBoolean     (LPCSTR csName, BOOL    bVal);                                    
PI_DLL_SPEC BOOL PI_SetPluginAttributeFloat       (LPCSTR csName, DOUBLE  fVal);
/// @}


//-----------------------------------------------------------------------------
// Features

/// \addtogroup Features   Features
/// Features interface of plugins.
/// Functions for iterating over features and retrieving features values, 
/// description or setting their values.
/// Used by settings pages of the callee.
/// @{

/// Get name of first Feature.
/// retrieves the name of the first feature and initializes internal "iterator".
/// \param [in] uModule Bit-Mask to indicate for which modules the callee wants 
/// to iterate the features list
///             Any combination of: 
///             - #SPPI_MOD_PERFORMER
///             - #SPPI_MOD_TRUELOGEXPLORER
///             - #SPPI_MOD_PERFORMANCEEXPLORER
/// \param [in, out] sName Name of the feature
/// \param [in, out] ruSize callee has to provide available size in Buffer, 
/// \param [out] eType specifies which SilkPerformer module uses the attribute.
/// function will return used or needed memory in this parameter.
/// \retval  TRUE   if successful
/// \retval  FALSE  if no features are available or error occurred. 
/// If no features available ruSize will be set to 0.
PI_DLL_SPEC BOOL PI_GetFirstPluginFeature         (ULONG uModule, LPSTR sName, ULONG& ruSize, EPI_Type& eType);

/// Get name of next feature.
/// retrieves the name of the next feature and moves internal "iterator" forward.
/// \param [in, out] sName Name of the feature
/// \param [in, out] ruSize callee has to provide available size in Buffer, 
/// \param [out] eType specifies which SilkPerformer module uses the attribute.
/// \retval  TRUE   if successful
/// \retval  FALSE  if no more features are available or error occurred. 
/// If no more features available, ruSize will be set to 0.
PI_DLL_SPEC BOOL PI_GetNextPluginFeature          (LPSTR sName, ULONG& ruSize, EPI_Type& eType);

/// Get description of specific feature.
/// retrieves the description of a specific feature.
/// \retval  TRUE   if successful
/// \retval  FALSE  on error. Call GetLastErrorMessage for details.
/// If memory too small than ruSize will be > uValueSize.
PI_DLL_SPEC BOOL PI_GetPluginFeatureDescription   (LPCSTR csName, LPSTR sDescription, ULONG uValueSize, ULONG& ruSize);

/// Get string value of specific feature.
/// retrieves the string value of a specific feature.
/// \retval  TRUE   if successful
/// \retval  FALSE  on error. Call GetLastErrorMessage for details. 
/// If memory too small than either ruSize will be > uValueSize 
/// or ruDefSize > uDefValueSize.
PI_DLL_SPEC BOOL PI_GetPluginFeatureString        (LPCSTR csName, LPSTR sVal, ULONG uValueSize, ULONG& ruSize, LPSTR sDefVal, ULONG uDefValueSize, ULONG& ruDefSize);

PI_DLL_SPEC BOOL PI_GetPluginFeatureNumber        (LPCSTR csName, LONG&   riVal, LONG&    riDefVal);
PI_DLL_SPEC BOOL PI_GetPluginFeatureBoolean       (LPCSTR csName, BOOL&   rbVal, BOOL&    rbDefVal);
PI_DLL_SPEC BOOL PI_GetPluginFeatureFloat         (LPCSTR csName, DOUBLE& rfVal, DOUBLE&  rfDefVal);

PI_DLL_SPEC BOOL PI_SetPluginFeatureString        (LPCSTR csName, LPCSTR  csValue);
PI_DLL_SPEC BOOL PI_SetPluginFeatureNumber        (LPCSTR csName, LONG    iVal);
PI_DLL_SPEC BOOL PI_SetPluginFeatureBoolean       (LPCSTR csName, BOOL    bVal);
PI_DLL_SPEC BOOL PI_SetPluginFeatureFloat         (LPCSTR csName, DOUBLE  fVal);
/// @}

//-----------------------------------------------------------------------------
// Actions

/// \addtogroup Actions   Actions
/// Interface of plugins for performing several actions.
/// @{

/// Function for synchronizing Performer load-tests with plugin.
/// Used by Performer to inform plugin of events like Start, Stop and kill of Loadtests.
/// \see PI_Notify() non-load test related notifications
/// \param[in]      eEvent indicates the event.
/// \param[in,out]  pParam dependens on eEvent
/// \retval  TRUE   if successful
/// \retval  FALSE  if error happened, call GetLastErrorMessage for details.
/// 
/// If eEvent is eEventStart then pParam will point to string which holds xml data.
/// 
/// \code
/// <?xml version='1.0' encoding='UTF-8'?>
/// <EventInfo>
///   <ProjectName>DefaultProjectName</ProjectName>
///   <ProjectDescription>Default Project</ProjectDescription>
///   <ProjectType>0</ProjectType>
///   <ProjectTypeText>Web business transaction (HTML/HTTP)</ProjectTypeText>
///   <LoadtestDescription></LoadtestDescription>
///   <ResultsDirectory>
///     C:\Program Files\Segue\SilkPerformer 7.3.1\Working\Projects\DefaultProject\RecentLoadTest\
///   </ResultsDirectory>
/// </EventInfo>
/// \endcode
PI_DLL_SPEC 
BOOL PI_OnEvent (EPI_Event eEvent, void* pParam);

/// Function for handling toolbar button requests.
/// Function is called by the callee when a toolbar button is clicked for this plugin.
/// \warning How to handle plugins without any resource dll --> this function will be useless.
/// \param[in]      iAction indicates which button was clicked.
/// \param[in,out]  pParam NOT USED
/// \retval  TRUE   if successful
/// \retval  FALSE  if error happened, call GetLastErrorMessage for details.
PI_DLL_SPEC 
BOOL PI_OnAction (LONG iAction, void* pParam);

/// Will be called after the plugin is enabled or disabled.
/// Function is used to indicate enable or disable by the user. 
/// The Function is also called after startup of the main application 
/// when the plugin was enabled before.
/// \param[in]      eNotification indicates the notification the callee wants to send
/// \param[in,out]  pParam NOT USED
/// \retval  TRUE   if successful
/// \retval  FALSE  if error happened, call GetLastErrorMessage for details.
///
/// \Note Currently the main application calls #eNotificationLoadPlugin and 
/// #eNotificationUnloadPlugin each time 
/// when Enable state is switched (Name does NOT make sense.
PI_DLL_SPEC 
BOOL PI_Notify (EPI_Notification eNotification, void* pParam);

/// Register callback function for logging.
/// Function for registerring a callback function for logging.
/// \warning pClient should be renamen into pParam...
/// \param[in]      pClient used as parameter for the callback function.
/// \param[in]      fCallback function pointer for callback, 
/// set to null if no logging should be done.
/// \return  void
PI_DLL_SPEC 
void PI_RegisterMessageLog (void* pClient, TMessageLogCallback fCallback); 


/// The Plugin can provide the callee with information which profile settings it wants to have changed. 
/// \param[in]      pszProfileName Name of current profile
/// \param[in,out]  pszXmlSettings Buffer for returning the xml settings string
/// \param[in,out]  rulSize        (in) Size of Buffer, (out) size of Buffer used or 
/// size of buffer needed if too small buffer
/// \retval TRUE  succeeded
/// \retval FALSE failure either size of buffer too small, or detailed error information 
/// available by calling GetLastErrorMessage().
/// \code
/// <?xml version="1.0" encoding="UTF-8"?>
/// <ProfileSettings>
///   <Setting name="Setting_Internet_Http_Tag_Header">PluginTag</Setting>
///   <Setting name="Setting_Internet_Http_Tag_Flags">1234</Setting>
///   <Setting name="Setting_Internet_Http_Tag_Probability">0.2</Setting>
/// </ProfileSettings>
/// \endcode
/// Setting_Internet_Http_Tag_Header: The plugin provider must specify a HTTP header name, which the 
/// SilkPerformer runtime engine is supposed to add to HTTP requests.
///
/// Setting_Internet_Http_Tag_Flags: A integer value specifying the tag information, which should be 
/// generated. The value is a bitmask of the following options:
///  - WEB_TAG_FLAG_ProjectName      := 0x0001;
///    HTTP header contains the name of the used SilkPerformer project: PN=\<ProjectName\>
///  - WEB_TAG_FLAG_UserGroup        := 0x0002;
///    HTTP header contains the name of the SilkPerformer user-group to which the virtual user sending the request belongs: UG=\<UserGroup\>
///  - WEB_TAG_FLAG_Profile          := 0x0004;
///    HTTP header contains the name of the SilkPerformer profile which the virtual user sending the request uses: PF=\<Profile\>
///  - WEB_TAG_FLAG_Workload         := 0x0008;
///    HTTP header contains the name of the SilkPerformer workload which the virtual user sending the request uses: WL=\<Workload\>
///  - WEB_TAG_FLAG_Transaction      := 0x0010;
///    HTTP header contains the name of the SilkPerformer transaction which the virtual user sending the request is currently executing: TN=\<Transaction\>
///  - WEB_TAG_FLAG_UserId           := 0x0020;
///    HTTP header contains a unique user id (cross loadtest): VU=\<Id\>
///  - WEB_TAG_FLAG_BrowserSession   := 0x0040;
///    HTTP header contains a browser session id. This session id is incremented when the virtual user logically opens a browser:SI=\<BrowserSessionId\>
///  - WEB_TAG_FLAG_PageContext      := 0x0080;
///    HTTP header contains a information about what document in the currently processed page is loaded. 
///    If it is a named frame then the value starts with the frame name. After a dot the page-unique document 
///    number is appended. Note that if embedded documents get cached this number need not be progresional: PC=\<FrameName\>.\<DocId\>
///  - WEB_TAG_FLAG_RequestId        := 0x0100;
///    HTTP header contains a user-unique request id: ID=\<RequestId\>
///  - WEB_TAG_FLAG_Timer            := 0x0200;
///    If the current request gets issued in a page context (page-level API) the HTTP header contains the specified timer name (if omitted the name is empty): NA=\<TimerName\>
///  - WEB_TAG_FLAG_WorkloadType     := 0x0400;
///    HTTP header contains the SilkPerformer workload type: WT=\<WorkloadType\>
///    possible workload types: 
///    - 0: SteadyState 
///    - 1: Increasing
///    - 2: Dynamic
///    - 3: Queuing
///    - 4: Monitoring
///    - 5: AllDay
///    - 6: Verify
///    - 7: SingleUser
///  - WEB_TAG_FLAG_Agent            := 0x0800;
///    HTTP header contains the name of the used SilkPerformer agent: AN=\<AgentName\>
///  - WEB_TAG_FLAG_LineNumber       := 0x1000;
///    HTTP header contains the line number of the currently executed SilkPerformer function: LN=\<LineNumber\>
///  - WEB_TAG_FLAG_Time             := 0x2000;
///    HTTP header contains the current controller time in the format of seconds since 1970: TI=\<ElapsedSeconds\>
///
/// Setting_Internet_Http_Tag_Probability: A floating point value between 0 and 1 specifying the 
/// percentage of virtual users, which should generate the HTTP tags.
/// 
///
PI_DLL_SPEC 
BOOL PI_GetProfileSettings(const char* pszProfileName, char* pszXmlSettings, unsigned long& rulSize);

/// Tests if specific Measure can be analyzed.
/// This function will be invoked, when user wants to analyze a specific measure.
/// This function is optional. If PlugIn doe snot support this funciton but supports PI_AnalyzeMeasure, 
/// then the analyze command will be available by default.
/// \param[in] hLT
/// \param[in] pszSessionInfo
/// \param[in] pszMeasure Name of measure which the user wants to analyze
/// \param[in] pszContext may hold additional information in the future (currently not used)
/// \retval  TRUE if supported.
/// \retval  FALSE if not supported. 
/// \Note Currently only Overviewreports in PerformanceExplorer use this function.
/// \see PI_AnalyzeMeasure
PI_DLL_SPEC
BOOL   PI_CanAnalyzeMeasure (HPILOADTEST hLT, LPCSTR pszSessionInfo, LPCSTR pszMeasure, LPCSTR pszContext);

/// Start Analyzing a Measure
/// This function will be invoked, when user wants to analyze a specific measure.
/// This function is optional. If not supported by the PlugIn, then 
/// the analyze command will not be available in applications.
/// \param[in] hLT
/// \param[in] pszSessionInfo
/// \param[in] pszMeasure Name of measure which the user wants to analyze
/// \param[in] pszContext may hold additional information in the future (currently not used)
/// \retval  TRUE if successful.
/// \retval  FALSE if error happened, request detailed information by calling 
/// GetLastErrorMessage.
/// \Note Currently only Overviewreports in PerformanceExplorer use this function.
/// \see PI_CanAnalyzeMeasure
PI_DLL_SPEC 
BOOL   PI_AnalyzeMeasure    (HPILOADTEST hLT, LPCSTR pszSessionInfo, LPCSTR pszMeasure, LPCSTR pszContext);

/// @}


//-----------------------------------------------------------------------------
// TrueLog Explorer PlugIn Functions

/// \addtogroup TrueLog   TrueLog Explorer
/// Interface for functions used by TrueLogExploerer.
/// @{

/// Inform Plugin that a loadtest has been loaded and request plugin internal handle.
/// \return  pointer to internal PlugIn structure which will be provided to 
/// subsequent calls.
/// \param[in] pLTContext can be used to retrieve information in context to the given loadtest.
/// \retval  NULL if error happened, call GetLastErrorMessage for details.
/// \see PI_GetInfo
PI_DLL_SPEC 
HPILOADTEST PI_InitLoadtest (HPILTCONTEXT pLTContext);


/// Inform Plugin that a loadtest is about to be unloaded and let plugin free the internal structure.
/// \param[in] hLT
PI_DLL_SPEC 
void        PI_FreeLoadtest (HPILOADTEST hLT);

/// Inform Plugin of a new PageBegin.
/// This will inform the PlugIn of a new page to begin.
/// \param[in] hLT
/// \param[in]      uId unique id.
/// \param[in]      pszName name of the page.
/// \return  BOOL   is the notification successfully handled?
/// \retval  TRUE   if successful
/// \retval  FALSE  if error happened, call GetLastErrorMessage for details.
/// \Note the unique Id is unique per Loadtest.
/// \Warning We have to add a Loadtest handle to the function.
/// \Warning The uId is only valid until FreeLoadtest() is called.
PI_DLL_SPEC 
BOOL PI_BeginPage (HPILOADTEST hLT, ULONG uId, LPCSTR pszName);


/// Inform Plugin that a page ends.
/// This will inform the PlugIn of the end of a previously with BeginPage started page.
/// \param[in] hLT
/// \param[in,out] ppContent Callee has to copy the data. Content is only 
///                          temporary valid.
/// \return  if succeeded or not
/// \retval  FALSE if ppContent is null call GetLastError for details otherwise
///                the plugin generates an error content which can be shown.
/// \Warning The uId is only valid until FreeLoadtest() is called.
PI_DLL_SPEC 
BOOL PI_EndPage  (HPILOADTEST hLT, const TContent** ppContent);


/// Add a document to current page.
/// Add a document to the current page, which was started using BeginPage.
/// \param[in] hLT
/// \param[in] uId unique id.
/// \param[in] sUrlAbs      URL of this document.
/// \param[in] sRequHeader  
/// \param[in] sRequBody    
/// \param[in] uRequBodyLen 
/// \param[in] sRespHeader  
/// \param[in] sRespBody    
/// \param[in] uRespBodyLen 
/// \param[in,out] ppContent Callee has to copy the data. Content is only 
///                          temporary valid.
/// \return  if succeeded or not
/// \retval  FALSE if ppContent is null call GetLastError for details otherwise
///                the plugin generates an error content which can be shown.
/// \Warning The uId is only valid until FreeLoadtest() is called.
/// \Note    would suggest a struct using size member to indicate which
/// version is used?
PI_DLL_SPEC 
BOOL PI_AddDocument (
                    HPILOADTEST hLT,
                    ULONG       uId, 
                    LPCSTR      sUrlAbs,
                    LPCSTR      sRequHeader,
                    LPCSTR      sRequBody,
                    ULONG       uRequBodyLen,
                    LPCSTR      sRespHeader, 
                    LPCSTR      sRespBody, 
                    ULONG       uRespBodyLen,
                    const TContent** ppContent
                    );

/// Retrieves extra Content from PlugIn.
/// This function will retrieve extra content from the Plugin, 
/// when content-link is clicked inside a html-content previously fetched from plugin.
/// \param[in] hLT
/// \param[in] uId unique id.
/// \param[in] pszContext context string provided by link from previously fetched html.
/// \param[in,out] ppContent Callee has to copy the data. Content is only 
///                          temporary valid.
/// \return  if succeeded or not
/// \retval  FALSE if ppContent is null call GetLastError for details otherwise
///                the plugin generates an error content which can be shown.
/// \see #TContent
PI_DLL_SPEC 
BOOL PI_GetContent (HPILOADTEST hLT, ULONG uId, LPCSTR pszContext, const TContent** ppContent);


/// Performs an action link.
/// This function will request the plugin to run an action, when action-link is clicked 
/// inside a html-content previously fetched from plugin.
/// \param[in] hLT
/// \param[in] uId unique id.
/// \param[in] pszLink link provided from previously fetched html.
/// \retval  TRUE if successful.
/// \retval  FALSE if error happened, request detailed information by calling 
/// GetLastErrorMessage.
/// \see #TContent
PI_DLL_SPEC 
BOOL PI_IndicateAction (HPILOADTEST hLT, ULONG uId, LPCSTR pszLink);

/// Set the callback function for selecting a node (in TrueLogExplorer).
/// \param[in] fnSelectNode pointer to function which should be called.
/// \param[in] pContext extra parameter which will be provided via the callback 
/// function to the callee.
/// \Note This function is optional and may not be supported by every plugin.
PI_DLL_SPEC 
void PI_SetFNSelectNode (TFNSelectNode fnSelectNode, void* pContext);

/// @}


/*! \page WB-Plugin Implementing PlugIn Dlls for SilkPerformer Workbench
\deprecated
PlugIn dlls for SilkPerformer Workbench need to implement the following functions:
\section sec_WB_NeededFN Required Functions for Workbench PlugIns
  - PI_GetFirstPluginAttribute()  
  - PI_GetNextPluginAttribute()
  - PI_GetPluginAttributeNumber()
  - PI_GetPluginAttributeString()
  - PI_GetPluginAttributeBoolean()
  - PI_GetPluginAttributeFloat()
  - PI_SetPluginAttributeNumber()
  - PI_SetPluginAttributeString()
  - PI_SetPluginAttributeBoolean()
  - PI_SetPluginAttributeFloat()
  - PI_GetPluginAttributeDescription()
  - PI_GetFirstPluginFeature()
  - PI_GetNextPluginFeature()
  - PI_GetPluginFeatureNumber()
  - PI_GetPluginFeatureString()
  - PI_GetPluginFeatureBoolean()
  - PI_GetPluginFeatureFloat()
  - PI_SetPluginFeatureNumber()
  - PI_SetPluginFeatureString()
  - PI_SetPluginFeatureBoolean()
  - PI_SetPluginFeatureFloat()
  - PI_GetPluginFeatureDescription()
  - PI_Notify()
  - PI_GetName()
  - PI_GetDescription()
  - PI_GetVersion()
  - PI_GetLastErrorMessage()
  - PI_GetProfileSettings()
  - PI_Save()
  - PI_Initialize()
  - PI_InitLoadtest()
  - PI_DeInitialize() -> optional
  - PI_GetIcon() -> optional
*/


/*! \page TLE-Plugin Implementing PlugIn Dlls for TrueLog-Explorer
\deprecated
PlugIn dlls for TrueLog Explorer need to implement following functions:
\section sec_TLE_NeededFN Required Functions for TrueLog Explorer PlugIns
  - PI_GetFirstPluginAttribute()  
  - PI_GetNextPluginAttribute()
  - PI_GetPluginAttributeNumber()
  - PI_GetPluginAttributeString()
  - PI_GetPluginAttributeBoolean()
  - PI_GetPluginAttributeFloat()
  - PI_SetPluginAttributeNumber()
  - PI_SetPluginAttributeString()
  - PI_SetPluginAttributeBoolean()
  - PI_SetPluginAttributeFloat()
  - PI_GetPluginAttributeDescription()
  - PI_GetFirstPluginFeature()
  - PI_GetNextPluginFeature()
  - PI_GetPluginFeatureNumber()
  - PI_GetPluginFeatureString()
  - PI_GetPluginFeatureBoolean()
  - PI_GetPluginFeatureFloat()
  - PI_SetPluginFeatureNumber()
  - PI_SetPluginFeatureString()
  - PI_SetPluginFeatureBoolean()
  - PI_SetPluginFeatureFloat()
  - PI_GetPluginFeatureDescription()
  - PI_Notify()
  - PI_GetName()
  - PI_GetDescription()
  - PI_GetVersion()
  - PI_GetLastErrorMessage()
  - PI_Save()
  - PI_Initialize()
  - PI_InitLoadtest()
  - PI_FreeLoadtest()
  - PI_BeginPage()
  - PI_EndPage()
  - PI_AddDocument()
  - PI_GetContent()
  - PI_IndicateAction()
  - PI_SetFNSelectNode() -> optional
  - PI_DeInitialize() -> optional
  - PI_GetIcon() -> optional
*/


//-----------------------------------------------------------------------------


/// \mainpage Welcome Page
///
/// \section main_sec_10 Introduction
/// This document describes the API for plugins to integrate into performer applications.
/// 
///
/// \section main_sec_20 Plugin Functions
/// 
/// The links below discusse the API from calling applications point of view.
///
/// - \subpage page_General_PlugIn "General Functions"
/// - \subpage page_Workbench_PlugIn "Workbench Functions"
/// - \subpage page_TLE_PlugIn "TrueLog Explorer Functions"
/// - \subpage page_PerfExp_PlugIn "Performance Explorer Functions"
/// .
///



/// \page page_General_PlugIn General Plugin Functions
/// General plugin functions used by all or some Performer applications.
/// \section General_PlugIn_sec_10 General Process
/// - Application Start
/// 	- Applications scans directory <WorkingDirectory>\Plugins
/// 	- For each directory found it tests if there exists a dll named pi<PluginName>.dll
/// 	- Loads dll and links tests if all needed functions are available
/// 	- loads dll and links tests if all needed functions are available
///   - calls #PI_SetFNInfo
/// 	- calls #PI_Initialize
/// 	- calls #PI_Notify
///   - calls #PI_RegisterMessageLog (if supported by application)
/// - User modifies settings
/// 	- calls the SetAttributes functions
/// 	- calls #PI_Save
/// - Application Ends
/// 	-	calls #PI_DeInitialize
/// .
/// 
/// Other functions may get called anytime in between #PI_Initialize and #PI_Deinitialize.\n
///
/// \note Namingconvention:\n
/// <WorkingDirectory>\\PlugIns\\<PluginName> \n
/// <WorkingDirectory>\\PlugIns\\<PluginName>\\pi<PluginName>.dll \n
/// .
/// \section General_PlugIn_sec_20 Functions
/// \n
/// - #PI_Initialize
/// - #PI_DeInitialize
/// - #PI_Save
/// - #PI_GetName
/// - #PI_GetDescription
/// - #PI_GetVersion
/// - #PI_GetIcon 
/// - #PI_GetLastErrorMessage
/// - #PI_RegisterMessageLog 
/// - #PI_Notify
/// - #PI_OnAction
/// - #PI_InitLoadtest
/// - #PI_FreeLoadtest
/// \n
/// \n
/// - Attribute Access
/// 	- #PI_GetFirstPluginAttribute
/// 	- #PI_GetNextPluginAttribute
/// 	- #PI_GetPluginAttributeDescription
/// 	- #PI_GetPluginAttributeString
/// 	- #PI_GetPluginAttributeNumber
/// 	- #PI_GetPluginAttributeBoolean
/// 	- #PI_GetPluginAttributeFloat
/// 	- #PI_SetPluginAttributeString
/// 	- #PI_SetPluginAttributeNumber
/// 	- #PI_SetPluginAttributeBoolean
/// 	- #PI_SetPluginAttributeFloat
/// .
/// \n
/// \n
/// - Feature Access
/// 	- #PI_GetFirstPluginFeature
/// 	- #PI_GetNextPluginFeature
/// 	- #PI_GetPluginFeatureDescription
/// 	- #PI_GetPluginFeatureString
/// 	- #PI_GetPluginFeatureNumber
/// 	- #PI_GetPluginFeatureBoolean
/// 	- #PI_GetPluginFeatureFloat
/// 	- #PI_SetPluginFeatureString
/// 	- #PI_SetPluginFeatureNumber
/// 	- #PI_SetPluginFeatureBoolean
/// 	- #PI_SetPluginFeatureFloat
/// 	.
/// .


/// \page page_Workbench_PlugIn Plugin Functions used by Workbench
///
/// \section Workbench_PlugIn_sec_10 General Process
/// - Before Start of Loadtest
/// 	- #PI_GetProfileSettings to modify the current profile
/// - After Start of Loadtest
/// 	- #PI_OnEvent using parameter eEventStart
/// 	- #PI_GetInfo using parameter ePIInfo_CurrentSessionId
/// - End of Loadtest
/// 	- #PI_OnEvent using parameter eEventStop or eEventKill
/// .
/// \section Workbench_PlugIn_sec_20 Functions
/// Beside the functions listed on \ref page_General_PlugIn "General Functions", Workbench uses following functions:
///
/// - #PI_OnEvent
/// - #PI_GetProfileSettings
/// .


/// \page page_TLE_PlugIn Plugin Functions used by TrueLogExplorer
///
/// \section TLE_PlugIn_sec_10 General Process
/// - Plugin has been initialized (loaded)
/// 	- #PI_SetFNSelectNode
/// - TrueLog is Loaded
/// 	- #PI_InitLoadtest to get a context handle from the PlugIn
/// - TrueLog is closed
/// 	- #PI_FreeLoadtest to allow the plugin to free loadtest dependend data. (Handle)
/// - Page node is selected
/// 	- #PI_BeginPage
/// 	- #PI_AddDocument for each subnode
/// 	- #PI_EndPage
/// - Document node is selected
/// 	- #PI_BeginPage (using parent page)
/// 	- #PI_AddDocument for each subnode (of parent page)
/// 	- #PI_EndPage (using parent page)
/// - User click link in html page provided by plugin
///   - If link starts with "Content.html?" #PI_GetContent
///   - If link starts with "Action.html?" #PI_IndicateAction
/// .
/// The content data provided by #PI_BeginPage, #PI_AddDocument and #PI_EndPage may get cached by the application and may only fetched once.\n
///
/// #PI_GetContent will get called whenever user clicks on a link "Content.html?pszContext" in a content page previously fetched from plugin (see TContent).\n
///   Application will call GetContent() using parmString for pszContext to get extra content for display.
///
/// \section TLE_PlugIn_sec_20 Functions
/// Beside the functions listed on \ref page_General_PlugIn "General Functions", TrueLogExplorer uses following functions:
///
/// - #PI_BeginPage
/// - #PI_EndPage
/// - #PI_AddDocument
/// - #PI_GetContent
/// - #PI_IndicateAction
/// - #PI_SetFNSelectNode
/// .


/// \page page_PerfExp_PlugIn Plugin Functions used by Performance Explorer
///
/// \section PerfExp_PlugIn_sec_10 General Process
/// - Loadtest (tsd) file is Loaded
/// 	- #PI_InitLoadtest to get a context handle from the PlugIn
/// - Loadtest is closed
/// 	- #PI_FreeLoadtest to allow the plugin to free loadtest dependend data. (Handle)
/// - Overviewreport is generated and #PI_AnalyzeMeasure is available
/// 	- #PI_GetInfo using parameter ePIInfo_AnalyzeImageFile to get image fiel for link
/// 	- #PI_CanAnalyzeMeasure (for each Ranking Measure)
/// - User clicks link for analyzing the measure in OverviewReport
///   - #PI_AnalyzeMeasure
/// .
///
/// \section PerfExp_PlugIn_sec_20 Functions
/// Beside the functions listed on \ref page_General_PlugIn "General Functions", Performance Explorer uses following functions:
/// 
/// - #PI_CanAnalyzeMeasure
/// - #PI_AnalyzeMeasure
/// .

#ifdef __cplusplus
};
#endif                              

//-----------------------------------------------------------------------------

#endif // _piWorkbenchPluginInterface_h
