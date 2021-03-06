#include<winsock2.h>
#include<stdio.h>
#include"DataInterface.h"
#include"fileMgr.h"


dataInterfaceWorkerThread_t workerThread[WORKER_THREADS];

struct  
{
	char *dbHost;
	char *dbUser;
	char *dbName;
	char *dbPass;
	int	 dbPort;
}dbConInfo;

struct  
{
	MYSQL *mysql_as;
	char *dbHost;
	char *dbUser;
	char *dbName;
	char *dbPass;
	int	 dbPort;
	char *ServerIP;
	int ServerIPHex;
	int ServerPort;
}AuthInfo;



MYSQL* _dataInterface_gs_connect()
{
	MYSQL *     dbHandle;
	dbHandle = mysql_init(0);
	MYSQL *dbHandleErr = dbHandle;
	dbHandle = mysql_real_connect(dbHandle, dbConInfo.dbHost,
								  dbConInfo.dbUser, dbConInfo.dbPass,  
								  dbConInfo.dbName, dbConInfo.dbPort, 0, 0);     
	if( dbHandle == 0 )
	{
		printf("GS Mysql: ");
		puts(mysql_error(dbHandleErr));
		Sleep(1000*60);
		return 0;
	}
	// set charset utf8
	mysql_set_character_set(dbHandle, "utf8");
	return dbHandle;
}

MYSQL* _dataInterface_as_connect()
{
	MYSQL *     dbHandle;
	dbHandle = mysql_init(0);
	MYSQL *dbHandleErr = dbHandle;
	dbHandle = mysql_real_connect(dbHandle, AuthInfo.dbHost,
								  AuthInfo.dbUser, AuthInfo.dbPass,  
								  AuthInfo.dbName, AuthInfo.dbPort, 0, 0);     
	if( dbHandle == 0 )
	{
		printf("AS Mysql: ");
		puts(mysql_error(dbHandleErr));
		Sleep(1000*60);
		return 0;
	}
	// set charset utf8
	mysql_set_character_set(dbHandle, "utf8");
	return dbHandle;
}

void dataInterface_queueJob(void *jobData, void *workCallback, void *doneCallback, void *param)
{
	dataInterfaceWorkerThread_t *wt = workerThread+0;
	// diJob_getCharacterPreviewInfo_t *job, void *cb, void *param
	int nwi = (wt->queueWriteIndex+1)%WORKER_QUEUELENGTH;
	if( wt->queueReadIndex == nwi )
		while( wt->queueReadIndex != nwi ) Sleep(10);
	// append job
	wt->jobQueue[wt->queueWriteIndex].jobData = jobData;
	wt->jobQueue[wt->queueWriteIndex].param = param;
	wt->jobQueue[wt->queueWriteIndex].workCallback = (void (__cdecl *)(MYSQL *,void *,void *,void *))workCallback;
	wt->jobQueue[wt->queueWriteIndex].doneCallBack = doneCallback;
	wt->queueWriteIndex = nwi;
}

int _dataInterface_work(dataInterfaceWorkerThread_t *wt)
{
	while( 1 )
	{
		while( wt->queueReadIndex == wt->queueWriteIndex )
			Sleep(1);
		// process job
		wt->jobQueue[wt->queueReadIndex].workCallback(wt->dbCon, wt->jobQueue[wt->queueReadIndex].jobData, wt->jobQueue[wt->queueReadIndex].doneCallBack, wt->jobQueue[wt->queueReadIndex].param);
		wt->queueReadIndex = (wt->queueReadIndex+1)%WORKER_QUEUELENGTH;
	}

	return 0;
}

void _dataInterface_initWorkerThread(int index)
{
	dataInterfaceWorkerThread_t *wt = workerThread+index;
	wt->dbCon = _dataInterface_gs_connect();
	wt->queueReadIndex = 0;
	wt->queueWriteIndex = 0;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_dataInterface_work, wt, 0, NULL);
}



