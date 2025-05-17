# skip-large.ps1

# Soglia dimensione in MB
$maxMB = 100
$maxBytes = $maxMB * 1MB
$repoRoot = Get-Location

# 1) Genera .gitignore.auto-large con i file > soglia
"# Ignora file > ${maxMB}MB" | Out-File .gitignore.auto-large -Encoding utf8
Get-ChildItem -Recurse -File | Where-Object { $_.Length -gt $maxBytes } |
  ForEach-Object {
    # percorso relativo slashato
    $rel = $_.FullName.Substring($repoRoot.Path.Length) -replace '\\','/'
    if ($rel -notmatch '^/') { $rel = '/' + $rel }
    $rel
  } | Out-File -Append .gitignore.auto-large -Encoding utf8

# 2) Unisci con .gitignore esistente
if (Test-Path .gitignore) {
  Get-Content .gitignore.auto-large, .gitignore | Set-Content .gitignore
} else {
  Rename-Item .gitignore.auto-large .gitignore
}
