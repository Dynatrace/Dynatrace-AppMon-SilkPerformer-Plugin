#ifndef RESTCLIENT__H__INCLUDED
/********************************************************************************
* dynaTrace Diagnostics (c) dynaTrace software GmbH
*
* Filename: RESTClient.h
* Created:  2010/03/20
*******************************************************************************/
#define RESTCLIENT__H__INCLUDED


#include <windows.h>
#include <wininet.h>

#include "pnapi/String.h"

#include <vector>

#define GET_CONN_HANDLE(mInternetHandle, server, clientPort) InternetConnect(mInternetHandle, server, clientPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
#define GET_REQ_HANDLE(connHandle, method, url) HttpOpenRequest(connHandle, method, url.getBytes(), NULL, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE, NULL)

//used for return value of isRESTServerReachable()
enum ClientVersion{
	STATE_NOT_REACHABLE,	//server not reachable, wrong port or not started
	STATE_UNSUPPORTED_VERSION,	//version pre 3.2.1
	STATE_321_35,	//version is >= 3.2.1 and < 4
	STATE_4PLUS,		//version >= 4
};


using namespace pnapi;

/**
 * A REST client. This class performs REST requests to query REST interface of dynaTrace client.
 * The get the port used for queries getClientPort() (defined in dynaTrace.h) is used.
 * Note: The REST server is the dynaTrace client
 */
class RESTClient
{
public:
	RESTClient();
	~RESTClient(void);

	/**
	 * Checks if the rest server (= dynaTrace client) is available
	 * @return status of the rest server
	 */
	ClientVersion getDtClientVersion();

	/**
	 * gets the name of the server configured in dynaTrace client.
	 * If more than one servers are configured the first server found is used
	 * @return name of the configured server if successful, empty string otherwise
	 */
	String getConfiguredServer();

	/**
	 * starts session recording
	 * @param systemProfile system profile to be recorded
	 * @param name name for recorded session
	 * @return id of the started session if successful, empty string otherwise
	 */
	String startSessionRecording(String systemProfile, String sessionName);

	/**
	 * stops session recording
	 * @param systemProfile system profile to be stopped
	 * @return TRUE if successful, FALSE otherwise
	 */
	BOOL stopSessionRecording(String systemProfile);
	
	/**
	 * gets all system profiles
	 * @return a vector containing all system profiles
	 */
	std::vector<String> getSystemProfiles();

	/**
	 * opens a pure path
	 * @param pure path id to be opende
	 * @return TRUE if successful, FALSE otherwise
	 */
	BOOL openPurePath(String purePathId);

	/**
	 * opens an API breakdown
	 * @param sessionId session id to be opened
	 * @param timerName name of timer on which filter is set
	 * @return TRUE if successful, FALSE otherwise
	 */
	BOOL openAPIBreakDown(String sessionId, String timerName);

	/**
	 * gets the session id of a session which is currently recored
	 * @param systemProfil 
	 * @return session id if successful, empty string otherwise
	 */
	String getSessionId(String systemProfil);

private:
	void init(); 
	BOOL checkConfiguredServerName(); 
	void closeHandles(HINTERNET connHandle, HINTERNET reqHandle);

	void logLastErrorMessage(enum EPI_SeverityTag logLevel, String messagePrefix);
	long getStatusCode(HINTERNET reqHandle);
	String readResult(HINTERNET reqHandle);

	/**
	 * returns the system profile on which recorind will be started if no system profile is specified
	 * @return session id if successful, empty string otherwise
	 */
	String getDefaultSystemProfile();

	//members
	HINTERNET mInternetHandle;
	String mConfiguredServerName;
};



#endif //RESTCLIENT__H__INCLUDED