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
		debugOutputNum(L"Error in addExpandedFile. ExpandEnvironmentStrings failed (%d)", GetLastError());
		return FALSE;
	}
	size_t expandedFnameSize = wcslen(expandedFname);
	size_t curIndexFileSize = sizeof(TCHAR) * (expandedFnameSize + 1);

	g_filesList[curIndex] = (TCHAR*)malloc(curIndexFileSize);
	
	if (NULL == g_filesList[curIndex]) {
		debugOutputNum(L"Error in addExpandedFile. Memory allocation for g_filesList[curIndex] failed (%d)", GetLastError());
		return FALSE;
	}
	debugOutputStr(L"Adding the file: %s\n", expandedFname);
	return NULL != memcpy(g_filesList[curIndex], expandedFname, curIndexFileSize);
}

static TCHAR* g_usersPath = NULL;

BOOL initUsersPath() {
	if (NULL != g_usersPath) {
		return TRUE;
	}
	
	DWORD userPathSize = -1;
	if (!GetProfilesDirectory(NULL, &userPathSize) || (-1 == userPathSize)) {
		DWORD lastError = GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER != lastError) {
			debugOutputNum(L"Error in getUsersPath. GetProfilesDirectory failed on first run (%d)", GetLastError());
			return FALSE;
		}
	}

	g_usersPath = (TCHAR*)malloc(sizeof(TCHAR) * (userPathSize + 1));
	if (NULL == g_usersPath) {
		debugOutputNum(L"Error in getUsersPath. Memory allocation failed (%d)", GetLastError());
		return FALSE;
	}
	if (!GetProfilesDirectory(g_usersPath, &userPathSize)) {
		debugOutputNum(L"Error in getUsersPath. GetProfilesDirectory failed on second run (%d)", GetLastError());
		return FALSE;
	}
	g_usersPath[userPathSize - 1] = '\\';
	g_usersPath[userPathSize] = '\0';
	return TRUE;
}

BOOL expandSingleUserProfile(const TCHAR* username, const TCHAR* filename, int* curIndex) {
	BOOL retVal = TRUE;

	const TCHAR* fnameWithNoUserPath = strReplace(filename, USER_PROFILE_ENV_VAR, L"");

	size_t usernameSize = wcslen(username);
	size_t fnameWithNoUserPathSize = wcslen(fnameWithNoUserPath);
	size_t usersPathSize = wcslen(g_usersPath);

	size_t outPathElems = fnameWithNoUserPathSize + usernameSize + usersPathSize + 1;
	size_t outPathSize = sizeof(TCHAR) * outPathElems;
	TCHAR* outPath = (TCHAR*)malloc(outPathSize);
	memset(outPath, 0, outPathSize);

	wcscpy_s(outPath, outPathElems, g_usersPath);
	wcscat_s(outPath, outPathElems, username);

	if (retVal = PathFileExists(outPath)) {
		wcscat_s(outPath, outPathElems, fnameWithNoUserPath);
		retVal = addExpandedFile(outPath, *curIndex);
	}

	if (fnameWithNoUserPath) {
		free((TCHAR*)fnameWithNoUserPath);
		fnameWithNoUserPath = NULL;
	}
	if (outPath) {
		free(outPath);
		outPath = NULL;
	}
	return retVal;
}

BOOL expandUsersProfile(const TCHAR usrsLst[][MAX_NUM_OF_USERS], const TCHAR* filename, DWORD numOfUsers, int* curIndex) {
	if (!initUsersPath()) {
		return FALSE; // Error message printed out in the getUsersPath function
	}
	for (unsigned int j = 0; j < numOfUsers; ++j) {
		const TCHAR* username = usrsLst[j];
		BOOL retVal = expandSingleUserProfile(username, filename, curIndex);
		if (retVal) {
			++(*curIndex);
		} else {
			--g_numOfFiles;
		}
	}
	return TRUE;
}

static BOOL isFileListInitialized = FALSE;
BOOL initFilesList() {
	if (isFileListInitialized) {
		return TRUE;
	}

	TCHAR usrsLst[MAX_PATH][MAX_NUM_OF_USERS] = { 0 };
	DWORD numOfUsers = getAllUsers(&usrsLst);
	DWORD fileWithUserProfile = numOfFilesWithUserProfile();

	g_numOfFiles = NUM_OF_FILES - fileWithUserProfile + (fileWithUserProfile * numOfUsers );
	g_filesList = malloc(sizeof(TCHAR*) * g_numOfFiles);

	if (NULL == g_filesList) {
		debugOutputNum(L"Error in initFilesList. Memory allocation for g_filesList failed (%d)", GetLastError());
		return FALSE;
	}
	DWORD counter = 0;
	for (int i = 0; i < NUM_OF_FILES; ++i) {
		const TCHAR* filename = g_initFilesList[i];
		if (0 != wcsstr(filename, USER_PROFILE_ENV_VAR)) {
			if (!expandUsersProfile(usrsLst, filename, numOfUsers, &counter)) {
				debugOutputNum(L"Error in initFilesList. expandUserProfile failed (%d)", GetLastError());
				return FALSE;
			}
		} else {
			if (!addExpandedFile(filename, counter)) {
				debugOutputNum(L"Error in initFilesList. expandUserProfile failed (%d)", GetLastError());
				return FALSE;
			}
			++counter;
		}
	}
	isFileListInitialized = TRUE;
	return TRUE;
}

const TCHAR ** getFilesList(){
	return g_filesList;
}

const TCHAR** getProcsToHideFrom() {
	return g_procsToHideFrom;
}