set PATH=%PATH%;"C:\Program Files\My Inno Setup Extensions 3";"C:\Program Files\HTML Help Workshop"

if not "%1" == "" (
	if exist scripts (
	cd scripts
	call ReVersionAll %1
	cd ..
	)
)

msdev matrix.dsw /MAKE "ZMatrixSS - Win32 Release Win9x" /REBUILD

if exist Config (
cd Config
bpr2mak -tdefault.bmk Config.bpr
make -B -DWIN9X -f Config.mak
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

rmdir /S /Q Distro9x
mkdir Distro9x

copy default.cfg Distro9x
copy matrix.exe Distro9x
copy zsMatrix.dll Distro9x
copy Config.dll Distro9x
copy MsgHook.dll Distro9x
copy readme.txt Distro9x
copy MatrixCodeFontSet.txt Distro9x
copy LICENSE.TXT Distro9x
copy "Matrix Code Font.ttf" Distro9x
copy ZMatrixHelp.chm Distro9x

mkdir Distro9x\ScreenSaver

copy ScreenSaver\ZMatrixSS.scr Distro9x\ScreenSaver

msdev matrix.dsw /MAKE "ZMatrixSS - Win32 Release Win9x" /CLEAN
del /Q Config.dll

if exist Setup (
cd Setup
isppcc ZMatrix.iss /dWIN9X
REM isppcc ZMatrix.iss /dWIN9X /dUPGRADE
cd ..
)

echo 
