@{
TEMPLATE(
    '__download_button.html.em',
    os_type              = "windows",
    os_name              = "Windows",
    ecal_version         = "1.2.3",
    ecal_download_url    = "https://github.com/continental/ecal/releases/download/v5.8.3/ecal_5.8.3-win64.exe",
    python_download_urls = {"3.9": "google.de", "3.8": "google.de", "3.7": "google.de"}
)
}@

@{
TEMPLATE(
    '__download_button.html.em',
    os_type              = "ubuntu",
    os_name              = "Ubuntu 18.04",
    ecal_version         = "1.2.3",
    ecal_download_url    = "google.de",
    python_download_urls = {"3.9": "google.de"}
)
}@

@{
TEMPLATE(
    '__download_button.html.em',
    os_type              = "ubuntu",
    os_name              = "Ubuntu 20.04",
    ecal_version         = "1.2.3",
    ecal_download_url    = "google.de",
    python_download_urls = {"3.6": "google.de"}
)
}@
