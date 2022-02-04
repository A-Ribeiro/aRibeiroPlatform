# powershell.exe -file ./unzip.ps1 -inputzipfile _MultiFramework.zip -outputpath ./tst3
param (
    [Parameter(Mandatory=$true)]
    [string]$inputzipfile,

    [Parameter(Mandatory=$true)]
    [string]$outputpath
    
)

# This is the path where my script is running
#$path = split-path -parent $MyInvocation.MyCommand.Definition

#if (Test-Path -Path $outputpath) {
#    Remove-Item -Recurse -Force $outputpath
#}

#."$path/check_path_creation.ps1" -path $outputpath


Expand-Archive $inputzipfile -DestinationPath $outputpath
