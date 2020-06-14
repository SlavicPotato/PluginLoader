#pragma once

#include "INIReader.h"

class IConfig
{
public:
	int Load(const std::string &path)
	{
		m_confReader.Load(path);
		return m_confReader.ParseError();
	}

	__inline
	bool Exists(const std::string& sect, const std::string& key)
	{
		return m_confReader.Exists(sect, key);
	}

	__inline
	int32_t Get(const std::string& sect, const std::string& key, int32_t def) const
	{
		return static_cast<int32_t>(m_confReader.GetInteger(sect, key, static_cast<long>(def)));
	}

	__inline
	uint32_t Get(const std::string& sect, const std::string& key, uint32_t def) const
	{
		return static_cast<uint32_t>(m_confReader.GetInteger(sect, key, static_cast<long>(def)));
	}

	__inline
	float Get(const std::string& sect, const std::string& key, float def) const
	{
		return m_confReader.GetFloat(sect, key, def);
	}

	__inline
	double Get(const std::string& sect, const std::string& key, double def) const
	{
		return m_confReader.GetReal(sect, key, def);
	}

	__inline
	bool Get(const std::string& sect, const std::string& key, bool def) const
	{
		return m_confReader.GetBoolean(sect, key, def);
	}

	__inline
	std::string Get(const std::string& sect, const std::string& key, const std::string& def) const
	{
		return m_confReader.Get(sect, key, def);
	}

	__inline
		std::string Get(const std::string& sect, const std::string& key, const char* def) const
	{
		return m_confReader.Get(sect, key, def);
	}

	template <typename T>
	__inline
	T Get(const std::string& sect, const std::string& key, T def) const
	{
		return static_cast<T>(m_confReader.GetInteger(sect, key, static_cast<long>(def)));
	}

private:
	INIReader m_confReader;
};