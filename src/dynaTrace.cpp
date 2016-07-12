#include "piWorkbenchPluginInterface.h"
#include "dynaTrace.h"
#include "String.h"
#include "resource.h"
#include <direct.h>
#include "Constants.h"
#include "RESTClient.h"

using namespace pnapi;

enum RESTRequestType {
	START_SESSIONRECORDING,
	STOP_SESSIONRECORDING,
	OPEN_PATH,
	OPEN_API,
	GET_SESSION_ID
};

// Global Variables
HMODULE hModule=NULL;

page g_page;

RESTClient *gp_Rc=NULL;

int giAttrNum=0;
int giFeatureNum=0;

// THE DLL MAIN
EXTERN_C BOOL WINAPI DllMain(HANDLE hmodule, DWORD dwReason, PVOID pvReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
		TextLog("--------------------------- Starting dynaTrace Plugin ----------------------------");
		getVersionInfo((HMODULE) hmodule);
		hModule=(HMODULE) hmodule;
		return TRUE;
    case DLL_THREAD_ATTACH:
        return TRUE;
    case DLL_PROCESS_DETACH:
		TextLog("--------------------------- Stopping dynaTrace Plugin ----------------------------");
		if (gp_Rc) delete gp_Rc;
        return TRUE;
    case DLL_THREAD_DETACH:
        return TRUE;
    default:
        return FALSE;
    }
}

// ----------------------------------------------------------------------
// module members (valid per process)
// members are initialized in Initialize function

TFNSelectNode       g_FNSelectNode;
void*               g_pSelectNodeContext;
TMessageLogCallback g_FNMessageLog;
TFNInfo				g_FNInfo=NULL;
void*               g_pMessageLogContext;
void*				g_pGetInfoContext;
TContent            g_tmpContent;
TContent            g_tmpPageContent;
TContent            g_tmpDocContent;
LPSTR               g_sErrorMsg;
String				g_strLastSessionID;
String				g_strPluginRegKey;
String              g_strLastErrorMessage;

long rowCount=0;
apr_pool_t *aprpool=NULL;

// --------------------------------------------------------------------
// Constants for internal use
//

// Version information
String objVerCompany("unset");
String objVerInternalName("unset");
String objVerFileDescription("unset");
String objVerProductVersion("unset");

// Helper functions:

void getVersionInfo(HMODULE hmodule) {
	char filename[1024];
	char stringData[512];
	void *dataPtr;
	UINT strLen=0;

	memset(filename,0,1024);
	DWORD dwDummy;
	char * pcInfoBuffer;
	DWORD res=GetModuleFileName(hmodule,filename,1023);

	TextLog((String("Filename is ")+String((const char *)filename)).getBytes());
	
	// check version info size
	DWORD dwVersionBlockSize=GetFileVersionInfoSize(filename,&dwDummy);
	pcInfoBuffer=new char[dwVersionBlockSize];

	GetFileVersionInfo(filename,NULL,dwVersionBlockSize,pcInfoBuffer);
	memset(stringData,0,512);

	// Read internal name
	VerQueryValue(pcInfoBuffer,TEXT("\\StringFileInfo\\000004b0\\InternalName"),&dataPtr,&strLen);
	memcpy(stringData,dataPtr,strLen);
	stringData[strLen]='\0';
	objVerInternalName.sprintf("%s",stringData);
	TextLog((String("InternalName is ")+objVerInternalName).getBytes());

	// Read company name
	VerQueryValue(pcInfoBuffer,TEXT("\\StringFileInfo\\000004b0\\CompanyName"),&dataPtr,&strLen);
	memcpy(stringData,dataPtr,strLen);
	stringData[strLen]='\0';
	objVerCompany.sprintf("%s",stringData);
	TextLog((String("CompanyName is ")+objVerCompany).getBytes());

	// Read Description
	VerQueryValue(pcInfoBuffer,TEXT("\\StringFileInfo\\000004b0\\FileDescription"),&dataPtr,&strLen);
	memcpy(stringData,dataPtr,strLen);
	stringData[strLen]='\0';
	objVerFileDescription.sprintf("%s",stringData);
	TextLog((String("FileDescription is ")+objVerFileDescription).getBytes());

	// Read Description
	if (!VerQueryValue(pcInfoBuffer,TEXT("\\StringFileInfo\\000004b0\\ProductVersion"),&dataPtr,&strLen)) {
		objVerProductVersion = "<unknown>";
	}
	else {
		int j = 0;
		for (int i = 0; i < (int) strLen; i++) {
			char ch = ((char *) dataPtr)[i];
			if (ch == ' ') continue; // omit whitespaces;
			if (ch == ',') ch = '.';
			stringData[j] = ch;
			j++;
			if (ch == '\0') break;
		}
		stringData[j] = '\0';

		objVerProductVersion.sprintf("%s",stringData);
	}
	TextLog((String("ProductVersion is ")+objVerProductVersion).getBytes());
	delete pcInfoBuffer;
}

apr_pool_t *getAprPool() {
	if (!aprpool) {
		apr_pool_initialize();
		apr_pool_create(&aprpool,NULL);
	} // if
	return aprpool;
}

BOOL checkIsLoadTest(void) {
	int iLoadTestType=0;
	char * ch=NULL;
	ULONG len=0;

	GetInfo(NULL,ePIPInfo_LoadtestType,ch,len);
	if (len>0) {
		len=len+1;
		ch=new char[len];
		if (GetInfo(NULL,ePIPInfo_LoadtestType,ch,len)) {
			DLOG(String().sprintf("Loadtesttype is %s",ch).getBytes());
			iLoadTestType=atoi(ch);
			delete ch;
		} // if
		else {
			Log(eSeverityWarning,"Error reading Loadtesttype",0);
			delete ch;
			return FALSE;
		} // elseif
	} // if
	else {
		Log(eSeverityWarning,"Error getting Loadtesttype",0);
		return FALSE;
	} // elseif

	if (iLoadTestType==ePI_TestType_LoadTest) {
		DLOG("LoadtestType is real loadtest.");
		return TRUE;
	} // if
	
	if (iLoadTestType==ePI_TestType_TryScriptTest) {
		DLOG("LoadtestType is real loadtest.");
		return TRUE;
	} // if

	DLOG("LoadtestType not one of [LoadTest,TryScript] - not starting session-recording.");
	return FALSE;
}

String getRecordSessionName(void *param) {
	ULONG ulBuffer=0;
	char *chBuffer;
	String objLTName;
	String str;

	GetInfo(NULL, ePIPInfo_ProjectName,NULL,ulBuffer);
	if (ulBuffer) {
		chBuffer=new char[ulBuffer];
		if (GetInfo(NULL,ePIPInfo_ProjectName,chBuffer,ulBuffer)) {
			str.sprintf("%s",chBuffer);
		} // if
		delete chBuffer;

		objLTName=str;
	} // if

	str.clear();

	GetInfo(NULL, ePIPInfo_LoadtestName,NULL,ulBuffer);
	if (ulBuffer) {
		chBuffer=new char[ulBuffer];
		if (GetInfo(NULL,ePIPInfo_LoadtestName,chBuffer,ulBuffer)) {
			str.sprintf("%s",chBuffer);
		} // if
		delete chBuffer;

		if ((objLTName.length()>0) && (str.length()>0))
			objLTName+="_";

		objLTName+=str;
	} // if

	return objLTName;
}

void TextLog(LPCSTR pszLogMsg) {
#ifdef _DEBUG
	FILE * f=fopen ("C:\\mylog.log","a");
	if (pszLogMsg) 
		fwrite((const void *)(pszLogMsg),sizeof(char),strlen(pszLogMsg),f);
	else
		fwrite((const void *)("null"),sizeof(char),4,f);
	fwrite("\n",sizeof(char),1,f);
	fclose(f);
#endif // _DEBUG
}

String getDynatraceHeader(String headers) {
    String retVal;
	char * che=new char[(headers.length()+1)*sizeof(char)];
	memcpy(che,headers.getBytes(),(headers.length()+1)*sizeof(char));
	if (!che) {
		Log(eSeverityError,"[getDynatraceHeader] Cannot create buffer for string tokinization!!",0);
		return retVal;
	} // if
	char * tmp=che;
	char * last=NULL;
	char * currtok=NULL;
	do {
		currtok=apr_strtok(tmp,"\n",&last);
		tmp=NULL;
		if (currtok) {
			String strTok=(const char *) currtok;
			strTok.trim();
			int start = 0;
			if (strTok.startsWith(DYNATRACE_HEADER_WITH_COLON)) {
				start = sizeof(DYNATRACE_HEADER_WITH_COLON);
			}
			else if (strTok.startsWith(DYNATRACE_HEADER_PRE4)) {
				start = sizeof(DYNATRACE_HEADER_PRE4) + 1;	//we don't want trailing ': '
			}
			if (start > 0) {	//header found
				retVal=strTok.substring(start);
				String strLog;
				strLog.sprintf("[getDynatraceHeader] DYNATRACE TOKEN FOUND \"%s\"",retVal.getBytes());
				Log(eSeverityInfo, strLog.getBytes(), 0);
				TextLog(strLog.getBytes());
				break;
			} // if
			else {
				TextLog(strTok.getBytes());
			} // else if
		} // if
	} while (currtok!=NULL);
	delete che;

	return retVal;
}

