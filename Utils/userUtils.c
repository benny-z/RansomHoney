#include "userUtils.h"

#pragma comment(lib, "netapi32.lib")

DWORD getAllUsers(TCHAR usersList[][MAX_NUM_OF_USERS]) {
	NET_API_STATUS result;
	LPTSTR pszServerName = NULL;
	LPUSER_INFO_0 pBuf = NULL;
	LPUSER_INFO_0 pTmpBuf;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD dwTotalCount = 0;
	do {
		result = NetUserEnum((LPCWSTR)pszServerName,
			0, // Return user account names. pBuf points to an array of USER_INFO_0 structures.
			FILTER_NORMAL_ACCOUNT,
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);

		if ((result == NERR_Success) || (result == ERROR_MORE_DATA)) {
			if ((pTmpBuf = pBuf) != NULL) {
				for (unsigned int i = 0; i < dwEntriesRead; ++i) {
					if (pBuf == NULL) {
						// TODO: error printf("An access violation has occurred\n");
						break;
					}
					TCHAR* username = pTmpBuf->usri0_name;
					wcscpy_s(usersList[i], MAX_PATH, username);

					++pTmpBuf;
					++dwTotalCount;
				}
			} else {
				// TODO: error printf("A system error has occurred: %d\n", result);
			}
			if (pBuf != NULL) {
				NetApiBufferFree(pBuf);
				pBuf = NULL;
			}
		}
	} while (ERROR_MORE_DATA == result);
	if (pBuf != NULL) {
		NetApiBufferFree(pBuf);
	}
	return dwTotalCount;
}