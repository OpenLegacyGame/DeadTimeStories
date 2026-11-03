param(
  [Parameter(Mandatory=$true)][string]$Portable,
  [Parameter(Mandatory=$true)][string]$Installer,
  [Parameter(Mandatory=$true)][string]$OutputDirectory
)
$ErrorActionPreference='Stop'
$ProgressPreference='SilentlyContinue'
function Require($condition,[string]$message) {if(-not $condition){throw $message}}
function Run-Checked([string]$program,[string]$arguments) {
  $process=Start-Process -FilePath $program -ArgumentList $arguments -PassThru
  if(-not $process.WaitForExit(30000)) {$process.Kill();throw "Timed out: $program"}
  $process.Refresh();Require ($process.ExitCode -eq 0) "Failed ($($process.ExitCode)): $program"
}
foreach($file in @($Portable,$Installer)){Require (Test-Path -LiteralPath $file -PathType Leaf) "Missing $file"}
Require (-not (Test-Path -LiteralPath $OutputDirectory)) 'Use a fresh verification output directory'
$output=New-Item -ItemType Directory -Path $OutputDirectory
$output=$output.FullName
$uninstallKey='HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLegacyGameDeadTimeStories'
$installKey='HKCU:\Software\OpenLegacyGame\DeadTimeStories'
Require (-not (Test-Path $installKey)) 'An existing rewrite installation must be preserved; use a separate Windows user for this check'
Require (-not (Test-Path $uninstallKey)) 'An existing uninstall registration must be preserved'
Expand-Archive -LiteralPath $Portable -DestinationPath "$output\portable"
$payload="$output\portable\DeadTimeStories"
$executable="$payload\deadtime.exe"
Require (Test-Path -LiteralPath $executable) 'Portable has no direct game executable'
$executables=@(Get-ChildItem -LiteralPath $payload -Recurse -File -Filter '*.exe')
Require ($executables.Count -eq 1) 'Portable includes an unexpected launcher, tool or test executable'
Require (@(Get-ChildItem -LiteralPath $payload -File -Filter '*.dll').Count -gt 0) 'Portable has no runtime libraries'
$bytes=[IO.File]::ReadAllBytes($executable)
$pe=[BitConverter]::ToInt32($bytes,0x3c)
Require ([BitConverter]::ToUInt16($bytes,$pe+24+68) -eq 2) 'Game still opens a console window'
foreach($archive in @('resources.pak','localization.pak')){Require (Test-Path -LiteralPath "$payload\data\$archive") "Missing $archive"}
Require (Test-Path -LiteralPath "$payload\licenses\runtime-packages.txt") 'Missing runtime version/license manifest'
$files=@(Get-ChildItem -LiteralPath $payload -Recurse -File)
$hashes=@{}
foreach($file in $files){$hashes[$file.FullName.Substring($payload.Length+1)]=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash}

