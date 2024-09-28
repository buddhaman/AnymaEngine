# Define variables
$ProjectName = "AnymaEngine"
$ItchUser = "buddhaman"
$ItchGame = "anyma-engine"
$Channel = "win64-release"
$BuildDir = "Build/Release"
$ZipName = "Build/${ProjectName}_${Channel}.zip"

# Change to the script's directory
Set-Location -Path $PSScriptRoot

# Zip the build directory
Write-Host "Zipping the build directory..."
try {
    Compress-Archive -Path "${BuildDir}\*" -DestinationPath $ZipName -Force -ErrorAction Stop
}
catch {
    Write-Error "Zipping failed: $_"
    exit 1
}

# Upload the zip file to itch.io using butler
Write-Host "Uploading the build to itch.io..."
$butlerPushCommand = "butler push $($ZipName) $($ItchUser)/$($ItchGame):$($Channel)"
Invoke-Expression $butlerPushCommand

Write-Host "Upload complete!"
