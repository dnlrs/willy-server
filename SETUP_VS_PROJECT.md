
How to setup Visual Studio project
==================================

// ignore these steps for now

 1. Clone repo into a folder which will be the "working directory"
 2. Open Visual Studio
 3. File -> New -> Project from existing code
    a. Type of project: Visual C++, then Next
    b. Project file location: browse to the working directory; Project name: wifi_watchdog_analyzer;
	   make sure "add files to the project from the folders" and "Show all files in solution Explorer" 
	   are ticked, then Next
	c. Use Visual Studio -> Project type: "Console application project", leave the rest as is, then Next
	d. Leave default settings, then Next
	e. Leave default settings, then Finish
	f. Now you will have a folder named 'wifi_watchdog analyzer'. Move the .sln in the GIT directory and the .vcxproj files into the src folder
	g. Modify with Notepad++ the .sln and modify the Project{xxxxx-xxx} row by changing the folder to src on the second argument
 4. In the "Solution Explorer" window (View->Solution Explorer), right click on the project 
    (wifi_watchdog_analyzer) and select "Properties"
 5. Under "Configuration Properties -> General" change the "Windows SDK Version" from 8.1 to 10.0.17...; then Apply
// 6. Under "C/C++ -> General" add "$(ProjectDir)include;" to " Additional Include Directories"; then Apply
 7. Under "Linker -> General" add "$(ProjectDir)lib;" to "Additional library Directories"; then Apply
 
You may now build and run the application.
 ++ (if the linker cannot find the sql3.dll press Alt+F7, then Debugging and modify the Working Directory to '$ProjectDir..\' ) ++

How to get the sqlite3 library (only for reference, not to be done)
===================================================================

 1. Download from sqlite.org "sqlite-amalgamation-VVV.zip" and "sqlite-dll-winXX-xXX-VVV.zip" where VVV 
    stands for laste version number and XX for "86" or "64" architecture
 2. Unzip "sqlite-amalgamation-VVV.zip" and copy "sqlite3.h" to "working directory"\include folder
 3. Unzip "sqlite-dll-winXX-xXX-VVV.zip"
 4. Open Visual Studio and select "Tools -> Visual Studio Command Prompt"
 5. in the new command prompt navigate where you extracted the .dll files and digit:
    ` lib /DEF:"sqlite3.def" /OUT:"sqlite3.lib" /MACHINE:"YYY" `
	where YYY can be aither "X86" or "X64"
 6. copy "sqlite3.lib" in the "working directory"\lib folder and "sqlite3.dll" directly in the working 
    directory
 
 