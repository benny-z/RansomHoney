#include "commonFilesList.h"

#define USER_PROFILE_ENV_VAR L"%UserProfile%"

TCHAR* g_initFilesList[NUM_OF_FILES] = { L"%SystemDrive%\\temp_file_do_not_touch.docx",
                                     L"%SystemDrive%\\Python27\\temp_file_do_not_touch.txt",
                                     L"%UserProfile%\\temp_file_do_not_touch.docx",
	                                 L"%SystemDrive%\\Windows\\temp_file_do_not_touch.txt",
	                                 L"%SystemDrive%\\Windows\\system32\\temp_file_do_not_touch.txt",
	                                 L"%SystemDrive%\\$Recycle.Bin\\temp_file_do_not_touch.docx"};

TCHAR** g_filesList = NULL;
DWORD g_numOfFiles = 0;
TCHAR* g_procsToHideFrom[NUM_OF_HIDE_FROM_PROCS] = { L"explorer.exe",
                                                     L"cmd.exe" };

DWORD numOfFilesWithUserProfile() {
	DWORD counter = 0;
	for (int i = 0; i < NUM_OF_FILES; ++i) {
		if (0 != wcsstr(g_initFilesList[i], USER_PROFILE_ENV_VAR)) {
			++counter;
		}
	}
	return counter;
}

DWORD getNumOfFiles() {
	return g_numOfFiles;
}

BOOL addExpandedFile(const TCHAR* filename, int curIndex) {
	TCHAR expandedFname[MAX_PATH] = { 0 };
	if (0 == ExpandEnvironmentStrings(filename, expandedFname, MAX_PATH)) {
		// TODO: error
		return FALSE;
	}
	DWORD expandedFnameSize = wcslen(expandedFname);
	DWORD curIndexFileSize = sizeof(TCHAR) * (expandedFnameSize + 1);
	g_filesList[curIndex] = (TCHAR*)malloc(curIndexFileSize);
	if (NULL == g_filesList[curIndex]) {
		// TODO: error message
		return FALSE;
	}
	return NULL != memcpy(g_filesList[curIndex], expandedFname, curIndexFileSize);
}

const TCHAR* getUsersPath() {
	TCHAR *curUserPath = USER_PROFILE_ENV_VAR;
	TCHAR curUsername[MAX_PATH];
	DWORD usernameSize;
	if (!GetUserName((LPWSTR)&curUsername, &usernameSize)) {
		// TODO: error
		return NULL;
	}
	TCHAR extendedCurUserPath[MAX_PATH];
	if (0 == ExpandEnvironmentStrings(curUserPath, extendedCurUserPath, MAX_PATH)) {
		// TODO: error
		return NULL;
	}
	return strReplace(extendedCurUserPath, curUsername, L"");
}

BOOL expandUserProfile(const TCHAR usrsLst[][MAX_NUM_OF_USERS], const TCHAR* filename, DWORD numOfUsers, int* curIndex) {
	BOOL retVal = TRUE;
	const TCHAR* usersPath = getUsersPath();

	if (NULL == usersPath) {
		return FALSE; // Error message printed out in the inner function
	}

	for (unsigned int j = 0; j < numOfUsers; ++j) {
		const TCHAR* username = usrsLst[j];
		const TCHAR* fnameWithNoUserPath = strReplace(filename, USER_PROFILE_ENV_VAR, L"");

		DWORD usernameSize = wcslen(username);
		DWORD fnameWithNoUserPathSize = wcslen(fnameWithNoUserPath);
		DWORD usersPathSize = wcslen(usersPath);

		DWORD outPathElems = fnameWithNoUserPathSize + usernameSize + usersPathSize + 1;
		DWORD outPathSize = sizeof(TCHAR) * outPathElems;
		TCHAR* outPath = (TCHAR*)malloc(outPathSize);
		memset(outPath, 0, outPathSize);

		wcscpy_s(outPath, outPathElems, usersPath);
		wcscat_s(outPath, outPathElems, username);
		wcscat_s(outPath, outPathElems, fnameWithNoUserPath);

		if (!addExpandedFile(outPath, *curIndex)) {
			retVal = FALSE;
		}
		if (fnameWithNoUserPath) {
			free((TCHAR*)fnameWithNoUserPath);
			fnameWithNoUserPath = NULL;
		}
		if (outPath) {
			free(outPath);
			outPath = NULL;
		}
		++(*curIndex);
		if (FALSE == retVal) {
			break;
		}
	}
	if (usersPath) {
		free((TCHAR*)usersPath);
		usersPath = NULL;
	}
	return retVal;
}

BOOL initFilesList() {
	TCHAR usrsLst[MAX_PATH][MAX_NUM_OF_USERS] = { 0 };
	DWORD numOfUsers = getAllUsers(&usrsLst);
	DWORD fileWithUserProfile = numOfFilesWithUserProfile();

	g_numOfFiles = NUM_OF_FILES - fileWithUserProfile + (fileWithUserProfile * numOfUsers) - 1;
	g_filesList = malloc(sizeof(TCHAR*) * g_numOfFiles);

	if (NULL == g_filesList) {
		// TODO: error message
		return FALSE;
	}

	DWORD counter = 0;
	for (int i = 0, j = 0; i < NUM_OF_FILES; ++i) {
		const TCHAR* filename = g_initFilesList[i];
		if (0 != wcsstr(filename, USER_PROFILE_ENV_VAR)) {
			expandUserProfile(usrsLst, filename, numOfUsers, &counter);
		} else {
			addExpandedFile(filename, counter);
			++counter;
		}
	}
	return TRUE;
}

const TCHAR ** getFilesList(){
	return g_filesList;
}

const TCHAR** getProcsToHideFrom() {
	return g_procsToHideFrom;
}