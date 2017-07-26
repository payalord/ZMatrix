; Pimpscript file v1.3.0
; Copyright (C) 1999 Nullsoft Inc.
; You can comment by starting lines with ; or #.

; Variables you can use in many of the strings:
;   $PROGRAMFILES (usually c:\program files)
;   $WINDIR (usually c:\windows)
;   $SYSDIR (usually c:\windows\system)
;   $DESKTOP (the desktop directory for the current user)
;   $INSTDIR (whatever the install directory ends up being)
;   $VISDIR  (visualization plug-in directory. DO NOT USE IN DefaultDir)
;   $DSPDIR  (dsp plug-in directory. DO NOT USE IN DefaultDir)

; Name specifies what the installer will use for its name
Name ZMatrix Winamp Visualization

; Text specifies what text will appear in the dialog window
Text This is the Winamp visualization module for ZMatrix.  See http://zmatrix.n3.net for more information.

; Specifies where to write the installer executable to
OutFile ..\DistroVis\Output\ZMatrixVizModule_1_5_2.exe

; DefaultDir lets you specify the default installation directory 
; (if you omit DefaultDir, it will default to the Winamp directory,
; determined at install time)

;DefaultDir $PROGRAMFILES\MyProgram

; You can add a line that just says Silent to make the installer silent.
Silent


; File listing section
; --------------------


; SetOutPath specifies the relative output Path
; Or, you can use any of the variables (see DefaultDir), or start with a \ to 
; specify the root directory of the install drive.
; You can use 
; SetOutPath -
; or 
; SetOutPath $INSTDIR
; to set the output directory to the install directory
SetOutPath $VISDIR

; Any number of AddFile's can follow. Wildcards (in limited form) can be used.
AddFile ..\DistroVis\vis_zmx.dll



; Post-install execute section
; ----------------------------

; ExecFile will execute a command after a successful installation.
; Note that it must list an executable. You can specify a parameter, 
; though. For example, to open a text file: 
;ExecFile "$WINDIR\notepad.exe" readme.txt



; end
