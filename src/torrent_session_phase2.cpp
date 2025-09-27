#include "torrent_session_phase2.h"
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
    _dht_running = false;
    _download_rate_limit = 0; // 0 means unlimited
    _upload_rate_limit = 0;   // 0 means unlimited  
    _listen_port_min = 6881;  // Default BitTorrent port
    _listen_port_max = 6889;  // Default range
    
    UtilityFunctions::print("Phase 2: TorrentSession initialized with enhanced architecture");
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
    
    // Phase 2: Enhanced session startup with proper structure
    _session_settings = create_default_settings();
    
    // Phase 2: Simulate libtorrent session creation process
    try {
        // This would be: _lt_session = std::make_unique<libtorrent::session>(settings);
        _session_running = true;
        _dht_running = true; // DHT starts by default
        
        UtilityFunctions::print("Phase 2: Enhanced session started (architecture ready for libtorrent)");
        return true;
    } catch (...) {
        UtilityFunctions::print_rich("[color=red]Phase 2: Session startup failed[/color]");
        return false;
    }
}

bool TorrentSession::start_session_with_settings(Dictionary settings) {
    if (_session_running) {
        return true;
    }
    
    // Phase 2: Enhanced settings processing
    _session_settings = settings;
    apply_settings_dictionary(settings);
    
    try {
        // This would be: _lt_session = std::make_unique<libtorrent::session>(lt_settings);
        _session_running = true;
        
        UtilityFunctions::print("Phase 2: Enhanced session started with custom settings");
        return true;
    } catch (...) {
        UtilityFunctions::print_rich("[color=red]Phase 2: Session startup with settings failed[/color]");
        return false;
    }
}

void TorrentSession::stop_session() {
    if (!_session_running) {
        return;
    }
    
    // Phase 2: Enhanced shutdown process
    try {
        // This would be: _lt_session.reset();
        _session_running = false;
        _dht_running = false;
        
        UtilityFunctions::print("Phase 2: Enhanced session stopped");
    } catch (...) {
        UtilityFunctions::print_rich("[color=red]Phase 2: Error during session shutdown[/color]");
    }
}

bool TorrentSession::is_running() const {
    return _session_running;
}

void TorrentSession::set_download_rate_limit(int bytes_per_second) {
    _download_rate_limit = bytes_per_second;
    
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Phase 2: Session not running, download limit will be applied when session starts[/color]");
        return;
    }
    
    // Phase 2: Enhanced rate limiting with realistic behavior
    UtilityFunctions::print("Phase 2: Download rate limit applied: " + String::num(bytes_per_second) + " bytes/s");
}

void TorrentSession::set_upload_rate_limit(int bytes_per_second) {
    _upload_rate_limit = bytes_per_second;
    
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Phase 2: Session not running, upload limit will be applied when session starts[/color]");
        return;
    }
    
    // Phase 2: Enhanced rate limiting 
    generate_status_alert("Upload rate limit set to " + String::num(bytes_per_second) + " bytes/s", "settings_changed");
    UtilityFunctions::print("Phase 2: Upload rate limit applied: " + String::num(bytes_per_second) + " bytes/s");
}

void TorrentSession::set_listen_port(int port) {
    set_listen_port_range(port, port);
}

void TorrentSession::set_listen_port_range(int min_port, int max_port) {
    _listen_port_min = min_port;
    _listen_port_max = max_port;
    
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Phase 2: Session not running, port range will be applied when session starts[/color]");
        return;
    }
    
    // Phase 2: Enhanced port configuration
    generate_status_alert("Listen port range set to " + String::num(min_port) + "-" + String::num(max_port), "settings_changed");
    UtilityFunctions::print("Phase 2: Listen port range applied: " + String::num(min_port) + "-" + String::num(max_port));
}

bool TorrentSession::is_dht_running() {
    return _dht_running && _session_running;
}

void TorrentSession::start_dht() {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Phase 2: Cannot start DHT - session not running[/color]");
        return;
    }
    
    if (_dht_running) {
        UtilityFunctions::print("Phase 2: DHT already running");
        return;
    }
    
    // Phase 2: Enhanced DHT management
    _dht_running = true;
    generate_status_alert("DHT started", "dht_started");
    UtilityFunctions::print("Phase 2: DHT started (enhanced simulation)");
}

void TorrentSession::stop_dht() {
    if (!_session_running || !_dht_running) {
        UtilityFunctions::print("Phase 2: DHT not running");
        return;
    }
    
    // Phase 2: Enhanced DHT shutdown
    _dht_running = false;
    generate_status_alert("DHT stopped", "dht_stopped");
    UtilityFunctions::print("Phase 2: DHT stopped");
}

Ref<TorrentHandle> TorrentSession::add_torrent_file(PackedByteArray torrent_data, String save_path) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=red]Phase 2: Cannot add torrent - session not running[/color]");
        return Ref<TorrentHandle>();
    }
    
    // Phase 2: Enhanced torrent addition simulation
    Ref<TorrentHandle> handle = memnew(TorrentHandle);
    
    generate_status_alert("Torrent added from file (" + String::num(torrent_data.size()) + " bytes) to " + save_path, "torrent_added");
    UtilityFunctions::print("Phase 2: Enhanced torrent added from file data (" + String::num(torrent_data.size()) + " bytes)");
    
    return handle;
}

