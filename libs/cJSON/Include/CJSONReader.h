#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <typeinfo>
#include "CJSON_Build_Data.h"

namespace efc 
{
#ifdef CJSON_STRINGTYPE_WSTRING
	typedef std::wstring CJString;
#endif
#ifdef CJSON_STRINGTYPE_STRING
	typedef std::string CJString;
#endif

	struct CJSONReturnStatus
	{
	public:
		enum class Code : uint8_t
		{
			OK = 0,
			STRING_EMPTY,
			UNEXPECTED_EOF,
			EXPECTED_KEY,
			EMPTY_KEY,
			EXPECTED_DECLARATION,
			UNKNOWN_LITERAL,
			UNEXPECTED_SCOPE_END,
			EXPECTED_SCOPE_CLOSURE,
			INVALID_ESCAPE_SEQUENCE,


			UNKNOWN_ERROR = -1
		};

		static std::wstring GetErrorString(Code errorCode)
		{
			switch (errorCode)
			{
			case efc::CJSONReturnStatus::Code::OK:
				return L"OK";
				break;
			case efc::CJSONReturnStatus::Code::STRING_EMPTY:
				return L"STRING_EMPTY";
				break;
			case efc::CJSONReturnStatus::Code::UNEXPECTED_EOF:
				return L"UNEXPECTED_EOF";
				break;
			case efc::CJSONReturnStatus::Code::EXPECTED_KEY:
				return L"EXPEKTED_KEY";
				break;
			case efc::CJSONReturnStatus::Code::EMPTY_KEY:
				return L"EMPTY_KEY";
				break;
			case efc::CJSONReturnStatus::Code::EXPECTED_DECLARATION:
				return L"EXPECTED_DECLARATION";
				break;
			case efc::CJSONReturnStatus::Code::UNKNOWN_LITERAL:
				return L"UNKNOWN_LITERAL";
				break;
			case efc::CJSONReturnStatus::Code::UNEXPECTED_SCOPE_END:
				return L"UNEXPECTED_SCOPE_END";
				break;
			case efc::CJSONReturnStatus::Code::EXPECTED_SCOPE_CLOSURE:
				return L"EXPECTED_SCOPE_CLOSURE";
				break;
			case efc::CJSONReturnStatus::Code::INVALID_ESCAPE_SEQUENCE:
				return L"INVALID_ESCAPE_SEQUENCE";
				break;
			default:
				return L"UNKNOWN_ERROR";
				break;
			}
		}

		CJSONReturnStatus(Code code, size_t position = 0, const char* sourceFileName = nullptr, size_t sourceFileLine = 0) 
			: 
			m_ReturnCode(code), 
			m_Position(position),
			m_SourceFileLine(sourceFileLine)
		{
			if (sourceFileName)
			{
				m_SourceFile = sourceFileName;
			}
		}

		_NODISCARD
		Code GetCode() const { return m_ReturnCode; }

		_NODISCARD
		size_t GetPosition() const { return m_Position; }

		_NODISCARD
		std::string GetSourceFile() const { return m_SourceFile; }
		
		_NODISCARD
		size_t GetSourceFileErrorPos() const { return m_SourceFileLine; }

	protected:
		Code m_ReturnCode;
		size_t m_Position;
		std::string m_SourceFile;
		size_t m_SourceFileLine;
	};

	enum class CJSONType : uint8_t
	{
		NONE = 0,
		Dictionary,
		Array,
		Integer, // Technically the JSON format does not have integers, however they are included anyway as they can be of use
		Float,
		String,
		Boolean,
		Null // Not to be confused with NONE, this one is a JSON type, NONE is for internal use
	};

	static std::string GetJSONTypeString(CJSONType type)
	{
		switch (type)
		{
		case efc::CJSONType::Dictionary:
			return "Dictionary";
			break;
		case efc::CJSONType::Array:
			return "Array";
			break;
		case efc::CJSONType::Integer:
			return "Integer";
			break;
		case efc::CJSONType::Float:
			return "Float";
			break;
		case efc::CJSONType::String:
			return "String";
			break;
		case efc::CJSONType::Boolean:
			return "Boolean";
			break;
		case efc::CJSONType::Null:
			return "Null";
			break;
		default:
			return "ERROR";
			break;
		}
	}

