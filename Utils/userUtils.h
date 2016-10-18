#ifndef __USER_UTILS_H__
#define __USER_UTILS_H__

#include <Windows.h>
#include <lm.h>

#define MAX_NUM_OF_USERS (50)

DWORD getAllUsers(TCHAR usersList[][MAX_NUM_OF_USERS]);

#endif // __USER_UTILS_H__