param (
    [Parameter(Mandatory=$true)]
    [string]$path
)

# check folder creation
$folders = $path.Split("/")
#$folders = $folders[0..($folders.Length-2)]

$path_to_check = ""

if ($folders[0][-1] -eq ":"){
    $path_to_check = $folders[0] + "/"
    $folders = $folders[1..($folders.Length-1)]
}
Foreach( $folder in $folders ){
    $path_to_check = $path_to_check + $folder + "/"
    if ( -Not ( Test-Path $path_to_check ) ) {
        New-Item $path_to_check -ItemType Directory
    }
}

