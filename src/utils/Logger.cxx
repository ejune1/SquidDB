#include "utils/Logger.h"

#include "utils/StringUtils.h"

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace squiddb { namespace utils {

Logger::Logger() {
	m_logMode = LogMode::Terminal;
	m_logLevel = LogLevel::Info;
	m_queue = new infra::Queue<LogItem*>();
	m_stop = false;
}

Logger::~Logger() {
	m_stop = true;
	if (m_thread.joinable() == true) {
		m_thread.join();
	}

	delete m_queue;

	if (m_fileStream.is_open() == true) {
		m_fileStream.close();
	}
}

Logger& Logger::getInstance() {
	static Logger instance;
	return instance;
}

void Logger::start() {
	m_thread = std::thread(&Logger::run, this);
}

void Logger::stop() {
	log(LogLevel::Info, "Logger::stop Logger got stop signal");
	std::this_thread::sleep_for(std::chrono::seconds(1));

	m_stop = true;
	m_cv.notify_one();
}

void Logger::setOutputMode(const Logger::LogMode logMode, const std::string filename) {
	if (logMode == LogMode::Unknown) {
		return;
	}

	if (m_logMode == LogMode::Terminal) {
		if (logMode == LogMode::Terminal) {
			return;
		}

		if (logMode == LogMode::File) {
			m_logMode = LogMode::File;
			m_filename = filename;
			m_fileStream.open(m_filename, std::ios::out | std::ios::app);
		}

	} else if (m_logMode == LogMode::File) {
		if (logMode == LogMode::Terminal) {
			m_logMode = LogMode::Terminal;
			m_filename.clear();

			if (m_fileStream.is_open() == true) {
				m_fileStream.close();
			}
		}

		if (logMode == LogMode::File) {
			if (m_filename == filename) {
				return;
			}
			m_filename = filename;
			if (m_fileStream.is_open() == true) {
				m_fileStream.close();
			}
			m_fileStream.open(m_filename, std::ios::out | std::ios::app);
		}
	}

	if ((m_logMode == LogMode::File) && (m_fileStream.is_open() == false)) {
		throw std::runtime_error("Logger::setOutputMode mode set to file but file not open");
	}
}

void Logger::log(const Logger::LogLevel logLevel, std::string message) {
	if (logLevel < m_logLevel) {
		return;
	}

	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	LogItem* logItem = new LogItem();

	logItem->time = now;
	logItem->logLevel = logLevel;
	logItem->message = std::move(message);
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue->enqueue(logItem);
	}
	m_cv.notify_one();
}

Logger::LogLevel Logger::getLogLevel() const {
	return m_logLevel;
}

void Logger::setLogLevel(const Logger::LogLevel logLevel) {
	if (logLevel == LogLevel::Unknown) {
		return;
	}

	m_logLevel = logLevel;
}

Logger::LogLevel Logger::parseLogLevel(const std::string& logLevelString) const {
	std::string logLevelStringUpper = StringUtils::toUpper(logLevelString);	

	if (logLevelStringUpper == "TRACE") {
		return LogLevel::Trace;
	}

	if (logLevelStringUpper == "DEBUG") {
		return LogLevel::Debug;
	}

	if (logLevelStringUpper == "INFO") {
		return LogLevel::Info;
	}

	if (logLevelStringUpper == "WARN") {
		return LogLevel::Warn;
	}
	
	if (logLevelStringUpper == "ERROR") {
		return LogLevel::Error;
	}

	return LogLevel::Unknown;
}

Logger::LogMode Logger::parseLogMode(const std::string& logModeString) const {
	std::string logModeStringUpper = StringUtils::toUpper(logModeString);

	if (logModeStringUpper == "TERMINAL") {
		return LogMode::Terminal;
	}

	if (logModeStringUpper == "FILE") {
		return LogMode::File;
	}

	return LogMode::Unknown;
}

void Logger::run() {
	size_t itemsRemaining = 0;

	while ((m_stop == false) || (itemsRemaining > 0)) {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_cv.wait(lock, [this] { return (m_queue->empty() == false) || (m_stop == true); });

		if (m_queue->empty() == false) {
			LogItem* logItem = m_queue->dequeue();
			lock.unlock();

			auto timeT = std::chrono::system_clock::to_time_t(logItem->time);
			std::tm* localTime = std::localtime(&timeT);

			std::stringstream ss;
			ss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");

			std::string output = ss.str() + " (" + logLevelString(logItem->logLevel) + ") " + logItem->message;

			delete logItem;

			if (m_logMode == LogMode::Terminal) {
				printf("%s\n", output.c_str());
			} else {
				m_fileStream << output << "\n";
				m_fileStream.flush();
			}
		} else {
			lock.unlock();
		}

		if (m_stop == true) {
			itemsRemaining = m_queue->size();
			if (itemsRemaining > 0) {
				std::string message = "Logger::run Logger purging " + std::to_string(itemsRemaining) + " items";
				log(LogLevel::Info, message);
			}
		}
	}
}

std::string Logger::logLevelString(const Logger::LogLevel logLevel) const {
	switch (logLevel) {
		case LogLevel::Trace:   return "TRACE";
		case LogLevel::Debug:   return "DEBUG";
		case LogLevel::Info:    return "INFO";
		case LogLevel::Warn:    return "WARN";
		case LogLevel::Error:   return "ERROR";
		case LogLevel::Unknown: return "UNKNOWN";
	}
	return "UNKNOWN";
}

}} // namespace
