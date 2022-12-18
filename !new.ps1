# args
param (
    [Parameter(Mandatory)][string]$Name,
    [Alias('message', 'm')][string]$Description,
    [Alias('vcpkg', 'v')][string[]]$AddDependencies = @()
)

$ErrorActionPreference = "Stop"

# templates
$Json = @'
{
    "name":  "",
    "version-string":  "1.0.0",
    "description":  "",
    "license":  "MIT",
    "dependencies":  [
                         "spdlog"
                     ]
}
'@ | ConvertFrom-Json

# checks
if (Test-Path "$PSScriptRoot/$Name" -PathType Container) {
    Write-Host "`tFolder with same name exists. Aborting" -ForegroundColor Red
    Exit
}

New-Item -Type dir $PSScriptRoot/$Name -Force | Out-Null

# update
if (Test-Path "$PSScriptRoot/Template/CMakeLists.txt" -PathType Leaf) {
    Write-Host "`tFound DFPE Template project" -ForegroundColor Green
} else {
    <###
    Write-Host "`t! Missing Template project! Downloading..." -ForegroundColor Red -NoNewline
    Remove-Item "$PSScriptRoot/Plugins/Template" -Recurse -Force -Confirm:$false -ErrorAction Ignore
    & git clone https://github.com/gottyduke/Template "$PSScriptRoot/Plugins/Template" -q
    $PSScriptRoot/Template = "$PSScriptRoot/Plugins/Template"
    Write-Host "`t* Installed Template project                " -ForegroundColor Yellow -NoNewline
    ###>
}

# populate
Copy-Item "$PSScriptRoot/Template/cmake" "$PSScriptRoot/$Name/cmake" -Recurse -Force
Copy-Item "$PSScriptRoot/Template/src" "$PSScriptRoot/$Name/src" -Recurse -Force
Copy-Item "$PSScriptRoot/Template/CMakeLists.txt" "$PSScriptRoot/$Name/CMakeLists.txt" -Force
Copy-Item "$PSScriptRoot/Template/CMakePresets.json" "$PSScriptRoot/$Name/CMakePresets.json" -Force
Copy-Item "$PSScriptRoot/Template/.gitattributes" "$PSScriptRoot/$Name/.gitattributes" -Force
Copy-Item "$PSScriptRoot/Template/.gitignore" "$PSScriptRoot/$Name/.gitignore" -Force
Copy-Item "$PSScriptRoot/Template/.clang-format" "$PSScriptRoot/$Name/.clang-format" -Force


# generate vcpkg.json
$Json.'name' = $Name.ToLower()
if ($Description) {
    $Json.'description' = $Description
}
if ($AddDependencies) {
    Write-Host "`tAdditional vcpkg dependency enabled" -ForegroundColor Yellow
    foreach ($dependency in $AddDependencies) {
        if ($dependency.Contains('[')) { # vcpkg-features
            $Json.'dependencies' += [PSCustomObject]@{
                'name' = $dependency.Substring(0, $dependency.IndexOf('['))
                'features' = $dependency.Substring($dependency.IndexOf('[') + 1).Replace(']', '').Split(',').Trim()
            }
        } else {
            $Json.'dependencies' += $dependency
            $Pakcages += $dependency
        }

        $Json.'dependencies' = $Json.'dependencies' | Select-Object -Unique | Sort-Object
    }
}

# vcpkg
$Json = $Json | ConvertTo-Json -Depth 9
[IO.File]::WriteAllText("$PSScriptRoot/$Name/vcpkg.json", $Json)

# CMakeLists
$CMake = [IO.File]::ReadAllLines("$PSScriptRoot/$Name/CMakeLists.txt") -replace 'dfplugin_template', $Name
[IO.File]::WriteAllLines("$PSScriptRoot/$Name/CMakeLists.txt", $CMake)

Push-Location $PSScriptRoot/$Name
& git init | Out-Null
& git add --all | Out-Null
& git commit -m 'Init' | Out-Null
Pop-Location

Write-Host "`tGenerated new project <$Name>" -ForegroundColor Green