String getDynatraceName(String headers, String respHdr) {
    String retVal;
#ifdef _DEBUG // this is to avoid the "unused local variable" warning if we do a non-debug-build
	String strDebug;
#endif 
	if (respHdr.length()>0) { // respHdr was found
		int startSR=respHdr.indexOf(String("RS="));
		if (startSR>=0) {
			// sessionrecording was active, assume that session is always in front
			int endSR=respHdr.indexOf(String(";"),startSR);
#ifdef _DEBUG // do not do the sprintf in release version
			strDebug.sprintf("index of startSR and endSR in \"%s\" is %i and %i",respHdr,startSR,endSR);
			TextLog(strDebug.getBytes());
#endif
			if (endSR>(startSR+3)) 
				retVal=respHdr.substring(startSR+3,endSR);	//+3 means RS= not needed!
			else 
				retVal=respHdr;
		} // if

		TextLog((String("[getDynatraceName] SessionRecording-Part in \"")+respHdr+String("\" is : ")+retVal).getBytes());
	} // if
	else {
		TextLog((String("[getDynatraceName] no Session-Recording Part found in : ")+respHdr).getBytes());
	} // else

	if (retVal.length()<1)
		retVal=NO_API_SEARCHPATH_SET;

	return retVal;
}

String getRegValue(HKEY key, String subkey, String name) {
	HKEY mykey;
	DWORD type=0;
	char buf[512];
	DWORD len=512;

	LONG status=RegOpenKeyEx(key,subkey.getBytes(),0,KEY_QUERY_VALUE,&mykey);
	if (status) {
		String errMsg;
		LONG errMsgLen=512;
		char chMsg[512];
		errMsg.ReAlloc(errMsgLen);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,status,0,(LPTSTR) chMsg,errMsgLen,NULL);
		errMsg.sprintf("getRegValue:RegOpenKeyEx returned: %s",chMsg);
		TextLog(errMsg.getBytes());
		return String();
	} // if

	status=RegQueryValueEx(mykey,name.getBytes(),NULL,&type,(BYTE *)buf,&len);
	RegCloseKey(mykey);
	if (status) {
		String errMsg;
		LONG errMsgLen=512;
		char chMsg[512];
		errMsg.ReAlloc(errMsgLen);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,status,0,(LPTSTR) chMsg,errMsgLen,NULL);
		errMsg.sprintf("RegQueryValueEx returned: %s",chMsg);
		TextLog(errMsg.getBytes());
		return String();
	} // if
	if (len<sizeof(buf)) 
		buf[len]='\0';
	else 
		buf[sizeof(buf)-1]='\0';

	return String(buf);
    
}

void setRegValue(HKEY key, String subkey, String name, String value) {
	HKEY mykey;
	
	LONG status=RegOpenKeyEx(key,subkey.getBytes(),0,KEY_SET_VALUE,&mykey);
	if (status) {
		String errMsg;
		LONG errMsgLen=512;
		char chMsg[512];
		errMsg.ReAlloc(errMsgLen);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,status,0,(LPTSTR) chMsg,errMsgLen,NULL);
		errMsg.sprintf("RegOpenKeyEx returned: %s",chMsg);
		TextLog(errMsg.getBytes());
		return ;
	} // if

	status=RegSetValueEx(mykey,name.getBytes(),NULL,REG_SZ,(BYTE *)value.getBytes(),value.length());
	RegCloseKey(mykey);
	if (status) {
		String errMsg;
		char chMsg[512];
		LONG errMsgLen=512;
		errMsg.ReAlloc(errMsgLen);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,status,0,chMsg,errMsgLen,NULL);
		errMsg.sprintf("RegSetValueEx returned: %s",chMsg);
		TextLog(errMsg.getBytes());
		return ;
	} // if
}

String findHighestDtVersion(HKEY myKey){

	DWORD type=0;
	DWORD len=512;
	DWORD nameLen;
	LONG status;

	TCHAR	subKey[512];   // buffer for subkey name
	FILETIME ftLastWriteTime;	  // last write time 

	String highestVersion = "0.0";
	DWORD i = 0;
	do { 
		nameLen = len;
		status = RegEnumKeyEx(myKey, i++,
					 subKey, 
					 &nameLen, 
					 NULL, 
					 NULL, 
					 NULL, 
					 &ftLastWriteTime); 
		if (status == ERROR_SUCCESS) 
		{
			if(subKey[0] > '0' && subKey[0] < '9'){		//name of subkey is a version number, 
				String currentVersion(subKey);
				if (currentVersion.compareTo(highestVersion) > 0){
					highestVersion = currentVersion;
				}
			}
		}
	} while (status != ERROR_NO_MORE_ITEMS);

	return highestVersion;
}


String getInstallationDirectory(){

	DWORD type=0;
	DWORD len=512;
	LONG status;
	HKEY myKey;
	String regKeyParent = "SOFTWARE\\dynaTrace\\";
	
	status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regKeyParent.getBytes(),type,KEY_READ, &myKey);
	
	if (status) {
		String errMsg;
		LONG errMsgLen=512;
		char chMsg[512];
		errMsg.ReAlloc(errMsgLen);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,status,0,(LPTSTR) chMsg,errMsgLen,NULL);
		errMsg.sprintf("getInstallationDirectory:RegOpenKeyEx returned: %s",chMsg);
		TextLog(errMsg.getBytes());
		return String();
	} // if

	String highestVersion = findHighestDtVersion(myKey);
	RegCloseKey(myKey);
	String regKeyHighestVersion = regKeyParent + highestVersion;
	return getRegValue(HKEY_LOCAL_MACHINE, regKeyHighestVersion, String("user.dir"));
}


BOOL startDtClient(){
	TextLog("Server is not present.");

	String str;
	String cmd("");
	// look if registry contains setting for dtd-client-install-dir
	cmd = getInstallationDirectory();

	if (cmd.length()>0)
		cmd=cmd+String("dtclient.exe");
	// look if the user has his own client to be started
	if(cmd.length()<1)
		cmd=getRegValue(HKEY_CURRENT_USER,String("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.dtds"),String("Application"));
	if(cmd.length()<1)
		cmd=getRegValue(HKEY_CURRENT_USER,String("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.dts"),String("Application"));
	// get System-wide setting
	if(cmd.length()<1)
		cmd=getRegValue(HKEY_LOCAL_MACHINE,String("SOFTWARE\\Classes\\Applications\\dtclient.exe\\shell\\open\\command"),String("command"));
	if(cmd.length()<1)
		cmd=getRegValue(HKEY_LOCAL_MACHINE,String("SOFTWARE\\Classes\\dtclient.exe\\shell\\open\\command"),String(""));

	cmd.replaceAll("%1", "");
	cmd.replaceAll("\"", "");

	//TODO: remove this !!!for testing only
	//cmd = "C:\\data\\branch\\3.1.x\\jloadtrace\\dtclient.exe";

	str.sprintf("[startDtClient] opening command \"%s\"",cmd.getBytes());
	TextLog(str.getBytes());
	String dir=cmd;
	if ((cmd.length()>0) && (cmd.lastIndexOf("\\")>0)) {
		dir=cmd.substring(0,cmd.lastIndexOf("\\"));
	} // if
	else {
		Log(eSeverityError,"Cannot find SilkPerformer Diagnostics Installation",0);
		return FALSE;
	} // else if 

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );


	String cmdLine=String("\"")+cmd.getBytes()+String("\"");

	if (!CreateProcess(NULL,(LPSTR) cmdLine.getBytes(),NULL,NULL,FALSE,0,NULL,dir.getBytes(),&si,&pi)) {
		DWORD status=GetLastError();
		LONG errMsgLen=512;
		char chPszErr[512];
		Log(eSeverityError,"CreateProcess Failed",0);
		String errMsg;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,status,0,chPszErr,errMsgLen,NULL);
		errMsg.sprintf("Cannot start dtclient: %s",chPszErr);
		Log(eSeverityError,errMsg.getBytes(),0);
		return FALSE;
	} // if
	else {
		Log(eSeverityInfo,"Starting SilkPerformer Diagnostics Client",0);
		//wait max. getWaitTime() for client
		int timeToWait = getWaitTime();
		time_t start = 0, now = 0;
		double elapsed = 0;
		time(&start);
		ClientVersion state = STATE_NOT_REACHABLE;
		while(elapsed < timeToWait){
			Log(eSeverityInfo,"Waiting for SilkPerformer Diagnostics Client...",0);
			apr_sleep(200000);
			state = gp_Rc->getDtClientVersion();
			if (state != STATE_NOT_REACHABLE)
				break;
			time(&now);
			elapsed = difftime(now, start);
		} 

		if (state != STATE_NOT_REACHABLE) {
			Log(eSeverityInfo,"SilkPerformer Diagnostics Client started",0);
			//let client finish startup, sometimes start/stop recording returns 404 although isRESTServerReachable() 
			//returns 200, TODO: should be checked in client
			apr_sleep(400000);	
		} // if
		else {
			Log(eSeverityError,"SilkPerformer Diagnostics Client did not respond",0);
			return FALSE;
		} // elseif
	} // else if
	return TRUE;
}

