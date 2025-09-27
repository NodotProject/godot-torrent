#include "torrent_session.h"
#include "torrent_handle.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_info.hpp>

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
    
    try {
        libtorrent::settings_pack settings = create_default_settings();
        _session = std::make_unique<libtorrent::session>(settings);
        _session_running = true;
        
        UtilityFunctions::print("Torrent session started successfully");
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::print_rich("[color=red]Failed to start torrent session: " + String(e.what()) + "[/color]");
        return false;
    }
}

bool TorrentSession::start_session_with_settings(Dictionary settings) {
    if (_session_running) {
        return true;
    }
    
    try {
        libtorrent::settings_pack lt_settings = dictionary_to_settings_pack(settings);
        _session = std::make_unique<libtorrent::session>(lt_settings);
        _session_running = true;
        
        UtilityFunctions::print("Torrent session started with custom settings");
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::print_rich("[color=red]Failed to start torrent session: " + String(e.what()) + "[/color]");
        return false;
    }
}

void TorrentSession::stop_session() {
    if (!_session_running) {
        return;
    }
    
    try {
        _session.reset();
        _session_running = false;
        UtilityFunctions::print("Torrent session stopped");
    } catch (const std::exception& e) {
        UtilityFunctions::print_rich("[color=red]Error stopping torrent session: " + String(e.what()) + "[/color]");
    }
}

bool TorrentSession::is_running() const {
    return _session_running;
}

void TorrentSession::set_download_rate_limit(int bytes_per_second) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Session not running, cannot set download rate limit[/color]");
        return;
    }
    
    libtorrent::settings_pack settings;
    settings.set_int(libtorrent::settings_pack::download_rate_limit, bytes_per_second);
    _session->apply_settings(settings);
}

void TorrentSession::set_upload_rate_limit(int bytes_per_second) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Session not running, cannot set upload rate limit[/color]");
        return;
    }
    
    libtorrent::settings_pack settings;
    settings.set_int(libtorrent::settings_pack::upload_rate_limit, bytes_per_second);
    _session->apply_settings(settings);
}

void TorrentSession::set_listen_port(int port) {
    set_listen_port_range(port, port);
}

void TorrentSession::set_listen_port_range(int min_port, int max_port) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Session not running, cannot set listen ports[/color]");
        return;
    }
    
    std::string listen_interfaces = "0.0.0.0:" + std::to_string(min_port);
    if (min_port != max_port) {
        listen_interfaces += "-" + std::to_string(max_port);
    }
    
    libtorrent::settings_pack settings;
    settings.set_str(libtorrent::settings_pack::listen_interfaces, listen_interfaces);
    _session->apply_settings(settings);
}

bool TorrentSession::is_dht_running() {
    if (!_session_running) {
        return false;
    }
    
    return _session->is_dht_running();
}

void TorrentSession::start_dht() {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Session not running, cannot start DHT[/color]");
        return;
    }
    
    libtorrent::settings_pack settings;
    settings.set_bool(libtorrent::settings_pack::enable_dht, true);
    _session->apply_settings(settings);
}

void TorrentSession::stop_dht() {
    if (!_session_running) {
        return;
    }
    
    libtorrent::settings_pack settings;
    settings.set_bool(libtorrent::settings_pack::enable_dht, false);
    _session->apply_settings(settings);
}

Ref<TorrentHandle> TorrentSession::add_torrent_file(PackedByteArray torrent_data, String save_path) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=red]Session not running, cannot add torrent[/color]");
        return Ref<TorrentHandle>();
    }
    
    try {
        // Create torrent info from the data
        std::string data_str(reinterpret_cast<const char*>(torrent_data.ptr()), torrent_data.size());
        auto ti = std::make_shared<libtorrent::torrent_info>(data_str);
        
        // Set up add torrent params
        libtorrent::add_torrent_params params;
        params.ti = ti;
        params.save_path = save_path.utf8().get_data();
        
        // Add torrent to session
        libtorrent::torrent_handle handle = _session->add_torrent(params);
        
        // Create and return TorrentHandle wrapper
        Ref<TorrentHandle> torrent_handle;
        torrent_handle.instantiate();
        torrent_handle->_set_internal_handle(handle);
        
        return torrent_handle;
    } catch (const std::exception& e) {
        UtilityFunctions::print_rich("[color=red]Failed to add torrent: " + String(e.what()) + "[/color]");
        return Ref<TorrentHandle>();
    }
}

Ref<TorrentHandle> TorrentSession::add_magnet_uri(String magnet_uri, String save_path) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=red]Session not running, cannot add magnet[/color]");
        return Ref<TorrentHandle>();
    }
    
    try {
        // Parse magnet URI
        libtorrent::add_torrent_params params = libtorrent::parse_magnet_uri(magnet_uri.utf8().get_data());
        params.save_path = save_path.utf8().get_data();
        
        // Add torrent to session
        libtorrent::torrent_handle handle = _session->add_torrent(params);
        
        // Create and return TorrentHandle wrapper
        Ref<TorrentHandle> torrent_handle;
        torrent_handle.instantiate();
        torrent_handle->_set_internal_handle(handle);
        
        return torrent_handle;
    } catch (const std::exception& e) {
        UtilityFunctions::print_rich("[color=red]Failed to add magnet URI: " + String(e.what()) + "[/color]");
        return Ref<TorrentHandle>();
    }
}

