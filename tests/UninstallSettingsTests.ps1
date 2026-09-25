# Exercise the production uninstall code with disposable profiles and simulated answers.
# The fixture does not register ZMatrix, install fonts or touch real user settings.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$WorkDirectory,
    [string]$InnoSetupPath = 'C:\Program Files (x86)\Inno Setup 6\ISCC.exe'
)

$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$run = Join-Path ([IO.Path]::GetFullPath($WorkDirectory)) ('uninstall-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $run | Out-Null
$encoding = New-Object Text.UTF8Encoding($true)
function Write-Text([string]$Path, [string]$Text) { [IO.File]::WriteAllText($Path, $Text, $encoding) }
function Check([bool]$Condition, [string]$Message) { if (-not $Condition) { throw $Message } }
function Run-Fixture([string]$Executable, [string]$Arguments, [string]$CompletionFile) {
    if (Test-Path -LiteralPath $CompletionFile) { Remove-Item -LiteralPath $CompletionFile }
    $process = Start-Process -FilePath $Executable -ArgumentList $Arguments -PassThru -WindowStyle Hidden
    if (-not $process.WaitForExit(60000)) {
        Stop-Process -Id $process.Id
        throw "Fixture timed out: $Executable"
    }
    Check ($process.ExitCode -eq 0) "Fixture failed with exit code $($process.ExitCode): $Executable"
    # Inno's launcher may exit before the second-phase process finishes callbacks.
    $deadline = [DateTime]::UtcNow.AddSeconds(60)
    while (-not (Test-Path -LiteralPath $CompletionFile) -and [DateTime]::UtcNow -lt $deadline) { Start-Sleep -Milliseconds 50 }
    Check (Test-Path -LiteralPath $CompletionFile) "Fixture callbacks did not finish: $Executable"
}

$installerScript = [IO.File]::ReadAllText((Join-Path $repo 'Setup\ZMatrix_payalord.iss'))
$code = ($installerScript -split '(?im)^\[Code\]\s*$', 2)[1]
Check (-not [string]::IsNullOrEmpty($code)) 'Production installer code was not found.'
$profileConstant = "'{userappdata}\.ZMatrix'"
Check ($code.Contains($profileConstant)) 'Production profile path changed; review fixture isolation.'
$code = $code.Replace($profileConstant, "'{app}\test-profile\.ZMatrix'")
Check (-not $code.Contains('{userappdata}')) 'The fixture still references real user settings.'
Check ($code.Contains('UninstallSilent()') -and $code.Contains('SuppressibleMsgBox(')) 'Uninstall UI hooks changed.'
# Run the actual uninstall lifecycle silently, substituting only user interaction.
$code = $code.Replace('UninstallSilent()', 'TestUninstallSilent()').Replace('SuppressibleMsgBox(', 'TestMessageBox(')
$interaction = @'
function TestUninstallSilent(): Boolean;
begin
  Result := ExpandConstant('{param:case|silent}') = 'silent';
end;

function TestMessageBox(const Text: String; const Typ: TMsgBoxType;
  const Buttons, Default: Integer): Integer;
begin
  if Typ = mbConfirmation then
  begin
    if (Default <> IDNO) or (Buttons <> (MB_YESNO or MB_DEFBUTTON2)) then
      RaiseException('Settings removal must default to No.');
    SaveStringToFile(ExpandConstant('{app}\test-messages.txt'), 'question'#13#10, True);
    if ExpandConstant('{param:case|silent}') = 'keep' then Result := IDNO
    else Result := IDYES;
  end
  else
  begin
    SaveStringToFile(ExpandConstant('{app}\test-messages.txt'), 'warning'#13#10, True);
    Result := IDOK;
  end;
end;
'@
# Pascal globals must precede function declarations.
$code = $code.Replace('function FontDoesntExist()', $interaction + "`r`nfunction FontDoesntExist()")
$fixture = @'
[Setup]
AppId=ZMatrix-Uninstall-Test
AppName=ZMatrix Uninstall Test
AppVersion=1
DefaultDirName={localappdata}\ZMatrix-Uninstall-Test-Unused
PrivilegesRequired=lowest
CreateUninstallRegKey=no
Uninstallable=yes
UninstallRestartComputer=no
DisableProgramGroupPage=yes
UsePreviousAppDir=no
OutputBaseFilename=fixture
OutputDir=.
[Files]
Source: "payload.txt"; DestDir: "{app}"; Flags: ignoreversion
[Code]
'@
Write-Text (Join-Path $run 'payload.txt') 'Disposable installer fixture.'
$completion = @'
procedure DeinitializeSetup();
begin
  SaveStringToFile(ExpandConstant('{app}\test-setup-complete.txt'), 'done', False);
end;
procedure DeinitializeUninstall();
begin
  SaveStringToFile(ExpandConstant('{app}\test-uninstall-complete.txt'), 'done', False);
end;
'@
Write-Text (Join-Path $run 'fixture.iss') ($fixture + "`r`n" + $code + "`r`n" + $completion)
& $InnoSetupPath (Join-Path $run 'fixture.iss') *> (Join-Path $run 'compile.log')
Check ($LASTEXITCODE -eq 0) "Fixture compilation failed; see $run\compile.log"

$names = @('ZMatrix.cfg', 'ZMatrixScreenSaver.cfg', 'ZMatrixMisc.cfg', 'Audio.cfg')
foreach ($case in @('keep', 'remove', 'presets', 'partial', 'absent', 'locked', 'redirected', 'silent')) {
    $app = Join-Path $run $case
    Check ([IO.Path]::GetFullPath($app).StartsWith($run + '\', [StringComparison]::OrdinalIgnoreCase)) 'Fixture escaped its work directory.'
    $profile = Join-Path $app 'test-profile\.ZMatrix'
    $other = Join-Path $app 'other-user\.ZMatrix'
    New-Item -ItemType Directory -Path $app, $other -Force | Out-Null
    Write-Text (Join-Path $other 'Audio.cfg') 'Other account settings.'
    if ($case -ne 'absent') {
        if ($case -eq 'redirected') {
            New-Item -ItemType Directory -Path (Split-Path $profile) -Force | Out-Null
            New-Item -ItemType Junction -Path $profile -Target $other | Out-Null
        } else { New-Item -ItemType Directory -Path $profile -Force | Out-Null }
        foreach ($name in $names) {
            if ($case -ne 'partial' -or $name -eq 'Audio.cfg') { Write-Text (Join-Path $profile $name) ('Settings: ' + $name) }
        }
        if ($case -eq 'presets') {
            Write-Text (Join-Path $profile 'My preset.cfg') 'Saved preset.'
            New-Item -ItemType Directory -Path (Join-Path $profile 'backup') | Out-Null
            Write-Text (Join-Path $profile 'backup\Audio.cfg') 'Backup settings.'
        }
    }
    $sentinel = [IO.File]::ReadAllText((Join-Path $other 'Audio.cfg'))
    $installArgs = '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /DIR="' + $app + '" /LOG="' + (Join-Path $run ($case + '-install.log')) + '"'
    Run-Fixture (Join-Path $run 'fixture.exe') $installArgs (Join-Path $app 'test-setup-complete.txt')
    if ($case -eq 'keep') {
        Run-Fixture (Join-Path $run 'fixture.exe') $installArgs (Join-Path $app 'test-setup-complete.txt')
        foreach ($name in $names) { Check ([IO.File]::ReadAllText((Join-Path $profile $name)) -eq ('Settings: ' + $name)) 'Reinstallation changed settings.' }
    }
    $lock = $null
    try {
        if ($case -eq 'locked') {
            $lock = [IO.File]::Open((Join-Path $profile 'Audio.cfg'), [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read)
        }
        Run-Fixture (Join-Path $app 'unins000.exe') ('/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /case=' + $case + ' /LOG="' + (Join-Path $run ($case + '.log')) + '"') (Join-Path $app 'test-uninstall-complete.txt')
    } finally { if ($lock) { $lock.Dispose() } }
    Check (-not (Test-Path -LiteralPath (Join-Path $app 'payload.txt'))) "The fixture was not uninstalled: $case"
    $messagePath = Join-Path $app 'test-messages.txt'
    $messages = if (Test-Path -LiteralPath $messagePath) { [IO.File]::ReadAllText($messagePath) } else { '' }
    Check (($messages.Contains('question')) -eq ($case -notin @('silent', 'absent'))) "Unexpected prompt behavior: $case"
    Check (($messages.Contains('warning')) -eq ($case -in @('locked', 'redirected'))) "Unexpected failure reporting: $case"
    foreach ($name in $names) {
        $preserved = $case -in @('keep', 'silent', 'redirected') -or ($case -eq 'locked' -and $name -eq 'Audio.cfg')
        Check ((Test-Path -LiteralPath (Join-Path $profile $name)) -eq $preserved) "Incorrect settings deletion: $case/$name"
        if ($preserved) { Check ([IO.File]::ReadAllText((Join-Path $profile $name)) -eq ('Settings: ' + $name)) "Preserved settings changed: $case/$name" }
    }
    if ($case -in @('remove', 'partial', 'absent')) { Check (-not (Test-Path -LiteralPath $profile)) "Empty profile retained: $case" }
    if ($case -eq 'presets') {
        Check ([IO.File]::ReadAllText((Join-Path $profile 'My preset.cfg')) -eq 'Saved preset.') 'Named preset was removed.'
        Check ([IO.File]::ReadAllText((Join-Path $profile 'backup\Audio.cfg')) -eq 'Backup settings.') 'Nested backup was removed.'
    }
    Check ([IO.File]::ReadAllText((Join-Path $other 'Audio.cfg')) -eq $sentinel) 'Another profile was changed.'
    Write-Output "PASS: $case"
}
Write-Output "Uninstall settings tests passed. Fixtures and logs: $run"
