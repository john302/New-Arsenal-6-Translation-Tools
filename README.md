# New Arsenal 6 Translation Tools
## Using the translation:
1. Download [New Arsenal 6](https://ap-pro.ru/forum/35-91737-80)
2. Copy the /gamedata/ folder in the /Translated Files/ folder into the main New Arsenal 6 folder (the one with fs.ltx in it)
3. Run the injector with the necessary arguments

## Using the injector:
The injector has a few command line arguments that it accepts:
#### Required:
|Command|Description|
|-----|-----|
| -exe "full_exe_path" | Path to the executable to be injected (xrEngine.exe in the /bin_x64/ folder) |
| -dll "full_dll_path" | Path to the dll to be injected	(Either DumperDLL.dll or LoaderDLL.dll) |

#### Optional:
|Command|Description|
|-----|-----|
|-args "cmd_line_args"|String with the command line arguments to pass to the executable|
|-folder "full_working_dir_path"|Path to the startup/working directory of the executable|
	
You only need to specify the -exe and -dll arguments for the injector to work.
	
You can either use the batch files provided in the release (you'll need to edit them with the path to xrEngine.exe) or you can create a shortcut to injector with the parameters appended after the injector path in quotes
	
## FAQ:
	
**Q:** 	Which DLL do I use?\
**A:** 	If you just want to use the translation, use LoaderDLL; if you want to dump the files yourself, use DumperDLL.

**Q:** 	How do I use the XMLTranslator?\
**A:** 	Read the readme.txt in the XMLTranslator folder.

**Q:** 	How does it work?\
**A:** 	Both of the DLLs hook a certain point in the function that loads files from within .db archives. When the file is loaded, depending on which DLL you used, the DLL will intercept the function and either write the file to the disk (if it does not already exist), or, if a file exists in the /gamedata/ folder matching the file being loaded, create a new buffer in memory that is filled with the file contents of the file in the /gamedata/ folder. The file contents pointer is changed to the new buffer and it then frees the old buffer and returns to the original function. The injector launches xrEngine.exe with it's thread suspended so that it can hook every file that is loaded. Once the DLL is injected the thread is resumed and the application continues. Files are dumped when loaded, so in order to dump _every_ file you'll need to play through most of the game with the DumperDLL injected.
	
**Q:**	The translation is garbage, can I improve it?\
**A:**	Absolutely. If you make a commit with some improved XMLs I'll look them over and if everything looks good I'll merge it.

**Q:**	How do I compile everything?\
**A:**	The Injector and XMLTranslator can be compiled with any version of Visual Studio with C++ and C# installed. The XMLTranslator has a dependency on [YandexTranslateCSharpSdk](https://github.com/anovik/YandexTranslateCSharpSdk). The DLLs require the [Intel Compiler](https://software.intel.com/en-us/parallel-studio-xe) to be compiled because MSVC does not support 64-bit inline assembly.

**Q:**	How do I configure the XMLTranslator?\
**A:**	Set the API key to your Yandex Translate API key. If you need to get one, go [here](https://translate.yandex.com/developers)

## TODO:

* Translate un-translated strings in the xml files.
* Translate the script files.
* Improve the XMLTranslator to better parse and translate the xml files.

## Dependencies
* Uses [YandexTranslateCSharpSdk](https://github.com/anovik/YandexTranslateCSharpSdk)