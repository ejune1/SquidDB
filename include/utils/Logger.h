#ifndef LOGGER_H
#define LOGGER_H

#include "infra/Queue.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

namespace squiddb { namespace utils {

class Logger {
	public:
		enum class LogMode {
			Terminal = 0,
			File = 1
		};

		enum class LogLevel {
			Trace = 0,
			Debug = 1,
			Info  = 2,
			Error = 3
		};

		// get singleton instance
		static Logger& getInstance();

		// start the logging thread
		void start();
		void stop();

		// causes output file to open/close
		void setOutputMode(const LogMode mode, const std::string filename = "");

		void log(const LogLevel logLevel, const std::string);

		LogLevel getLogLevel() const;
		void setLogLevel(LogLevel logLevel);
	
	private:
		struct LogItem {
			std::chrono::time_point<std::chrono::system_clock> time;
			LogLevel logLevel;
			std::string message;
		};

		Logger();
		~Logger();

		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		void run();
		std::string logLevelString(LogLevel logLevel);

		LogMode m_logMode;
		std::string m_filename;
		std::ofstream m_fileStream;

		LogLevel m_logLevel;
		infra::Queue<LogItem*>* m_queue;
		std::atomic<bool> m_stop;

		std::thread m_thread;
		std::mutex m_mutex;
		std::condition_variable m_cv;
};

}} // namespace

#endif