bool TorrentSession::remove_torrent(Ref<TorrentHandle> handle, bool delete_files) {
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Session not running, cannot remove torrent[/color]");
        return false;
    }
    
    if (handle.is_null()) {
        UtilityFunctions::print_rich("[color=yellow]Invalid torrent handle[/color]");
        return false;
    }
    
    try {
        libtorrent::torrent_handle lt_handle = handle->_get_internal_handle();
        libtorrent::remove_flags_t flags = delete_files ? libtorrent::session::delete_files : libtorrent::remove_flags_t{};
        _session->remove_torrent(lt_handle, flags);
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::print_rich("[color=red]Failed to remove torrent: " + String(e.what()) + "[/color]");
        return false;
    }
}

Dictionary TorrentSession::get_session_stats() {
    Dictionary stats;
    
    if (!_session_running) {
        return stats;
    }
    
    try {
        auto session_stats = _session->get_stats();
        
        stats["download_rate"] = session_stats.download_rate;
        stats["upload_rate"] = session_stats.upload_rate;
        stats["total_download"] = static_cast<int64_t>(session_stats.total_download);
        stats["total_upload"] = static_cast<int64_t>(session_stats.total_upload);
        stats["num_peers"] = session_stats.num_peers;
        stats["dht_nodes"] = session_stats.dht_nodes;
        
        return stats;
    } catch (const std::exception& e) {
        UtilityFunctions::print_rich("[color=red]Failed to get session stats: " + String(e.what()) + "[/color]");
        return stats;
    }
}

Array TorrentSession::get_alerts() {
    Array alerts;
    
    if (!_session_running) {
        return alerts;
    }
    
    try {
        std::vector<libtorrent::alert*> alert_list;
        _session->pop_alerts(&alert_list);
        
        for (auto alert : alert_list) {
            Dictionary alert_dict;
            alert_dict["type"] = alert->type();
            alert_dict["message"] = String(alert->message().c_str());
            alert_dict["timestamp"] = static_cast<int64_t>(alert->timestamp().time_since_epoch().count());
            alerts.append(alert_dict);
        }
        
        return alerts;
    } catch (const std::exception& e) {
        UtilityFunctions::print_rich("[color=red]Failed to get alerts: " + String(e.what()) + "[/color]");
        return alerts;
    }
}

void TorrentSession::clear_alerts() {
    if (!_session_running) {
        return;
    }
    
    std::vector<libtorrent::alert*> alerts;
    _session->pop_alerts(&alerts);
}

libtorrent::settings_pack TorrentSession::create_default_settings() {
    libtorrent::settings_pack settings;
    
    // Basic settings
    settings.set_str(libtorrent::settings_pack::user_agent, "Godot-Torrent/1.0.0");
    settings.set_str(libtorrent::settings_pack::listen_interfaces, "0.0.0.0:6881");
    
    // Enable DHT
    settings.set_bool(libtorrent::settings_pack::enable_dht, true);
    
    // Enable local peer discovery
    settings.set_bool(libtorrent::settings_pack::enable_lsd, true);
    
    // Enable UPnP and NAT-PMP
    settings.set_bool(libtorrent::settings_pack::enable_upnp, true);
    settings.set_bool(libtorrent::settings_pack::enable_natpmp, true);
    
    // Alert settings
    settings.set_int(libtorrent::settings_pack::alert_mask, 
        libtorrent::alert::error_notification |
        libtorrent::alert::status_notification |
        libtorrent::alert::storage_notification);
    
    return settings;
}

libtorrent::settings_pack TorrentSession::dictionary_to_settings_pack(Dictionary settings) {
    libtorrent::settings_pack lt_settings = create_default_settings();
    
    // Override with custom settings
    for (int i = 0; i < settings.keys().size(); i++) {
        String key = settings.keys()[i];
        Variant value = settings[key];
        
        // Handle common settings
        if (key == "user_agent") {
            lt_settings.set_str(libtorrent::settings_pack::user_agent, value.operator String().utf8().get_data());
        } else if (key == "listen_interfaces") {
            lt_settings.set_str(libtorrent::settings_pack::listen_interfaces, value.operator String().utf8().get_data());
        } else if (key == "enable_dht") {
            lt_settings.set_bool(libtorrent::settings_pack::enable_dht, value.operator bool());
        } else if (key == "enable_lsd") {
            lt_settings.set_bool(libtorrent::settings_pack::enable_lsd, value.operator bool());
        } else if (key == "enable_upnp") {
            lt_settings.set_bool(libtorrent::settings_pack::enable_upnp, value.operator bool());
        } else if (key == "enable_natpmp") {
            lt_settings.set_bool(libtorrent::settings_pack::enable_natpmp, value.operator bool());
        } else if (key == "download_rate_limit") {
            lt_settings.set_int(libtorrent::settings_pack::download_rate_limit, value.operator int());
        } else if (key == "upload_rate_limit") {
            lt_settings.set_int(libtorrent::settings_pack::upload_rate_limit, value.operator int());
        }
    }
    
    return lt_settings;
}