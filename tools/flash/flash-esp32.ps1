param([Parameter(Mandatory)][string]$Port)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
$buildRoot = Join-Path $repoRoot 'build\esp32s3'
$flashData = Get-Content -LiteralPath (Join-Path $buildRoot 'flasher_args.json') -Raw | ConvertFrom-Json
# esptool verifies the chip before writing. The port must have been identified first.
$flashArgs = @('-m','esptool','--chip','esp32s3','--port',$Port,'--baud','460800','write-flash')
foreach ($entry in $flashData.flash_files.PSObject.Properties) {
    $flashArgs += $entry.Name
    $flashArgs += Join-Path $buildRoot $entry.Value
}
& python @flashArgs
if ($LASTEXITCODE -ne 0) { throw 'ESP32-S3 flash failed' }
