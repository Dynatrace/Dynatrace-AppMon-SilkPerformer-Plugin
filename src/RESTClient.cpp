/********************************************************************************
* dynaTrace Diagnostics (c) dynaTrace software GmbH
*
* Filename: RESTClient.cpp
* Created:  2010/03/20
*******************************************************************************/


#include "RESTClient.h"

#include "piWorkbenchPluginInterface.h"
#include "dynaTrace.h"

#include <tchar.h>

//used to create rest urls and parameters
#define REST_SERVER "localhost"
#define AGENT_NAME "SilkPerfomerDiagnostics"
#define GET_REQUEST "GET"
#define POST_REQUEST "POST"
#define URL_GET_SERVERS "rest/management/servers"
#define URL_PROFILES "rest/management/profiles/"
#define URL_OPEN_PATH "rest/integration/openpurepath?purepathid="
#define URL_OPEN_API_BREAKDOWN "rest/integration/openapibreakdown"
#define URL_GET_SESSION_ID "rest/management/recordingsessions/"
#define URL_GET_VERSION "rest/management/version"
#define POSTPARAM_SERVER "server="
#define POSTPARAM_SESSIONNAME "sessionname="
#define POSTPARAM_APPEND_TIMESTAMP "appendtimestamp=true"
#define POSTPARAM_SESSION_ID "sessionid="
#define POSTPARAM_TIMERNAME "timername="
#define START_RECORDING "startrecording"
#define STOP_RECORDING "stoprecording"
#define GET_PARAM_SYS_PROF "systemprofile="

#define DYNATRACE_SELF_MONITORING_PROFILE "dynaTrace Self-Monitoring"

//used for xml "parsing"
#define SERVER_START_TAG " id=\""
#define SERVER_END_TAG "\""
#define SYSTEMPROFIL_START_TAG "<systemprofile id=\""
#define SYSTEMPROFIL_END_TAG "\"/>"
#define SESSIONID_START_TAG "<result value=\""	//used to get session id in startSessionRecording()
#define SESSIONID_END_TAG "\"/>"
#define GET_SESS_ID_START_TAG "<sessionid>"		//used to get session id in getSessionId()
#define GET_SESS_ID_END_TAG "</sessionid>"
#define VERSION_START_TAG "<result value=\""
#define VERSION_END_TAG "\"/>"

#define PROFILES_START "<profiles>"
#define PROFILES_END "</profiles>"
#define SYSTEMPROFILE_START "<systemprofile"
#define ELEMENT_END "/>"
#define ATTRIBUT_END "\""
#define ID_START "id=\""
#define AGENT_CNT_START "connectedagents=\""

#define STOP_CONINUOUS_SESSION_RECORIDNG "Please disable continuous transaction storage in dynaTrace server."


RESTClient::RESTClient(void): mInternetHandle(NULL), mConfiguredServerName("")
{
	init();
}


RESTClient::~RESTClient(void)
{
	if(this->mInternetHandle){
		InternetCloseHandle(mInternetHandle);
		mInternetHandle = NULL;
	}
}

