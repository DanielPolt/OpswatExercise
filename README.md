# OpswatExercise
Program involving OPSWAT API for the OPSWAT Exercise
Run the program with Visual Studio Code.

Ensure libcurl is setup within the Visual Studio Code 2019 project containing the code. One should first in Powershell run the commands:
PS ${curl-dir}> git clone https://github.com/curl/curl.git
PS ${curl-dir}> cd winbuild
PS ${curl-dir}\winbuild> nmake /f Makefile.vc mode=static
One should then within the Project on Visual Studio Code:
-Open Project Properties.
-Add ${curl-dir}\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl\include to Configuration Properties -> VC++ Directories -> Include Directories.
-Add ${curl-dir}\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl\lib to Configuration Properties -> VC++ Directories -> Library Directories.
-Add CURL_STATICLIB to Configuration Properties -> C/C++ -> Preprocessor -> Preprocessor Definitions.
-In Configuration Properties -> Linker -> Input -> Additional Dependencies, add these followings lines-
    ${curl-dir}\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl\lib\libcurl_a.lib
    Ws2_32.lib
    Wldap32.lib
    Crypt32.lib
    Normaliz.lib
Additional details found here: https://medium.com/@farhabihelal/how-to-set-up-libcurl-on-visual-studio-2019-a9fdacce6945

JSON and PicoSHA2, publically available resources from GitHub, are also used for JSON and Hashing. Download them respectively at:
https://github.com/nlohmann/json
and
https://github.com/okdshin/PicoSHA2
Then ensure that their corresponding #include lines at the beginning of the program follow the correct directory to the referenced files.
More details can be found on their respective GitHub pages.

To utilize the project, set it up in a project within Visual Studio Code 2019. Also ensure there is a valid file within the repository for it to be tested with. The program is set up to test the included "TestDatFile.dat", but any file of a valid test can be used if the code itself is modified to use it instead. Run the program with the "Local Windows Debugger" button on the top of the interface. A console will be brought up. The user will then be asked for a valid API Key. Enter one that works with MetaDefender Cloud. The file will then be hashed and a hash lookup is performed with this hash value. If previously cached results are not found, the file is uploaded and the results are retrieved through the Data ID of the file. The results are then outputted to the console.
