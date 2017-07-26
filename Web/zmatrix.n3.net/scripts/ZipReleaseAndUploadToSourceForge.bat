set START_DIR=%CD%

call ZipRelease

cd ..
cd ..

for /F "tokens=1-4 delims=/ " %%i in ('date /t') do (
set DayOfWeek=%%i
set Month=%%j
set Day=%%k
set Year=%%l
)

scp zmatrix.n3.net-release-%YEAR%-%MONTH%-%DAY%.zip deadc0de@zmatrix.sourceforge.net:/home/groups/z/zm/zmatrix/htdocs/

cd %START_DIR%
