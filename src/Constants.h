#ifndef __DTD_PLUGIN_CONSTANSTS_H__
#define __DTD_PLUGIN_CONSTANSTS_H__



const char * CONST_GENERIC_SESSIONID      = "##unknown##";

/// Strings used for Plugin Representation and Registry handling
#define NAME_FEATURE_RECORDING "Automatic Session Recording"
#define DESC_FEATURE_RECORDING "Activate SilkPerformer Diagnostics Session-Recording during loadtest"
#define REG_VALNAME_RECORDING  "SessionRecording"
#define REG_DEFVAL_RECORDING TRUE

#define NAME_FEATURE_WAITTIME "Wait Timeout [sec]"
#define DESC_FEATURE_WAITTIME "Wait timeout for SilkPerformer Diagnostics operations"
#define REG_VALNAME_WAITTIME  "WaitTimeout"
#define REG_DEFVAL_WAITTIME 30

#define NAME_FEATURE_APPSESSION "System Profile"
#define DESC_FEATURE_APPSESSION "Specify the System Profile to be used for Loadtest-Synchronization"
#define REG_VALNAME_APPSESSION  "AppSession"
#define REG_DEFVAL_APPSESSION ""

#define NAME_FEATURE_DTCLIENTPORT "SilkPerformer Diagnostics Client Port"
#define DESC_FEATURE_DTCLIENTPORT "SilkPerformer Diagnostics Client Rest Interface Port"
#define REG_VALNAME_DTCLIENPORT "dtClientPort"
#define REG_DEFVAL_DTCLIENTPORT 8030

#define FILE_ICON_REPORT "dtreporticon.gif"

#define NO_API_SEARCHPATH_SET "notset"
#define DYNATRACE_HEADER "X-dynaTrace"			//we need this to define the out header
#define DYNATRACE_HEADER_WITH_COLON "X-dynaTrace:"		//used to parse in header. we need the colon to avoid a mix-up with 'X-dynaTrace-JS-Agent' header
#define DYNATRACE_HEADER_PRE4 "dynaTrace"

#define PROFILE_SETTINGS_XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
    "<ProfileSettings>" \
    "<Setting name=\"Setting_Internet_Http_Tag_Header\">"DYNATRACE_HEADER"</Setting>" \
    "<Setting name=\"Setting_Internet_Http_Tag_Flags\">928</Setting>" \
    "<Setting name=\"Setting_Internet_Http_Tag_Probability\">1</Setting>" \
    "</ProfileSettings>" // Flags: 928 = 0x03A0: means: WEB_TAG_FLAG_UserId | WEB_TAG_FLAG_PageContext | WEB_TAG_FLAG_RequestId | WEB_TAG_FLAG_Timer 

#define PROFILE_SETTINGS_XML_PRE_4 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
    "<ProfileSettings>" \
    "<Setting name=\"Setting_Internet_Http_Tag_Header\">"DYNATRACE_HEADER_PRE4"</Setting>" \
    "<Setting name=\"Setting_Internet_Http_Tag_Flags\">928</Setting>" \
    "<Setting name=\"Setting_Internet_Http_Tag_Probability\">1</Setting>" \
    "</ProfileSettings>" // Flags: 928 = 0x03A0: means: WEB_TAG_FLAG_UserId | WEB_TAG_FLAG_PageContext | WEB_TAG_FLAG_RequestId | WEB_TAG_FLAG_Timer 


#endif