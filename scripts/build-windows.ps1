# build-windows.ps1
# ---------------------------------------------------------------------------
# Builds the prebuilt llama.cpp core (rnllama.lib) for react-native-windows.
#
# Like the Android/iOS prebuilt core, this produces only the llama.cpp static
# libraries; the RNW JSI glue (windows/*.cpp + cpp/jsi/**) is always compiled
# from source by the consuming app so it matches the RN/RNW version.
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File scripts\build-windows.ps1 [-Arch x64] [-BuildType Release] [-CmakePath cmake]
#
# Output: windows/rnllama-<arch>/lib/rnllama.lib + include/ headers
# ---------------------------------------------------------------------------
param(
  [string]$Arch = "x64",
  [string]$BuildType = "Release",
  [string]$Generator = "Ninja",
  [string]$CmakePath = "cmake"
)

$ErrorActionPreference = "Stop"
$rootDir = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$arch = $Arch.ToLower()
$buildRoot = Join-Path $rootDir ".build-windows-$arch"

# Map RNW/Visual Studio arch names to CMake `-A` arguments. The Ninja generator
# (default) ignores `-A` and uses whatever compiler is on PATH — pair it with
# the `ilammy/msvc-dev-cmd` action (or a VS dev prompt) so cl.exe is visible.
$cmakeArch = switch ($arch) {
  "x64"   { "x64" }
  "arm64" { "ARM64" }
  "x86"   { "Win32" }
  default { throw "Unsupported -Arch '$arch'. Use x64 or arm64." }
}

# 1. Configure
$genArgs = @("-S", (Join-Path $rootDir "windows"), "-B", $buildRoot, "-G", $Generator, "-DCMAKE_BUILD_TYPE=$BuildType")
if ($Generator -ne "Ninja") {
  $genArgs += @("-A", $cmakeArch)
}
& $CmakePath @genArgs $args
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

# 2. Build
& $CmakePath --build $buildRoot --config $BuildType --parallel
if ($LASTEXITCODE -ne 0) { throw "CMake build failed" }

# 3. Stage output into windows/rnllama-<arch>/
$stageRoot = Join-Path $rootDir "windows\rnllama-$arch"
$libDir = Join-Path $stageRoot "lib"
$incDir = Join-Path $stageRoot "include"
New-Item -ItemType Directory -Force -Path $libDir, $incDir | Out-Null

$libSearch = Join-Path $buildRoot "lib\rnllama.lib"
if (Test-Path $libSearch) {
  Copy-Item $libSearch (Join-Path $libDir "rnllama.lib") -Force
} else {
  # CMake multi-config places outputs under <build>/<config>/<target>/...
  $alt = Get-ChildItem -Recurse -Path $buildRoot -Filter "rnllama-$arch*.lib" -ErrorAction SilentlyContinue
  if ($alt) { Copy-Item $alt[0].FullName (Join-Path $libDir "rnllama.lib") -Force }
}

if (-not (Test-Path (Join-Path $libDir "rnllama.lib"))) {
  Write-Error "Build succeeded but '$libDir\rnllama.lib' was not produced."
}

# Copy public headers (the JSI glue and a consumer's compile need them).
$headerRoots = @("cpp")
Get-ChildItem (Join-Path $rootDir "cpp") -Filter "*.h" -File | ForEach-Object { Copy-Item $_.FullName $incDir -Force }
Copy-Item (Join-Path $rootDir "cpp\common") $incDir -Recurse -Force
Copy-Item (Join-Path $rootDir "cpp\ggml-cpu") $incDir -Recurse -Force
Copy-Item (Join-Path $rootDir "cpp\common\jinja") $incDir -Recurse -Force
Copy-Item (Join-Path $rootDir "cpp\hash") $incDir -Recurse -Force
Copy-Item (Join-Path $rootDir "cpp\nlohmann") $incDir -Recurse -Force

Write-Host ""
Write-Host "Prebuilt Windows core is in $stageRoot"
Write-Host "  lib/    rnllama.lib"
Write-Host "  include/ (headers)"