$oldPath=$env:PATH;$oldVideo=$env:SDL_VIDEODRIVER;$oldAudio=$env:SDL_AUDIODRIVER
$installed="$output\安装 验证"
$profile="$output\独立用户"
$desktop=[Environment]::GetFolderPath('Desktop')
$programs=[Environment]::GetFolderPath('Programs')
Require (-not [string]::IsNullOrEmpty($desktop) -and -not [string]::IsNullOrEmpty($programs)) 'Run this verifier as a normal Windows user with Desktop and Programs folders'
$shortcuts=@("$desktop\亡灵故事.lnk","$programs\Deadtime Stories\亡灵故事.lnk")
$backups=@{}
foreach($shortcut in $shortcuts){if(Test-Path -LiteralPath $shortcut){$backups[$shortcut]=[IO.File]::ReadAllBytes($shortcut)}}
try {
  # A clean Windows search path proves the compiler's bin directory is unnecessary.
  $env:PATH="$env:SystemRoot\System32;$env:SystemRoot"
  $env:SDL_VIDEODRIVER='dummy';$env:SDL_AUDIODRIVER='dummy'
  Run-Checked $executable "--frames 3 --snapshot `"$output\portable-start.png`" --user-data `"$profile`""
  Require (Test-Path "$output\portable-start.png") 'Portable normal startup did not render'
  Run-Checked $executable "--menu --frames 3 --snapshot `"$output\portable-menu.png`""
  Run-Checked $executable "--hob zones/jessie_home/hob_plants.hob --frames 3 --snapshot `"$output\portable-hob.png`""


  # NSIS /D must be the last argument and is intentionally unquoted, including spaces.
  Run-Checked $Installer "/S /D=$installed"
  Require (Test-Path -LiteralPath "$installed\deadtime.exe") 'Installer did not produce the game'
  Require ((Get-ItemProperty $installKey).InstallDir -eq $installed) 'Wrong install registration'
  Require ((Get-ItemProperty $uninstallKey).DisplayName -eq '亡灵故事 / Deadtime Stories') 'Installer misdecoded the Chinese product name'
  Require ((Get-ItemProperty $uninstallKey).UninstallString -eq ('"'+$installed+'\Uninstall.exe"')) 'Invalid quoted uninstall command'
  $installedFiles=@(Get-ChildItem -LiteralPath $installed -Recurse -File | Where-Object {$_.Name -ne 'Uninstall.exe'})
  Require ($installedFiles.Count -eq $files.Count) 'Installer and portable payload file counts differ'
  foreach($file in $installedFiles){
    $relative=$file.FullName.Substring($installed.Length+1)
    Require ($hashes.ContainsKey($relative)) "Installer added unexpected payload: $relative"
    Require ((Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash -eq $hashes[$relative]) "Installer payload differs: $relative"
  }
  $shell=New-Object -ComObject WScript.Shell
  foreach($shortcut in $shortcuts){
    Require (Test-Path -LiteralPath $shortcut) "Missing shortcut: $shortcut"
    Require ($shell.CreateShortcut($shortcut).TargetPath -eq "$installed\deadtime.exe") 'Shortcut starts an unexpected launcher'
  }
  Run-Checked "$installed\deadtime.exe" "--frames 3 --snapshot `"$output\installed-start.png`" --user-data `"$profile`""
  Require (Test-Path "$output\installed-start.png") 'Installed normal startup did not render'
  Set-Content -LiteralPath "$installed\preserve-user-file.txt" -Value 'Keep this extra file'
  New-Item -ItemType Directory -Path $profile -Force | Out-Null
  Set-Content -LiteralPath "$profile\preserve-user-data.txt" -Value 'Keep this user data'
  $userHash=(Get-FileHash -LiteralPath "$profile\preserve-user-data.txt").Hash
  $extraHash=(Get-FileHash -LiteralPath "$installed\preserve-user-file.txt").Hash

  # Reinstall over the isolated installation after a missing file and a bad shortcut.
  Remove-Item -LiteralPath "$installed\release-info.txt" -Force
  $brokenShortcut=$shell.CreateShortcut($shortcuts[0])
  $brokenShortcut.TargetPath="$output\missing.exe"
  $brokenShortcut.Save()
  Run-Checked $Installer "/S /D=$installed"
  foreach($relative in $hashes.Keys){
    Require ((Get-FileHash -LiteralPath "$installed\$relative" -Algorithm SHA256).Hash -eq $hashes[$relative]) "Reinstall did not restore payload: $relative"
  }
  foreach($shortcut in $shortcuts){
    Require ($shell.CreateShortcut($shortcut).TargetPath -eq "$installed\deadtime.exe") 'Reinstall did not restore the direct shortcut'
  }
  Require ((Get-FileHash -LiteralPath "$installed\preserve-user-file.txt").Hash -eq $extraHash) 'Reinstall changed an unowned file'
  Require ((Get-FileHash -LiteralPath "$profile\preserve-user-data.txt").Hash -eq $userHash) 'Reinstall changed user data'
  # Exercise the observed deletion-failure path with a real Windows file lock.
  # _?= waits for this exact uninstaller process instead of its temporary child.
  $locked=[IO.File]::Open("$installed\libiconv-2.dll",'Open','Read','Read')
  try {
    $blocked=Start-Process -FilePath "$installed\Uninstall.exe" -ArgumentList "/S _?=$installed" -PassThru
    if(-not $blocked.WaitForExit(30000)){$blocked.Kill();throw 'Locked-file uninstall timed out'}
    $blocked.Refresh()
    Require ($blocked.ExitCode -ne 0) 'Uninstaller reported success with a locked payload file'
    Require ((Test-Path -LiteralPath "$installed\Uninstall.exe") -and (Test-Path $installKey) -and (Test-Path $uninstallKey)) 'Failed uninstall removed its retry entry point or registration'
  } finally {$locked.Dispose()}
  Run-Checked "$installed\Uninstall.exe" '/S'
  $deadline=[DateTime]::UtcNow.AddSeconds(30)
  while((Test-Path -LiteralPath "$installed\Uninstall.exe") -and [DateTime]::UtcNow -lt $deadline){Start-Sleep -Milliseconds 100}
  foreach($relative in $hashes.Keys){Require (-not (Test-Path -LiteralPath "$installed\$relative")) "Uninstaller left payload: $relative"}
  Require (-not (Test-Path $installKey) -and -not (Test-Path $uninstallKey)) 'Uninstall registrations remain'
  Require (Test-Path -LiteralPath "$installed\preserve-user-file.txt") 'Uninstaller deleted a file outside its manifest'
  Require ((Get-FileHash -LiteralPath "$profile\preserve-user-data.txt").Hash -eq $userHash) 'Uninstaller altered user data'
  foreach($shortcut in $shortcuts){Require (-not (Test-Path -LiteralPath $shortcut)) 'Uninstaller left its shortcut'}
  [ordered]@{portable=(Get-FileHash -LiteralPath $Portable).Hash;installer=(Get-FileHash -LiteralPath $Installer).Hash;payloadFiles=$files.Count;cleanPathStartup=$true;identicalPayload=$true;reinstallRepairsPayloadAndShortcuts=$true;reinstallPreservesExtraFilesAndUserData=$true;lockedUninstallRetainsRetry=$true;uninstallPreservesExtraFilesAndUserData=$true} |
    ConvertTo-Json | Set-Content -LiteralPath "$output\verification.json" -Encoding UTF8
  Write-Output "Verified portable, direct shortcuts, startup, repair by reinstall and safe uninstall: $output"
} finally {
  $env:PATH=$oldPath;$env:SDL_VIDEODRIVER=$oldVideo;$env:SDL_AUDIODRIVER=$oldAudio
  # On failure, clean only the isolated installation made by this verification.
  if((Test-Path -LiteralPath "$installed\Uninstall.exe") -and (Test-Path $installKey) -and (Get-ItemProperty $installKey).InstallDir -eq $installed){
    try {Run-Checked "$installed\Uninstall.exe" '/S'}catch{Write-Warning $_}
  }
  foreach($shortcut in $backups.Keys){[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($shortcut)) | Out-Null;[IO.File]::WriteAllBytes($shortcut,$backups[$shortcut])}
}