ClientVersion getClientVersion(bool forceStart) {
	
	if (!gp_Rc){
		Log(eSeverityInfo,"Initializing REST Interface Client",0L);
		gp_Rc = new RESTClient();
	}

	ClientVersion version = gp_Rc->getDtClientVersion();
	
	//check if dynatrace client is already started
	if (version == STATE_NOT_REACHABLE){
		//dynatrace client did not respond a supported version number
		Log(eSeverityInfo,"SilkPerformer Diagnostics Client not present",0L);
		
		if (forceStart) {
			BOOL clientStarted = startDtClient();
			if (clientStarted){	
				return gp_Rc->getDtClientVersion();
			}
			else {
				return STATE_NOT_REACHABLE;
			}
		}
	}
	return version;
}

void performRESTRequest(RESTRequestType queryType, String strData, String strData1){

	ClientVersion version = getClientVersion(queryType != GET_SESSION_ID);
	if (version == STATE_NOT_REACHABLE)
		return;
	
	switch (queryType){
		case START_SESSIONRECORDING: {
			String str = gp_Rc->startSessionRecording(getSystemProfile(), strData);
			if (str.equals("")) {
				g_strLastSessionID = CONST_GENERIC_SESSIONID;
			} else {
				g_strLastSessionID = str;
			}
			break;
		}
		case STOP_SESSIONRECORDING:
			//apr_sleep(3 * 1000 * 1000);
			gp_Rc->stopSessionRecording(getSystemProfile());
			break;
		case OPEN_PATH:
			gp_Rc->openPurePath(strData);
			break;
		case OPEN_API:
			gp_Rc->openAPIBreakDown(strData, strData1);
			break;
		case GET_SESSION_ID:
			String str = gp_Rc->getSessionId(getSystemProfile());
			if (str.equals("")) {
				g_strLastSessionID = CONST_GENERIC_SESSIONID;
			} else {
				g_strLastSessionID = str;
			}
			break;
	}
}

BOOL checkIsSessionRecording(void) {
	if (g_strPluginRegKey.length()<1) return FALSE;
	BOOL retVal=REG_DEFVAL_RECORDING;

	String strRegVal=getRegValue(HKEY_CURRENT_USER,g_strPluginRegKey,REG_VALNAME_RECORDING);
	
	if (strRegVal.length()>0) 
		retVal=strRegVal.equalsIgnoreCase("true");

#ifdef _DEBUG
	Log(eSeverityInfo,retVal?"checkIsSessionRecording Returning TRUE":"checkIsSessionRecording Returning FALSE",0);
	Log(eSeverityInfo,(String(REG_VALNAME_RECORDING)+String(" is ")+getRegValue(HKEY_CURRENT_USER,g_strPluginRegKey,REG_VALNAME_RECORDING)).getBytes(),0);
#endif
	return retVal;
}

int getWaitTime(void) {
	if (g_strPluginRegKey.length()<1) return REG_DEFVAL_WAITTIME;
	int retVal=REG_DEFVAL_WAITTIME;

	String strRegVal=getRegValue(HKEY_CURRENT_USER,g_strPluginRegKey,REG_VALNAME_WAITTIME);
	
	if (strRegVal.length()>0) 
		retVal=atoi(strRegVal.getBytes());

#ifdef _DEBUG
	String str;
	str.sprintf("getWaitTime Returning %i",retVal);
	Log(eSeverityInfo,str.getBytes(),0);
#endif
	return retVal;
}

int getClientPort(void) {
	if (g_strPluginRegKey.length()<1) return REG_DEFVAL_DTCLIENTPORT;
	int retVal=REG_DEFVAL_DTCLIENTPORT;

	String strRegVal=getRegValue(HKEY_CURRENT_USER,g_strPluginRegKey,REG_VALNAME_DTCLIENPORT);
	
	if (strRegVal.length()>0) 
		retVal=atoi(strRegVal.getBytes());

#ifdef _DEBUG
	String str;
	str.sprintf("getClientPort Returning %i",retVal);
	Log(eSeverityInfo,str.getBytes(),0);
#endif
	return retVal;
}

String getSystemProfile(void) {
	if (g_strPluginRegKey.length()<1) return String(REG_DEFVAL_APPSESSION);
	String retVal(REG_DEFVAL_APPSESSION);
	String strRegVal=getRegValue(HKEY_CURRENT_USER,g_strPluginRegKey,REG_VALNAME_APPSESSION);
	
	if (strRegVal.length()>0) 
		retVal=strRegVal;

#ifdef _DEBUG
	Log(eSeverityInfo,String().sprintf("getSystemProfile Returning %s",retVal.getBytes()).getBytes(),0);
#endif
	return retVal;
}
// ----------------------------------------------------------------------

void StrFree     (LPSTR* ppStr)
{
  if (!ppStr || !(*ppStr))
    return;

  free (*ppStr);
  *ppStr = NULL;
}

// ----------------------------------------------------------------------

BOOL StrSet     (LPSTR* ppStr, LPCSTR pszValue, ULONG lLength = ULONG_MAX, ULONG* pNewLength = NULL)
{
  if (!ppStr)
    return FALSE;

  StrFree (ppStr);

  free (*ppStr);
  *ppStr = NULL;

  if (pNewLength)
    (*pNewLength) = 0;

  if (!pszValue)
    return TRUE;

  if (lLength == ULONG_MAX)
    lLength = strlen (pszValue);

  (*ppStr) = (LPSTR) malloc (lLength + 1);
  if (!(*ppStr))
    return FALSE;

  strncpy ((*ppStr), pszValue, lLength);
  (*ppStr)[lLength] = '\0';
  if (pNewLength)
    *pNewLength = lLength;

  return TRUE;
}

// ----------------------------------------------------------------------

BOOL StrAppend  (LPSTR* ppStr, LPCSTR pszValue, ULONG lLength = ULONG_MAX, ULONG* pNewLength = NULL)
{
  if (!ppStr)
    return FALSE;

  if (!pszValue || (!pszValue[0] && lLength == ULONG_MAX))
    return TRUE;

  if (!(*ppStr))  
    return StrSet (ppStr, pszValue, lLength, pNewLength);      

  if (pNewLength)
    *pNewLength = NULL;

  LPCSTR pszStrOrg  = *ppStr;
  ULONG  lLenOrg    = strlen(*ppStr);

  // lAppendLen is always > 0
  ULONG  lAppendLen  = (lLength == ULONG_MAX ? strlen (pszValue) : lLength);

  // allocate memory
  *ppStr = (LPSTR) malloc (lLenOrg + lAppendLen + 1);
  if (!(*ppStr))
    return FALSE; // Could not allocate memory  

  // copy data
  if (lLenOrg)
    strncpy ((*ppStr), pszStrOrg, lLenOrg);

  // Append string (copy string AND ending '\0'
  strncpy ((*ppStr) + lLenOrg, pszValue, lAppendLen);
  (*ppStr)[lLenOrg + lAppendLen] = '\0';

  // clean up old memory  
  free ((VOID*)pszStrOrg);
  if (pNewLength)
    *pNewLength = lLenOrg + lAppendLen;
  return TRUE;
}


// ----------------------------------------------------------------------

// Initialize Content members
void ContentInit (TContent* pContent)
{
  pContent->lLength        = 0;
  pContent->pszContentType = NULL;
  pContent->pszContent     = NULL;
}

// ----------------------------------------------------------------------

void ContentFree (TContent* pContent)
{
  if (!pContent)
    return;

  StrFree ((LPSTR*) &(pContent->pszContent));
  StrFree ((LPSTR*) &(pContent->pszContentType));
  pContent->lLength = 0;
}

// ----------------------------------------------------------------------

// Append data to existing content
BOOL ContentAppend (TContent* pContent, LPCSTR pszAppend, ULONG lLength = ULONG_MAX)
{
  if (!pContent)
    return FALSE;

  return StrAppend ((LPSTR*) &(pContent->pszContent), pszAppend, lLength, &(pContent->lLength));  
}

// ----------------------------------------------------------------------

// Content has to be initialized once before
// old content will be freed
BOOL ContentSet (TContent* pContent, LPCSTR pszType, LPCSTR pszContent)
{
  if (!pContent)
    return FALSE;

  ContentFree (pContent);

  if (!StrSet  ((LPSTR*) &(pContent->pszContentType), pszType))
    return FALSE;

  return StrSet  ((LPSTR*) &(pContent->pszContent), pszContent, ULONG_MAX, &(pContent->lLength));
}

//-----------------------------------------------------------------------------

