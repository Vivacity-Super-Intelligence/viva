param([Parameter(Mandatory)][string]$OpenOcd,[Parameter(Mandatory)][string]$ProbeSerial)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
$binary = (Join-Path $repoRoot 'build\stm32g491re\zephyr\zephyr.elf').Replace('\','/')
& $OpenOcd -f interface/stlink.cfg -c "adapter serial $ProbeSerial" -f target/stm32g4x.cfg -c "program {$binary} verify reset exit"
if ($LASTEXITCODE -ne 0) { throw 'STM32 flash failed' }
