#ifndef MESSAGES_H
#define MESSAGES_H

/** ERROR MESSAGES **/
//UTILS
#define ERROR_MALLOC "Could not allocate memory. Probably no more RAM. Exiting."
#define ERROR_EASY_SPRINTF "Could not vsprintf. Probably allocated memory was too little."
//LOG
#define ERROR_LOG_FILE_OPEN "[ERROR] Log file could not be opened. Create directory %s with appropiate permissions. Exiting.\n"
#define ERROR_LOG_FILE_CLOSE_FIRST "First log file could not be closed. Missing permissions? Exiting.\n"
#define ERROR_LOG_FILE_CLOSE_SECOND "Second log file could not be closed. Missing permissions? Exiting.\n"
#define ERROR_LOG_ROTATING "Could not rotate logs. Missing permissions? Exiting."

#endif