	class CJSONTypeError : public std::exception
	{
	public:
		CJSONTypeError(CJSONType expectedType, CJSONType givenType, std::string sourceFile, size_t sourceLine)
		{
			m_Message = "Expected type: " + GetJSONTypeString(expectedType) + "; got type: " + GetJSONTypeString(givenType) +
							" on line " + std::to_string(sourceLine) + " of file " + sourceFile;
		}

		const char* what() const throw()
		{
			return m_Message.c_str();
		}

	private:
		std::string m_Message;
	};

	#define CJSON_VALIDATE_TYPE(type) if(m_Type != type) throw CJSONTypeError(type, m_Type, __FILE__, __LINE__)

	class CJSONObject
	{
	public:
		inline CJSONType GetType() const { return m_Type; }
	protected:
		CJSONType m_Type = CJSONType::NONE;
	};

	class CJSONDict : public CJSONObject
	{
	public:
		CJSONDict() { m_Type = CJSONType::Dictionary; }
		~CJSONDict();

		inline void Add(CJString key, CJSONObject* value) { CJSON_VALIDATE_TYPE(CJSONType::Dictionary);  m_Value.emplace(key, value); }

		_NODISCARD
		inline CJSONObject* Get(CJString key) const { CJSON_VALIDATE_TYPE(CJSONType::Dictionary); return m_Value.at(key); }

		template<class T>
		_NODISCARD
		inline T* Get(CJString key) const { CJSON_VALIDATE_TYPE(CJSONType::Dictionary); return (T*)(m_Value.at(key)); }

		_NODISCARD
		inline const std::unordered_map<CJString, CJSONObject*>* GetMap() const { CJSON_VALIDATE_TYPE(CJSONType::Dictionary); return &m_Value; }

		_NODISCARD
		inline std::unordered_map<CJString, CJSONObject*>* GetMap() { CJSON_VALIDATE_TYPE(CJSONType::Dictionary); return &m_Value; }

	private:
		std::unordered_map<CJString, CJSONObject*> m_Value; // stack
	};

	class CJSONList : public CJSONObject
	{
	public:
		CJSONList() { m_Type = CJSONType::Array; }
		~CJSONList();

		inline void Add(CJSONObject* value) { CJSON_VALIDATE_TYPE(CJSONType::Array);  m_Value.push_back(value); }

		_NODISCARD
		inline CJSONObject* Get(size_t index) const { CJSON_VALIDATE_TYPE(CJSONType::Array); return m_Value.at(index); }

		template<class T>
		_NODISCARD
		inline T* Get(size_t index) const { CJSON_VALIDATE_TYPE(CJSONType::Array); return (T*)(m_Value.at(index)); }

		_NODISCARD
		inline size_t GetSize() const { CJSON_VALIDATE_TYPE(CJSONType::Array); return m_Value.size(); }

		_NODISCARD
		inline const std::vector<CJSONObject*>* GetList() const { CJSON_VALIDATE_TYPE(CJSONType::Array); return &m_Value; }

		_NODISCARD
		inline std::vector<CJSONObject*>* GetList() { CJSON_VALIDATE_TYPE(CJSONType::Array); return &m_Value; }

	private:
		std::vector<CJSONObject*> m_Value; // stack
	};

	template<class T>
	class CJSONIntBase : public CJSONObject
	{
	public:
		CJSONIntBase(T value) : m_Value(value) { m_Type = CJSONType::Integer; }

		inline operator T() const { CJSON_VALIDATE_TYPE(CJSONType::Integer);  return m_Value; }

		inline void SetValue(T value) { CJSON_VALIDATE_TYPE(CJSONType::Integer); m_Value = value; }

		_NODISCARD
		inline T GetValue() const { CJSON_VALIDATE_TYPE(CJSONType::Integer); return m_Value; }

	private:
		T m_Value;
	};

#ifdef CJSON_INTSIZE_32
	typedef CJSONIntBase<int32_t> CJSONInt;
	typedef int32_t JInt;
	#define ToInt(val) _wtoi(val)
#endif
#ifdef CJSON_INTSIZE_64
	typedef CJSONIntBase<int64_t> CJSONInt;
	typedef int64_t JInt;
	#define ToInt(val) _wtoi64(val)
#endif


