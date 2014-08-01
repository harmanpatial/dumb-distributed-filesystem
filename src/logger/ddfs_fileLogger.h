/*
 * @file ddfs_fileLogger.h
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
#ifndef DDFS_FILE_LOGGER_H
#define DDFS_FILE_LOGGER_H

#include <fstream>
#include <string>

using std::string;

/**
 * @class ddfsLogger
 *
 * @brief Logger class for DDFS.
 *
 * A simple Logger implementation for the DDFS.
 *
 * @note As all logger implementations are, this is
 * 	 a singleton class.
 *
 * @note Most of the code in this file has been shamefully
 *	 copied from Simple File Logger (by mateandmetal).
 */
class ddfsLogger {
	public:
		enum e_logType {LOG_ERROR, LOG_WARNING, LOG_INFO };

		/**
 		 * getInstance
		 * 
		 * To get the instance of the ddfsGlobal class.
		 * ddfsGlobal is a singleton class.
		 *
		 * @return reference to a class object
		 */
		static ddfsLogger& getInstance(const string fname = "/tmp/ddfs.log");
		
		/* Log Message */
		friend ddfsLogger &operator << (ddfsLogger &logger, const e_logType l_type);

		// Overload << operator using C style strings
		// No need for std::string objects here
		friend ddfsLogger &operator << (ddfsLogger &logger, int int_value);

		// Overload << operator using C style strings
		// No need for std::string objects here
		friend ddfsLogger &operator << (ddfsLogger &logger, const string text);

	private:
		static ddfsLogger	*singleton_logger;
		static std::ofstream	myFile;
		unsigned int		numWarnings;
		unsigned int		numErrors;

		// Explicit private Constructor.
		explicit ddfsLogger (string fname);

		// Private Destructor.
		~ddfsLogger ();
}; // class end

#endif /* Ending DDFS_FILE_LOGGER_H */
