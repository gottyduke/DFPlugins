#pragma once


#include "Data.hpp"

#include "SimpleIni.h"


namespace DKUtil::Config::detail
{
	class TokenDescriptor
	{
	};


	class TokenAccounter
	{
	};


	class Token
	{
		struct meta
		{
			std::string_view file;
			std::size_t line;
			std::size_t pos;
		};

		struct eval_res_t
		{

		};

	public:
		explicit constexpr Token(std::string_view a_value, meta a_meta, Token* a_prev = nullptr, Token* a_next = nullptr) noexcept :
			_value(a_value), _meta(a_meta), _prev(a_prev), _next(a_next)
		{}

		// accessor
		[[nodiscard]] constexpr std::string_view operator*() const noexcept { return get(); }
		[[nodiscard]] constexpr std::string_view get() const noexcept { return _value; }
		[[nodiscard]] constexpr auto& get_meta() const noexcept { return _meta; }
		[[nodiscard]] constexpr std::string_view file_name() const noexcept { return _meta.file; }
		[[nodiscard]] constexpr auto line() const noexcept { return _meta.line; }
		[[nodiscard]] constexpr auto pos() const noexcept { return _meta.pos; }
		[[nodiscard]] constexpr auto* prev() const noexcept { return _prev; }
		[[nodiscard]] constexpr auto* next() const noexcept { return _next; }
		[[nodiscard]] std::string meta_str() noexcept { return fmt::format("@ FILE [{}], Line [{}], Pos [{}]", _meta.file, _meta.line, _meta.pos); }

		// processor
		virtual eval_res_t Process() noexcept = 0;

		// derived
		template <std::derived_from<Token> derived_t>
		[[nodiscard]] constexpr derived_t* As() noexcept
		{
			return dynamic_cast<derived_t*>(this);
		}

	private:
		const std::string _value;
		const meta _meta;
		Token* _prev{ nullptr };
		Token* _next{ nullptr };
	};


	class Schema final : public IParser
	{
	public:
		using IParser::IParser;
		using buf_iterator = std::istreambuf_iterator<char>;

		// assume file exists
		Schema(std::string_view a_file, const std::uint32_t a_id, manager& a_manager) noexcept :
			IParser(a_file, a_id, a_manager)
		{
			std::basic_ifstream<char> file{ _filepath.data(), std::ios_base::binary | std::ios_base::in };
			if (!file.is_open() || !file) {
				ERROR("DKU_C: Parser#{}: Parsing file failed! -> {}\nifstream cannot be opened", _id, _filepath);
			}

			_content = { buf_iterator{ file }, {} };
		}

		// accessor
		[[nodiscard]] constexpr const auto& get_tokens() const noexcept { return _tokens; }
		[[nodiscard]] auto& get_lines() const noexcept 
		{
			static std::vector<std::string> Lines{ dku::string::split(_content, "\n") };
			return Lines;
		}
		[[nodiscard]] auto& get_line(std::size_t a_line) const noexcept
		{
			const auto& lines = get_lines();
			return a_line < lines.size() ? lines[a_line] : lines[lines.size()];
		}

		// tokenizer

		// override
		void Parse(const char* a_data) noexcept override
		{
			DEBUG("DKU_C: Parser#{}: Parsing finished", _id);
		}

		void Write(const std::string_view a_filePath) noexcept override
		{
		}

	private:
		std::vector<Token*> _tokens;
	};
}