void Log (EPI_Severity eSeverity, LPCSTR pszLogMsg, ULONG uCode)
{
  TextLog((String("##LOG : ")+String((const char *)pszLogMsg)).getBytes());
  if (g_FNMessageLog)
    g_FNMessageLog (g_pMessageLogContext, eSeverity, pszLogMsg, uCode);
}

//-----------------------------------------------------------------------------
int GetInfo(HPILTCONTEXT hLt, EPI_PerformerInfo eInformation, char *pszBuffer, unsigned long &ruSize) {

	if (g_FNInfo)
		return g_FNInfo(g_pGetInfoContext,hLt,eInformation,pszBuffer,ruSize);
	else
		return 0;
}

//-----------------------------------------------------------------------------

BOOL PI_GetFirstPluginAttribute (ULONG uModule, LPSTR sName, ULONG& ruSize, EPI_Type& eType)
{
  DLOG("-> PI_GetFirstPluginAttribute");
  g_strLastErrorMessage.clear();
  if (!sName) {
	  ruSize=sizeof(NAME_FEATURE_RECORDING);
	  eType=eTypeBoolean;
	  return TRUE;
  } // if
  else {
	  ruSize=sizeof(NAME_FEATURE_RECORDING);
	  memcpy(sName,NAME_FEATURE_RECORDING,sizeof(NAME_FEATURE_RECORDING));
	  giAttrNum=0;
	  eType=eTypeBoolean;
	  return TRUE;
  }
  return FALSE;
}


//-----------------------------------------------------------------------------

BOOL PI_GetNextPluginAttribute (LPSTR sName, ULONG& ruSize, EPI_Type& eType)
{
  String strLog;
  strLog.sprintf("-> PI_GetNextPluginAttribute #%i",giAttrNum);
  DLOG(strLog.getBytes());
  g_strLastErrorMessage.clear();

  ruSize=0;
  switch (giAttrNum) {
	  case 0:
		  if (sName) {
			  giAttrNum++;
			  ruSize=sizeof(NAME_FEATURE_WAITTIME);
			  memcpy(sName,NAME_FEATURE_WAITTIME,ruSize);
			  eType=eTypeNumber;
			  return TRUE;
		  } // if
		  else {
			  ruSize=sizeof(NAME_FEATURE_WAITTIME);
			  eType=eTypeNumber;
			  return TRUE;
		  } // elseif
		  break;
	  case 1:
		  if (sName) {
			  giAttrNum++;
			  ruSize=sizeof(NAME_FEATURE_APPSESSION);
			  memcpy(sName,NAME_FEATURE_APPSESSION,ruSize);
			  eType=eTypeString;
			  return TRUE;
		  } // if
		  else {
			  ruSize=sizeof(NAME_FEATURE_APPSESSION);
			  eType=eTypeString;
			  return TRUE;
		  } // elseif
	  case 2:
		  if (sName) {
			  giAttrNum++;
			  ruSize=sizeof(NAME_FEATURE_DTCLIENTPORT);
			  memcpy(sName,NAME_FEATURE_DTCLIENTPORT,ruSize);
			  eType=eTypeNumber;
			  return TRUE;
		  } // if
		  else {
			  ruSize=sizeof(NAME_FEATURE_DTCLIENTPORT);
			  eType=eTypeNumber;
			  return TRUE;
		  } // elseif
	  default:
		  return FALSE;
  } // switch
  return FALSE;
}
//-----------------------------------------------------------------------------

BOOL PI_GetPluginAttributeNumber(LPCSTR csName, LONG& riVal, LONG& riDefVal)
{
  DLOG("-> PI_GetPluginAttributeNumber");
  g_strLastErrorMessage.clear();

  if (strcmp(NAME_FEATURE_WAITTIME,csName)==0) { 
	  if (g_strPluginRegKey.length()<1)
		  return FALSE;

	  riVal=::getWaitTime();
	  riDefVal=REG_DEFVAL_WAITTIME;
	  return TRUE;
  } // if

  if (strcmp(NAME_FEATURE_DTCLIENTPORT, csName)==0) {
	if (g_strPluginRegKey.length()<1)
		  return FALSE;

	riVal=::getClientPort();
	riDefVal=REG_DEFVAL_DTCLIENTPORT;
	return TRUE;
  }
  return FALSE;
}
//-----------------------------------------------------------------------------
                                    
BOOL PI_GetPluginAttributeString(LPCSTR csName, LPSTR sValue, ULONG uValueSize, ULONG& ruSize, LPSTR sDefValue, ULONG uDefValueSize, ULONG& ruDefSize)
{
  DLOG("-> PI_GetPluginAttributeString");
  g_strLastErrorMessage.clear();

  if (strcmp(NAME_FEATURE_APPSESSION,csName)==0) { 
	  if (g_strPluginRegKey.length()<1)
		  return FALSE;

	  String value=::getSystemProfile();

	  if (!sValue) {
		  ruSize=value.length()+1;
	  } // if
	  else {
		  ruSize=value.length()+1;
		  if (uValueSize < (ULONG)(value.length()+1))
			  return FALSE;
		  strcpy(sValue,value.getBytes());
	  }

	  if (!sDefValue) {
		  ruDefSize=value.length()+1;
	  } // if
	  else {
		  ruDefSize=value.length()+1;
		  if (uDefValueSize<(sizeof(REG_DEFVAL_APPSESSION)))
			  return FALSE;
		  strcpy(sDefValue,REG_DEFVAL_APPSESSION);
	  }

	  return TRUE;
  } // if

  return FALSE;  
}
//-----------------------------------------------------------------------------
                                   
BOOL PI_GetPluginAttributeBoolean(LPCSTR csName, BOOL& rbVal, BOOL& rbDefVal)
{
  DLOG("-> PI_GetPluginAttributeBoolean");
  g_strLastErrorMessage.clear();
  
  if (strcmp(NAME_FEATURE_RECORDING,csName)==0) { // Attribute: Sessionrecording
	  if (g_strPluginRegKey.length()<1)
		  return FALSE;

	  rbVal=checkIsSessionRecording();
	  rbDefVal=TRUE;
	  return TRUE;
  } // if

  return FALSE;  
}
//-----------------------------------------------------------------------------
                                   
BOOL PI_GetPluginAttributeFloat(LPCSTR csName, DOUBLE& rfVal, DOUBLE& rfDefVal)
{
  DLOG("-> PI_GetPluginAttributeFloat");
  g_strLastErrorMessage.clear();

  return FALSE;  
}
//-----------------------------------------------------------------------------

BOOL PI_SetPluginAttributeNumber(LPCSTR csName, LONG iVal)
{
	DLOG("-> PI_SetPluginAttributeNumber");
	g_strLastErrorMessage.clear();

	if (strcmp(NAME_FEATURE_WAITTIME,csName)==0) {
		String str;
		if (g_strPluginRegKey.length()<1)
			return FALSE;
		if (iVal<=0) return FALSE;

		str.sprintf("%i",iVal);
		setRegValue(HKEY_CURRENT_USER,g_strPluginRegKey,REG_VALNAME_WAITTIME,str);
		return TRUE;
	} // if

	if (strcmp(NAME_FEATURE_DTCLIENTPORT,csName)==0) {
		String str;
		if (g_strPluginRegKey.length()<1)
			return FALSE;
		if (iVal<=0) return FALSE;

		str.sprintf("%i",iVal);
		setRegValue(HKEY_CURRENT_USER,g_strPluginRegKey,REG_VALNAME_DTCLIENPORT,str);
	
		return TRUE;
	} // if

	return FALSE;  
}
//-----------------------------------------------------------------------------
                                    
BOOL PI_SetPluginAttributeString(LPCSTR csName, LPCSTR csValue)
{
  DLOG("-> PI_SetPluginAttributeString");
  g_strLastErrorMessage.clear();

  if (strcmp(NAME_FEATURE_APPSESSION,csName)==0) {
	  if (g_strPluginRegKey.length()<1)
		  return FALSE;
	  if (!csValue) return FALSE;

	  setRegValue(HKEY_CURRENT_USER,g_strPluginRegKey,REG_VALNAME_APPSESSION,String(csValue));
	  return TRUE;
  } // if

  return FALSE;
}
//-----------------------------------------------------------------------------

