set PATH=%PATH%;"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE";"C:\Program Files (x86)\Inno Setup 6";"C:\Program Files (x86)\HTML Help Workshop";"C:\Program Files (x86)\Borland\CBuilder6\Bin\";"C:\Program Files (x86)\Windows Kits\10\bin\10.0.17763.0\x86"

if not "%1" == "" (
	if exist scripts (
	cd scripts
	call ReVersionAll %1
	cd ..
	)
)

devenv matrix.sln /rebuild Release /project "ZMatrixSS"

if exist Config (
cd Config
bpr2mak -tdefault.bmk Config.bpr
make -B -f Config.mak
cd ..
)

if exist Help\help.zmatrix.n3.net\src (
cd Help\help.zmatrix.n3.net\src
call make
if exist ..\release (
cd ..\release
hhc ZMatrixHelp.hhp
)
cd ..\..\..
)

rmdir /S /Q DistroNT
mkdir DistroNT

copy default.cfg DistroNT
copy matrix.exe DistroNT
copy zsMatrix.dll DistroNT
copy Config.dll DistroNT
copy MsgHook.dll DistroNT
copy readme.txt DistroNT
copy LICENSE.TXT DistroNT
copy ORIGINALREADME.md DistroNT
copy JapaneseSet.txt DistroNT
copy MatrixCodeFontSet.txt DistroNT
copy "Matrix Code Font.ttf" DistroNT
copy ZMatrixHelp.chm DistroNT
copy WinampVis\vis_zmx.dll DistroNT

mkdir DistroNT\ScreenSaver

copy ScreenSaver\ZMatrixSS.scr DistroNT\ScreenSaver

devenv matrix.sln /clean Release /project "ZMatrixSS"
REM del /Q Config.dll

if exist Setup (
cd Setup
iscc ZMatrix_payalord.iss
REM isppcc ZMatrix.iss /dUPGRADE
cd ..
)

echo 
