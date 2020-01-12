
$work_dir = "$(Get-Location)\ext"

Add-Type -AssemblyName System.IO.Compression.FileSystem

iex "git submodule update --init --recursive"

iex "msbuild ext\distorm\make\win32\distorm.sln  /p:Configuration=dll /p:Platform=x64 /p:PlatformToolset=v141 /p:WindowsTargetPlatformVersion=10.0.17763.0"

$qscintilla_version = "2.11.4"
$qscintilla_file = "QScintilla-$($qscintilla_version).zip"
$qscintilla_url = "https://www.riverbankcomputing.com/static/Downloads/QScintilla/$($qscintilla_version)/QScintilla-$($qscintilla_version).zip"

echo $qscintilla_url

$AllProtocols = [System.Net.SecurityProtocolType]'Tls,Tls11,Tls12'
[System.Net.ServicePointManager]::SecurityProtocol = $AllProtocols

if (-not (Test-Path $work_dir\$($qscintilla_file))) {
    Invoke-WebRequest -Uri $qscintilla_url -OutFile "$($work_dir)\$($qscintilla_file)" -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::FireFox
}

[System.IO.Compression.ZipFile]::ExtractToDirectory("$($work_dir)\$($qscintilla_file)", "$($work_dir)")

Rename-Item "$($work_dir)\QScintilla-$($qscintilla_version)" "$($work_dir)\QScintilla_gpl"

Set-Location "$($work_dir)\QScintilla_gpl\Qt4Qt5"

iex "qmake"
iex "nmake"
