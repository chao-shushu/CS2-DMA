# CS2 Offset Updater - 使用 cs2-dumper 获取最新偏移值
# 使用方法:
#   1. 在运行 CS2 的目标机器上，以管理员权限运行此脚本（默认使用 memflow-native 本地模式）
#   2. 如需通过 DMA 硬件: .\update-offsets.ps1 -Connector pcileech -ConnectorArgs ":device=FPGA"
# 注意: 偏移值是静态的（版本相关），dump 一次即可，无需每次启动都运行

param(
    [string]$Connector = "",
    [string]$ConnectorArgs = "",
    [string]$ProcessName = "cs2.exe"
)

$ErrorActionPreference = "Stop"

$dumperDir = "$PSScriptRoot\..\external\dumper"
$dumperExe = "$dumperDir\target\release\cs2-dumper.exe"
$outputDir = "$dumperDir\output"
$rootDir   = "$PSScriptRoot\.."

# 检查 cs2-dumper 是否已编译
if (-not (Test-Path $dumperExe)) {
    Write-Host "[!] cs2-dumper.exe not found, building..." -ForegroundColor Yellow
    Push-Location $dumperDir
    cargo build --release
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[X] Build failed!" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    Pop-Location
    Write-Host "[OK] Build successful" -ForegroundColor Green
}

# 运行 cs2-dumper
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  CS2 Offset Dumper (DMA/PCILeech)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
if ($Connector) {
    Write-Host "[*] Mode: DMA ($Connector)" -ForegroundColor Gray
    Write-Host "[*] Args: $ConnectorArgs" -ForegroundColor Gray
} else {
    Write-Host "[*] Mode: Native (local memory)" -ForegroundColor Gray
}
Write-Host "[*] Process: $ProcessName" -ForegroundColor Gray
Write-Host "[*] Output: $outputDir" -ForegroundColor Gray
Write-Host ""

Write-Host "[*] Running cs2-dumper..." -ForegroundColor Yellow
if ($Connector) {
    & $dumperExe -c $Connector -a $ConnectorArgs -p $ProcessName -f json -o $outputDir -vv
} else {
    & $dumperExe -p $ProcessName -f json -o $outputDir -vv
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "[X] cs2-dumper failed! (exit code: $LASTEXITCODE)" -ForegroundColor Red
    Write-Host "[?] Check:" -ForegroundColor Yellow
    Write-Host "    - CS2 is running (main menu is enough)" -ForegroundColor Yellow
    Write-Host "    - Running as Administrator" -ForegroundColor Yellow
    if ($Connector) {
        Write-Host "    - FPGA device is connected" -ForegroundColor Yellow
    }
    exit 1
}

Write-Host "[OK] cs2-dumper completed" -ForegroundColor Green

# 复制偏移文件到项目根目录
$filesToCopy = @("offsets.json", "client_dll.json")
$copied = 0

foreach ($file in $filesToCopy) {
    $src = Join-Path $outputDir $file
    $dst = Join-Path $rootDir "data" $file

    if (Test-Path $src) {
        # 备份旧文件
        if (Test-Path $dst) {
            $backupName = "$file.bak"
            Copy-Item $dst (Join-Path $rootDir $backupName) -Force
        }
        Copy-Item $src $dst -Force
        $copied++
        Write-Host "[OK] Copied: $file" -ForegroundColor Green
    } else {
        Write-Host "[!] Missing: $src" -ForegroundColor Yellow
    }
}

# 显示 info.json 中的版本信息
$infoPath = Join-Path $outputDir "info.json"
if (Test-Path $infoPath) {
    $info = Get-Content $infoPath | ConvertFrom-Json
    Write-Host ""
    Write-Host "[*] Build Number: $($info.build_number)" -ForegroundColor Cyan
    Write-Host "[*] Timestamp: $($info.timestamp)" -ForegroundColor Cyan
}

Write-Host ""
if ($copied -eq $filesToCopy.Count) {
    Write-Host "[OK] All offset files updated successfully!" -ForegroundColor Green
} else {
    Write-Host "[!] Some files were not copied ($copied/$($filesToCopy.Count))" -ForegroundColor Yellow
}
