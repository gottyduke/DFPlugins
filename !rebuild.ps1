#Requires -Version 5

# args
param(
	[switch]$WhatIf,
	[Alias('DBG')][string[]]$EnableDebugger,
	[Alias('D')][string[]]$ExtraCMakeArgument
)

$ErrorActionPreference = 'Stop'
$PSDefaultParameterValues['Out-File:Encoding'] = 'utf8'

$env:DKScriptVersion = '21212'
$env:RebuildInvoke = $true
$env:ScriptCulture = (Get-Culture).Name -eq 'zh-CN'


function L {
	param (
		[Parameter(Mandatory)][string]$en,
		[string]$zh = ''
	)
	
	process {
		if ($env:ScriptCulture -and $zh) {
			return $zh
		}
		else {
			return $en
		}
	}
}


[IO.Directory]::SetCurrentDirectory($PSScriptRoot)
Set-Location $PSScriptRoot


function Normalize ($text) {
	return $text -replace '\\', '/'
}


function Add-Subdirectory ($Name, $Path) {
	return Normalize "message(CHECK_START `"Rebuilding $Name`")`nadd_subdirectory($Path)`nmessage(CHECK_PASS `"Complete`")"
}


Write-Host "`tDKScriptVersion $env:DKScriptVersion`t for DFPE`n"

# @@CLib
$CMakeLists = [System.Collections.ArrayList]::new(256)
# @@CMake Targets
$ExcludedSubfolder = @('cmake', 'vcpkg', 'build', '.git', '.vs', 'template')
Write-Host "`tFinding CMake targets..."
$ProjectVCPKG = [IO.File]::ReadAllText("$PSScriptRoot/cmake/vcpkg.json.in") | ConvertFrom-Json
Get-ChildItem "$PSScriptRoot/*" -Directory | ? {
	$_.Name -notin $ExcludedSubfolder
} | Resolve-Path -Relative | % {
	if (Test-Path "$_/CMakeLists.txt" -PathType Leaf) {
		$Target = $_.Substring(2)
		Write-Host "`t`t[ $Target ]"

		$vcpkg = [IO.File]::ReadAllText("$_/vcpkg.json") | ConvertFrom-Json
		$ProjectVCPKG.dependencies += $vcpkg.'dependencies'

		if ($EnableDebugger -and $EnableDebugger.Contains($Target)) {
			$Target += '_debugger'
		}

		$CMakeLists.Add((Add-Subdirectory $Target $_)) | Out-Null
		$CMakeLists.Add((Normalize "fipch($Target $($_.Substring(2)))")) | Out-Null
		$CMakeLists.Add((Normalize "define_external($Target)")) | Out-Null
		$CMakeLists.Add("") | Out-Null
	}
}
$Deps = @()
foreach ($dep in $ProjectVCPKG.dependencies) {
	$tmp = $dep
	if ($dep.name) {
		$tmp = $dep.name
	}
	if ($dep.features) {
		$tmp += " <$($dep.features)>"
	}
	$Deps += $tmp
}
$Deps = $Deps | Sort-Object -Unique
$ProjectVCPKG.dependencies = $ProjectVCPKG.dependencies | Sort-Object -Unique
$Header = @((Get-Date -UFormat '# !Rebuild generated @ %R %B %d'), "# DKScriptVersion $env:DKScriptVersion")
$Boiler = [IO.File]::ReadAllLines("$PSScriptRoot/cmake/DFPE.CMakeLists.txt")
$CMakeLists = $Header + $Boiler + $CMakeLists
[IO.File]::WriteAllLines("$PSScriptRoot/CMakeLists.txt", $CMakeLists)
$ProjectVCPKG = $ProjectVCPKG | ConvertTo-Json -Depth 9
[IO.File]::WriteAllText("$PSScriptRoot/vcpkg.json", $ProjectVCPKG)


# @@Debugger
if ($EnableDebugger.Count) {
	Write-Host "`tEnabled debugger:"
	foreach ($enabledDebugger in $EnableDebugger) {
		Write-Host "`t`t- $enabledDebugger"
	}
}


# @@WhatIf
if ($WhatIf) {
	Write-Host "`tPrebuild complete" -ForegroundColor Green
	Invoke-Item "$PSScriptRoot/CMakeLists.txt"
	Exit
}


# @@CMake Generator
Remove-Item "$PSScriptRoot/build" -Recurse -Force -Confirm:$false -ErrorAction:Ignore | Out-Null
Write-Host "`tCleaned build folder"

$Arguments = @(
	'-Wno-dev'
)
foreach ($enabledDebugger in $EnableDebugger) {
	$Arguments += "-D$($enabledDebugger.ToUpper())_DEBUG_BUILD:BOOL=1"
}
foreach ($extraArg in $ExtraCMakeArgument) {
	$Arguments += "-D$extraArg"
}
$CurProject = $null
Write-Host "`tListing vcpkg dependencies: "
$Deps | % {
	"`t`t[ $_ ]"
}
Write-Host "`tBuilding dependencies & generating solution..."
$CMake = & cmake.exe -B $PSScriptRoot/build -S $PSScriptRoot --preset=build $Arguments | % {
	if ($_.StartsWith('-- Rebuilding ') -and !($_.EndsWith(' - Complete'))) {
		$CurProject = $_.Substring(14)
		Write-Host "`t`t! [Building] $CurProject" -ForegroundColor Yellow -NoNewline
	}
	elseif ($_.Contains('CMake Error')) {
		Write-Host "`r`t`t* [Failed] $CurProject               " -ForegroundColor Red
	}
	elseif ($_.StartsWith('-- Rebuilding ') -and $_.EndsWith(' - Complete')) {
		Write-Host "`r`t`t* [Complete] $CurProject               " -ForegroundColor Cyan
	}
	$_
}

if ($CMake[-2] -ne '-- Generating done') {
	Write-Host "`tFailed generating solution!" -ForegroundColor Red
}
else {
	Write-Host "`tFinished generating solution!`n`n`tYou may open the dfpe.sln and starting coding." -ForegroundColor Green

	Invoke-Item "$PSScriptRoot/build"

	# @@QuickBuild
	$Invocation = "@echo off`n" + 'powershell -ExecutionPolicy Bypass -Command "& %~dp0/!Rebuild.ps1 '
	if ($EnableDebugger) {
		$Invocation += " -dbg"
		foreach ($enabledDebugger in $EnableDebugger) {
			$Invocation += " $($enabledDebugger)"
		}
	}
	if ($ExtraCMakeArgument) {
		$Invocation += " -d"
		foreach ($extraArg in $ExtraCMakeArgument) {
			$Invocation += " $($extraArg)"
		}
	}

	$Invocation += '"'

	$Batch = Get-ChildItem "$PSSciptRoot" -File | ? {($_.Extension -eq '.cmd') -and ($_.BaseName.StartsWith('!_LAST_'))} | % {
		Remove-Item "$_" -Confirm:$false -Force -ErrorAction:SilentlyContinue | Out-Null
	}

	[IO.File]::WriteAllText("$PSScriptRoot/!rebuild_last.cmd", $Invocation)

	Write-Host "`tTo rebuild with same configuration, use the generated batch file.`n`t* !build_last.cmd *" -ForegroundColor Green
	Write-Host "`n`t!Rebuild will now exit." -ForegroundColor Green
}


