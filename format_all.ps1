#Requires -Version 5

# args
param (
    [string]$CF
)

if ([string]::IsNullOrEmpty($CF)) {
    $CF = $Env:clang_format_instance
}
$CF
$Env:clang_format_instance


if (!(Test-Path $CF -PathType Leaf)) {
    "Failed to locate clang-format.exe"
    Exit
}

$projects = gci -Directory | ? { Test-Path "$_/CMakeLists.txt" -PathType Leaf} 


foreach ($proj in $projects) {
    $proj.BaseName
    '================='

    $files = gci $proj -Recurse -File -ErrorAction SilentlyContinue 
    | ? { $_.Extension -in @('.c', '.cpp', '.cxx', '.h', '.hpp', '.hxx', '.inl', 'ixx') }

    foreach ($file in $files) {
        $file.Name
        & $CF -i -style=file $file
    }
    '================='
}