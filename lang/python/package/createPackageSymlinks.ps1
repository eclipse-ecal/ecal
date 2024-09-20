$mergeFolder = "ecal"

# Create the merge folder
New-Item -ItemType Directory -Path "$mergeFolder"

# Define source folders
$sourceFolder1 = "..\core\ecal"
$sourceFolder2 = "..\ecalhdf5\ecal\"


# Create symbolic links for the contents of the first folder
Get-ChildItem -Path $sourceFolder1 | ForEach-Object {
    New-Item -ItemType SymbolicLink -Path "$mergeFolder\$($_.Name)" -Target "$sourceFolder1\$($_.Name)"
}

# Create symbolic links for the contents of the second folder
Get-ChildItem -Path $sourceFolder2 | ForEach-Object {
    New-Item -ItemType SymbolicLink -Path "$mergeFolder\$($_.Name)" -Target "$sourceFolder2\$($_.Name)"
}