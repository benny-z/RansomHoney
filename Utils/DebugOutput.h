#ifndef __DEBUG_OUTPUT_H__
#define __DEBUG_OUTPUT_H__
#include <Windows.h>

#pragma warning( disable : 4996)

#define DEBUG

#ifdef DEBUG
/**
 * @brief      Warper of OutputDebugString
 *
 * @param[in]  message  The message to output
 */
inline void debugOutput(const wchar_t* message) {
	OutputDebugString(message);
}

/**
 * @brief      Warper of OutputDebugString
 *
 * @param[in]  errorMessage  The message to output
 * @param[in]  num           Parameter for the message
 */
inline void debugOutputNum(const wchar_t* errorMessage, int num) {
	wchar_t msgbuf[300] = { 0 };
	wsprintf(msgbuf, errorMessage, num);
	OutputDebugString(msgbuf);
}

/**
 * @brief      Warper of OutputDebugString
 *
 * @param[in]  errorMessage  The message to output
 * @param[in]  param         Parameter for the message
 */
inline void debugOutputStr(const wchar_t* errorMessage, const void* param) {
	wchar_t msgbuf[300] = { 0 };
	wsprintf(msgbuf, errorMessage, param);
	OutputDebugString(msgbuf);
}
#else // not DEBUG
inline void debugOutput(const char* message) {}
inline void debugOutputNum(const char* errorMessage, int lastError) {}
inline void debugOutputStr(const char* errorMessage, const char* param) {}
#endif 


#endif // __DEBUG_OUTPUT_H__