BOOL PI_GetPluginAttributeDescription(LPCSTR csName, LPSTR sDescription, ULONG uValueSize, ULONG& ruSize)
{
  String strLog;
  strLog.sprintf("-> PI_GetPluginAttributeDescription for %s",csName);
  DLOG(strLog.getBytes());
  g_strLastErrorMessage.clear();

  if (strcmp(NAME_FEATURE_RECORDING,csName)==0) {
	  if (!sDescription) {
		  ruSize=sizeof(DESC_FEATURE_RECORDING);
		  return TRUE;
	  } // if
	  if (ruSize>=sizeof(DESC_FEATURE_RECORDING)) {
		  ruSize=sizeof(DESC_FEATURE_RECORDING);
		  memcpy(sDescription,DESC_FEATURE_RECORDING,sizeof(DESC_FEATURE_RECORDING));
		  return TRUE;
	  } // if
  } // if

  if (strcmp(NAME_FEATURE_WAITTIME,csName)==0) {
	  if (!sDescription) {
		  ruSize=sizeof(DESC_FEATURE_WAITTIME);
		  return TRUE;
	  } // if
	  if (ruSize>=sizeof(DESC_FEATURE_WAITTIME)) {
		  ruSize=sizeof(DESC_FEATURE_WAITTIME);
		  memcpy(sDescription,DESC_FEATURE_WAITTIME,sizeof(DESC_FEATURE_WAITTIME));
		  return TRUE;
	  } // if
  } // if

  if (strcmp(NAME_FEATURE_APPSESSION,csName)==0) {
	  if (!sDescription) {
		  ruSize=sizeof(DESC_FEATURE_APPSESSION);
		  return TRUE;
	  } // if
	  if (ruSize>=sizeof(DESC_FEATURE_APPSESSION)) {
		  ruSize=sizeof(DESC_FEATURE_APPSESSION);
		  memcpy(sDescription,DESC_FEATURE_APPSESSION,sizeof(DESC_FEATURE_APPSESSION));
		  return TRUE;
	  } // if
  } // if

  if (strcmp(NAME_FEATURE_DTCLIENTPORT,csName)==0) {
	  if (!sDescription) {
		  ruSize=sizeof(DESC_FEATURE_DTCLIENTPORT);
		  return TRUE;
	  } // if
	  if (ruSize>=sizeof(DESC_FEATURE_DTCLIENTPORT)) {
		  ruSize=sizeof(DESC_FEATURE_DTCLIENTPORT);
		  memcpy(sDescription,DESC_FEATURE_DTCLIENTPORT,sizeof(DESC_FEATURE_DTCLIENTPORT));
		  return TRUE;
	  } // if
  } // if

  return FALSE;
}
//-----------------------------------------------------------------------------
                                   
BOOL PI_SetPluginAttributeBoolean(LPCSTR csName, BOOL bVal)
{
  DLOG("-> PI_SetPluginAttributeBoolean");
  g_strLastErrorMessage.clear();
  if (strcmp(NAME_FEATURE_RECORDING,csName)==0) {
	  if (g_strPluginRegKey.length()<1)
		  return FALSE;

	  setRegValue(HKEY_CURRENT_USER,g_strPluginRegKey,REG_VALNAME_RECORDING,bVal?"true":"false");
	  return TRUE;
  } // if

  return FALSE;
}
//-----------------------------------------------------------------------------
                                    
BOOL PI_SetPluginAttributeFloat(LPCSTR csName, DOUBLE fVal)
{
  DLOG("-> PI_SetPluginAttributeFloat");
  g_strLastErrorMessage.clear();

  return FALSE;
}
//-----------------------------------------------------------------------------

// Features
BOOL PI_GetFirstPluginFeature(ULONG uModule, LPSTR sName, ULONG& ruSize, EPI_Type& eType)
{
  DLOG("-> PI_GetFirstPluginFeature");
  g_strLastErrorMessage.clear();

  if (!sName) {
	  ruSize=sizeof(NAME_FEATURE_RECORDING);
	  eType=eTypeBoolean;
	  return TRUE;
  } // if
  else {
	  ruSize=sizeof(NAME_FEATURE_RECORDING);
	  memcpy(sName,NAME_FEATURE_RECORDING,sizeof(NAME_FEATURE_RECORDING));
	  eType=eTypeBoolean;
	  giFeatureNum=0;
	  return TRUE;
  }

  return FALSE;
}
//-----------------------------------------------------------------------------

BOOL PI_GetNextPluginFeature(LPSTR sName, ULONG& ruSize, EPI_Type& eType)
{
  String strLog;
  strLog.sprintf("-> PI_GetNextPluginFeature #%i",giAttrNum);
  DLOG(strLog.getBytes());
  g_strLastErrorMessage.clear();

  ruSize=0;
  switch (giFeatureNum) {
	  case 0:
		  if (sName) {
			  giFeatureNum++;
			  ruSize=sizeof(NAME_FEATURE_WAITTIME);
			  memcpy(sName,NAME_FEATURE_WAITTIME,ruSize);
			  eType=eTypeBoolean;
			  return TRUE;
		  } // if
		  else {
			  ruSize=sizeof(NAME_FEATURE_WAITTIME);
			  eType=eTypeBoolean;
			  return TRUE;
		  } // elseif
		  break;
	  case 1:
		  if (sName) {
			  giFeatureNum++;
			  ruSize=sizeof(NAME_FEATURE_APPSESSION);
			  memcpy(sName,NAME_FEATURE_APPSESSION,ruSize);
			  eType=eTypeBoolean;
			  return TRUE;
		  } // if
		  else {
			  ruSize=sizeof(NAME_FEATURE_APPSESSION);
			  eType=eTypeBoolean;
			  return TRUE;
		  } // elseif
		  break;
	  case 2:
		  if (sName) {
			  giFeatureNum++;
			  ruSize=sizeof(NAME_FEATURE_DTCLIENTPORT);
			  memcpy(sName,NAME_FEATURE_DTCLIENTPORT,ruSize);
			  eType=eTypeBoolean;
			  return TRUE;
		  } // if
		  else {
			  ruSize=sizeof(NAME_FEATURE_DTCLIENTPORT);
			  eType=eTypeBoolean;
			  return TRUE;
		  } // elseif
		  break;
	  default:
		  return FALSE;
  } // switch

}
//-----------------------------------------------------------------------------

BOOL PI_GetPluginFeatureNumber(LPCSTR csName, LONG& riVal, LONG& riDefVal)
{
  DLOG("-> PI_GetPluginFeatureNumber");
  g_strLastErrorMessage.clear();

  return FALSE;
}
//-----------------------------------------------------------------------------
                                   
BOOL PI_GetPluginFeatureString(LPCSTR csName, LPSTR sVal, ULONG uValueSize, ULONG& ruSize, LPSTR sDefValue, ULONG uDefValueSize, ULONG& ruDefSize)
{
  DLOG("-> PI_GetPluginFeatureString");
  g_strLastErrorMessage.clear();

  return FALSE;
}
//-----------------------------------------------------------------------------
                                    
BOOL PI_GetPluginFeatureBoolean(LPCSTR csName, BOOL& rbVal, BOOL& rbDefVal)
{
  DLOG("-> PI_GetPluginFeatureBoolean");
  g_strLastErrorMessage.clear();

  if (strcmp(NAME_FEATURE_RECORDING,csName)==0) { // Session Recording Feature
	  rbVal=TRUE;
	  rbDefVal=TRUE;
	  return TRUE;
  } // if

  if (strcmp(NAME_FEATURE_WAITTIME,csName)==0) { // Session Recording Feature
	  rbVal=TRUE;
	  rbDefVal=TRUE;
	  return TRUE;
  } // if

  if (strcmp(NAME_FEATURE_APPSESSION,csName)==0) { // Session Recording Feature
	  rbVal=TRUE;
	  rbDefVal=TRUE;
	  return TRUE;
  } // if

  if (strcmp(NAME_FEATURE_DTCLIENTPORT,csName)==0) {
	  rbVal=TRUE;
	  rbDefVal=TRUE;
	  return TRUE;
  } // if
  return FALSE;
}
//-----------------------------------------------------------------------------
                                    
BOOL PI_GetPluginFeatureFloat(LPCSTR csName, DOUBLE& rfVal, DOUBLE& rfDefVal)
{
  DLOG("-> PI_GetPluginFeatureFloat");
  g_strLastErrorMessage.clear();

  return FALSE;
}
//-----------------------------------------------------------------------------

BOOL PI_GetPluginFeatureDescription(LPCSTR csName, LPSTR sDescription, ULONG uValueSize, ULONG& ruSize)
{
  DLOG("-> PI_GetPluginFeatureDescription");
  g_strLastErrorMessage.clear();

  if (strcmp(NAME_FEATURE_RECORDING,csName)==0) {
	  if (!sDescription) {
		  ruSize=sizeof(DESC_FEATURE_RECORDING);
		  return TRUE;
	  } // if
	  if (ruSize>=sizeof(DESC_FEATURE_RECORDING)) {
		  ruSize=sizeof(DESC_FEATURE_RECORDING);
		  memcpy(sDescription,DESC_FEATURE_RECORDING,sizeof(DESC_FEATURE_RECORDING));
		  return TRUE;
	  } // if
  } // if

  return FALSE;
}
//-----------------------------------------------------------------------------


BOOL PI_SetPluginFeatureNumber(LPCSTR csName, LONG iVal)
{
  DLOG("-> PI_SetPluginFeatureNumber");
  g_strLastErrorMessage.clear();

  return FALSE;
}
//-----------------------------------------------------------------------------
                               
BOOL PI_SetPluginFeatureString(LPCSTR csName, LPCSTR csValue)
{
  DLOG("-> PI_SetPluginFeatureString");
  g_strLastErrorMessage.clear();

  return FALSE;
}
//-----------------------------------------------------------------------------
                                    