Ref<TorrentHandle> TorrentSession::add_magnet_uri(String magnet_uri, String save_path) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=red]Phase 2: Cannot add magnet - session not running[/color]");
        return Ref<TorrentHandle>();
    }
    
    // Phase 2: Enhanced magnet processing
    if (!magnet_uri.begins_with("magnet:")) {
        UtilityFunctions::print_rich("[color=red]Phase 2: Invalid magnet URI format[/color]");
        return Ref<TorrentHandle>();
    }
    
    Ref<TorrentHandle> handle = memnew(TorrentHandle);
    
    generate_status_alert("Magnet URI added: " + magnet_uri.substr(0, 60) + "...", "torrent_added");
    UtilityFunctions::print("Phase 2: Enhanced magnet URI processed");
    
    return handle;
}

bool TorrentSession::remove_torrent(Ref<TorrentHandle> handle, bool delete_files) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=red]Phase 2: Cannot remove torrent - session not running[/color]");
        return false;
    }
    
    if (handle.is_null()) {
        UtilityFunctions::print_rich("[color=red]Phase 2: Cannot remove torrent - invalid handle[/color]");
        return false;
    }
    
    // Phase 2: Enhanced torrent removal
    String action = delete_files ? "removed with file deletion" : "removed (files kept)";
    generate_status_alert("Torrent " + action, "torrent_removed");
    UtilityFunctions::print("Phase 2: Torrent " + action);
    
    return true;
}

Dictionary TorrentSession::get_session_stats() {
    Dictionary stats;
    
    if (!_session_running) {
        stats["error"] = "Session not running";
        return stats;
    }
    
    // Phase 2: Enhanced statistics simulation with realistic data structure
    stats["dht_nodes"] = _dht_running ? 1247 : 0;
    stats["has_incoming_connections"] = true;
    stats["download_rate"] = 0;  // No active downloads in simulation
    stats["upload_rate"] = 0;    // No active uploads in simulation
    stats["total_download"] = 0;
    stats["total_upload"] = 0;
    stats["num_peers"] = 0;
    stats["num_unchoked"] = 0;
    stats["allowed_upload_slots"] = 8;
    stats["up_bandwidth_queue"] = 0;
    stats["down_bandwidth_queue"] = 0;
    stats["optimistic_unchoke_counter"] = 0;
    stats["unchoke_counter"] = 0;
    stats["disk_read_queue"] = 0;
    stats["disk_write_queue"] = 0;
    stats["download_rate_limit"] = _download_rate_limit;
    stats["upload_rate_limit"] = _upload_rate_limit;
    stats["listen_port_min"] = _listen_port_min;
    stats["listen_port_max"] = _listen_port_max;
    
    return stats;
}

Array TorrentSession::get_alerts() {
    // Phase 2: Simplified alert system - return empty array for now
    Array alerts;
    // In Phase 3, this will return real alerts from libtorrent
    return alerts;
}

void TorrentSession::clear_alerts() {
    // Phase 2: Simplified alert clearing
    UtilityFunctions::print("Phase 2: Alerts cleared");
}

// Phase 2: Enhanced private methods
Dictionary TorrentSession::create_default_settings() {
    Dictionary settings;
    
    // Phase 2: Realistic default settings structure matching libtorrent
    settings["listen_interfaces"] = "0.0.0.0:" + String::num(_listen_port_min);
    settings["enable_dht"] = true;
    settings["enable_lsd"] = true;
    settings["enable_upnp"] = true;
    settings["enable_natpmp"] = true;
    settings["anonymous_mode"] = false;
    settings["user_agent"] = "godot-torrent/2.0";
    settings["download_rate_limit"] = _download_rate_limit;
    settings["upload_rate_limit"] = _upload_rate_limit;
    
    return settings;
}

void TorrentSession::apply_settings_dictionary(Dictionary settings) {
    // Phase 2: Enhanced settings processing
    if (settings.has("download_rate_limit")) {
        _download_rate_limit = settings["download_rate_limit"];
    }
    if (settings.has("upload_rate_limit")) {
        _upload_rate_limit = settings["upload_rate_limit"];  
    }
    if (settings.has("listen_port")) {
        int port = settings["listen_port"];
        _listen_port_min = port;
        _listen_port_max = port;
    }
    
    UtilityFunctions::print("Phase 2: Applied " + String::num(settings.size()) + " custom settings");
}

void TorrentSession::generate_status_alert(const String& message, const String& type) {
    // Phase 2: Simplified alert generation (removed complex data structures)
    UtilityFunctions::print("Phase 2 Alert [" + type + "]: " + message);
}

void TorrentSession::simulate_network_operations() {
    // Phase 2: Simulate realistic network initialization sequence
    generate_status_alert("Network interfaces initialized", "network_ready");
    generate_status_alert("Listening on port range " + String::num(_listen_port_min) + "-" + String::num(_listen_port_max), "port_listening");
    
    if (_dht_running) {
        generate_status_alert("DHT bootstrapping initiated", "dht_bootstrap");
    }
}