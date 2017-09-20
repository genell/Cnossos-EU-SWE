#include <windows.h>
#include <psapi.h>
#include <stdio.h>

#ifdef _SHOW_LIBRARIES_
void show_libraries(unsigned int processID = -1)
{
	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	unsigned int i;

	if (processID == -1) processID = GetCurrentProcessId() ;

	// Print the process identifier.

	printf ("---------------------------------------------------------------------------------\n") ;
	printf ("Process ID: %u\n", processID );
	printf ("Using dynamic libraries \n") ;

	// Get a handle to the process.

	hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
	if (NULL == hProcess) return;

	// Get a list of all the modules in this process.

	if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL))
	{
		for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
		{
			char szModName[MAX_PATH];

			// Get the full path to the module's file.

			if ( GetModuleFileNameA (hMods[i], szModName, MAX_PATH))
			{
				printf("%s\n", szModName);
			}
		}
	}

	// Release the handle to the process.

	CloseHandle( hProcess );

	printf ("---------------------------------------------------------------------------------\n") ;
}
#endif