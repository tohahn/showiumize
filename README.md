# showiumize
Daemon to download episodes from showRSS through premiumize.

# How it works
Every 30 minutes, the feed from showRSS is downloaded, and the entries not already fetched are added as a premiumize transfer. Once the torrent finishes, the series file is downloaded into a local folder, with the show name from the feed as last path name.

# Installation
## Requirements
curl, libcurl, libcurl-dev
## Files & Folders
### Folders
All read & write, as well as search for the directory

$HOME/.config/showiumize

/var/log/showiumize
### Files
$HOME/.config/showiumize/config with the following content:

showrss:(your feed id)

premiumize_id:(your premiumize id)

premiumize_pin:(your premiumize pin)

series_folder:(the local folder where you want series to be downloaded to)
## Download
* Clone this repo.
* Switch to release branch.
* Go to src folder.
* make
* ./showiumize

The process should be running in the background.
