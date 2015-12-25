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
#include <time.h>
#include <iostream>
#include <thread>

#include "ddfs_fileLogger.hpp"

ddfsLogger *ddfsLogger::singleton_logger= 0;
std::ofstream ddfsLogger::myFile;

ddfsLogger& ddfsLogger::getInstance(const string fname) {
	if(singleton_logger == NULL)
		singleton_logger = new ddfsLogger(fname);
	return *singleton_logger;
}

/*  log message   */
/* Format of the log message is 
 * [2013-11-08].08-09-04:[log level]:log message
 *
 * Eg.
 * [2013-11-08].08-09-04:[INFO]:DDFS is starting
 */
ddfsLogger &operator << (ddfsLogger &logger, const ddfsLogger::e_logType l_type) {
	time_t current = time(0);
	char       time_buf[80];
	struct tm * now = localtime(& current);
    std::thread::id this_id = std::this_thread::get_id();

	strftime(time_buf, sizeof(time_buf), "[%Y-%m-%d].%H-%M-%S:", now);
	logger.myFile << time_buf;
	switch (l_type) {
    	case ddfsLogger::LOG_ERROR:
			logger.myFile << "[ERROR]";
			++logger.numErrors;
			break;
    	case ddfsLogger::LOG_WARNING:
			logger.myFile << "[WARNING]";
			++logger.numWarnings;
			break;
    	default:
			logger.myFile << "[INFO]";
			break;
	} // sw

    logger.myFile << "[" << this_id << "]: ";

	return logger;
}

// Overload << operator using C style strings
// No need for std::string objects here
ddfsLogger &operator << (ddfsLogger &logger, string text) {
	logger.myFile << text;
    logger.myFile.flush();
	return logger;
}

ddfsLogger &operator << (ddfsLogger &logger, int int_value) {
	logger.myFile << int_value;
    logger.myFile.flush();
	return logger;
}

// Explicit private Constructor.
ddfsLogger::ddfsLogger (string fname)
:   numWarnings (0U),
  numErrors (0U)
{
	myFile.open(fname.c_str(), std::ofstream::app);
	// Write the first lines
	myFile << std::endl << std::endl;
	myFile << "******************************" << std::endl;
	myFile << "DDFS Log file created" << std::endl;
	myFile << "******************************" << std::endl << std::endl;
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
