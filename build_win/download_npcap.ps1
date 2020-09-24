
$target_dir = "$PSScriptRoot\..\thirdparty\npcap\"
New-Item "$target_dir" -ItemType "directory" -Force

$webclient = New-Object System.Net.WebClient

$webclient.DownloadFile("https://nmap.org/npcap/dist/npcap-sdk-1.05.zip","$target_dir\npcap-sdk-1.05.zip")
$webclient.DownloadFile("https://github.com/seladb/PcapPlusPlus/releases/download/v20.08/pcapplusplus-20.08-windows-vs2015.zip","$target_dir\pcapplusplus-20.08-windows-vs2015.zip")

Expand-Archive -Force -Path "$target_dir\npcap-sdk-1.05.zip" -DestinationPath "$target_dir\npcap-sdk"
Expand-Archive -Force -Path "$target_dir\pcapplusplus-20.08-windows-vs2015.zip" -DestinationPath "$target_dir"