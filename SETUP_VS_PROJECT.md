# How to download and setup project

1. Clone repo into your working folder *$(SolutionDir)*
2. In Visual studio:
   a. File -> Open -> Project/Solution 
   b. Go to your working directory and select `wifi_watchdog_analyzer.sln`
3. solution will be opened in solution explorer, already configured

Within the solution there are different projects, one is compiled as an 
application executable (the entry point of the whole application) and 
the others as static liraries.

There are 2 global project properties sheets:
- `lib_modules.props`: for projects that are compiled as libraries
- `VS_project_config_x64.props`: for project compiled as application executable

##### Make sure you are building and debugging for x64 architecture

- Build -> configuration Manager... -> Active Solution Platform: `x64`

#### Visual studio and SDK version

It may happen that on different computers different SDK and C++ versions are
installed. These can be changed for each project:
1. Select all interested project in Solution Explorer
2. Right click -> Properties
3. General -> Windows SDK version -> select appropriate version 

> repo version is: 10.0.17763.0

4. General -> Windows SDK version -> select appropriate version 

> repo version is: Visual Studio 2017 (v141)

5. C/C++ -> Language -> C++ Language standard -> select appropriate version

> repo version is: ISO C++17 Standard (/std:c++17)

#### Character Set

1. Select all projects in the Solution Explorer.
2. right Click -> Properties -> General -> Character Set: `Not Set`

> repo option is: Not Set

#### Important Default properties

These other properties are setted by the properties sheets, check them and 
change them if they do not comply. If you have to change them, if available, 
select `<inherit from parent or project defaults>`.

##### Static Library projects

Select all static library probject in Solution Explorer then 
Right Click -> Properties:

- In `General` section:
  - Output Directory: 
   `$(SolutionDir)$(Platform)\$(Configuration)\$(ProjectName)\`
  - Intermediate Directory: 
   `$(SolutionDir)$(Platform)\$(Configuration)\$(ProjectName)_intermediate\`
- In `C/C++` section:
  - General -> Additional Include Directories: 
   `$(ProjectDir)include;$(ProjectDir)src;$(ProjectDir)ext;$(SolutionDir)src\conf\include;
    $(SolutionDir)src\db\include;$(SolutionDir)src\localization\include;
    $(SolutionDir)src\log\include;$(SolutionDir)src\server\include;
    $(SolutionDir)src\utils\include;$(SolutionDir)src;%(AdditionalIncludeDirectories)`
  - General -> Warning Level: `Level4 (/W4)`
  - Language -> Conformance Mode: `No`
  - Precompiled Header -> Precompiled Header: `Not Using Precompiled header`

> NOTE: to correctly connect client and server you should turn off the firewall

---
	  
#### How to get the sqlite3 library (reference only, not to be done)

1. Download from sqlite.org *sqlite-amalgamation-VVV.zip* and *sqlite-dll-win64-x64-VVV.zip* 
 where *VVV* stands for last version number
2. Unzip *sqlite-amalgamation-VVV.zip* and copy *sqlite3.h* to *$(SolutionDir)\include* 
 folder
3. Unzip *sqlite-dll-win64-x64-VVV.zip*
4. In Visual Studio and select Tools -> Visual Studio Command Prompt
5. In the new command prompt navigate where you extracted the *.dll* files (from step 3)
 and digit: 

```
 lib /DEF:"sqlite3.def" /OUT:"sqlite3.lib" /MACHINE:"x64" 
```

6. Copy *sqlite3.lib* in the *$(SolutionDir)\lib* folder and *sqlite3.dll* directly in the
 working directory
 
 