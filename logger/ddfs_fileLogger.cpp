/*
 * @file ddfs_fileLogger.cpp
 *
 * @breif Module containing the logger class.
 *
 * This is the module that contains logging class.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 * @note Most of the code in this file has been shamefully
 *	 copied from Simple File Logger (by mateandmetal).
 */

#include <fstream>

#include "ddfs_fileLogger.h"

ddfsLogger& ddfsLogger::getInstance(const char *fname) {
	if(single_logger == NULL)
		single_logger = new ddfsLogger(fname);
	return single_logger;
}

// log message
ddfsLogger &operator << (ddfsLogger &logger, const ddfsLogger::e_logType l_type) {
	switch (l_type) {
    	case ddfsLogger::LOG_ERROR:
			logger.myFile << "[ERROR]: ";
			++logger.numErrors;
			break;
    	case ddfsLogger::LOG_WARNING:
			logger.myFile << "[WARNING]: ";
			++logger.numWarnings;
			break;
    	default:
			logger.myFile << "[INFO]: ";
			break;
	} // sw
	return logger;
}

// Overload << operator using C style strings
// No need for std::string objects here
ddfsLogger &operator << (ddfsLogger &logger, const char *text) {
	logger.myFile << text << std::endl;
	return logger;
}

ddfsLogger &operator << (ddfsLogger &logger, int int_value) {
	logger.myFile << int_value << std::endl;
	return logger;
}

// Explicit private Constructor.
ddfsLogger::ddfsLogger (const char *fname)
:   numWarnings (0U),
  numErrors (0U)
{
	myFile.open(fname);
	// Write the first lines
	myFile << "Log file created" << std::endl << std::endl;
}

// Private Destructor.
ddfsLogger::~ddfsLogger () {
	if (myFile.is_open()) {
		myFile << std::endl << std::endl;

		// Report number of errors and warnings
		myFile << numWarnings << " warnings" << std::endl;
		myFile << numErrors << " errors" << std::endl;

		myFile.close();
	} // if
}
