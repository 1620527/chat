#include "ConfigMgr.h"
#include "const.h"

ConfigMgr::ConfigMgr() {
	boost::filesystem::path current_path = boost::filesystem::current_path();
	boost::filesystem::path config_path = current_path / "config.ini";

	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);

	for (const auto& pair_section : pt) {
		const std::string& section_name = pair_section.first;
		const boost::property_tree::ptree& section_tree = pair_section.second;
		std::map<std::string, std::string> section_config;
		for (const auto& key_value_pair : section_tree) {
			const std::string& key = key_value_pair.first;
			const std::string& value = key_value_pair.second.get_value<std::string>();
			section_config[key] = value;
		}

		SectionInfo sectionInfo;
		sectionInfo._section_datas = section_config;
		_config_map[section_name] = sectionInfo;
	}

	for (const auto& section_entry : _config_map) {
		std::string section_name = section_entry.first;
		SectionInfo section_config = section_entry.second;
		print("[", section_name, "]:");
		for (const auto& key_value_pair : section_config._section_datas) {
			print(key_value_pair.first, "=", key_value_pair.second);
		}
	}
}