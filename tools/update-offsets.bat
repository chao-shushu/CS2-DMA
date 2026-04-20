@echo off
chcp 65001 >nul 2>&1
setlocal enabledelayedexpansion

:: ========================================
::  CS2 Offset Updater - Batch Version
:: ========================================

set "dumperDir=%~dp0..\external\dumper"
set "dumperExe=%dumperDir%\target\release\cs2-dumper.exe"
set "outputDir=%dumperDir%\output"
set "rootDir=%~dp0.."
set "ProcessName=cs2.exe"
set "connector="
set "connectorArgs="

:: ========================================
::  Main Menu
:: ========================================
:menu
cls
echo.
echo ========================================
echo        CS2 Offset Dumper
echo ========================================
echo.
echo   1. Native Mode (Local Memory)
echo      - Run on the same machine as CS2
echo      - Requires Administrator privileges
echo.
echo   2. DMA Mode (PCILeech/FPGA)
echo      - Use FPGA hardware to read memory
echo      - Requires FPGA device connected
echo.
echo   3. Exit
echo.
echo ----------------------------------------
set /p choice="Select mode [1-3]: "

if "%choice%"=="1" goto native_mode
if "%choice%"=="2" goto dma_mode
if "%choice%"=="3" goto :eof
echo [X] Invalid choice!
timeout /t 2 >nul
goto menu

:: ========================================
::  Native Mode
:: ========================================
:native_mode
cls
echo.
echo ========================================
echo   Native Mode (Local Memory)
echo ========================================
echo.
call :check_admin
if errorlevel 1 goto menu_after_error
echo [*] Mode: Native (local memory)
echo [*] Process: %ProcessName%
echo [*] Output: %outputDir%
echo.
goto run_dumper

:: ========================================
::  DMA Mode
:: ========================================
:dma_mode
cls
echo.
echo ========================================
echo   DMA Mode (PCILeech/FPGA)
echo ========================================
echo.
echo [*] FPGA device must be connected
echo.
set "dma_device="
set /p dma_device="Enter FPGA device [default: FPGA]: "
if "%dma_device%"=="" set "dma_device=FPGA"
set "connectorArgs=:device=%dma_device%"
set "connector=pcileech"
echo.
echo [*] Mode: DMA (pcileech)
echo [*] Args: %connectorArgs%
echo [*] Process: %ProcessName%
echo [*] Output: %outputDir%
echo.
goto run_dumper

:: ========================================
::  Check Administrator
:: ========================================
:check_admin
net session >nul 2>&1
if errorlevel 1 (
    echo [X] ERROR: Administrator privileges required!
    echo [?] Right-click this script and select 'Run as administrator'
    pause
    exit /b 1
)
echo [OK] Running as Administrator
exit /b 0

:: ========================================
::  Run cs2-dumper
:: ========================================
:run_dumper

:: Check if cs2-dumper.exe exists
if exist "%dumperExe%" goto :dumper_found
echo [!] cs2-dumper.exe not found, building...
pushd "%dumperDir%"
cargo build --release
if errorlevel 1 (
    echo [X] Build failed!
    popd
    goto menu_after_error
)
popd
echo [OK] Build successful
echo.

:dumper_found
echo [*] Running cs2-dumper...
echo.

if "%connector%"=="pcileech" goto run_dma
goto run_native

:run_native
"%dumperExe%" -p "%ProcessName%" -f json -o "%outputDir%" -vv
goto check_result

:run_dma
"%dumperExe%" -c pcileech -a "%connectorArgs%" -p "%ProcessName%" -f json -o "%outputDir%" -vv
goto check_result

:check_result
if errorlevel 1 goto dumper_failed
echo [OK] cs2-dumper completed
echo.
goto copy_files

:dumper_failed
echo [X] cs2-dumper failed!
echo.
echo [?] Check:
echo     - CS2 is running (main menu is enough)
echo     - Running as Administrator
if "%connector%"=="pcileech" echo     - FPGA device is connected
goto menu_after_error

:: ========================================
::  Copy offset files
:: ========================================
:copy_files
set copied=0

:: Copy offsets.json
if not exist "%outputDir%\offsets.json" goto missing_offsets
if exist "%rootDir%\data\offsets.json" copy "%rootDir%\data\offsets.json" "%rootDir%\data\offsets.json.bak" >nul 2>&1
copy "%outputDir%\offsets.json" "%rootDir%\data\offsets.json" >nul 2>&1
set /a copied+=1
echo [OK] Copied: offsets.json
goto copy_client

:missing_offsets
echo [!] Missing: offsets.json

:copy_client
:: Copy client_dll.json
if not exist "%outputDir%\client_dll.json" goto missing_client
if exist "%rootDir%\data\client_dll.json" copy "%rootDir%\data\client_dll.json" "%rootDir%\data\client_dll.json.bak" >nul 2>&1
copy "%outputDir%\client_dll.json" "%rootDir%\data\client_dll.json" >nul 2>&1
set /a copied+=1
echo [OK] Copied: client_dll.json
goto gen_version

:missing_client
echo [!] Missing: client_dll.json
goto gen_version

:gen_version
:: Generate version.json from dumper info.json
if not exist "%outputDir%\info.json" goto show_info
echo [*] Generating version.json from info.json...
powershell -NoProfile -Command "$info = Get-Content '%outputDir%\info.json' | ConvertFrom-Json; $ts = [DateTimeOffset]::Parse($info.timestamp).ToUnixTimeSeconds(); $date = $info.timestamp.Substring(0,10); @{game_update_date=$date; game_update_timestamp=$ts} | ConvertTo-Json | Set-Content '%rootDir%\data\version.json' -Encoding UTF8"
if errorlevel 1 (
    echo [!] Failed to generate version.json
) else (
    echo [OK] Copied: version.json
    set /a copied+=1
)

:show_info
:: Show version info
if not exist "%outputDir%\info.json" goto done
echo.
echo [*] Build info:
for /f "tokens=2 delims=:" %%a in ('findstr "build_number" "%outputDir%\info.json" 2^>nul') do echo     Build:%%a
for /f "tokens=2 delims=:" %%a in ('findstr "timestamp" "%outputDir%\info.json" 2^>nul') do echo     Time:%%a

:done
echo.
if %copied%==3 (
    echo [OK] All offset files updated successfully!
) else (
    echo [!] Some files were not copied (%copied%/3)
)
echo.
echo Press any key to return to menu...
pause >nul
goto menu

:: ========================================
::  Error handler - return to menu
:: ========================================
:menu_after_error
echo.
echo Press any key to return to menu...
pause >nul
goto menu
