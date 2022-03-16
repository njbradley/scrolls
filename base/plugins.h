#ifndef SCROLLS_PLUGINS
#define SCROLLS_PLUGINS

#include "common.h"

/*
this file defines the plugin system used to allow parts
of the project to be swapped easily, for example to allow
for different graphics implementations.

When making a base plugin, use the BASE_PLUGIN(...) macro in the class
definition, and then use the DEFINE_PLUGIN(...) or DEFINE_AND_EXPORT_PLUGIN(...)
macro in a .cc file. (Choose between them depending on whether you want the base
class to be a valid plugin, for example if your base class is abstract you woudln't
want to export it)

When making a plugin, inherit from the base plugin class, and use the macro
PLUGIN(...) macro in the class definition. Then use the EXPORT_PLUGIN(...)
macro in a .cc file to export it.

At runtime, all exported plugins are collected (including any plugins that are
in dlls loaded at runtime), and the "deepest" child
is chosen to be the selected plugin. Calls to BaseType::plugnew(...) will
return instances of that type. Make sure to delete these instances with
plugdelete(...)
*/



using PluginId = string;

vector<void(*)()>* plugin_choosers();

void no_plugin_error(PluginId id);
void cant_find_plugin_error(PluginId baseid, PluginId search_id);

template <typename BaseType>
void choose_plugin();

template <typename BaseType>
struct PluginDef {
	using ctor_func = typename BaseType::ctor_func;
	using destr_func = typename BaseType::destr_func;
	
	ctor_func newfunc = nullptr;
	destr_func delfunc = nullptr;
	PluginId id;
	int level;
	PluginDef<BaseType>* next = nullptr;
	PluginDef<BaseType>* children = nullptr;
	
	PluginDef(PluginId newid): id(newid), level(0) {
		plugin_choosers()->push_back(choose_plugin<BaseType>);
	}
	
	PluginDef(PluginId newid, PluginDef<BaseType>* parent): id(newid) {
		next = parent->children;
		parent->children = this;
		level = parent->level + 1;
	}
	
	void export_plugin(ctor_func nfunc, destr_func dfunc) {
    newfunc = nfunc;
    delfunc = dfunc;
  }
	
	PluginDef<BaseType>* find(PluginId search_id);
	PluginDef<BaseType>* find_deepest();
};


template <typename Func>
struct MakeCtorFunc {

};

template <typename Btype, typename ... Args>
struct MakeCtorFunc<Btype* (*)(Args...)> {
	template <typename CType>
	struct GetFunc {
		static Btype* newfunc(Args... args) {
			return new CType(args...);
		}
		static void delfunc(Btype* ptr) {
			delete ptr;
		}
	};
};



template <typename Ptype>
struct ExportPlugin {
	using BaseType = typename Ptype::Plugin_BaseType;
	using Funcs = typename MakeCtorFunc<typename Ptype::ctor_func>::template GetFunc<Ptype>;
	
	ExportPlugin() {
		Ptype::plugindef()->export_plugin(Funcs::newfunc, Funcs::delfunc);
	}
};

template <typename Ptype, Ptype* ptr>
struct ExportPluginSingleton {
	using BaseType = typename Ptype::Plugin_BaseType;
	
	PluginDef<BaseType> plugindef;
	ExportPluginSingleton(const char* name): plugindef(PluginId(name), Ptype::plugindef()) {
		plugindef.export_plugin(newfunc, delfunc);
	}
	static BaseType* newfunc() {
		return ptr;
	}
	static void delfunc(BaseType* newptr) {
		
	}
};

template <typename NewType>
struct RequirePlugin {
	void** plugptr;
	RequirePlugin(void** ptr): plugptr(ptr) { }
	NewType* operator->() {
		return (NewType*) *plugptr;
	}
};

#define CONCAT_INNER(a, b) a ## b
#define CONCAT(a, b) CONCAT_INNER(a, b)

#define UNIQUENAME(name) CONCAT(name, __COUNTER__)


