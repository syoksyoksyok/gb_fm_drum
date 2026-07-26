@echo off
setlocal

if "%GBDK_HOME%"=="" (
  echo GBDK_HOME is not set. Set it to your GBDK-2020 directory.
  exit /b 1
)

if not exist build mkdir build

set LCC=%GBDK_HOME%\bin\lcc.exe
if not exist "%LCC%" (
  set LCC=%GBDK_HOME%\bin\lcc
)

for %%f in (src\*.c) do (
  "%LCC%" -Iinclude -c -o build\%%~nf.o %%f
  if errorlevel 1 exit /b 1
)

"%LCC%" -Wm-yt0x1B -Wm-yo4 -Wm-ya4 -Wm-ynFMDRUMTRACKER -o build\fm_drum_tracker.gb build\audio.o build\font.o build\input.o build\main.o build\pattern.o build\randomizer.o build\sequencer.o build\storage.o build\tracker_ui.o
if errorlevel 1 exit /b 1

echo Built build\fm_drum_tracker.gb
