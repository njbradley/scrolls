#include "plugins.h"


vector<void(*)()>* plugin_choosers() {
	static vector<void(*)()> choosers;
	return &choosers;
}


void no_plugin_error(PluginId id) {
	std::cerr << "ERR: no_plugin_error: No plugin provided for base plugin '" << id << "'" << endl;
	std::cerr << "\tmake sure any external plugins are being loaded correctly" << endl;
	exit(1);
}

void cant_find_plugin_error(PluginId baseid, PluginId search_id) {
	std::cerr << "ERR: cant_find_plugin_error: Cannot find plugin '" << search_id;
	std::cerr << "' of base type '" << baseid << "'" << endl;
	std::cerr << "\tmake sure any external plugins are being loaded correctly" << endl;
	exit(1);
}

PluginLoader::PluginLoader() {
	
}

void PluginLoader::load(int numargs, char** args) {
	for (int i = 0; i < numargs; i ++) {
		parse_request(args[i]);
	}
	choose_plugins();
}

void PluginLoader::choose_plugins() {
	for (void (*func)() : *plugin_choosers()) {
		func();
	}
}

void PluginLoader::parse_request(string request) {
	string basename = request.substr(0, request.find('='));
	string reqname = request.substr(request.find('=')+1);
	requirements.push_back(PluginReq{basename, reqname});
}

string PluginLoader::find_path(string search_path) {
	return "resources/" + search_path;
}

PluginLoader pluginloader;
