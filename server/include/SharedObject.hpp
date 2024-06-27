#pragma once

#include <string>

class SharedObject final
{
public:
	
	SharedObject();
	~SharedObject();
	
	bool Open(const std::string &file);
	void Close();
	
	template<typename T>
	T GetSymbol(const std::string &symbol)
	{
		return (T)GetVoidSymbol(symbol);
	}
	
	void *GetVoidSymbol(const std::string &symbol);
	
	inline bool IsValid() const { return handle != nullptr; }
	inline const std::string &GetErrorMessage() const { return errorMessage; }
	inline const std::string &GetFilePath() const { return filePath; }
	
public:
	void *handle = nullptr;
	std::string errorMessage;
	std::string filePath;
};
