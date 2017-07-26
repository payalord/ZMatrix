rmdir /S /Q Distro_SRC
mkdir Distro_SRC

if not "%1" == "" (
	if exist scripts (
	cd scripts
	call ReVersionAll %1
	cd ..
	)
)

mkdir Distro_SRC\Config
mkdir Distro_SRC\zsMatrix
mkdir Distro_SRC\MsgHook
mkdir Distro_SRC\ScreenSaver
mkdir Distro_SRC\WinampVis
mkdir Distro_SRC\scripts
mkdir Distro_SRC\Help
mkdir Distro_SRC\Help\help.zmatrix.n3.net
mkdir Distro_SRC\Web
mkdir Distro_SRC\Web\zmatrix.n3.net
mkdir Distro_SRC\Setup

copy Readme.txt Distro_SRC
copy SourceCodeReadme.txt Distro_SRC
copy LICENSE.TXT Distro_SRC

copy CreateDistro.bat Distro_SRC
copy CreateDistro9x.bat Distro_SRC
copy CreateDistroNT.bat Distro_SRC
copy CreateDistroSRC.bat Distro_SRC

copy default.cfg Distro_SRC

copy globals.cpp Distro_SRC
copy globals.h Distro_SRC
copy matrix.cpp Distro_SRC
copy matrix.dsp Distro_SRC
copy matrix.dsw Distro_SRC
copy matrix.rc Distro_SRC
copy mmgr.cpp Distro_SRC
copy mmgr.h Distro_SRC
copy nommgr.h Distro_SRC
copy mem_manager_readme.txt Distro_SRC
copy RegistryListenerThread.cpp Distro_SRC
copy RegistryListenerThread.h Distro_SRC
copy resource.h Distro_SRC
copy TopLevelListenerWindow.cpp Distro_SRC
copy TopLevelListenerWindow.h Distro_SRC

copy ZMatrix.ico Distro_SRC

copy "Matrix Code Font.ttf" Distro_SRC
copy "Matrix Code Font ReadMe.txt" Distro_SRC
copy JapaneseSet.txt Distro_SRC
copy MatrixCodeFontSet.txt Distro_SRC

copy Config\AboutUnit.cpp Distro_SRC\Config
copy Config\AboutUnit.dfm Distro_SRC\Config
copy Config\AboutUnit.h Distro_SRC\Config
copy Config\CharSetForm.cpp Distro_SRC\Config
copy Config\CharSetForm.dfm Distro_SRC\Config
copy Config\CharSetForm.h Distro_SRC\Config
copy Config\comments.html Distro_SRC\Config
copy Config\Config.bpr Distro_SRC\Config
copy Config\Config.cpp Distro_SRC\Config
copy Config\ConfigForm.cpp Distro_SRC\Config
copy Config\ConfigForm.dfm Distro_SRC\Config
copy Config\ConfigForm.h Distro_SRC\Config
copy Config\ConfigResources.rc Distro_SRC\Config
copy Config\default.bmk Distro_SRC\Config
copy Config\happy_dude.wav Distro_SRC\Config
copy Config\hire.html Distro_SRC\Config
copy Config\HireFormImage.bmp Distro_SRC\Config
copy Config\HireUnit.cpp Distro_SRC\Config
copy Config\HireUnit.dfm Distro_SRC\Config
copy Config\HireUnit.h Distro_SRC\Config
copy Config\me.bmp Distro_SRC\Config
copy Config\resource.h Distro_SRC\Config

copy zsMatrix\IzsMatrix.h Distro_SRC\zsMatrix
copy zsMatrix\resource.h Distro_SRC\zsMatrix
copy zsMatrix\zsMatrix.cpp Distro_SRC\zsMatrix
copy zsMatrix\zsMatrix.dsp Distro_SRC\zsMatrix
copy zsMatrix\zsMatrix.h Distro_SRC\zsMatrix
copy zsMatrix\zsMatrix.rc Distro_SRC\zsMatrix
copy zsMatrix\zsMatrix.def Distro_SRC\zsMatrix

copy MsgHook\MsgHook.h Distro_SRC\MsgHook
copy MsgHook\MsgHook.cpp Distro_SRC\MsgHook
copy MsgHook\MsgHook.rc Distro_SRC\MsgHook
copy MsgHook\resource.h Distro_SRC\MsgHook
copy MsgHook\MsgHook.dsp Distro_SRC\MsgHook

copy ScreenSaver\ZMatrixSS.cpp Distro_SRC\ScreenSaver
copy ScreenSaver\ZMatrixSS.rc Distro_SRC\ScreenSaver
copy ScreenSaver\resource.h Distro_SRC\ScreenSaver
copy ScreenSaver\ZMatrix.ico Distro_SRC\ScreenSaver
copy ScreenSaver\ZMatrixSS.ini Distro_SRC\ScreenSaver
copy ScreenSaver\ZMatrixSS.dsp Distro_SRC\ScreenSaver

copy WinampVis\main.cpp Distro_SRC\WinampVis
copy WinampVis\vis.h Distro_SRC\WinampVis
copy WinampVis\vis_zmx.bpf Distro_SRC\WinampVis
copy WinampVis\vis_zmx.bpr Distro_SRC\WinampVis
copy WinampVis\vis_zmx.cpp Distro_SRC\WinampVis
copy WinampVis\vis_zmx.h Distro_SRC\WinampVis
copy WinampVis\vis_zmx.res Distro_SRC\WinampVis
copy WinampVis\zmxCommonConfigUnit.cpp Distro_SRC\WinampVis
copy WinampVis\zmxCommonConfigUnit.dfm Distro_SRC\WinampVis
copy WinampVis\zmxCommonConfigUnit.h Distro_SRC\WinampVis
copy WinampVis\zmxFreqModulateModule.cpp Distro_SRC\WinampVis
copy WinampVis\zmxFreqModulateModule.h Distro_SRC\WinampVis
copy WinampVis\zmxVUModulateModule.cpp Distro_SRC\WinampVis
copy WinampVis\zmxVUModulateModule.h Distro_SRC\WinampVis

copy Setup\ZMatrix.iss Distro_SRC\Setup
copy Setup\ZMatrixVis.psc Distro_SRC\Setup

xcopy scripts\* Distro_SRC\scripts\ /S /exclude:GeneralXCopyExclusions.txt

xcopy Help\help.zmatrix.n3.net\* Distro_SRC\Help\help.zmatrix.n3.net\ /S /exclude:GeneralXCopyExclusions.txt

xcopy Web\zmatrix.n3.net\* Distro_SRC\Web\zmatrix.n3.net\ /S /exclude:GeneralXCopyExclusions.txt


cd Distro_SRC
zip -r ..\ZMatrixSRC.zip *
cd ..

if not "%1" == "" (
	for /F "tokens=1-3 delims=. " %%i in ("%1") do (
		if not "%%i" == "" (
		set MajorVer=%%i
		) else (
		set MajorVer=0
		)
		if not "%%j" == "" (
		set MinorVer=%%j
		) else (
		set MinorVer=0
		)
		if not "%%k" == "" (
		set ReleaseVer=%%k
		) else (
		set ReleaseVer=0
		)
	)
) else (
	for /F "tokens=1-4 delims=/ " %%i in ('date /t') do (
		set DayOfWeek=%%i
		set Month=%%j
		set Day=%%k
		set Year=%%l
	)
)

if not "%1" == "" (
	move ZMatrixSRC.zip ZMatrixSRC_%MajorVer%_%MinorVer%_%ReleaseVer%.zip
) else (
	move ZMatrixSRC.zip ZMatrixSRC-%Year%-%Month%-%Day%.zip
)


echo 
