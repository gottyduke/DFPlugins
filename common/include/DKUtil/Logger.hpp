#pragma once

/*
 * @@ File modification from https://github.com/gottyduke/DKUtil @@
 * 
 * 1.0.0
 * Adaptation of file structural changes;
 * 
 */


#ifndef LOG_PATH
#	define LOG_PATH "logs"sv
#endif


#define DKU_L_VERSION_MAJOR 1
#define DKU_L_VERSION_MINOR 0
#define DKU_L_VERSION_REVISION 0


namespace DKUtil
{
	constexpr auto DKU_L_VERSION = DKU_L_VERSION_MAJOR * 10000 + DKU_L_VERSION_MINOR * 100 + DKU_L_VERSION_REVISION;
}  // namespace DKUtil


#include "DFPE/Prefix.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#define __SHORTF__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define LOG(LEVEL, ...)                                             \
	{                                                               \
		std::source_location src = std::source_location::current(); \
		DKUtil::Logger::Log({                                       \
								src,                                \
								__SHORTF__,                         \
								__FUNCTION__,                       \
								spdlog::level::LEVEL,               \
							},                                      \
			__VA_ARGS__);                                           \
	}
#define INFO(...) LOG(info, __VA_ARGS__)
#define DEBUG(...) LOG(debug, __VA_ARGS__)
#define WARN(...)                                                                 \
	{                                                                             \
		std::source_location src = std::source_location::current();               \
		DKUtil::Logger::Warn(false,                                               \
			{ src, __SHORTF__, __FUNCTION__, spdlog::level::warn }, __VA_ARGS__); \
	}
#define ERROR(...)                                                                    \
	{                                                                                 \
		std::source_location src = std::source_location::current();                   \
		DKUtil::Logger::Warn(true,                                                    \
			{ src, __SHORTF__, __FUNCTION__, spdlog::level::critical }, __VA_ARGS__); \
	}

namespace DKUtil::Logger
{
	struct LogHeader
	{
		const std::source_location src;
		const std::string_view abbrFunc;
		const std::string_view fullFunc;
		const spdlog::level::level_enum level = spdlog::level::critical;
	};

	template <typename... Args>
	inline void Log(const LogHeader a_header, fmt::format_string<Args...> a_fmt, Args&&... a_args) noexcept
	{
		spdlog::default_logger_raw()->log(spdlog::source_loc{
											  a_header.src.file_name(),
											  static_cast<int>(a_header.src.line()),
											  a_header.src.function_name() },
			a_header.level, a_fmt, std::forward<Args>(a_args)...);
	}

	template <typename... Args>
	inline void Warn(const bool a_exit, const LogHeader a_header, fmt::format_string<Args...> a_fmt, Args&&... a_args) noexcept
	{
		Log(a_header, a_fmt, std::forward<Args>(a_args)...);

		const auto msg = fmt::format(a_fmt, std::forward<Args>(a_args)...);
		const auto error = fmt::format("Error occured at code -> [{}:{}]\n{}\n{}\n",
			a_header.abbrFunc, a_header.src.line(), a_header.fullFunc, msg);

		MessageBoxA(nullptr, error.c_str(), Plugin::NAME.data(), MB_OK | MB_ICONEXCLAMATION);
		
		if (a_exit) {
			std::exit('EXIT');
		}
	}

	inline void SetLevel(const spdlog::level::level_enum a_level) noexcept
	{
		spdlog::default_logger()->set_level(a_level);
	}

	inline void EnableDebug(bool a_enable = true) noexcept
	{
		SetLevel(a_enable ? spdlog::level::level_enum::debug : spdlog::level::level_enum::info);
	}

	inline void Init(spdlog::sink_ptr a_extraSink= {}) noexcept
	{
		std::filesystem::path path{ std::filesystem::current_path() / LOG_PATH };

		path /= Plugin::NAME;
		path += ".log"sv;

		std::vector<spdlog::sink_ptr> sinks;
		if (a_extraSink && a_extraSink.get()) {
			sinks.push_back(a_extraSink);
		}

		sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true));

		for (auto& sink : sinks) {
#ifndef NDEBUG
			sink->set_pattern("[%i][%l](%s:%#) %v"s);
#else
			sink->set_pattern("[%T][%l](%s:%#) %v"s);
#endif
		}

		auto log = std::make_shared<spdlog::logger>("global log"s, sinks.begin(), sinks.end());

#ifndef NDEBUG
		log->set_level(spdlog::level::debug);
#else
		log->set_level(spdlog::level::info);
#endif
		log->flush_on(spdlog::level::debug);

		set_default_logger(std::move(log));

		INFO("{} v{} is loading...", Plugin::NAME, Plugin::VERSION);
	}
}  // namespace DKUtil::Logger


#undef DKU_L_VERSION_MAJOR
#undef DKU_L_VERSION_MINOR
#undef DKU_L_VERSION_REVISION

