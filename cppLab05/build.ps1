param([string]$Compiler = 'auto')

Write-Host "== Maze GUI Build Script =="

function Has-Cmd($name){ return (Get-Command $name -ErrorAction SilentlyContinue) -ne $null }

if ($Compiler -eq 'auto') {
    if (Has-Cmd 'cl.exe') { $Compiler = 'msvc' }
    elseif (Has-Cmd 'g++.exe') { $Compiler = 'mingw' }
    else { $Compiler = 'none' }
}

switch ($Compiler) {
    'msvc' {
        Write-Host "[*] Using MSVC cl.exe"
        cl /nologo /EHsc /DUNICODE /D_UNICODE main.cpp maze.cpp user32.lib gdi32.lib /Fe:MazeWin.exe /link /SUBSYSTEM:WINDOWS
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
        'mingw' {
        Write-Host "[*] Using MinGW g++"
            g++ -DUNICODE -D_UNICODE -municode -O2 -Wall -Wextra -mwindows main.cpp maze.cpp -lgdi32 -luser32 -o MazeWin.exe
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    default {
        Write-Host "No C++ compiler found." -ForegroundColor Red
        Write-Host "Install Visual Studio Build Tools (cl.exe) or MinGW-w64 (g++)."
        exit 1
    }
}

if (Test-Path .\MazeWin.exe) {
    Write-Host "Build OK: MazeWin.exe"
}
