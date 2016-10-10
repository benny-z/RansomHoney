#include <windows.h>
#include <stdio.h>
#include "ServiceInstaller.h"
#include "..\RansomHoneyService\RansomHoneyService.h"

int main(int argc, char *argv[])  {
	if (argc > 1) {
		if (0 == strcmp("install", argv[1])) {
			if (!install(RH_SERVICE_NAME, 
						 RH_SERVICE_DISPLAY_NAME, 
						 RH_SERVICE_START_TYPE,
						 RH_SERVICE_DEPENDENCIES,
						 RH_SERVICE_ACCOUNT, 
						 RH_SERVICE_PASSWORD)) {
				wprintf(L"Error in main. install failed (0x%08lx)", GetLastError());
			}
		} else if (0 == strcmp("uninstall", argv[1])) {
			if (!uninstall(RH_SERVICE_NAME)) {
				wprintf(L"Error in main. uninstall failed (0x%08lx)", GetLastError());
			}
		} else if (0 == strcmp("run", argv[1])) {
			if (!runService(RH_SERVICE_NAME)) {
				wprintf(L"Error in main. uninstall failed (0x%08lx)", GetLastError());
			}
		}
		else {
			wprintf(L"Invalid argument");
		}
	} else {
		if (!run()) {
			wprintf(L"Error in main. run failed (0x%08lx)", GetLastError());
		}
	}

	return 0;
}