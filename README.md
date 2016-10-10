## Executive summary
RansomHoney is a honeypot for ransomware (that where the name comes from) that aims to detect suspicious behaviour. Once such behavior is detected, RansomHoney queries the user upon the validity of the observed activities, and only when the user approves, it resumes the suspended operation.

## Technical details
#### TL;DR
RansomHoney creates and hides some files. If any thread tries to access those files, RansomHoney suspends the rebellious thread and notifies the user.

#### Detailed Technical Details
* To obtain SYSTEM permissions, RansomHoney installs a service called RansomHoneyService.
* RansomHoney creates some hidden files. They are created as hidden (FILE_ATTRIBUTE_HIDDEN) and to increase the secrecy fileHider.dll is injected to a list of processes,
  * fileHider.dll hooks the methods FindFirstFileA/W and FindNextFileA/W and thus makes the hidden file opaque to the user. **note** For some reason, explorer.exe doesn't use these functions to iterate over files on Windows 10, so, for now, this method is useful only for hiding files from the command line (the process cmd.exe).
* RansomHoney injects fileWatcher.dll to all of the processes,
   * fileWatcher.dll hooks the methods CreateFileA/W so that whenever they are called, it checks whether someone tries to access any of the "hidden files", suspends the thread that did it, and queries the user for the verdict for this thread. If the user does not approve, it simply returns INVALID_HANDLE_VALUE or calls the original method otherwise.

## Usage
Run the command line as an administrator, and run,
```
RansomHoneyService.exe install
RansomHoneyService.exe run
```
To remove the service simply run
```
RansomHoneyService.exe uninstall
```

## Projects in solution
* DummyDLL - a DLL that is used mainly for testing.
* fileHider - hooks the FindFirstFileA/W and FindNextFileA/W methods and hides the relevant files.
* fileWatcher - hooks CreateFileA/W and notifies the user of suspicious behavior.
* RansomHoney - a static library that contains most of the code responsible for hooking, injecting DLLs, and some high-level methods such as "file all files"
* RansomHoneyService - an executable that contains both the service and the code that installs it
* Tests - contains some very basic tests of various functionalities.
* Utils - various utils modules.

## TODOs:
1. Successfully hide the files from explorer.exe on Windows 10 as well.
2. Inject fileWatcher.dll to every new process.
