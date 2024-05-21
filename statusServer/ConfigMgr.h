#pragma once
#include "const.h"

struct SectionInfo {
	SectionInfo(){}
	~SectionInfo() { _section_datas.clear();}

	SectionInfo(const SectionInfo& src) {
		_section_datas = src._section_datas;
	}

	SectionInfo& operator=(const SectionInfo& src) {
		if (&src == this)
			return *this;
		_section_datas = src._section_datas;
		return *this;
	}

	std::map<std::string, std::string> _section_datas;

	std::string operator[](std::string key) {
		if (_section_datas.find(key) == _section_datas.end())
			return "";
		return _section_datas[key];
	}
};

class ConfigMgr
{
public:
	~ConfigMgr() {
		_config_map.clear();
	}

	SectionInfo operator[](std::string section_name) {
		if (_config_map.find(section_name) == _config_map.end())
			return SectionInfo();
		return _config_map[section_name];
	}

	static ConfigMgr& GetInstance() {
		static ConfigMgr config_mgr;
		return config_mgr;
	}
private:
	std::map<std::string, SectionInfo> _config_map;
	
	ConfigMgr();
	ConfigMgr(const ConfigMgr&) = delete;
	ConfigMgr& operator=(const ConfigMgr&) = delete;
};

