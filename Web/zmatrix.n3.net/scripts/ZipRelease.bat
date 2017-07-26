
set START_DIR=%CD%

if NOT EXIST ..\release goto DONE

cd ..\release

zip -r ..\..\zmatrix.n3.net-release.zip *

cd %START_DIR%

for /F "tokens=1-4 delims=/ " %%i in ('date /t') do (
set DayOfWeek=%%i
set Month=%%j
set Day=%%k
set Year=%%l
)
move ..\..\zmatrix.n3.net-release.zip ..\..\zmatrix.n3.net-release-%Year%-%Month%-%Day%.zip

:DONE