void dataInterface_init()
{
	// read connect config details
	sData_t *config = sData_open("config.txt");
	sData_nextCategory(config);

	// host
	char *option = sData_findOption(config, "DBHOST");
	if( option == NULL )
	{ printf("[Game Server Database] dbHost option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	dbConInfo.dbHost = strdup(option);

	// port
	option = sData_findOption(config, "DBPORT");
	if( option == NULL )
	{ printf("[Game Server Database] dbPort option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	dbConInfo.dbPort = atoi(option);

	// dbName
	option = sData_findOption(config, "DBNAME");
	if( option == NULL )
	{ printf("[Game Server Database] dbName option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	dbConInfo.dbName = strdup(option);

	// dbUser
	option = sData_findOption(config, "DBUSER");
	if( option == NULL )
	{ printf("[Game Server Database] dbUser option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	dbConInfo.dbUser = strdup(option);

	// dbPass
	option = sData_findOption(config, "DBPASS");
	if( option == NULL )
	{ printf("[Game Server Database] dbPass option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	dbConInfo.dbPass = strdup(option);

	// Next Category ////////////////////////////////////////////////////////////////////////////////////
	sData_nextCategory(config);

	// host
	option = sData_findOption(config, "DBHOST");
	if( option == NULL )
	{ printf("[Auth Server Database] dbHost option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	AuthInfo.dbHost = strdup(option);

	// port
	option = sData_findOption(config, "DBPORT");
	if( option == NULL )
	{ printf("[Auth Server Database] dbPort option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	AuthInfo.dbPort = atoi(option);

	// dbName
	option = sData_findOption(config, "DBNAME");
	if( option == NULL )
	{ printf("[Auth Server Database] dbName option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	AuthInfo.dbName = strdup(option);

	// dbUser
	option = sData_findOption(config, "DBUSER");
	if( option == NULL )
	{ printf("[Auth Server Database] dbUser option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	AuthInfo.dbUser = strdup(option);

	// dbPass
	option = sData_findOption(config, "DBPASS");
	if( option == NULL )
	{ printf("[Auth Server Database] dbPass option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	AuthInfo.dbPass = strdup(option);

	// Next Category ////////////////////////////////////////////////////////////////////////////////////
	sData_nextCategory(config);

	// IP Address
	option = sData_findOption(config, "IPAddress");
	if( option == NULL )
	{ printf("[Game Server] IPAddress option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	AuthInfo.ServerIP = strdup(option);

	// IP Address Hex
	option = sData_findOption(config, "IPAddressHex");
	if( option == NULL )
	{ printf("[Game Server] IPAddressHex option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	sscanf(option, "%x", &AuthInfo.ServerIPHex); 

	// Port
	option = sData_findOption(config, "Port");
	if( option == NULL )
	{ printf("[Game Server] Port option missing in config.txt\n"); Sleep(1000*10); ExitProcess(-2); }
	AuthInfo.ServerPort = atoi(option);

	sData_close(config);


	MYSQL *mysql_gs = _dataInterface_gs_connect();
	if( !mysql_gs )
		ExitProcess(-1);
	mysql_close(mysql_gs);

	AuthInfo.mysql_as = _dataInterface_as_connect();
	if( !AuthInfo.mysql_as )
		ExitProcess(-1);

	for(int i=0; i<WORKER_THREADS; i++)
		_dataInterface_initWorkerThread(i);
}

void* dataInterface_allocJob(int size)
{
	return malloc(size);
}

void dataInterface_freeJob(void *job)
{
	free(job);
}

void dataInterface_registerServerForAuth()
{

	char queryText1[1024];
	wsprintf(queryText1, "SELECT server_id FROM game_servers WHERE host=INET_ATON('%s') AND port ='%i'  LIMIT 1", AuthInfo.ServerIP, AuthInfo.ServerPort);
	if( mysql_query(AuthInfo.mysql_as, queryText1) )
	{
		printf("MySQL: Error in query\n");
		Sleep(1000*60);
		return;
	}
	MYSQL_RES *dbResult = mysql_store_result(AuthInfo.mysql_as);
	MYSQL_ROW dbRow;
	dbRow = mysql_fetch_row(dbResult);
	if (dbRow != NULL)
	{
		printf("The server is already registered in the database\n");
		return;
	}
	
	char queryText2[1024];
	wsprintf(queryText2, "INSERT INTO game_servers ("
	"`server_id`,`host`,`port`,`age_limit`,`pk_flag`,`current_users`,`max_users`,`status`,`static`)"
	" VALUES(234,INET_ATON('%s'),%i,18,0,0,10,1,1);", AuthInfo.ServerIP, AuthInfo.ServerPort);
	if( mysql_query(AuthInfo.mysql_as, queryText2) )
	{
		printf("Error registering the server in the database\n");
		while(1) Sleep(1000);	
	}
	printf("The server has been registered in the database\n");
	return;
}

unsigned int dataInterface_getMyIP()
{
	return (unsigned int)AuthInfo.ServerIPHex;
}

int dataInterface_QuerySession(unsigned int ID1, unsigned int ID2, authSessionInfo_t *asiOut)
{
	char queryText[1024];
	wsprintf(queryText, "SELECT account,uid FROM sessions WHERE session_id1='%u' AND session_id2='%u' LIMIT 1", ID1, ID2);
	
	if( mysql_query(AuthInfo.mysql_as, queryText) )
	{
		printf("Error in query for sessions\n");
		return 0;	
	}
	MYSQL_RES *dbResult = mysql_store_result(AuthInfo.mysql_as);
	MYSQL_ROW dbRow;
	dbRow = mysql_fetch_row(dbResult);
	if (dbRow == NULL) { return 0; }
	strcpy(asiOut->Accountname, (char*)dbRow[0]);
	asiOut->ID = atoi(dbRow[1]);
	printf("User ID: %u\n", asiOut->ID);
	asiOut->UID1 = ID1;
	asiOut->UID2 = ID2;
	return 1;
}