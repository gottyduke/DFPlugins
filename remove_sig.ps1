@('!new.ps1', '!rebuild.ps1', '!update.ps1') | ForEach-Object {
    $Script = [IO.File]::ReadAllLines("$PSScriptRoot/$_")
    $SigPos = 0
    $SigPattern = '# SIG # Begin signature block'
    for ($SigPos = 0; $SigPos -lt $Script.Count; ++$SigPos) {
        if ($Script[$SigPos] -eq $SigPattern) {
            --$SigPos
            break
        }
    }

    [IO.File]::WriteAllLines("$PSScriptRoot/$_", $Script[0 .. $SigPos])
}
