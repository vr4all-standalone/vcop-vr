param([string]$RomDirectory = (Join-Path $PSScriptRoot 'roms'))
$ErrorActionPreference='Stop'
$taskRoot=$PSScriptRoot
if(!(Test-Path -LiteralPath (Join-Path $taskRoot 'vcop.exe'))) {$taskRoot=Split-Path -Parent $PSScriptRoot}
$taskRom=[IO.Path]::GetFullPath($RomDirectory)
foreach($taskFile in @('vcop.zip')) {
    if(!(Test-Path -LiteralPath (Join-Path $taskRom $taskFile) -PathType Leaf)) {throw "Missing owned game file: $taskRom\$taskFile"}
}
Push-Location -LiteralPath $taskRoot
try {
    & (Join-Path $taskRoot 'vcop.exe') -rompath $taskRom -verifyroms vcop
    if($LASTEXITCODE){throw 'MAME rejected the game data. Use the complete Virtua Cop Model 2 arcade set plus the Model1IO2 I/O firmware and HD44780 LCD font.'}
    $taskData=Join-Path $taskRoot 'userdata'
    foreach($taskDirectory in @('nvram','cfg','diff','state','snap')) {New-Item -ItemType Directory -Path (Join-Path $taskData $taskDirectory) -Force | Out-Null}
    $env:VCOP_VR='1'
    $env:VCOP_XR_LIBRARY=Join-Path $taskRoot 'vcop_xr.dll'
    $env:VCOP_ASSETS=Join-Path $taskRoot 'assets'
    $env:VCOP_SETTINGS=Join-Path $taskData 'settings.ini'
    $env:VCOP_XR_LOG=Join-Path $taskData 'vcop-xr.log'
    & (Join-Path $taskRoot 'vcop.exe') vcop -rompath $taskRom -nvram_directory (Join-Path $taskData 'nvram') `
        -cfg_directory (Join-Path $taskData 'cfg') -diff_directory (Join-Path $taskData 'diff') `
        -state_directory (Join-Path $taskData 'state') -snapshot_directory (Join-Path $taskData 'snap') `
        -video none -sound auto -skip_gameinfo -noreadconfig
    if(Test-Path -LiteralPath $env:VCOP_XR_LOG) {Get-Content -LiteralPath $env:VCOP_XR_LOG -Tail 15}
} finally {Pop-Location}
