
$target_dir = "$PSScriptRoot\..\thirdparty\npcap\"

New-Item "$target_dir" -ItemType "directory" -Force

$webclient = New-Object System.Net.WebClient

$webclient.DownloadFile("https://npcap.com/dist/npcap-sdk-1.13.zip","$target_dir\npcap-sdk.zip")
$webclient.DownloadFile("https://github.com/seladb/PcapPlusPlus/releases/download/v22.05/pcapplusplus-22.05-windows-vs2015.zip","$target_dir\pcapplusplus.zip")