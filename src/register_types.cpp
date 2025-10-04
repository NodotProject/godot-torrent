#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

// Include your classes here when implemented
#include "torrent_session.h"
#include "torrent_handle.h"
#include "torrent_info.h"
#include "torrent_status.h"
#include "peer_info.h"
#include "alert_manager.h"
#include "torrent_error.h"
#include "torrent_result.h"
#include "torrent_logger.h"

using namespace godot;

void initialize_godot_torrent_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    
    // Register error handling and logging classes first
    ClassDB::register_class<TorrentError>();
    ClassDB::register_class<TorrentResult>();
    ClassDB::register_class<TorrentLogger>();

    // Register your classes here when implemented
    ClassDB::register_class<TorrentSession>();
    ClassDB::register_class<TorrentHandle>();
    ClassDB::register_class<TorrentInfo>();
    ClassDB::register_class<TorrentStatus>();
    ClassDB::register_class<PeerInfo>();
    ClassDB::register_class<AlertManager>();
}

void uninitialize_godot_torrent_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    
    // Nothing to do here for now
}

extern "C" {
    // Initialization
    GDExtensionBool GDE_EXPORT godot_torrent_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_godot_torrent_module);
        init_obj.register_terminator(uninitialize_godot_torrent_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}