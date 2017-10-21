#ifndef MESSAGES_H
#define MESSAGES_H

/** ERROR MESSAGES **/
//SHOWIUMIZE
#define ERROR_SID "Invalid SID. Exiting."
#define ERROR_WORKING_DIR "Couldn't change working dir."

//UTILS
#define ERROR_MALLOC "Could not allocate memory. Probably no more RAM. Exiting."
#define ERROR_EASY_SPRINTF "Could not vsprintf. Probably allocated memory was too little."
#define ERROR_FILE_OPEN "Could not open file %s with mode %s. Missing permissions? Exiting."
#define ERROR_FILE_TEMP_OPEN "Could not open temp file with template %s. Missing permissions? Exiting."
#define ERROR_FILE_CLOSE "Could not close file. Exiting."
#define ERROR_FILE_REMOVE "Could not remove file %s. Exiting."
#define ERROR_DIR_CREATE "Could not create folder %s with mode %d. Missing permissions? Exiting."
#define ERROR_DIR_OPEN "Could not open dir %s. Missing permissions? Exiting."
#define ERROR_DIR_CHANGE "Could not change into dir %s. Missing permissions? Exiting."
#define ERROR_DIR_CLOSE "Could not close dir. Exiting."
#define ERROR_PATH_NO_ACCESS "Can't access folder/file %s with mode %d. Trying to create it."
#define ERROR_PREMATURE_END "Reached premature end of file while searching for %s."

//LOG
#define ERROR_LOG_FILE_OPEN "[ERROR] Log file could not be opened. Create directory %s with appropiate permissions. Exiting.\n"
#define ERROR_LOG_FILE_CLOSE_FIRST "First log file could not be closed. Missing permissions? Exiting.\n"
#define ERROR_LOG_FILE_CLOSE_SECOND "Second log file could not be closed. Missing permissions? Exiting.\n"
#define ERROR_LOG_ROTATING "Could not rotate logs. Missing permissions? Exiting."

//CONFIG
#define ERROR_XDG_CONFIG "No $XDG_CONFIG_HOME variable, using $HOME/.config instead."
#define ERROR_HOME "No home variable set. Exiting."
#define ERROR_MALFORMED_CONFIG "Malformed line: Two colons in one line. Check file %s. Exiting."
#define ERROR_CONF_VALUE_NOT_FOUND "Configuration value %s couldn't be found. Exiting."

//SHOWRSS
#define ERROR_CURL_FEED "Curl failed to get feed. Exiting."
#define ERROR_FEED_NO_ITEMS "Feed contains no items. Probably wrong URL. Exiting."
#define ERROR_MALFORMED_ENTRY_FILE "Reached premature end of entry file."

//PREMIUMIZE
#define ERROR_CREATING_TRANSFER_RESPONSE "No response to transfer creation request. No network? Exiting."
#define ERROR_CHECKING_STATUS_RESPONSE "No response to transfer status request. No network? Exiting."
#define ERROR_CURL_TRANSFER "Failed to process curl request. Exiting."
#define ERROR_MALFORMED_RESTART_FILE "Reached premature end of restart file."
#define ERROR_MALFORMED_DOWNLOAD_FILE "Reached premature end of download file."

#endif
