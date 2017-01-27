 @echo off
 
 set IgnoreWarnings=-wd4201 -wd4204 -wd4127
 set CommonCompilerFlags=-nologo -MTd -fp:fast -Gm- -GR- -EHa -WX -Oi -W4 -FC %IgnoreWarnings% -IE:\Documents\Coding\C\h -DINTERNAL=1 -DLIVE_DEV=1
 REM set DebugCompilerFlags=-O2 -Z7 
REM use argument for optimisation level
set DebugCompilerFlags=%1
REM -Z7
 set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib
 
 IF NOT EXIST E:\Documents\Coding\C\build mkdir E:\Documents\Coding\C\build
 pushd E:\Documents\Coding\C\build
 
 call E:\Documents\Coding\C\shell64.bat
 
 del *.pdb > NUL 2> NUL
 echo WAITING FOR PDB > lock.tmp
 
timethis cl %CommonCompilerFlags% %DebugCompilerFlags% E:\Documents\Coding\C\twinstick\twinstick.c -Fmtwinstick.map -LD /link %CommonLinkerFlags% -PDB:twinstick%random%.pdb -EXPORT:UpdateAndRender
 del lock.tmp
 
 timethis cl %CommonCompilerFlags% %DebugCompilerFlags% E:\Documents\Coding\C\twinstick\win32_twinstick.c -Fmwin32_twinstick.map /link %CommonLinkerFlags%
 
 echo Finished at %time%
 REM ./win32_twinstick.exe
 popd
 
 
 REM Flag Meanings:
 REM ==============
 REM
 REM -nologo	- no Microsoft logo at the beginning of compilation
 REM -Od		- no optimisation of code at all
 REM -Oi		- use intrinsic version of function if exists
 REM -Z7		- compatible debug info for debugger (replaced -Zi)
 REM -GR-		- turn off runtime type info (C++)
 REM -EHa-		- turn off exception handling (C++)
 REM -W4		- 4th level of warnings
 REM -WX 		- treat warnings as errors
 REM -wd#### 	- remove warning ####
 REM -D#####	- #define #### (=1)
 REM -Gm-		- turn off 'minimal rebuild' - no incremental build
 REM -Fm####	- provides location for compiler to put a .map file
 REM -I####		- search for include files at ####
 REM
 REM -MTd		- use (d => debug version of) static CRT library - needed for running on XP
 REM /link -subsystem:windows,5.1 - ONLY FOR 32-BIT BUILDS!!! - needed for running on XP
 
 REM Warnings Removed:
 REM =================
 REM
 REM C4201: nonstandard extension used: nameless struct/union
 REM C4100: unreferenced formal parameter
 REM C4189: local variable is initialized but not referenced
 REM C4204: nonstandard extension used: non-constant aggregate initializer