void RESTClient::init(){
	mInternetHandle = InternetOpen(AGENT_NAME,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
	if(mInternetHandle) {
		Log(eSeverityInfo,"RESTClient initialized",0);
	}
	else {
		Log(eSeverityError,"Could not initialize REST client! REST will not work",0);
	}
}

long RESTClient::getStatusCode(HINTERNET reqHandle){
		TCHAR szStatusCode[32];
		DWORD dwInfoSize = 32;
		BOOL b = HttpQueryInfo(reqHandle, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwInfoSize, NULL);
		return  _ttol(szStatusCode);
}

String RESTClient::readResult(HINTERNET reqHandle){
	char buf[512];
	DWORD numberOfBytes = 0;
	DWORD bufsize;
	bufsize = sizeof buf;
	String result;

	while (InternetReadFile(reqHandle,buf,bufsize-1,&numberOfBytes)){
		if(numberOfBytes == 0)
			break;
		buf[numberOfBytes] = '\0';
		result += buf;
	};
	return result;
}

ClientVersion RESTClient::getDtClientVersion(){

	
	ClientVersion state = STATE_NOT_REACHABLE;

	HINTERNET connHandle = InternetConnect(mInternetHandle,REST_SERVER,getClientPort(),NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	
	HINTERNET reqHandle = HttpOpenRequest(connHandle,GET_REQUEST,URL_GET_VERSION,NULL,NULL,NULL,INTERNET_FLAG_NO_CACHE_WRITE,NULL);
	
	
	if(!HttpSendRequest(reqHandle,NULL,0,NULL,0)){
		Log(eSeverityInfo,"Could not get client version. REST request failed",0);
		return state;
	}

	int statusCode = getStatusCode(reqHandle);
	if (statusCode == HTTP_STATUS_OK){
		String result = readResult(reqHandle);
		int start = result.indexOf(VERSION_START_TAG);
		int end = result.indexOf(VERSION_END_TAG);

		if(start > 0 && end  > start){
			String version = result.substring(start + sizeof(VERSION_START_TAG)-1, end);
			
			if (version.startsWith("3.2.1") || version.startsWith("3.5"))	//3.2.1 or 3.5
				return STATE_321_35;

			if (version.compareTo("4") >= 0)	//This works due to implementation of used compareTo(): 4.0 > 4; 5.0 > 4
				return STATE_4PLUS;

			//INVALID
			String log = "No supported client version found: " + version;
			Log(eSeverityInfo,log.getBytes(), statusCode);
			return STATE_UNSUPPORTED_VERSION;
		}
		else {
			Log(eSeverityInfo,"No supported client version found", statusCode);
			state = STATE_UNSUPPORTED_VERSION;
		}
	} else {
		Log(eSeverityInfo,"No supported client version found", statusCode);
		state = STATE_UNSUPPORTED_VERSION;
	}

	closeHandles(connHandle, reqHandle);
	return state;
}

String RESTClient::getConfiguredServer(){
	String ret;

	HINTERNET connHandle = InternetConnect(mInternetHandle,REST_SERVER,getClientPort(),NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	HINTERNET reqHandle = HttpOpenRequest(connHandle,GET_REQUEST,URL_GET_SERVERS,NULL,NULL,NULL,INTERNET_FLAG_NO_CACHE_WRITE,NULL);
	
	BOOL sendResult = HttpSendRequest(reqHandle,NULL,0,NULL,0);

	if(!sendResult){
		Log(eSeverityError,"Could not get configured server, REST request failed.",0);
		return ret;
	}

	if((getStatusCode(reqHandle) == HTTP_STATUS_OK) ){
		String result = readResult(reqHandle);
		int start = result.indexOf(SERVER_START_TAG);
		int end = result.indexOf(SERVER_END_TAG, start  + sizeof(SERVER_START_TAG));

		if(start > 0 && end  > start){
			ret = result.substring(start + sizeof(SERVER_START_TAG)-1, end);
			//check if more than one server is configured in client
			result = result.substring (end + sizeof(SERVER_END_TAG)-1);
			if (result.indexOf(SERVER_START_TAG) >= 0){
				Log(eSeverityWarning,"More than one server configured in client. First server is used.",0);
			}
		}
		else {
			Log(eSeverityError,"Client did not respond a configured server.",getStatusCode(reqHandle));
			ret = "";
		}
	} 
	else {
		Log(eSeverityError,"Client did not respond a configured server.",getStatusCode(reqHandle));
		ret = "";
	}

	closeHandles(connHandle, reqHandle);
	return ret;
}

String RESTClient::startSessionRecording(String systemProfile, String sessionName){

	String ret;
	if(!checkConfiguredServerName())
		return ret;

	//check system profile
	if(systemProfile.equals("")){
		systemProfile = getDefaultSystemProfile();
		if(systemProfile.equals("")){
			Log(eSeverityError,"No system profile specified and no default system profile found.",0);
			return ret;
		} else {
			String s = "No system profile specified. Using default system profile " + systemProfile;
			Log(eSeverityWarning,s.getBytes(),0);
		}
	}

	HINTERNET connHandle = InternetConnect(mInternetHandle,REST_SERVER,getClientPort(),NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	String url = URL_PROFILES + systemProfile + "/" + START_RECORDING;
	String postParam = POSTPARAM_SERVER + mConfiguredServerName;
	postParam += "&";
	postParam += POSTPARAM_SESSIONNAME;
	postParam += sessionName;
	postParam += "&";
	postParam += POSTPARAM_APPEND_TIMESTAMP;
	
	HINTERNET reqHandle = HttpOpenRequest(connHandle,POST_REQUEST,url.getBytes(),NULL,NULL,NULL,INTERNET_FLAG_NO_CACHE_WRITE,NULL);
	
	BOOL sendResult = HttpSendRequest(reqHandle,NULL,0,(LPVOID)postParam.getBytes(),postParam.length());
	if(!sendResult){
		Log(eSeverityWarning,"Could not start session recording, REST request failed.",0);
		return ret;
	}

	int statusCode = getStatusCode(reqHandle);
	if(statusCode == HTTP_STATUS_OK){
		String result = readResult(reqHandle);
		int start = result.indexOf(SESSIONID_START_TAG);
		int end = result.indexOf(SESSIONID_END_TAG);

		if(start > 0 && end  > start){
			ret = result.substring(start + sizeof(SESSIONID_START_TAG)-1, end);
			String log = "Session recording started. Session id: " + ret;
			Log(eSeverityInfo,log.getBytes(),0);
		}
		else {
			Log(eSeverityWarning,"Client did not respond a session id. Session recording not started.",statusCode);
			ret = "";
		}
	}
	else {
		String s = "Could not start session recording for server \"";
		s += mConfiguredServerName;
		s += "\" and system profile \"";
		s += systemProfile;
		s += "\". ";
		if (statusCode == HTTP_STATUS_FORBIDDEN){ 
			s += STOP_CONINUOUS_SESSION_RECORIDNG;
		}
		Log(eSeverityWarning, s.getBytes(),statusCode);
	}

	closeHandles(connHandle, reqHandle);
	return ret;
}


BOOL RESTClient::stopSessionRecording(String systemProfile){
	
	BOOL ret = FALSE;
	if(!checkConfiguredServerName())
		return ret;

	if(systemProfile.equals("")){
		systemProfile = getDefaultSystemProfile();
		if(systemProfile.equals("")) return FALSE;
	}

	HINTERNET connHandle = InternetConnect(mInternetHandle,REST_SERVER,getClientPort(),NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	String url = URL_PROFILES + systemProfile + "/" + STOP_RECORDING;
	String postParam = POSTPARAM_SERVER + mConfiguredServerName;
	HINTERNET reqHandle = HttpOpenRequest(connHandle,POST_REQUEST,url.getBytes(),NULL,NULL,NULL,INTERNET_FLAG_NO_CACHE_WRITE,NULL);
	
	Log(eSeverityInfo,"Stopping session recording...",0);
	BOOL sendResult = HttpSendRequest(reqHandle,NULL,0,(LPVOID)postParam.getBytes(),postParam.length());
	if(!sendResult){
		Log(eSeverityInfo,"Session recording not yet stopped. Please check in dynaTrace client if session recording was stopped.",0);
		DWORD error = GetLastError();
		return ret;
	}

	int statusCode = getStatusCode(reqHandle);
	if(sendResult && (statusCode == HTTP_STATUS_OK) ){
		String result = readResult(reqHandle);
		//TODO: process result (REST interface has to be extended)
		Log(eSeverityInfo,"Session recording stopped",0);
		ret = TRUE;
	}
	else {
		String s = "Could not stop session recording. ";
		if (statusCode == HTTP_STATUS_FORBIDDEN) {
			s += STOP_CONINUOUS_SESSION_RECORIDNG;
		}
		Log(eSeverityWarning,s.getBytes(),statusCode);
	}
	closeHandles(connHandle, reqHandle);
	return ret;
}

std::vector<String> RESTClient::getSystemProfiles(){

	std::vector<String> profiles;
	if(!checkConfiguredServerName())
		return profiles;

	HINTERNET connHandle = InternetConnect(mInternetHandle,REST_SERVER,getClientPort(),NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	String url = URL_PROFILES + mConfiguredServerName;
	HINTERNET reqHandle = HttpOpenRequest(connHandle,GET_REQUEST,url.getBytes(),NULL,NULL,NULL,INTERNET_FLAG_NO_CACHE_WRITE,NULL);
	
	BOOL sendResult = HttpSendRequest(reqHandle,NULL,0,NULL,0);
	if(!sendResult){
		Log(eSeverityWarning,"Could not get system profiles, REST request failed!",0);
		return profiles;
	}

	int statusCode = getStatusCode(reqHandle);
	if(statusCode == HTTP_STATUS_OK){
		
		String result = readResult(reqHandle);
		int start = result.indexOf(SYSTEMPROFIL_START_TAG);
		int end = result.indexOf(SYSTEMPROFIL_END_TAG);
		while(start >= 0 && end >= start) {
			end = result.indexOf(SYSTEMPROFIL_END_TAG);
			String profile = result.substring(start + sizeof(SYSTEMPROFIL_START_TAG)-1, end);
			if(!profile.equals(DYNATRACE_SELF_MONITORING_PROFILE)) 
				profiles.push_back(profile);
			//check for next result
			result = result.substring (end + sizeof(SYSTEMPROFIL_END_TAG)-1);
			start = result.indexOf(SYSTEMPROFIL_START_TAG);
		}
	}
	else {
		Log(eSeverityWarning,"Could not get system profiles!",statusCode);
	}

	closeHandles(connHandle, reqHandle);
	return profiles;
}

BOOL RESTClient::openPurePath(String purePathId){

	BOOL ret = FALSE;

	if(!checkConfiguredServerName())
		return ret;

	HINTERNET connHandle = InternetConnect(mInternetHandle,REST_SERVER,getClientPort(),NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	String url = URL_OPEN_PATH + purePathId;
	HINTERNET reqHandle = HttpOpenRequest(connHandle,GET_REQUEST,url.getBytes(),NULL,NULL,NULL,INTERNET_FLAG_NO_CACHE_WRITE,NULL);
	
	BOOL sendResult = HttpSendRequest(reqHandle,NULL,0,NULL,0);
	if(!sendResult){
		Log(eSeverityWarning,"Could not open pure path, REST request failed!",0);
		return ret;
	}

	int statusCode = getStatusCode(reqHandle);
	if(statusCode == HTTP_STATUS_OK){
		ret = TRUE;
	}
	else { 
		Log(eSeverityWarning,"Could not open purePath!",statusCode);
	}
	closeHandles(connHandle, reqHandle);

	return ret;
}

BOOL RESTClient::openAPIBreakDown(String sessionId, String timerName){
	BOOL ret = FALSE;

	if(!checkConfiguredServerName())
		return ret;

	HINTERNET connHandle = InternetConnect(mInternetHandle,REST_SERVER,getClientPort(),NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	String url = URL_OPEN_API_BREAKDOWN;
	
	String postParam;
	postParam += POSTPARAM_SERVER + mConfiguredServerName;
	postParam += "&";
	postParam += POSTPARAM_SESSION_ID;
	postParam += sessionId;
	if(timerName.length() > 0){
		postParam += "&";
		postParam += POSTPARAM_TIMERNAME;
		postParam += timerName;
	}
	HINTERNET reqHandle = HttpOpenRequest(connHandle,POST_REQUEST,url.getBytes(),NULL,NULL,NULL,INTERNET_FLAG_NO_CACHE_WRITE,NULL);
	
	BOOL sendResult = HttpSendRequest(reqHandle,NULL,0,(LPVOID)postParam.getBytes(),postParam.length());
	if(!sendResult){
		Log(eSeverityWarning,"Could not perform API breakdown, REST request failed!",0);
		return ret;
	}

	int statusCode = getStatusCode(reqHandle);
	if(statusCode == HTTP_STATUS_OK){
		ret = TRUE;
	}
	else { 
		Log(eSeverityWarning,"Could not perform API breakdown!",statusCode);
	}
	closeHandles(connHandle, reqHandle);

	return ret;

}

String RESTClient::getSessionId(String systemProfile){
	String ret;
	if(!checkConfiguredServerName())
		return ret;

	//check system profile
	if(systemProfile.equals("")){
		systemProfile = getDefaultSystemProfile();
		if(systemProfile.equals("")){
			Log(eSeverityError,"No system profile specified and no default system profile found.",0);
			return ret;
		} else {
			String s = "No system profile specified. Using default system profile " + systemProfile;
			Log(eSeverityWarning,s.getBytes(),0);
		}
	}

	HINTERNET connHandle = InternetConnect(mInternetHandle,REST_SERVER,getClientPort(),NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	String url = URL_GET_SESSION_ID;
	url += mConfiguredServerName;
	url += "?";
	url += GET_PARAM_SYS_PROF;
	url += systemProfile;

	HINTERNET reqHandle = HttpOpenRequest(connHandle,GET_REQUEST,url.getBytes(),NULL,NULL,NULL,INTERNET_FLAG_NO_CACHE_WRITE,NULL);
	
	BOOL sendResult = HttpSendRequest(reqHandle,NULL,0,NULL,0);

	if(!sendResult){
		Log(eSeverityError,"Could not get session id, REST request failed.",0);
		return ret;
	}

	int statusCode = getStatusCode(reqHandle);
	if(statusCode == HTTP_STATUS_OK){
		String result = readResult(reqHandle);
		int start = result.indexOf(GET_SESS_ID_START_TAG);
		int end = result.indexOf(GET_SESS_ID_END_TAG);

		if(start > 0 && end > start){
			ret = result.substring(start + sizeof(GET_SESS_ID_START_TAG)-1, end);
			String log = "Got sessionId: ";
			log += ret;
			Log(eSeverityInfo, log.getBytes(),0);
			//check if more than one server is configured in client
			result = result.substring (end + sizeof(GET_SESS_ID_START_TAG)-1);
			if (result.indexOf(GET_SESS_ID_START_TAG) >= 0){
				Log(eSeverityWarning,"More than one sessions are recorded. First session id is used.",0);
			}
		}
		else {
			Log(eSeverityWarning,"Client did not respond a session id.", statusCode);
			ret = "";
		}
	} 
	else {
		Log(eSeverityWarning,"Client did not respond a session id.",statusCode);
		ret = "";
	}

	closeHandles(connHandle, reqHandle);
	return ret;
}

BOOL RESTClient::checkConfiguredServerName(){
	if(mConfiguredServerName.equals("")){
		mConfiguredServerName = getConfiguredServer();
	}
	return !mConfiguredServerName.equals("");
}


void RESTClient::closeHandles(HINTERNET connHandle, HINTERNET reqHandle){
	if(reqHandle){
		InternetCloseHandle(reqHandle);
		reqHandle = NULL;
	}
	if(connHandle){
		InternetCloseHandle(connHandle);
		connHandle = NULL;
	}
}


String RESTClient::getDefaultSystemProfile(){

	String profile;
	if(!checkConfiguredServerName())
		return profile;

	HINTERNET connHandle = InternetConnect(mInternetHandle,REST_SERVER,getClientPort(),NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	String url = URL_PROFILES + mConfiguredServerName;
	HINTERNET reqHandle = HttpOpenRequest(connHandle,GET_REQUEST,url.getBytes(),NULL,NULL,NULL,INTERNET_FLAG_NO_CACHE_WRITE,NULL);
	
	BOOL sendResult = HttpSendRequest(reqHandle,NULL,0,NULL,0);
	if(!sendResult){
		Log(eSeverityWarning,"Could not get system profiles, REST request failed!",0);
		return profile;
	}

	int statusCode = getStatusCode(reqHandle);
	if(statusCode == HTTP_STATUS_OK){

		String result = readResult(reqHandle);

		if(result.indexOf(AGENT_CNT_START) < 0){
			Log(eSeverityInfo,"Client version does not provide number of connected agents",0);
			return profile;
		}


		int start = result.indexOf(SYSTEMPROFILE_START) +  sizeof(SYSTEMPROFILE_START)-1;
		int end =  result.indexOf(ELEMENT_END, start);

		while(start < end ) {
			String currProfile = result.substring(start, end);
			int idStart = currProfile.indexOf(ID_START) + sizeof(ID_START)-1;
			int idEnd = currProfile.indexOf(ATTRIBUT_END, idStart+1);
			if(idStart < 0 || idEnd < 0){
				Log(eSeverityWarning,"Invalid XML!",statusCode);
				return profile;
			}
			String currId = currProfile.substring(idStart, idEnd);
				

			int agentCntStart = currProfile.indexOf(AGENT_CNT_START) + sizeof(AGENT_CNT_START)-1;
			int agentCntEnd = currProfile.indexOf(ATTRIBUT_END, agentCntStart+1);

			if(agentCntStart < agentCntEnd){
				String agentCnt = currProfile.substring(agentCntStart , agentCntEnd);
				int agents = atoi(agentCnt.getBytes());
				if(agents > 0){	//first connected found, return
					return currId;
				}
			}
				
			//next result
			result = result.substring(end + sizeof(ELEMENT_END)-1);
			start = result.indexOf(SYSTEMPROFILE_START);
			end =  result.indexOf(ELEMENT_END);			
		}
	}
	else {
		Log(eSeverityWarning,"Could not get default profile!",statusCode);
	}

	closeHandles(connHandle, reqHandle);
	return profile;
}