	template<class T>
	class CJSONFloatBase : public CJSONObject
	{
	public:
		CJSONFloatBase(T value) : m_Value(value) { m_Type = CJSONType::Float; }

		inline void SetValue(T value) { CJSON_VALIDATE_TYPE(CJSONType::Float); m_Value = value; }

		_NODISCARD
		inline T GetValue() const { CJSON_VALIDATE_TYPE(CJSONType::Float); return m_Value; }

	private:
		T m_Value;
	};

#ifdef CJSON_FLOATTYPE_FLOAT
	typedef CJSONFloatBase<float> CJSONFloat;
#endif
#ifdef CJSON_FLOATTYPE_DOUBLE
	typedef CJSONFloatBase<double> CJSONFloat;
#endif

	class CJSONString : public CJSONObject
	{
	public:
		CJSONString(CJString value) : m_Value(value) { m_Type = CJSONType::String; }

		inline void SetValue(CJString value) { CJSON_VALIDATE_TYPE(CJSONType::String); m_Value = value; }

		_NODISCARD
		inline CJString GetValue() const { CJSON_VALIDATE_TYPE(CJSONType::String); return m_Value; }

		_NODISCARD
		inline CJString* GetRefValue() { CJSON_VALIDATE_TYPE(CJSONType::String); return &m_Value; }

	private:
		CJString m_Value;
	};

	class CJSONBool : public CJSONObject
	{
	public:
		CJSONBool(bool value) : m_Value(value) { m_Type = CJSONType::Boolean; }

		inline void SetValue(bool value) { CJSON_VALIDATE_TYPE(CJSONType::Boolean); m_Value = value; }

		_NODISCARD
		inline bool GetValue() const { CJSON_VALIDATE_TYPE(CJSONType::Boolean); return m_Value; }

	private:
		bool m_Value;
	};

	class CJSONNull : public CJSONObject
	{
	public: 
		CJSONNull() { m_Type = CJSONType::Null; }
	};

	struct CJSONResult
	{
		CJSONResult(CJSONDict* dict, CJSONReturnStatus status, std::wstring errorMessage)
			:
			Dictionary(dict),
			Status(status),
			ErrorMessage(errorMessage)
		{}

		~CJSONResult()
		{
			delete Dictionary;
		}

		const CJSONDict* Dictionary;
		const CJSONReturnStatus Status;
		const std::wstring ErrorMessage;
	};

	class CJSONReader
	{
	public:
		CJSONReader(CJString inStr);
		~CJSONReader() { if (m_OwnsRoot) delete m_Root; }

		[[deprecated("Replaced by CJSONResult FromString(std::wstring string)")]]
		static CJSONDict* FromString(std::wstring string, CJSONReturnStatus* outStatus, std::wstring* outErrorMessage = nullptr); // Deprecated
		static CJSONResult FromString(std::wstring string);

		std::wstring ParseError(CJSONReturnStatus status) const;

		_NODISCARD
		inline CJSONDict* GetRoot() { m_OwnsRoot = false; return m_Root; } // Transfers ownership of root, root must be deleted later

		_NODISCARD
		inline bool GetIsRootOwner() const { return m_OwnsRoot; }

		_NODISCARD
		inline std::wstring GetString() const { return m_String; }

		CJSONReturnStatus ParseString();

		//_NODISCARD
		//static JSONDict FromString(efstring inStr);

	private:
		void SkipEmpty();
		inline CJSONReturnStatus ReadString(std::wstring* outString);
		inline CJSONReturnStatus ReadNextPair();

		CJSONReturnStatus ReadDict(bool requireClosure = true);
		CJSONReturnStatus ReadList();

		CJSONReturnStatus ParseObject(CJSONObject** outObject);

		_NODISCARD
		inline bool IsEOF() const { return m_Cursor >= m_StringSize; }

		_NODISCARD
		inline wchar_t GetCurrentChar() { return m_String[m_Cursor]; }

		_NODISCARD
		inline bool CheckCommentStart(bool isInComment, int32_t* outMLCommentCount);

		size_t m_Cursor = 0;

		const CJString m_String;
		const size_t m_StringSize;

		//bool m_StartsWithBrace = false;

		CJSONDict* m_Root;
		bool m_OwnsRoot = true;

		CJSONObject* m_CurrentObject;
	};
}