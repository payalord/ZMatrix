set PATH_TO_ROOT=.
set SCRIPTS_DIR=%PATH_TO_ROOT%\..\scripts
set OUTPUT_DIR=%PATH_TO_ROOT%\..\release
set FILEPP_INCLUDES=-I./ -I./../
set FILEPP_COMMAND=call filepp %FILEPP_INCLUDES% "-DPATH_TO_ROOT=%PATH_TO_ROOT%"


rmdir /S /Q %OUTPUT_DIR%
mkdir %OUTPUT_DIR%

%FILEPP_COMMAND% -m %SCRIPTS_DIR%\html_module.pm index.html > %OUTPUT_DIR%\index.html
%FILEPP_COMMAND% -m %SCRIPTS_DIR%\html_module.pm donate.html > %OUTPUT_DIR%\donate.html
%FILEPP_COMMAND% -m %SCRIPTS_DIR%\html_module.pm wallpaper.html > %OUTPUT_DIR%\wallpaper.html
copy *.xml %OUTPUT_DIR%\


mkdir %OUTPUT_DIR%\images

xcopy images\* %OUTPUT_DIR%\images /S /exclude:%SCRIPTS_DIR%\XCopyExclusions.txt



mkdir %OUTPUT_DIR%\help
cd help
set PATH_TO_ROOT=.\..
set SCRIPTS_DIR=%PATH_TO_ROOT%\..\scripts
set OUTPUT_DIR=%PATH_TO_ROOT%\..\release\help

xcopy * %OUTPUT_DIR% /S
xcopy %PATH_TO_ROOT%\..\..\..\Help\help.zmatrix.n3.net\release\* %OUTPUT_DIR% /S

cd ..



REM xcopy MakeSymbolicLinks %OUTPUT_DIR%


