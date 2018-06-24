
$work_dir = "$(Get-Location)\ext"

Add-Type -AssemblyName System.IO.Compression.FileSystem

iex "git submodule update --init --recursive"

$qscintilla_file = "QScintilla_gpl-2.10.6.zip"
$qscintilla_url = "https://sourceforge.net/projects/pyqt/files/QScintilla2/QScintilla-2.10.6/QScintilla_gpl-2.10.6.zip"

echo $qscintilla_url

$AllProtocols = [System.Net.SecurityProtocolType]'Tls,Tls11,Tls12'
[System.Net.ServicePointManager]::SecurityProtocol = $AllProtocols

if (-not (Test-Path $work_dir\$($qscintilla_file))) {
    Invoke-WebRequest -Uri $qscintilla_url -OutFile "$($work_dir)\$($qscintilla_file)" -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::FireFox
}

[System.IO.Compression.ZipFile]::ExtractToDirectory("$($work_dir)\$($qscintilla_file)", "$($work_dir)")

Rename-Item "$($work_dir)\QScintilla_gpl-2.10.6" "$($work_dir)\QScintilla_gpl"

Set-Location "$($work_dir)\QScintilla_gpl\Qt4Qt5"

iex "qmake"
iex "nmake"