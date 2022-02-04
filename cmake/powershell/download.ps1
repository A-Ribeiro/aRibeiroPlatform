# powershell.exe -noexit -file ./download.ps1 -uri http://alessandroribeiro.thegeneralsolution.com/wp-content/uploads/2021/09/Image284.jpg -outfile ./img.jpg
# powershell.exe -file ./download.ps1 -repository http://alessandroribeiro.thegeneralsolution.com/wp-content/uploads/2021/09 -uri Image284.jpg -outfile ./tst/tst2/img.jpg

param (
    [Parameter(Mandatory=$true)]
    [string]$repository,

    [Parameter(Mandatory=$true)]
    [string]$uri,
    
    [Parameter(Mandatory=$true)]
    [string]$outdir
)

if (-Not (Test-Path -Path $outdir/$uri -PathType Leaf) ) {
    Invoke-WebRequest -Uri $repository/$uri -OutFile $outdir/$uri
}