BOOL PI_SetPluginFeatureBoolean(LPCSTR csName, BOOL bVal)
{
  DLOG("-> PI_SetPluginFeatureBoolean");
  g_strLastErrorMessage.clear();

  if (strcmp(NAME_FEATURE_RECORDING,csName)==0) { // Session Recording Feature
	  //g_fSessionRecording=bVal;
	  return TRUE;
  } // if

  //if (strcmp(NAME_FEATURE_REPORT,csName)==0) { // Show in Report - Feature
	 // //g_fShowInSilkPerformerReport=bVal; -- feature is disabled in current release
	 // return TRUE;
  //} // if

  return FALSE;
}
//-----------------------------------------------------------------------------
                                    
BOOL PI_SetPluginFeatureFloat(LPCSTR csName, DOUBLE fVal)
{
  DLOG("-> PI_SetPluginFeatureFloat");
  g_strLastErrorMessage.clear();

  return FALSE;
}
//-----------------------------------------------------------------------------

// Actions
BOOL PI_OnEvent(EPI_Event eEvent, void* pParam)
{
  DLOG("-> PI_OnEvent");
  g_strLastErrorMessage.clear();
  String objLTName=String("Silkperformer Loadtest");
  switch (eEvent) {
	  case eEventStart: 
		  DLOG("Starting loadtest");
		  if (!checkIsLoadTest()) // do nothing, if this is not a real loadtest
			break;
		  if (checkIsSessionRecording()) {
			  String str=getRecordSessionName(pParam);
			  if (str.length()>0) objLTName=str;
			  performRESTRequest(START_SESSIONRECORDING, objLTName, String());
		  } // if
		  else {
			  performRESTRequest(GET_SESSION_ID, String(), String());
		  }
		  break;
	  case eEventStop: 
		  DLOG("Stopped loadtest");
		  if (!checkIsLoadTest()) // do nothing, if this is not a real loadtest
			break;
		  if (checkIsSessionRecording()) {
			performRESTRequest(STOP_SESSIONRECORDING, String(),String());
		  } // if
		  break;
	  case eEventKill: 
		  DLOG("Killed loadtest");
		  if (!checkIsLoadTest()) // do nothing, if this is not a real loadtest
			break;
		  if (checkIsSessionRecording()) {
			performRESTRequest(STOP_SESSIONRECORDING, String(),String());
		  } // if
		  break;
	  default:
		  DLOG(" -> PI_OnEvent");
  } // switch


  return TRUE;
}
//-----------------------------------------------------------------------------

BOOL PI_OnAction(LONG iAction, void* pParam)
{
  DLOG("-> PI_OnAction");
  g_strLastErrorMessage.clear();

  return FALSE;
}
//-----------------------------------------------------------------------------

BOOL PI_Notify(EPI_Notification eNotification, void* pParam)
{
  DLOG("-> PI_Notify");
  g_strLastErrorMessage.clear();

  return TRUE;
}
//-----------------------------------------------------------------------------

void  PI_RegisterMessageLog(void* pClient, TMessageLogCallback fCallback)
{
  unsigned long ulInfoSize=0;
  char *chBuf;
  DLOG("-> PI_RegisterMessageLog");
  g_strLastErrorMessage.clear();

  g_FNMessageLog        = fCallback;
  g_pMessageLogContext  = pClient;

#ifdef _DEBUG
  DLOG("MessageLog registered");
  if (!g_FNInfo)
	  DLOG("SetInfoCallback not set yet");
#endif

  if (!GetInfo(NULL,ePIPInfo_RegKey_Plugin,(char *) NULL,ulInfoSize)) {
	  ulInfoSize+=1;
      chBuf=new char[ulInfoSize];
	  GetInfo(NULL,ePIPInfo_RegKey_Plugin,chBuf,ulInfoSize);
	  pnapi::String str;
	  chBuf[ulInfoSize-1]='\0';
	  str.sprintf("RegKey is : %s",chBuf);
	  delete chBuf;
	  DLOG(str.getBytes());
  } // if
  else {
	  Log(eSeverityWarning,"Query of Reg-Key failed",0);
  } // elseif
}

//-----------------------------------------------------------------------------

BOOL PI_GetProfileSettings(const char* pszProfileName, char* pszXmlSettings, unsigned long& rulSize)
{
	DLOG("-> PI_GetProfileSettings");
	g_strLastErrorMessage.clear();

	ClientVersion state = STATE_4PLUS;
	state = getClientVersion(true);

	//STATE_4PLUS is used as default
	int sizeNeeded = (state == STATE_321_35) ? rulSize = sizeof(PROFILE_SETTINGS_XML_PRE_4)+1 : sizeof(PROFILE_SETTINGS_XML)+1;

	if ((!pszXmlSettings) || (rulSize < sizeNeeded))
	{
		DLOG("Sending Modification-Stringbuffer-Size");
		rulSize = sizeNeeded;
		return FALSE;
	}

	Log(eSeverityInfo, "Modifying Loadtest-Profile",0);
	if (state == STATE_321_35)
		strcpy (pszXmlSettings,PROFILE_SETTINGS_XML_PRE_4);
	else
		strcpy (pszXmlSettings,PROFILE_SETTINGS_XML);
	rulSize = sizeNeeded; // Ending '\0' included

	return TRUE;
}


//-----------------------------------------------------------------------------

// General
LPCSTR PI_GetName(ULONG uModule)
{
  DLOG("-> PI_GetName");
  g_strLastErrorMessage.clear();
  //GetFileVersionInfoSize();

  return objVerInternalName.getBytes();
}

//-----------------------------------------------------------------------------

LPCSTR PI_GetDescription(ULONG uModule)
{
  DLOG("-> PI_GetDescription");
  g_strLastErrorMessage.clear();

  return objVerFileDescription.getBytes();
}
//-----------------------------------------------------------------------------

LPCSTR PI_GetVersion()
{
  DLOG("-> PI_GetVersion");
  g_strLastErrorMessage.clear();

  return objVerProductVersion.getBytes();
}


HMODULE GetCurrentModule()
{
#if _MSC_VER < 1300    // earlier than .NET compiler (VC 6.0)

  MEMORY_BASIC_INFORMATION mbi;
  static int dummy;
  VirtualQuery( &dummy, &mbi, sizeof(mbi) );

  return reinterpret_cast<HMODULE>(mbi.AllocationBase);

#else    // VC 7.0

  // from ATL 7.0 sources

  return reinterpret_cast<HMODULE>(&__ImageBase);
#endif
}

//-----------------------------------------------------------------------------
HICON PI_GetIcon ( ULONG uModule, EPI_Icon eIcon )
{
	g_strLastErrorMessage.clear();
  // always use same icon
	switch (eIcon) {
		case ePIIcon_Settings:
			return (HICON) LoadImage(hModule /*GetCurrentModule()*/, MAKEINTRESOURCE(IDI_DTDICON32),IMAGE_ICON,0,0,0);
		case ePIIcon_WindowTab:
			return (HICON) LoadImage(hModule /*GetCurrentModule()*/, MAKEINTRESOURCE(IDI_DTDICON16),IMAGE_ICON,0,0,0);
		default:
			return (HICON) LoadImage(hModule /*GetCurrentModule()*/, MAKEINTRESOURCE(IDI_DTDICON16),IMAGE_ICON,0,0,0);
	} // switch
}

//-----------------------------------------------------------------------------
BOOL  PI_GetInfo (ULONG  uModule, HPILOADTEST hLT, EPI_Info ePIInfo, LPCSTR pszContext, LPSTR pszBuffer, ULONG&  ruSize) {
	DLOG("-> PI_GetInfo");
	g_strLastErrorMessage.clear();
#ifdef _DEBUG
	String str;
	str.sprintf(" -> PiGetInfoData: HPILOADTEST is %s and ePIInfo is %i, SessionID is \"%s\"",hLT?"set":"not set",ePIInfo,g_strLastSessionID.getBytes());
	DLOG(str.getBytes());
#endif

	switch (ePIInfo) {
		case ePIInfo_CurrentSessionInfo:
			if (!checkIsLoadTest()) {
				g_strLastErrorMessage="Is no regular Loadtest";
				ruSize=0;
				return FALSE;
			} // if
/*
			if (!checkIsSessionRecording()) {
				g_strLastErrorMessage="Is not recording the Session";
				ruSize=0;
				return FALSE;
			} // if
*/
			if (g_strLastSessionID.length()<1) {
				g_strLastErrorMessage="No SessionID present";
				ruSize=0;
				return FALSE;
			} // if

			if (ruSize<((ULONG)(g_strLastSessionID.length()+1))) {
				ruSize=g_strLastSessionID.length()+1;
				DLOG("Returning length info for sessionid");
				return TRUE;
			} // if
			else {
				ruSize=g_strLastSessionID.length()+1;
				strcpy(pszBuffer,g_strLastSessionID.getBytes());
				DLOG("Returning sessionid");
				return TRUE;
			} // elseif
			break;
		case ePIInfo_AnalyzeImageFile:
			if (ruSize<((ULONG)(sizeof(FILE_ICON_REPORT)+1))) {
				ruSize=sizeof(FILE_ICON_REPORT)+1;
				return TRUE;
			} // if
			else {
				ruSize=sizeof(FILE_ICON_REPORT)+1;
				strcpy(pszBuffer,FILE_ICON_REPORT);
				return TRUE;
			} // elseif
			break;
		default:
			ruSize=0;
	} // switch

	return FALSE;
}

