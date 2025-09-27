#include "torrent_session_stub.h"
#include "torrent_handle.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void TorrentSession::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start_session"), &TorrentSession::start_session);
    ClassDB::bind_method(D_METHOD("start_session_with_settings", "settings"), &TorrentSession::start_session_with_settings);
    ClassDB::bind_method(D_METHOD("stop_session"), &TorrentSession::stop_session);
    ClassDB::bind_method(D_METHOD("is_running"), &TorrentSession::is_running);
    
    ClassDB::bind_method(D_METHOD("set_download_rate_limit", "bytes_per_second"), &TorrentSession::set_download_rate_limit);
    ClassDB::bind_method(D_METHOD("set_upload_rate_limit", "bytes_per_second"), &TorrentSession::set_upload_rate_limit);
    ClassDB::bind_method(D_METHOD("set_listen_port", "port"), &TorrentSession::set_listen_port);
    ClassDB::bind_method(D_METHOD("set_listen_port_range", "min_port", "max_port"), &TorrentSession::set_listen_port_range);
    
    ClassDB::bind_method(D_METHOD("is_dht_running"), &TorrentSession::is_dht_running);
    ClassDB::bind_method(D_METHOD("start_dht"), &TorrentSession::start_dht);
    ClassDB::bind_method(D_METHOD("stop_dht"), &TorrentSession::stop_dht);
    
    ClassDB::bind_method(D_METHOD("add_torrent_file", "torrent_data", "save_path"), &TorrentSession::add_torrent_file);
    ClassDB::bind_method(D_METHOD("add_magnet_uri", "magnet_uri", "save_path"), &TorrentSession::add_magnet_uri);
    ClassDB::bind_method(D_METHOD("remove_torrent", "handle", "delete_files"), &TorrentSession::remove_torrent, DEFVAL(false));
    
    ClassDB::bind_method(D_METHOD("get_session_stats"), &TorrentSession::get_session_stats);
    ClassDB::bind_method(D_METHOD("get_alerts"), &TorrentSession::get_alerts);
    ClassDB::bind_method(D_METHOD("clear_alerts"), &TorrentSession::clear_alerts);
}

TorrentSession::TorrentSession() {
    _session_running = false;
}

TorrentSession::~TorrentSession() {
    if (_session_running) {
        stop_session();
    }
}

bool TorrentSession::start_session() {
    if (_session_running) {
        return true;
    }
    
    // STUB: Simulate session startup without libtorrent
    _session_running = true;
    UtilityFunctions::print("STUB: Torrent session started (without libtorrent)");
    return true;
}

bool TorrentSession::start_session_with_settings(Dictionary settings) {
    if (_session_running) {
        return true;
    }
    
    // STUB: Simulate session startup with custom settings
    _session_running = true;
    UtilityFunctions::print("STUB: Torrent session started with custom settings (without libtorrent)");
    return true;
}

void TorrentSession::stop_session() {
    if (!_session_running) {
        return;
    }
    
    _session_running = false;
    UtilityFunctions::print("STUB: Torrent session stopped");
}

bool TorrentSession::is_running() const {
    return _session_running;
}

void TorrentSession::set_download_rate_limit(int bytes_per_second) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]STUB: Session not running, cannot set download rate limit[/color]");
        return;
    }
    
    UtilityFunctions::print("STUB: Set download rate limit to " + String::num(bytes_per_second) + " bytes/s");
}

void TorrentSession::set_upload_rate_limit(int bytes_per_second) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]STUB: Session not running, cannot set upload rate limit[/color]");
        return;
    }
    
    UtilityFunctions::print("STUB: Set upload rate limit to " + String::num(bytes_per_second) + " bytes/s");
}

void TorrentSession::set_listen_port(int port) {
    set_listen_port_range(port, port);
}

void TorrentSession::set_listen_port_range(int min_port, int max_port) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]STUB: Session not running, cannot set listen ports[/color]");
        return;
    }
    
    UtilityFunctions::print("STUB: Set listen port range " + String::num(min_port) + "-" + String::num(max_port));
}

bool TorrentSession::is_dht_running() {
    return _session_running; // STUB
}

void TorrentSession::start_dht() {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]STUB: Session not running, cannot start DHT[/color]");
        return;
    }
    
    UtilityFunctions::print("STUB: DHT started");
}

void TorrentSession::stop_dht() {
    if (!_session_running) {
        return;
    }
    
    UtilityFunctions::print("STUB: DHT stopped");
}

Ref<TorrentHandle> TorrentSession::add_torrent_file(PackedByteArray torrent_data, String save_path) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=red]STUB: Session not running, cannot add torrent[/color]");
        return Ref<TorrentHandle>();
    }
    
    UtilityFunctions::print("STUB: Added torrent file to " + save_path);
    
    // Return a stub handle
    Ref<TorrentHandle> torrent_handle;
    torrent_handle.instantiate();
    return torrent_handle;
}

Ref<TorrentHandle> TorrentSession::add_magnet_uri(String magnet_uri, String save_path) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=red]STUB: Session not running, cannot add magnet[/color]");
        return Ref<TorrentHandle>();
    }
    
    UtilityFunctions::print("STUB: Added magnet URI to " + save_path);
    
    // Return a stub handle
    Ref<TorrentHandle> torrent_handle;
    torrent_handle.instantiate();
    return torrent_handle;
}

bool TorrentSession::remove_torrent(Ref<TorrentHandle> handle, bool delete_files) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]STUB: Session not running, cannot remove torrent[/color]");
        return false;
    }
    
    if (handle.is_null()) {
        UtilityFunctions::print_rich("[color=yellow]STUB: Invalid torrent handle[/color]");
        return false;
    }
    
    UtilityFunctions::print("STUB: Removed torrent (delete_files: " + String(delete_files ? "true" : "false") + ")");
    return true;
}

Dictionary TorrentSession::get_session_stats() {
    Dictionary stats;
    
    if (!_session_running) {
        return stats;
    }
    
    // STUB: Return fake stats
    stats["download_rate"] = 0;
    stats["upload_rate"] = 0;
    stats["total_download"] = 0;
    stats["total_upload"] = 0;
    stats["num_peers"] = 0;
    stats["dht_nodes"] = 0;
    
    return stats;
}

Array TorrentSession::get_alerts() {
    Array alerts;
    
    if (!_session_running) {
        return alerts;
    }
    
    // STUB: Return empty alerts
    return alerts;
}

void TorrentSession::clear_alerts() {
    // STUB: Nothing to clear
}

void TorrentSession::create_default_settings() {
    // STUB: This won't be called in stub mode
}

void TorrentSession::dictionary_to_settings_pack(Dictionary settings) {
    // STUB: This won't be called in stub mode
}