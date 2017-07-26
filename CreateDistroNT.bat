set PATH=%PATH%;"C:\Program Files\My Inno Setup Extensions 3";"C:\Program Files\HTML Help Workshop"

if not "%1" == "" (
	if exist scripts (
	cd scripts
	call ReVersionAll %1
	cd ..
	)
)

msdev matrix.dsw /MAKE "ZMatrixSS - Win32 Release" /REBUILD

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
copy JapaneseSet.txt DistroNT
copy MatrixCodeFontSet.txt DistroNT
copy "Matrix Code Font.ttf" DistroNT
copy ZMatrixHelp.chm DistroNT

mkdir DistroNT\ScreenSaver

copy ScreenSaver\ZMatrixSS.scr DistroNT\ScreenSaver

msdev matrix.dsw /MAKE "ZMatrixSS - Win32 Release" /CLEAN
del /Q Config.dll

if exist Setup (
cd Setup
isppcc ZMatrix.iss
REM isppcc ZMatrix.iss /dUPGRADE
cd ..
)

echo 