//-----------------------------------------------------------------------------
BOOL PI_CanAnalyzeMeasure (HPILOADTEST hLT, LPCSTR pszSessionInfo, LPCSTR pszMeasure, LPCSTR pszContext) {
	DLOG("-> PI_CanAnalyzeMeasure");
	g_strLastErrorMessage.clear();

	return TRUE; // can analyze in any case 
}

//-----------------------------------------------------------------------------
BOOL PI_AnalyzeMeasure(HPILOADTEST hLT, LPCSTR pszSessionInfo, LPCSTR pszMeasure, LPCSTR pszContext) {
	char *chBuffer=NULL;
	unsigned long ulBuffer=0;
	String strContent;
	String strLog;

	DLOG("-> PI_AnalyzeMeasure");
	g_strLastErrorMessage.clear();
	
	if (!pszSessionInfo) pszSessionInfo=CONST_GENERIC_SESSIONID;
	if (strlen(pszSessionInfo)<1) pszSessionInfo=CONST_GENERIC_SESSIONID;
	if (!pszMeasure) pszMeasure="";
	if (!pszContext) pszContext="";

	performRESTRequest(OPEN_API,pszSessionInfo, pszMeasure); 

	return TRUE;
}

//-----------------------------------------------------------------------------
LPCSTR PI_GetLastErrorMessage()
{
  DLOG("-> PI_GetLastErrorMessage");

  if (g_strLastErrorMessage.length()>0) {
	  return g_strLastErrorMessage.getBytes();
  } // if

  return NULL;  
}
//-----------------------------------------------------------------------------

BOOL PI_Save()
{    
  DLOG("-> PI_Save");

  g_strLastErrorMessage.clear();

  return TRUE;
}
//-----------------------------------------------------------------------------

