@echo off
chcp 936 >nul
echo ========================================
echo   QtStock - Windows Build Script
echo   Qt 5.14.2 + MinGW 7.3 32bit + CMake 3.14
echo ========================================
echo.

set QT_DIR=D:\Qt\Qt5.14.2\5.14.2\mingw73_32
set MINGW_DIR=D:\Qt\Qt5.14.2\Tools\mingw730_32\bin
set LOGFILE=build_log.txt
set QT_DIR_FWD=D:/Qt/Qt5.14.2/5.14.2/mingw73_32
set MINGW_DIR_FWD=D:/Qt/Qt5.14.2/Tools/mingw730_32/bin

REM 移除已知cygwin/msys2路径(硬编码常见安装路径)
set "PATH=%PATH:D:\Program Files\cygwin64\bin;=%"
set "PATH=%PATH:D:\Program Files\cygwin64\bin=%"
set "PATH=%PATH:C:\cygwin64\bin;=%"
set "PATH=%PATH:C:\cygwin64\bin=%"
set "PATH=%PATH:C:\cygwin\bin;=%"
set "PATH=%PATH:C:\cygwin\bin=%"
set "PATH=%PATH:C:\msys64\usr\bin;=%"
set "PATH=%PATH:C:\msys64\usr\bin=%"
set "PATH=%PATH:C:\msys32\usr\bin;=%"
set "PATH=%PATH:C:\msys32\usr\bin=%"

REM 动态检测sh.exe并移除其所在目录(cygwin/msys2/Git Bash都会干扰MinGW)
for /f "tokens=*" %%i in ('where sh.exe 2^>nul') do (
    call set "PATH=%%PATH:%%~dpi=%%"
)

set "PATH=%MINGW_DIR%;%QT_DIR%\bin;%PATH%"

REM 验证sh.exe是否已从PATH移除
where sh.exe >nul 2>&1
if not errorlevel 1 (
    echo [WARNING] sh.exe still in PATH! MinGW Makefiles may fail.
    echo   sh.exe location:
    where sh.exe
    echo.
)

if not exist "%QT_DIR%\lib\cmake\Qt5\Qt5Config.cmake" (
    echo [ERROR] Qt5 not found at %QT_DIR%
    pause
    exit /b 1
)

if not exist "%MINGW_DIR%\gcc.exe" (
    echo [ERROR] MinGW gcc not found at %MINGW_DIR%
    pause
    exit /b 1
)

echo   Compiler: %MINGW_DIR%\gcc.exe
gcc --version | findstr "gcc"
echo.

if exist build rmdir /s /q build

echo [1/6] CMake configure...
echo === Configure === > %LOGFILE%
cmake -G "MinGW Makefiles" -B build -S . -DCMAKE_PREFIX_PATH="%QT_DIR_FWD%" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="%MINGW_DIR_FWD%/gcc.exe" -DCMAKE_CXX_COMPILER="%MINGW_DIR_FWD%/g++.exe" >> %LOGFILE% 2>&1
if errorlevel 1 ( echo [ERROR] Configure failed! & type %LOGFILE% & pause & notepad %LOGFILE% & exit /b 1 )
echo   OK

echo [2/6] Build...
echo === Build === >> %LOGFILE%
cmake --build build >> %LOGFILE% 2>&1
if errorlevel 1 ( echo [ERROR] Build failed! & type %LOGFILE% & pause & notepad %LOGFILE% & exit /b 1 )
echo   OK

echo [3/6] Deploy Qt dependencies (windeployqt)...
echo === Deploy windeployqt === >> %LOGFILE%
set WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe
if exist "%WINDEPLOYQT%" (
    "%WINDEPLOYQT%" --release --no-translations --no-quick-import build\QtStock.exe >> %LOGFILE% 2>&1
    echo   windeployqt done
) else (
    echo   [WARN] windeployqt not found, will use manual deploy
)

