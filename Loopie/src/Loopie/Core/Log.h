#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <deque>

#define LOOPIE_ENABLE_LOG

namespace Loopie {
	class Log
	{
	public:

        const static unsigned int MAX_LOGS = 1000;

        struct LogEntry {
            int level;
            std::string msg;

            LogEntry(int level, const std::string& msg)
                : level(level), msg(msg)
            {
            }
        };

		static void Init();

        template <typename... Args>
        static void Trace(const char* msg, Args&&... args) {
            LogMessage(spdlog::level::trace, fmt::format(msg, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Info(const char* msg, Args&&... args) {
            LogMessage(spdlog::level::info, fmt::format(msg, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Debug(const char* msg, Args&&... args) {
            LogMessage(spdlog::level::debug, fmt::format(msg, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Warn(const char* msg, Args&&... args) {
            LogMessage(spdlog::level::warn, fmt::format(msg, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Error(const char* msg, Args&&... args) {
            LogMessage(spdlog::level::err, fmt::format(msg, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Critical(const char* msg, Args&&... args) {
            LogMessage(spdlog::level::critical, fmt::format(msg, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void ByType(int level, const char* msg, Args&&... args) {
            LogMessage((spdlog::level::level_enum)level, fmt::format(msg, std::forward<Args>(args)...));
        }

		static const std::deque<LogEntry>& GetLogEntries() { return s_LogEntries; }
        static void Clear() { s_LogEntries.clear(); }

	private:
		static void LogMessage(spdlog::level::level_enum level, const std::string& text) {
            #ifdef LOOPIE_ENABLE_LOG
			spdlog::log(level, text);
            if (s_LogEntries.size() >= MAX_LOGS)
                s_LogEntries.pop_front();
			s_LogEntries.push_back(LogEntry(level, fmt::format("[{}] {}", spdlog::level::to_string_view(level), text)));
            #endif
		}

	private:
		static std::deque<LogEntry> s_LogEntries;
	};
}