BOOL PI_Initialize(ULONG uModule)
{  
  DLOG("-> PI_Initialize");
  g_strLastErrorMessage.clear();
  ContentInit (&g_tmpContent);
  ContentInit (&g_tmpPageContent);  

  g_FNSelectNode        = NULL;
  g_pSelectNodeContext  = NULL;

  g_FNMessageLog        = NULL;
  g_pMessageLogContext  = NULL;

  return TRUE;  
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// TLE Specific PlugIn-Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

HPILOADTEST PI_InitLoadtest (HPILTCONTEXT pLTContext)
{ 
  DLOG("-> PI_InitLoadtest");
  g_strLastErrorMessage.clear();

  loadtestdata * data=new loadtestdata;
  data->sessionID.clear();
  return (HPILOADTEST) data;
}
//-----------------------------------------------------------------------------

void PI_FreeLoadtest (HPILOADTEST hLT)
{
  DLOG("-> PI_FreeLoadtest");
  g_strLastErrorMessage.clear();
  loadtestdata * data=(loadtestdata *) hLT;
  if (data!=NULL)
	  delete data;
}
//-----------------------------------------------------------------------------

BOOL PI_BeginPage (HPILOADTEST hLT, ULONG uId, LPCSTR pszName)
{
  DLOG("-> PI_BeginPage ");
  g_strLastErrorMessage.clear();

  g_page.dtUrlAbs.clear();
  g_page.dtHeader.clear();
  g_page.dtApiBDString.clear();
  g_page.subpages.clear();
  g_page.uid=uId;
  if (pszName)
	  g_page.pageName.sprintf("%s",pszName);
  else
	  g_page.pageName.clear();

  return TRUE;
}
//-----------------------------------------------------------------------------

BOOL PI_EndPage  (HPILOADTEST hLT, const TContent** ppContent)
{
  DLOG("-> PI_EndPage ");
  g_strLastErrorMessage.clear();


  *ppContent = renderComplexPage(g_page);
  return TRUE;
}
//-----------------------------------------------------------------------------

BOOL PI_AddDocument (HPILOADTEST hLT,          
                                  ULONG       uId, 
                                  LPCSTR      sUrlAbs,      
                                  LPCSTR      sRequHeader, 
                                  LPCSTR      sRequBody, 
                                  ULONG       uRequBodyLen,
                                  LPCSTR      sRespHeader, 
                                  LPCSTR      sRespBody, 
                                  ULONG       uRespBodyLen,
                                  const TContent** ppContent
                                  )
{
  DLOG("-> PI_AddDocument ");
  g_strLastErrorMessage.clear();

  String header;
  String str;
  String strApi;
  if (sRespHeader) {
	  header=sRespHeader;
	  str=getDynatraceHeader(header);
  } // if

  if (sRequHeader) 
	  header=sRequHeader;
  else 
	  header=String();

  strApi=getDynatraceName(header,str);

  page tmpPage;
  tmpPage.pageName=g_page.pageName;
  tmpPage.dtUrlAbs=String(sUrlAbs);
  tmpPage.dtHeader=str;
  tmpPage.dtApiBDString=strApi;
  tmpPage.uid=uId;

  if (str.length()>0) {
	  g_page.subpages.push_back(tmpPage);
	  g_page.dtHeader=str;
  } // if
  if (!strApi.equals(NO_API_SEARCHPATH_SET)) {
		g_page.dtApiBDString=strApi;
  } // if

  *ppContent = renderSimpleContentPage(str, strApi, sUrlAbs, g_page.pageName.getBytes(),tmpPage);/*&g_tmpContent*/;
  return TRUE;
}

//-----------------------------------------------------------------------------

BOOL PI_GetContent (HPILOADTEST hLT, ULONG uId, LPCSTR pszContext, const TContent** ppContent)
{  
  char sId[33];
  ultoa (uId, sId, 10);
  g_strLastErrorMessage.clear();

  ContentSet    (&g_tmpContent, "text", "");
  ContentAppend (&g_tmpContent, "<br/><a href=\"Home.html\">Home</a><br/>");
  ContentAppend (&g_tmpContent, "->PI_GetContent");
  ContentAppend (&g_tmpContent, " uid: ");
  ContentAppend (&g_tmpContent, sId);  
  ContentAppend (&g_tmpContent, " context: ");
  ContentAppend (&g_tmpContent, pszContext);  

  DLOG(g_tmpContent.pszContent);

  *ppContent = &g_tmpContent;
  return TRUE;
}
//-----------------------------------------------------------------------------

BOOL PI_IndicateAction (HPILOADTEST hLT, ULONG uId, LPCSTR pszLink)
{
  char sId[33];
  ultoa (uId, sId, 10);
  g_strLastErrorMessage.clear();
	
  DLOG("-> PI_IndicateAction");
  ContentSet    (&g_tmpContent, "text", "-> PI_IndicateAction");
  ContentAppend (&g_tmpContent, " uid: ");
  ContentAppend (&g_tmpContent, sId);  
  ContentAppend (&g_tmpContent, " link: ");
  ContentAppend (&g_tmpContent, pszLink);  

  String strContent;
  if (pszLink) {
	  strContent=pszLink;
  } // if
  else {
	  Log(eSeverityError, "Link contained no data",0);
	  TextLog("Link contained no data");
	  return FALSE;
  } // else if

  DLOG(g_tmpContent.pszContent);
  TextLog(g_tmpContent.pszContent);

  if (pszLink && stricmp (pszLink, "Beep")==0)
  {
    Beep(440, 500);
    return TRUE;
  }
 
  if (strContent.startsWith("apipath=")) {
	  strContent=strContent.substring(8);
	  performRESTRequest(OPEN_API, strContent, String());
	  return TRUE;
  } // if

  if (strContent.startsWith("purepathid=")) {
	  strContent=strContent.substring(11);
	  performRESTRequest(OPEN_PATH, strContent, String());
	  return TRUE;
  } // if

  return FALSE;
}
//-----------------------------------------------------------------------------

void PI_SetFNSelectNode (TFNSelectNode fnSelectNode, void* pContext)
{
  DLOG("-> PI_SetFNSelectNode");

  g_FNSelectNode        = fnSelectNode;
  g_pSelectNodeContext  = pContext;
}
//-----------------------------------------------------------------------------


LPCSTR PI_GetSessionId()  {
	if (g_strLastSessionID.length()>0)
		return g_strLastSessionID.getBytes();
	else
		return CONST_GENERIC_SESSIONID;
}


LPCSTR PI_GetPluginInfo(ULONG uModule, DWORD eType) {
	DLOG("-> PI_SetFNInfo");
	g_strLastErrorMessage.clear();
	return NULL;
}

void PI_SetFnInfo (TFNInfo fnInfo, void* pContext) {
	PI_SetFNInfo(fnInfo,pContext);
}

void PI_SetFNInfo (TFNInfo fnInfo, void * pContext) { // register callback for eg. Reg key, ...)
	unsigned long ulInfoSize=0;
	char * chBuf=NULL;

	TextLog("--->> PI_SetFNInfo");
	//DLOG("-> PI_GetPluginInfo");
	g_FNInfo=fnInfo;
	g_pGetInfoContext=pContext;

	// read registry key
	GetInfo(NULL,ePIPInfo_RegKey_Plugin,(char *) NULL,ulInfoSize);
	if (ulInfoSize) {
		ulInfoSize+=1;
		chBuf=new char[ulInfoSize];
		GetInfo(NULL,ePIPInfo_RegKey_Plugin,chBuf,ulInfoSize);
		pnapi::String str;
		chBuf[ulInfoSize-1]='\0';
		str.sprintf("%s",chBuf);
		delete chBuf;
		g_strPluginRegKey=str;
		//g_strPluginRegKey.replace(String("SOFTWARE\\"),String("Software\\"));
	} // if
	else {
		Log(eSeverityWarning,"Query of Reg-Key failed",0);
		g_strPluginRegKey.clear();
	} // elseif

}

String renderHelp() {
	String retVal;
	retVal="<br />\n"
		"<br />\n"
		"<p>\n"
        "<div class=\"help z0\">\n"
        "<div class=\"helphdr z1\">Info</div>\n"
        "<div class=\"helpcontent z0\">\n";

	// Please typt the help-text in here
    retVal+="Make sure to add the WebSetDiagnostics() command to your Web/HTTP BDL script. "
		"A good location is the TInit Transaction as the following example demonstrates:\n"
		"<div class=\"code_border\" >\n<code>"
		"&nbsp;&nbsp;&nbsp;transaction TInit<br/>\n"
	    "&nbsp;&nbsp;&nbsp;begin<br/>\n"
		"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;WebSetDiagnostics(WEB_DIAG_dynaTraceDiagnostics);<br/>\n"
		"&nbsp;&nbsp;&nbsp;end TInit;<br/>"
		"</code>\n</div>\n"
		;

    retVal+="</div>\n"
        "</div>\n"
        "</p>\n";
	return retVal;
}

String renderNoData() {
	String retVal;
	retVal="<br />\n"
		"<br />\n"
		"<p>\n"
        "<div class=\"help z0\">\n"
        "<div class=\"helphdr z1\">Info</div>\n"
        "<div class=\"helpcontent z0\">\n";

	// Please typt the help-text in here
    retVal+="<strong>This page contains no data for displaying the Breakdown in dynaTrace</strong>\n"
		/*"<br><br>There is no content that can be matched with dynaTrace diagnostics server Data.\n"
		"\n" */
		;

    retVal+="</div>\n"
        "</div>\n"
        "</p>\n";
	return retVal;
}

/**
 * Rendering functions for pages
 */
TContent *renderSimpleContentPage(String strDTHeader, String strApi, LPCTSTR lpszUrlAbs, LPCTSTR lpszPageName, page sSubPageInfo) {
	ContentSet(&g_tmpDocContent,"html","<html>\n"
	  "  <head>\n"
	  "    <title>title</title>\n"
	  "    <link href=\"file.html?StyleSheet.css\" type=\"text/css\" rel=\"stylesheet\" />\n"
	  "  </head>\n"
	  "  <body>\n");
  
     ContentAppend   (&g_tmpDocContent, "<table cellpadding=\"0\" cellspacing=\"0\">\n    <tr>\n" 
     "     <td width=\"722\" >\n"
	 "        <img src=\"file.html?banner_left.jpg\" style=\"border-width:0px;\" />\n"
	 "     </td>\n"
	 "    </tr>\n"
	 "  </table>\n");
	 
	 if (strDTHeader.length()>0) {
		ContentAppend(&g_tmpDocContent, 
			"\n"
			"<div class=\"the_content\">\n");
	  
		ContentAppend(&g_tmpDocContent, 
			"<p>\n<div class=\"bluetext\"><strong>Diagnose the PurePath of this Web Page Request</strong></div>\n"
			"<table cellspacing=\"0\" cellpadding=\"0\" class=\"table_content\" >\n"
			"  <tr>\n"
			"    <td class=\"border_table_content_head\" nowrap><span style=\"vertical-align: middle\">Drill-down</span></td>\n"
			"    <td class=\"border_table_content_head\" width=\"100%\" nowrap><span style=\"vertical-align: middle\">Pure Path</span></td>\n"
			"  </tr>\n");
	    renderSubPage(&g_tmpDocContent, sSubPageInfo, FALSE);
		ContentAppend(&g_tmpDocContent, "</table></p>\n"
			"</div>\n");
  } // if
  else {
	  if ((strApi.length()<1) || strApi.equals(NO_API_SEARCHPATH_SET)) {

		ContentAppend(&g_tmpDocContent,
			renderNoData().getBytes()
			);
	  } // if
	  else {
		ContentAppend(&g_tmpDocContent,
			renderNoData().getBytes()
			);
	  } // elseif
  } // elseif

  ContentAppend(&g_tmpDocContent,
	  "  </body>\n"
	  "</html>\n"
	  );

  return &g_tmpDocContent;
}

TContent *renderComplexPage(page pg) {
  rowCount=0;
  ContentSet      (&g_tmpPageContent, "html", "<html><head><meta http-equiv=Content-Type content=\"text/html; charset=windows-1252\"><title></title><link href=\"file.html?StyleSheet.css\" type=\"text/css\" rel=\"stylesheet\" /></head><body>");
 
   ContentAppend   (&g_tmpPageContent, "<table cellpadding=\"0\" cellspacing=\"0\">\n    <tr>\n" 
     "     <td width=\"722\" >\n"
	 "        <img src=\"file.html?banner_left.jpg\" style=\"border-width:0px;\" />\n"
	 "     </td>\n"
	 "    </tr>\n"
	 "  </table>\n");

   if (pg.dtHeader.length()>0) {
		ContentAppend   (&g_tmpPageContent, "<div class=\"the_content\">\n");
		ContentAppend   (&g_tmpPageContent, 
			"<p>\n<div class=\"bluetext\"><strong>Diagnose the PurePath of each Web Page Request</strong></div>\n"
			"<table cellspacing=\"0\" cellpadding=\"0\" class=\"table_content\" >\n"
			"  <tr>\n"
			"    <td class=\"border_table_content_head\" nowrap><span style=\"vertical-align: middle\">Drill-down</span></td>\n"
			"    <td class=\"border_table_content_head\" width=\"100%\" ><span style=\"vertical-align: middle\">Pure Path</span></td>\n"
			"  </tr>\n"
			);
		// BEGIN PAGE CONTENT RENDERING

		int iItems=pg.subpages.size();
		for (int i=0;i<iItems;i++) {
			renderSubPage(&g_tmpPageContent,pg.subpages[i],TRUE);
		} // for

		// END PAGE CONTENT RENDERING
		ContentAppend   (&g_tmpPageContent, "</table></p>\n"
			"</div>\n");
  } // if
  else {
	  if ((pg.dtApiBDString.length()<1) || pg.dtApiBDString.equals(NO_API_SEARCHPATH_SET)) {

		  ContentAppend   (&g_tmpPageContent, renderNoData().getBytes());
	  } // if
	  else {
		  ContentAppend   (&g_tmpPageContent, renderNoData().getBytes());
	  } // elseif
  } // elseif
  
  ContentAppend   (&g_tmpPageContent, "</body>\n</html>\n");

  return &g_tmpPageContent;
}

void renderSubPage(TContent *pageContent, page pg, BOOL linkPage) {
    char sId[33];
	ultoa (pg.uid, sId, 10);

	if (pg.dtHeader.length()>0) {
		ContentAppend   (pageContent, "<tr>\n");
		ContentAppend   (pageContent, "<td class=\"border_table_content_icon\">");
		ContentAppend   (pageContent, "<a href=\"Action.html?purepathid=");  
		ContentAppend   (pageContent, pg.dtHeader.getBytes());
		ContentAppend   (pageContent, "\"><img src=\"file.html?path.gif\" align=\"middle\" border=\"0\" /></a>"
							"</td>");
		ContentAppend   (pageContent, "<td class=\"border_table_content\"><span style=\"vertical-align: middle\">");
		ContentAppend   (pageContent, "<a href=\"Action.html?purepathid=");  
		ContentAppend   (pageContent, pg.dtHeader.getBytes());
		ContentAppend   (pageContent, "\">");
		ContentAppend   (pageContent, pg.dtUrlAbs.getBytes());
		ContentAppend   (pageContent, "</a>");
		ContentAppend   (pageContent, "</span></td>");
		ContentAppend   (pageContent, "</tr>\n");
  } // if

  int iItems=pg.subpages.size();
  for (int i=0;i<iItems;i++) {
	  renderSubPage(pageContent,pg.subpages[i],TRUE);
  } // for
}