echo [4/6] Ensure critical DLLs exist (manual verify + copy)...
echo === Deploy manual === >> %LOGFILE%
REM 显式复制所有关键Qt DLL(双重保险,即使windeployqt遗漏也能补上)
for %%f in (Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll Qt5Network.dll Qt5Charts.dll Qt5Svg.dll libGLESV2.dll libEGL.dll D3DCompiler_47.dll opengl32sw.dll) do (
    if not exist "build\%%f" (
        if exist "%QT_DIR%\bin\%%f" (
            copy "%QT_DIR%\bin\%%f" build\ >nul 2>&1
            echo   [COPIED] %%f
        ) else (
            echo   [MISSING] %%f not found in Qt bin!
        )
    )
)
REM Qt插件
if not exist build\platforms mkdir build\platforms
if not exist "build\platforms\qwindows.dll" (
    if exist "%QT_DIR%\plugins\platforms\qwindows.dll" copy "%QT_DIR%\plugins\platforms\qwindows.dll" build\platforms\ >nul 2>&1
    echo   [COPIED] platforms\qwindows.dll
)
if not exist build\imageformats mkdir build\imageformats
if not exist "build\imageformats\qjpeg.dll" (
    if exist "%QT_DIR%\plugins\imageformats\qjpeg.dll" copy "%QT_DIR%\plugins\imageformats\qjpeg.dll" build\imageformats\ >nul 2>&1
)
if not exist build\styles mkdir build\styles
if exist "%QT_DIR%\plugins\styles\qwindowsvistastyle.dll" (
    if not exist "build\styles\qwindowsvistastyle.dll" copy "%QT_DIR%\plugins\styles\qwindowsvistastyle.dll" build\styles\ >nul 2>&1
)
REM MinGW运行时DLL
for %%f in (libgcc_s_dw2-1.dll libstdc++-6.dll libwinpthread-1.dll) do (
    if not exist "build\%%f" (
        if exist "%MINGW_DIR%\%%f" copy "%MINGW_DIR%\%%f" build\ >nul 2>&1
    )
)
REM 复制CSV数据文件
if not exist build\data mkdir build\data
if exist "data\stocks.csv" copy "data\stocks.csv" build\data\ >nul 2>&1
echo   OK

echo [5/6] Verify deployment...
echo === Verify === >> %LOGFILE%
set DEPLOY_OK=1
for %%f in (Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll Qt5Network.dll Qt5Charts.dll libgcc_s_dw2-1.dll libstdc++-6.dll libwinpthread-1.dll) do (
    if exist "build\%%f" (
        echo   [OK] %%f
    ) else (
        echo   [FAIL] %%f MISSING!
        set DEPLOY_OK=0
    )
)
if exist "build\platforms\qwindows.dll" (
    echo   [OK] platforms\qwindows.dll
) else (
    echo   [FAIL] platforms\qwindows.dll MISSING!
    set DEPLOY_OK=0
)
echo.

echo [6/6] Create portable ZIP package...
echo === Package ZIP === >> %LOGFILE%
if exist QtStock.zip del QtStock.zip
REM 使用PowerShell Compress-Archive创建zip(兼容性最好,Windows压缩文件夹可识别)
powershell -Command "Compress-Archive -Path 'build\*' -DestinationPath 'QtStock.zip' -Force" >> %LOGFILE% 2>&1
if exist QtStock.zip (
    echo   [OK] QtStock.zip created in project root
    for %%F in (QtStock.zip) do echo   Size: %%~zF bytes
) else (
    echo   [FAIL] Failed to create QtStock.zip
)

echo ========================================
if exist "build\QtStock.exe" (
    if %DEPLOY_OK%==1 (
        echo   BUILD SUCCESS: build\QtStock.exe
        echo   All critical DLLs verified.
        echo   ZIP包: QtStock.zip (开箱即用,在项目根目录)
        echo   Log: %LOGFILE%
        echo   运行日志: build\qtstock.log
        echo   如无法启动, 运行 check_deploy.bat 诊断
    ) else (
        echo   [WARNING] build\QtStock.exe built but some DLLs missing!
        echo   运行 deploy.bat 重新部署, 或 check_deploy.bat 诊断
    )
) else (
    echo   [ERROR] Output not found!
    type %LOGFILE%
)
echo ========================================
pause
