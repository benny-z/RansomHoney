#include "commonFilesList.h"

TCHAR* g_filesList[NUM_OF_FILES] = { L"%SystemDrive%\\temp_file_do_not_touch.docx",
                                     L"%UserProfile%\\temp_file_do_not_touch.docx",
	                                 L"%SystemDrive%\\Windows\\temp_file_do_not_touch.txt",
	                                 L"%SystemDrive%\\Windows\\system32\\temp_file_do_not_touch.txt" };

TCHAR* g_procsToHideFrom[NUM_OF_HIDE_FROM_PROCS] = { L"explorer.exe",
                                                     L"cmd.exe" };

const TCHAR ** getFilesList(){
	return g_filesList;
}

const TCHAR** getProcsToHideFrom() {
	return g_procsToHideFrom;
}