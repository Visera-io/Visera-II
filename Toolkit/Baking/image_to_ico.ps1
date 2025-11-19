param(
    [Parameter(Mandatory = $true)]
    [string]$InputImage,

    [Parameter(Mandatory = $true)]
    [string]$OutputIco
)

# Normalize and sanity-check the path
$InputImage = (Resolve-Path $InputImage).ProviderPath
Write-Host "InputImage =" $InputImage

if (-not (Test-Path $InputImage)) {
    throw "Input image not found: $InputImage"
}

Add-Type -AssemblyName System.Drawing

$png  = [System.Drawing.Image]::FromFile($InputImage)
$icon = [System.Drawing.Icon]::FromHandle($png.GetHicon())

$fs = [System.IO.File]::Create($OutputIco)
$icon.Save($fs)
$fs.Close()

$png.Dispose()
$icon.Dispose()
