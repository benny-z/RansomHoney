#ifndef __SERVICE_INSTALLER_H__
#define __SERVICE_INSTALLER_H__

#include <Windows.h>
#include <stdio.h>

/**
 * @brief      Installs the specified service.
 *
 * @param[in]  pszServiceName   The name of the service to install. The maximum string length 
 *                              is 256 characters. 
 * @param[in]  pszDisplayName   The display name to be used by user interface programs to 
 *                              identify the service. 
 * @param[in]  dwStartType      The service start options. 
 * @param[in]  pszDependencies  A pointer to a double null-terminated array of null-separated 
 *                              names of services or load ordering groups that the system must
 *                              start before this service. 
 * @param[in]  pszAccount       The name of the account under which the service should run.
 * @param[in]  pszPassword      The password to the account name specified by the pszAccount 
 *                              parameter.
 *
 * @note       For detailed explanation regarding the parameters above, please see the 
 *             documentation for CreateService function in the MSDN:
 *             https://msdn.microsoft.com/en-us/library/windows/desktop/ms682450(v=vs.85).aspx
 *             
 * @return     TRUE iff the service was installed successfully.
 */
BOOL install(PWSTR pszServiceName,
	PWSTR pszDisplayName,
	DWORD dwStartType,
	PWSTR pszDependencies,
	PWSTR pszAccount,
	PWSTR pszPassword);

/**
 * @brief      Uninstalls the specified service.
 *
 * @param[in]  pszServiceName  Name of the service to uninstall.
 *
 * @return     TRUE iff the service was uninstalled successfully.
 */
BOOL uninstall(PWSTR pszServiceName);

/**
 * @brief      Runs the specified service.
 *
 * @param[in]  szSvcName  Name of the service to run.
 *
 * @return     TRUE iff the service ran successfully.
 */
BOOL runService(const LPCWSTR szSvcName);

#endif // __SERVICE_INSTALLER_H__