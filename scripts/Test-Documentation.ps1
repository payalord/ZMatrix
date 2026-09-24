[CmdletBinding()]
param([string]$Root)

$ErrorActionPreference = 'Stop'
if (-not $Root) { $Root = Join-Path $PSScriptRoot '..' }
$rootPath = (Resolve-Path -LiteralPath $Root).Path
$documents = @('README.md', 'BUILDING.md', 'docs\USER_GUIDE.md',
    'docs\DEVELOPMENT.md', 'docs\SCRIPTS.md', 'ORIGINALREADME.md',
    'LICENSE.TXT', 'Matrix Code Font ReadMe.txt')
$utf8 = New-Object System.Text.UTF8Encoding($false, $true)
foreach ($document in $documents) {
    $path = Join-Path $rootPath $document
    $bytes = [IO.File]::ReadAllBytes($path)
    if ($bytes.Length -gt 1MB) { throw "Documentation exceeds the viewer limit: $document" }
    $content = $utf8.GetString($bytes).TrimStart([char]0xfeff)
    if ([string]::IsNullOrWhiteSpace($content)) { throw "Empty documentation: $document" }
    if ($document.EndsWith('.md')) {
        if ($content -notmatch '(?m)^# .+') { throw "Missing document title: $document" }
        foreach ($link in [regex]::Matches($content, '\[[^\]\r\n]+\]\(([^)\r\n]+)\)')) {
            $target = $link.Groups[1].Value
            if ($target -match '^https?://' -or $target.StartsWith('#')) { continue }
            $relative = [Uri]::UnescapeDataString(($target -split '#', 2)[0])
            $resolved = [IO.Path]::GetFullPath((Join-Path ([IO.Path]::GetDirectoryName($path)) $relative))
            if (-not $resolved.StartsWith($rootPath + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
                throw "Documentation link escapes the project: $document -> $target"
            }
            if (-not (Test-Path -LiteralPath $resolved -PathType Leaf)) { throw "Broken documentation link: $document -> $target" }
            $actualName = (Get-Item -LiteralPath $resolved).Name
            if ($actualName -cne [IO.Path]::GetFileName($resolved)) { throw "Incorrect filename case: $document -> $target" }
        }
    }
}
Write-Output 'Documentation checks passed: required files are nonempty UTF-8 and local Markdown links resolve.'
