#include "stringUtils.h"

DWORD getNumOfSubstrings(const TCHAR* str, const TCHAR* substr, DWORD substrLength) {
	TCHAR* temp = (TCHAR*)str;
	TCHAR* curSubstrPos = NULL;
	DWORD counter;
	for (counter = 0; curSubstrPos = wcsstr(temp, substr); ++counter) {
		temp = curSubstrPos + substrLength;
	}
	return counter;
}

const TCHAR* strReplace(const TCHAR* srcStr, const TCHAR* strToReplace, const TCHAR* replaceWith) {
	if (!srcStr || !strToReplace || !replaceWith) {
		// TODO: error message
		return FALSE;
	}
	DWORD offsetInSrcStr = 0;

	DWORD strToReplaceLength = wcslen(strToReplace);
	DWORD replaceWithLength = wcslen(replaceWith);
	DWORD srcStrLength = wcslen(srcStr);
	
	DWORD numOfSubstrings = getNumOfSubstrings(srcStr, strToReplace, strToReplaceLength);
	if (0 == numOfSubstrings) {
		return srcStr;
	}

	DWORD outStrElems = srcStrLength + numOfSubstrings * (replaceWithLength - strToReplaceLength) + 1;
	DWORD outStrLength = sizeof(TCHAR) * outStrElems;
	TCHAR* outStr = (TCHAR*)malloc(outStrLength);

	TCHAR* strToReplacePtr = NULL;
	TCHAR* temp = outStr;
	
	while (numOfSubstrings--) {
		strToReplacePtr = wcsstr(srcStr + offsetInSrcStr, strToReplace);
		DWORD indexInStr = strToReplacePtr - srcStr;
		if (0 != indexInStr) {
			wcsncpy_s(temp, outStrElems, srcStr, indexInStr);
			temp += indexInStr;
		}
		if (0 != replaceWithLength){
			wcscpy_s(temp, replaceWithLength, replaceWith);
		}
		temp += replaceWithLength;
		offsetInSrcStr += indexInStr + strToReplaceLength;
	}
	if (offsetInSrcStr != srcStrLength) {
		DWORD restOfStrLen = wcslen(srcStr + offsetInSrcStr) + 1;
		wcscpy_s(temp, restOfStrLen, srcStr + offsetInSrcStr);
	}

	return outStr;
}