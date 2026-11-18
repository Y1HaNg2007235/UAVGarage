param([string]$ZigPath = 'zig')
$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path $PSScriptRoot -Parent
Push-Location -LiteralPath $taskRoot
try {
    New-Item -ItemType Directory -Force 'Build/host' | Out-Null
    $env:ZIG_GLOBAL_CACHE_DIR = Join-Path $taskRoot 'Build/zig-global'
    $env:ZIG_LOCAL_CACHE_DIR = Join-Path $taskRoot 'Build/zig-local'
    & $ZigPath c++ -std=c++03 -Wall -Wextra -Werror -O1 -I . 'Tests/sbus_tests.cpp' 'Libraries/Remote/SbusDecoder.cpp' 'Libraries/Remote/StickTarget.cpp' -o 'Build/host/sbus_tests.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Protocol/target test compilation failed' }
    & './Build/host/sbus_tests.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Protocol/target tests failed' }
    & $ZigPath c++ -std=c++03 -Wall -Wextra -Werror -O1 -I 'Tests/Stubs' -I . 'Tests/sbus_uart_tests.cpp' 'Libraries/Remote/SbusDecoder.cpp' 'Libraries/Remote/SbusUartReceiver.cpp' -o 'Build/host/sbus_uart_tests.exe'
    if ($LASTEXITCODE -ne 0) { throw 'UART test compilation failed' }
    & './Build/host/sbus_uart_tests.exe'
    if ($LASTEXITCODE -ne 0) { throw 'UART tests failed' }
} finally { Pop-Location }
