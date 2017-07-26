set FILEPP_INCLUDES=-I.\ -I.\.. -I.\..\.. -I.\..\..\..
set FILEPP_COMMAND=call filepp %FILEPP_INCLUDES%


set PATH_TO_ROOT=.
set OUTPUT_DIR=%PATH_TO_ROOT%\..\release
rmdir /S /Q %OUTPUT_DIR%
mkdir %OUTPUT_DIR%

%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm index.html > %OUTPUT_DIR%\index.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Uninstall.html > %OUTPUT_DIR%\Uninstall.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm FAQ.html > %OUTPUT_DIR%\FAQ.html
copy ZMatrixHelp.hhp %OUTPUT_DIR%
copy ZMatrixHelp.hhc %OUTPUT_DIR%
copy ZMatrixHelp.hhk %OUTPUT_DIR%


cd RightClick
set PATH_TO_ROOT=..
set OUTPUT_DIR=%PATH_TO_ROOT%\..\release\RightClick
mkdir %OUTPUT_DIR%

%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Overview.html > %OUTPUT_DIR%\Overview.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Main.html > %OUTPUT_DIR%\Main.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Autostart.html > %OUTPUT_DIR%\Autostart.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm ScreenSaver.html > %OUTPUT_DIR%\ScreenSaver.html

cd ..


cd Config
set PATH_TO_ROOT=..
set OUTPUT_DIR=%PATH_TO_ROOT%\..\release\Config
mkdir %OUTPUT_DIR%

%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Overview.html > %OUTPUT_DIR%\Overview.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm LoadingAndSaving.html > %OUTPUT_DIR%\LoadingAndSaving.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm TextOptions.html > %OUTPUT_DIR%\TextOptions.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm BackgroundOptions.html > %OUTPUT_DIR%\BackgroundOptions.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm ColorOptions.html > %OUTPUT_DIR%\ColorOptions.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm ProcessPriorityOptions.html > %OUTPUT_DIR%\ProcessPriorityOptions.html


	cd OtherOptions
	set PATH_TO_ROOT=..\..
	set OUTPUT_DIR=%PATH_TO_ROOT%\..\release\Config\OtherOptions
	mkdir %OUTPUT_DIR%
	
	%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Overview.html > %OUTPUT_DIR%\Overview.html
	%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm General.html > %OUTPUT_DIR%\General.html
	%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Cleanup.html > %OUTPUT_DIR%\Cleanup.html
	cd ..

cd ..



cd ScreenSaver
set PATH_TO_ROOT=..
set OUTPUT_DIR=%PATH_TO_ROOT%\..\release\ScreenSaver
mkdir %OUTPUT_DIR%

%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Overview.html > %OUTPUT_DIR%\Overview.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm ScreenSaverOnly.html > %OUTPUT_DIR%\ScreenSaverOnly.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm ScreenSaverAndBackground.html > %OUTPUT_DIR%\ScreenSaverAndBackground.html

cd ..


cd WinampVis
set PATH_TO_ROOT=..
set OUTPUT_DIR=%PATH_TO_ROOT%\..\release\WinampVis
mkdir %OUTPUT_DIR%

%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Overview.html > %OUTPUT_DIR%\Overview.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm Modules.html > %OUTPUT_DIR%\Modules.html
%FILEPP_COMMAND% -m %PATH_TO_ROOT%\..\scripts\html_module.pm CommonConfiguration.html > %OUTPUT_DIR%\CommonConfiguration.html

cd ..


set PATH_TO_ROOT=.
set OUTPUT_DIR=%PATH_TO_ROOT%\..\release\images\

xcopy images\* %OUTPUT_DIR% /S /exclude:%PATH_TO_ROOT%\..\scripts\XCopyExclusions.txt



