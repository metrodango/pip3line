param (
    [Parameter(Mandatory=$true)][string]$qtdir,
    [Parameter(Mandatory=$true)][string]$version,
    [Parameter(Mandatory=$true)][string]$openssldir
)

if (-not (Test-Path $qtdir)) {
    Write-Output "QT dir cannot be found => $($qtdir)"
    Exit 1
} else {
    Write-Output "QT dir => $($qtdir)"
}

if (-not (Test-Path $openssldir)) {
    Exit 1
} else {
    Write-Output "OpenSSL dir => $($openssldir)"
}

Write-Output "Visual Studio selected => $($version)"

$packagedir =  "$($PSScriptRoot)\package"
if (-not (Test-Path $packagedir)) {
     Write-Output "Creating package dir"
     New-Item -ItemType directory -Path $packagedir
}

Write-Output "Copying base files"

Copy-Item -Path "$($PSScriptRoot)\lib\transform.dll" -Destination "$($packagedir)"
Copy-Item -Path "$($PSScriptRoot)\bin\pip3line.exe" -Destination "$($packagedir)"
Copy-Item -Path "$($PSScriptRoot)\bin\pip3linecmd.exe" -Destination "$($packagedir)"

# missing librarY (??)
Copy-Item -Path "$($qtdir)\bin\Qt5PrintSupport.dll" -Destination "$($packagedir)"

Write-Output "Copying distorm lib"

if (-not (Test-Path "$($PSScriptRoot)\ext\distorm")) {
    Write-Output "Distorm dir cannot be found => $($PSScriptRoot)\ext\distorm"
    Exit 1
}
Copy-Item -Path "$($PSScriptRoot)\ext\distorm\distorm3.dll" -Destination "$($packagedir)"

Write-Output "Copying QScintilla lib"

if (-not (Test-Path "$($PSScriptRoot)\ext\QScintilla_gpl\Qt4Qt5\release")) {
    Write-Output "Distorm dir cannot be found => $($PSScriptRoot)\ext\QScintilla_gpl\Qt4Qt5\release"
    Exit 1
}
Copy-Item -Path "$($PSScriptRoot)\ext\QScintilla_gpl\Qt4Qt5\release\qscintilla2_qt5.dll" -Destination "$($packagedir)"

Write-Output "Copying plugins libs"

if (-not (Test-Path "$($packagedir)\plugins")) {
     Write-Output "Creating package plugins dir"
     New-Item -ItemType directory -Path "$($packagedir)\plugins"
}
Copy-Item -Path "$($PSScriptRoot)\bin\plugins\*.dll"  -Destination "$($packagedir)\plugins"

Write-Output "Copying OpenSSL libs"

Copy-Item -Path "$($openssldir)\bin\*.dll"  -Destination "$($packagedir)"

Write-Output "Copying extras"

if (-not (Test-Path "$($packagedir)\extras")) {
     Write-Output "Creating extras dir"
     New-Item -ItemType directory -Path "$($packagedir)\extras"
}
Get-ChildItem -Path "$($PSScriptRoot)\extras" -Recurse | Copy-Item -Destination "$($packagedir)\extras" -Force

Write-Output "Running windeployqt"

Push-Location -Path  "package"

$plugins = Get-ChildItem -Path "plugins" -Name | ForEach-Object {"plugins\$_"}

Write-Output "$($plugins)"

iex "$($qtdir)\bin\windeployqt.exe --release pip3line.exe transform.dll $($plugins)"

Pop-Location