#define DEFINE_PLUGIN(X) \
	static_assert(std::is_same<X,X::Plugin_BaseType>::value, \
		"Only base plugins (defined with the macro BASE_PLUGIN_HEAD in the class definition) " \
		"can be used with the DEFINE_PLUGIN macro"); \
	PluginDef<X>* X::plugindef() { \
		static PluginDef<X> plugdef (PluginId(#X)); \
		return &plugdef; \
	} \
	PluginDef<X>* X::selected_plugin = nullptr;

#define DEFINE_PLUGIN_TEMPLATE(X) public: \
	static_assert(std::is_same<X,X::Plugin_BaseType>::value, \
		"Only base plugins (defined with the macro BASE_PLUGIN_HEAD in the class definition) " \
		"can be used with the DEFINE_PLUGIN macro"); \
	template <> \
	PluginDef<X>* X::plugindef() { \
		static PluginDef<X> plugdef (PluginId(#X)); \
		return &plugdef; \
	} \
	template <> \
	PluginDef<X>* X::selected_plugin = nullptr;


#define DEFINE_AND_EXPORT_PLUGIN(X) \
	DEFINE_PLUGIN(X); \
	static ExportPlugin<X> UNIQUENAME(_export_plugin_);

#define DEFINE_AND_EXPORT_PLUGIN_TEMPLATE(X) \
	DEFINE_PLUGIN_TEMPLATE(X); \
	static ExportPlugin<X> UNIQUENAME(_export_plugin_);


#define EXPORT_PLUGIN(X) \
	static_assert(!std::is_same<X,X::Plugin_BaseType>::value, \
		"Only non base plugins can be used with the EXPORT_PLUGIN macro. " \
		"If you are trying to export a base plugin, use the DEFINE_AND_EXPORT_PLUGIN macro"); \
	PluginDef<X::Plugin_BaseType>* X::plugindef() { \
		static PluginDef<X::Plugin_BaseType> plugdef (PluginId(#X), X::Plugin_ParentType::plugindef()); \
		return &plugdef; \
	} \
	static ExportPlugin<X> UNIQUENAME(_export_plugin_);

#define EXPORT_PLUGIN_TEMPLATE(X) \
	static_assert(!std::is_same<X,X::Plugin_BaseType>::value, \
		"Only non base plugins can be used with the EXPORT_PLUGIN macro. " \
		"If you are trying to export a base plugin, use the DEFINE_AND_EXPORT_PLUGIN macro"); \
	template <> \
	PluginDef<X::Plugin_BaseType>* X::plugindef() { \
		static PluginDef<X::Plugin_BaseType> plugdef (PluginId(#X), X::Plugin_ParentType::plugindef()); \
		return &plugdef; \
	} \
	static ExportPlugin<X> UNIQUENAME(_export_plugin_);


#define EXPORT_PLUGIN_SINGLETON(X) \
	static ExportPluginSingleton<std::remove_pointer<decltype(X)>::type,X> UNIQUENAME(_export_singleton_) (#X);


/*
Makes a class a base plugin. Put in the class defenition, like:
class World {
	BASE_PLUGIN(World, (string name));
	....
The parameters are:
	X is the type,
	params is a list of the parameters of the constructor, surrounded in parentesis.
			There can be other constructors on the type, this is just the parameters
			that plugnew() will take.
*/
#define BASE_PLUGIN(X, params) public: \
	typedef X Plugin_BaseType; \
	typedef X Plugin_Type; \
	\
	typedef X* (*ctor_func) params; \
	typedef void (*destr_func) (X*); \
	\
	static PluginDef<X>* plugindef(); \
	static PluginDef<X>* selected_plugin; \
	\
	template <typename ... Args> \
	static X* plugnew(Args&& ... args) { \
		if (selected_plugin == nullptr) no_plugin_error(plugindef()->id); \
		return selected_plugin->newfunc(std::forward<Args>(args)...); \
	} \
	template <typename ... Args> \
	static X* plugnew(PluginId search_id, Args&& ... args) { \
		PluginDef<X>* def = plugindef()->find(search_id); \
		if (def == nullptr) cant_find_plugin_error(def->id, search_id); \
		return def->newfunc(std::forward<Args>(args)...); \
	} \
	virtual PluginDef<X>* get_plugindef() const { return plugindef(); }

#define PLUGIN(X...) public: \
	static PluginDef<Plugin_BaseType>* plugindef(); \
	typedef Plugin_Type Plugin_ParentType; \
	typedef X Plugin_Type; \
	virtual PluginDef<Plugin_BaseType>* get_plugindef() const { return plugindef(); }



#define REQUIRE_PLUGIN(oldname, newname, NewType) \
	RequirePlugin<NewType> newname = RequirePlugin<NewType>(&oldname);


template <typename T>
void plugdelete(T* ptr) {
	ptr->get_plugindef()->delfunc(ptr);
}

class PluginLoader {
public:
	PluginLoader();
	void load();
	void choose_plugins();
	
	string find_path(string search_path);
};

extern PluginLoader pluginloader;














template <typename BaseType>
void choose_plugin() {
	cout << "Choosing plugin " << BaseType::plugindef()->id << " = ";
	BaseType::selected_plugin = BaseType::plugindef()->find_deepest();
	if (BaseType::selected_plugin != nullptr) {
		cout << BaseType::selected_plugin->id << endl;
	} else {
		cout << endl;
	}
}

template <typename BaseType>
PluginDef<BaseType>* PluginDef<BaseType>::find(PluginId search_id) {
	if (search_id == id) {
		return this;
	}
	PluginDef<BaseType>* result = nullptr;
	if (children != nullptr) {
		result = children->find(search_id);
	}
	if (result != nullptr and next != nullptr) {
		result = next->find(search_id);
	}
	return result;
}

template <typename BaseType>
PluginDef<BaseType>* PluginDef<BaseType>::find_deepest() {
	PluginDef<BaseType>* def = children;
	PluginDef<BaseType>* chosen = this;
	while (def != nullptr) {
		PluginDef<BaseType>* result = def->find_deepest();
		if (result != nullptr and result->newfunc != nullptr and result->level > chosen->level) {
			chosen = result;
		}
		def = def->next;
	}
	return chosen;
}
















template <typename BaseType>
class Plugin {
	BaseType* pointer;
public:
	template <typename ... Args>
	Plugin(Args&& ... args) {
		pointer = BaseType::plugnew(std::forward<Args>(args)...);
	}
	~Plugin() {
		plugdelete(pointer);
	}
	
	BaseType* operator->() { return pointer; }
	operator BaseType*() { return pointer; }
};






#endif
