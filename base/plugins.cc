#include "plugins.h"


vector<void(*)()>* plugin_choosers() {
	static vector<void(*)()> choosers;
	return &choosers;
}


void no_plugin_error(PluginId id) {
	std::cerr << "ERR: no_plugin_error: No plugin provided for base plugin '" << id << "'" << endl;
	std::cerr << "\tmake sure any external plugins are being loaded correctly" << endl;
	std::terminate();
}

void cant_find_plugin_error(PluginId baseid, PluginId search_id) {
	std::cerr << "ERR: cant_find_plugin_error: Cannot find plugin '" << search_id;
	std::cerr << "' of base type '" << baseid << "'" << endl;
	std::cerr << "\tmake sure any external plugins are being loaded correctly" << endl;
	std::terminate();
}

PluginLoader::PluginLoader() {
	
}

void PluginLoader::load() {
	choose_plugins();
}

void PluginLoader::choose_plugins() {
	for (void (*func)() : *plugin_choosers()) {
		func();
	}
}

PluginLoader pluginloader;
