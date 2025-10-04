#include "torrent_session.h"
#include "torrent_handle.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/time.hpp>
#include <string>
#include <thread>
#include <chrono>

// Include real headers only when not in stub mode
#ifndef TORRENT_STUB_MODE
    #include <libtorrent/session.hpp>
    #include <libtorrent/settings_pack.hpp>
    #include <libtorrent/magnet_uri.hpp>
    #include <libtorrent/alert_types.hpp>
    #include <libtorrent/torrent_info.hpp>
    #include <libtorrent/add_torrent_params.hpp>
    #include <libtorrent/session_stats.hpp>
    #include <libtorrent/hex.hpp>
#endif

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
    ClassDB::bind_method(D_METHOD("get_dht_state"), &TorrentSession::get_dht_state);
    ClassDB::bind_method(D_METHOD("set_dht_bootstrap_nodes", "nodes"), &TorrentSession::set_dht_bootstrap_nodes);
    
    // Network interface and port management
    ClassDB::bind_method(D_METHOD("bind_network_interface", "interface_ip"), &TorrentSession::bind_network_interface, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("get_listening_ports"), &TorrentSession::get_listening_ports);
    ClassDB::bind_method(D_METHOD("get_network_status"), &TorrentSession::get_network_status);
    ClassDB::bind_method(D_METHOD("test_port_accessibility", "port"), &TorrentSession::test_port_accessibility);
    
    // UPnP/NAT-PMP port mapping
    ClassDB::bind_method(D_METHOD("enable_upnp_port_mapping", "enable"), &TorrentSession::enable_upnp_port_mapping, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("enable_natpmp_port_mapping", "enable"), &TorrentSession::enable_natpmp_port_mapping, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("get_port_mapping_status"), &TorrentSession::get_port_mapping_status);
    
    // IPv6 support
    ClassDB::bind_method(D_METHOD("enable_ipv6", "enable"), &TorrentSession::enable_ipv6, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("is_ipv6_enabled"), &TorrentSession::is_ipv6_enabled);
    
    // Network diagnostics
    ClassDB::bind_method(D_METHOD("run_network_diagnostics"), &TorrentSession::run_network_diagnostics);
    ClassDB::bind_method(D_METHOD("get_network_interfaces"), &TorrentSession::get_network_interfaces);
    
    ClassDB::bind_method(D_METHOD("add_torrent_file", "torrent_data", "save_path"), &TorrentSession::add_torrent_file);
    ClassDB::bind_method(D_METHOD("add_magnet_uri", "magnet_uri", "save_path"), &TorrentSession::add_magnet_uri);
    ClassDB::bind_method(D_METHOD("remove_torrent", "handle", "delete_files"), &TorrentSession::remove_torrent, DEFVAL(false));
    
    ClassDB::bind_method(D_METHOD("get_session_stats"), &TorrentSession::get_session_stats);
    ClassDB::bind_method(D_METHOD("get_alerts"), &TorrentSession::get_alerts);
    ClassDB::bind_method(D_METHOD("clear_alerts"), &TorrentSession::clear_alerts);
}

TorrentSession::TorrentSession() {
    _session_ptr = nullptr;
    _session_running = false;
    _initialization_error = false;
    _download_rate_limit = 0; // 0 means unlimited
    _upload_rate_limit = 0;   // 0 means unlimited  
    _listen_port_min = 6881;  // Default BitTorrent port
    _listen_port_max = 6889;  // Default range
    _dht_enabled = true;      // Enable DHT by default
    _ipv6_enabled = true;     // Enable IPv6 by default
    _upnp_enabled = true;     // Enable UPnP by default
    _natpmp_enabled = true;   // Enable NAT-PMP by default
    _bound_interface = "";    // Bind to all interfaces by default
    
    detect_build_mode();
    
    if (_is_stub_mode) {
        log_session_operation("TorrentSession initialized in STUB mode (real libtorrent integration pending)");
    } else {
        log_session_operation("TorrentSession initialized with REAL libtorrent integration");
    }
}

TorrentSession::~TorrentSession() {
    if (_session_running) {
        stop_session();
    }
    cleanup_session();
}

void TorrentSession::detect_build_mode() {
#ifdef TORRENT_STUB_MODE
    _is_stub_mode = true;
#else
    _is_stub_mode = false;
#endif
}

bool TorrentSession::start_session() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (_session_running) {
        return true;
    }
    
    try {
        initialize_session_resources();
        
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            // Real libtorrent session creation
            void* settings_ptr = create_default_settings();
            if (settings_ptr) {
                _session_ptr = new libtorrent::session(*static_cast<libtorrent::settings_pack*>(settings_ptr));
                delete static_cast<libtorrent::settings_pack*>(settings_ptr);
                _session_running = true;
                log_session_operation("Real libtorrent session started successfully");
            } else {
                log_session_operation("Failed to create default settings", false);
                return false;
            }
#endif
        } else {
            // Stub mode - simulate session creation
            _current_settings = create_stub_default_settings();
            _session_running = true;
            log_session_operation("Stub session started (waiting for real libtorrent integration)");
        }
        
        return true;
    } catch (const std::exception& e) {
        handle_session_error(std::string("start_session"), e);
        return false;
    }
}

bool TorrentSession::start_session_with_settings(Dictionary settings) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (_session_running) {
        return true;
    }
    
    // Validate settings first
    if (!validate_settings_dictionary(settings)) {
        UtilityFunctions::print_rich("[color=red]Invalid settings provided[/color]");
        return false;
    }
    
    try {
        initialize_session_resources();
        
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            // Real libtorrent session with custom settings
            void* settings_ptr = dictionary_to_settings_pack(settings);
            if (settings_ptr) {
                _session_ptr = new libtorrent::session(*static_cast<libtorrent::settings_pack*>(settings_ptr));
                delete static_cast<libtorrent::settings_pack*>(settings_ptr);
                _session_running = true;
                log_session_operation("Real libtorrent session started with custom settings");
            } else {
                log_session_operation("Failed to create settings pack", false);
                return false;
            }
#endif
        } else {
            // Stub mode - store settings and simulate
            _current_settings = settings;
            apply_stub_settings(settings);
            _session_running = true;
            log_session_operation("Stub session started with custom settings");
        }
        
        return true;
    } catch (const std::exception& e) {
        handle_session_error(std::string("start_session_with_settings"), e);
        return false;
    }
}

void TorrentSession::stop_session() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (!_session_running) {
        return;
    }
    
    try {
        cleanup_session();
        _session_running = false;
        log_session_operation("Session stopped cleanly");
    } catch (const std::exception& e) {
        handle_session_error(std::string("stop_session"), e);
        _session_running = false; // Force stop even on error
    }
}

bool TorrentSession::is_running() const {
    std::lock_guard<std::mutex> lock(_session_mutex);
    return _session_running && !_initialization_error;
}

void TorrentSession::set_download_rate_limit(int bytes_per_second) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _download_rate_limit = bytes_per_second;
    
    if (!_session_running) {
        log_session_operation("Download rate limit cached (session not running)", true);
        return;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            libtorrent::settings_pack settings;
            settings.set_int(libtorrent::settings_pack::download_rate_limit, bytes_per_second);
            static_cast<libtorrent::session*>(_session_ptr)->apply_settings(settings);
            log_session_operation("Download rate limit applied: " + String::num(bytes_per_second) + " bytes/s");
        } catch (const std::exception& e) {
            handle_session_error(std::string("set_download_rate_limit"), e);
        }
#endif
    } else {
        simulate_session_operation("set_download_rate_limit");
    }
}

void TorrentSession::set_upload_rate_limit(int bytes_per_second) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _upload_rate_limit = bytes_per_second;
    
    if (!_session_running) {
        log_session_operation("Upload rate limit cached (session not running)", true);
        return;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            libtorrent::settings_pack settings;
            settings.set_int(libtorrent::settings_pack::upload_rate_limit, bytes_per_second);
            static_cast<libtorrent::session*>(_session_ptr)->apply_settings(settings);
            log_session_operation("Upload rate limit applied: " + String::num(bytes_per_second) + " bytes/s");
        } catch (const std::exception& e) {
            handle_session_error(std::string("set_upload_rate_limit"), e);
        }
#endif
    } else {
        simulate_session_operation("set_upload_rate_limit");
    }
}

void TorrentSession::set_listen_port(int port) {
    set_listen_port_range(port, port);
}

void TorrentSession::set_listen_port_range(int min_port, int max_port) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _listen_port_min = min_port;
    _listen_port_max = max_port;
    
    if (!_session_running) {
        log_session_operation("Listen ports cached (session not running)", true);
        return;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            // Build listen interfaces string with IPv4 and IPv6 support
            std::string listen_interfaces;
            
            // Determine base interface
            std::string base_interface = _bound_interface.is_empty() ? "0.0.0.0" : _bound_interface.utf8().get_data();
            
            // IPv4 interface
            listen_interfaces = base_interface + ":" + std::to_string(min_port);
            if (min_port != max_port) {
                listen_interfaces += "-" + std::to_string(max_port);
            }
            
            // IPv6 interface (if enabled)
            if (_ipv6_enabled) {
                std::string ipv6_interface = _bound_interface.is_empty() ? "[::]" : "[" + _bound_interface.utf8().get_data() + "]";
                listen_interfaces += "," + ipv6_interface + ":" + std::to_string(min_port);
                if (min_port != max_port) {
                    listen_interfaces += "-" + std::to_string(max_port);
                }
            }
            
            libtorrent::settings_pack settings;
            settings.set_str(libtorrent::settings_pack::listen_interfaces, listen_interfaces);
            
            // Apply additional network settings
            settings.set_bool(libtorrent::settings_pack::enable_upnp, _upnp_enabled);
            settings.set_bool(libtorrent::settings_pack::enable_natpmp, _natpmp_enabled);
            
            static_cast<libtorrent::session*>(_session_ptr)->apply_settings(settings);
            
            String port_range = String::num(min_port);
            if (min_port != max_port) {
                port_range += "-" + String::num(max_port);
            }
            
            String protocol_info = _ipv6_enabled ? " (IPv4+IPv6)" : " (IPv4)";
            String interface_info = _bound_interface.is_empty() ? " on all interfaces" : " on interface " + _bound_interface;
            
            log_session_operation("Listen ports set: " + port_range + protocol_info + interface_info);
        } catch (const std::exception& e) {
            handle_session_error(std::string("set_listen_port_range"), e);
        }
#endif
    } else {
        simulate_session_operation("set_listen_port_range (IPv4/IPv6 with port mapping)");
    }
}

bool TorrentSession::is_dht_running() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (!_session_running) {
        return false;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            return static_cast<libtorrent::session*>(_session_ptr)->is_dht_running();
        } catch (const std::exception& e) {
            handle_session_error(std::string("is_dht_running"), e);
            return false;
        }
#endif
    } else {
        return _dht_enabled;
    }
    return false;
}

void TorrentSession::start_dht() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _dht_enabled = true;
    
    if (!_session_running) {
        log_session_operation("DHT start cached (session not running)", true);
        return;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            libtorrent::settings_pack settings;
            settings.set_bool(libtorrent::settings_pack::enable_dht, true);
            static_cast<libtorrent::session*>(_session_ptr)->apply_settings(settings);
            log_session_operation("DHT started");
        } catch (const std::exception& e) {
            handle_session_error(std::string("start_dht"), e);
        }
#endif
    } else {
        simulate_session_operation("start_dht");
    }
}

void TorrentSession::stop_dht() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _dht_enabled = false;
    
    if (!_session_running) {
        return;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            libtorrent::settings_pack settings;
            settings.set_bool(libtorrent::settings_pack::enable_dht, false);
            static_cast<libtorrent::session*>(_session_ptr)->apply_settings(settings);
            log_session_operation("DHT stopped");
        } catch (const std::exception& e) {
            handle_session_error(std::string("stop_dht"), e);
        }
#endif
    } else {
        simulate_session_operation("stop_dht");
    }
}

// Enhanced DHT Management
Dictionary TorrentSession::get_dht_state() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    Dictionary state;
    
    if (!_session_running) {
        state["running"] = false;
        state["nodes"] = 0;
        state["status"] = "session_not_running";
        return state;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            auto session = static_cast<libtorrent::session*>(_session_ptr);
            state["running"] = session->is_dht_running();
            
            // Get DHT statistics if available
            auto stats = session->get_stats();
            state["nodes"] = stats.dht_nodes;
            state["torrents"] = stats.dht_torrents;
            state["node_cache"] = stats.dht_node_cache;
            state["status"] = state["running"] ? "running" : "stopped";
            
            log_session_operation("DHT state retrieved");
        } catch (const std::exception& e) {
            handle_session_error(std::string("get_dht_state"), e);
            state["status"] = "error";
        }
#endif
    } else {
        state["running"] = _dht_enabled;
        state["nodes"] = _dht_enabled ? 42 : 0;  // Stub values
        state["torrents"] = 0;
        state["node_cache"] = 0;
        state["status"] = "stub_mode";
    }
    
    return state;
}

void TorrentSession::set_dht_bootstrap_nodes(Array nodes) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _dht_bootstrap_nodes = nodes;
    
    if (!_session_running) {
        log_session_operation("DHT bootstrap nodes cached (session not running)", true);
        return;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            // Convert Array to libtorrent bootstrap nodes
            // Format expected: ["hostname:port", "ip:port", ...]
            std::vector<std::pair<std::string, int>> bootstrap_nodes;
            
            for (int i = 0; i < nodes.size(); i++) {
                String node_str = nodes[i];
                std::string node = node_str.utf8().get_data();
                
                // Parse "hostname:port" format
                size_t colon_pos = node.find(':');
                if (colon_pos != std::string::npos) {
                    std::string host = node.substr(0, colon_pos);
                    int port = std::stoi(node.substr(colon_pos + 1));
                    bootstrap_nodes.push_back({host, port});
                }
            }
            
            // Apply bootstrap nodes (this would require DHT restart in real implementation)
            // For now, we'll apply them on next DHT start
            log_session_operation("DHT bootstrap nodes set: " + String::num(bootstrap_nodes.size()) + " nodes");
        } catch (const std::exception& e) {
            handle_session_error(std::string("set_dht_bootstrap_nodes"), e);
        }
#endif
    } else {
        simulate_session_operation("set_dht_bootstrap_nodes");
    }
}

// Network Interface Management
bool TorrentSession::bind_network_interface(String interface_ip) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _bound_interface = interface_ip;
    
    if (!_session_running) {
        log_session_operation("Network interface binding cached (session not running)", true);
        return true;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            // Apply the new interface binding by reconfiguring listen ports
            set_listen_port_range(_listen_port_min, _listen_port_max);
            
            String interface_info = interface_ip.is_empty() ? "all interfaces" : interface_ip;
            log_session_operation("Network interface bound to: " + interface_info);
            return true;
        } catch (const std::exception& e) {
            handle_session_error(std::string("bind_network_interface"), e);
            return false;
        }
#endif
    } else {
        simulate_session_operation("bind_network_interface");
        return true;
    }
    return false;
}

Array TorrentSession::get_listening_ports() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    Array ports;
    
    if (!_session_running) {
        return ports;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            auto session = static_cast<libtorrent::session*>(_session_ptr);
            
            // Get actual listening ports from session
            // This would require accessing session's listening ports
            for (int port = _listen_port_min; port <= _listen_port_max; port++) {
                Dictionary port_info;
                port_info["port"] = port;
                port_info["protocol"] = "tcp";
                port_info["interface"] = _bound_interface.is_empty() ? "0.0.0.0" : _bound_interface;
                port_info["ipv6"] = _ipv6_enabled;
                ports.append(port_info);
            }
            
            log_session_operation("Retrieved " + String::num(ports.size()) + " listening ports");
        } catch (const std::exception& e) {
            handle_session_error(std::string("get_listening_ports"), e);
        }
#endif
    } else {
        // Stub mode - return configured ports
        for (int port = _listen_port_min; port <= _listen_port_max; port++) {
            Dictionary port_info;
            port_info["port"] = port;
            port_info["protocol"] = "tcp";
            port_info["interface"] = _bound_interface.is_empty() ? "0.0.0.0" : _bound_interface;
            port_info["ipv6"] = _ipv6_enabled;
            port_info["status"] = "stub";
            ports.append(port_info);
        }
    }
    
    return ports;
}

Dictionary TorrentSession::get_network_status() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    Dictionary status;
    
    status["session_running"] = _session_running;
    status["listen_port_min"] = _listen_port_min;
    status["listen_port_max"] = _listen_port_max;
    status["bound_interface"] = _bound_interface;
    status["ipv6_enabled"] = _ipv6_enabled;
    status["upnp_enabled"] = _upnp_enabled;
    status["natpmp_enabled"] = _natpmp_enabled;
    status["dht_enabled"] = _dht_enabled;
    
    if (!_session_running) {
        status["connection_status"] = "not_running";
        return status;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            auto session = static_cast<libtorrent::session*>(_session_ptr);
            auto session_stats = session->get_stats();
            
            status["num_peers"] = session_stats.num_peers;
            status["num_unchoked"] = session_stats.num_unchoked;
            status["download_rate"] = session_stats.download_rate;
            status["upload_rate"] = session_stats.upload_rate;
            status["dht_nodes"] = session_stats.dht_nodes;
            status["connection_status"] = "connected";
            
            log_session_operation("Network status retrieved");
        } catch (const std::exception& e) {
            handle_session_error(std::string("get_network_status"), e);
            status["connection_status"] = "error";
        }
#endif
    } else {
        status["num_peers"] = 0;
        status["num_unchoked"] = 0;
        status["download_rate"] = 0;
        status["upload_rate"] = 0;
        status["dht_nodes"] = _dht_enabled ? 42 : 0;
        status["connection_status"] = "stub_mode";
    }
    
    return status;
}

bool TorrentSession::test_port_accessibility(int port) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            // In a real implementation, this would attempt to bind to the port
            // or perform actual connectivity tests
            log_session_operation("Port accessibility test for port " + String::num(port));
            
            // For now, return true if port is in valid range
            bool accessible = (port >= 1024 && port <= 65535);
            return accessible;
        } catch (const std::exception& e) {
            handle_session_error(std::string("test_port_accessibility"), e);
            return false;
        }
#endif
    } else {
        simulate_session_operation("test_port_accessibility");
        return true; // Stub always returns accessible
    }
    return false;
}

// UPnP/NAT-PMP Port Mapping
bool TorrentSession::enable_upnp_port_mapping(bool enable) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _upnp_enabled = enable;
    
    if (!_session_running) {
        log_session_operation("UPnP setting cached (session not running)", true);
        return true;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            libtorrent::settings_pack settings;
            settings.set_bool(libtorrent::settings_pack::enable_upnp, enable);
            static_cast<libtorrent::session*>(_session_ptr)->apply_settings(settings);
            
            log_session_operation("UPnP port mapping " + String(enable ? "enabled" : "disabled"));
            return true;
        } catch (const std::exception& e) {
            handle_session_error(std::string("enable_upnp_port_mapping"), e);
            return false;
        }
#endif
    } else {
        simulate_session_operation("enable_upnp_port_mapping");
        return true;
    }
    return false;
}

bool TorrentSession::enable_natpmp_port_mapping(bool enable) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _natpmp_enabled = enable;
    
    if (!_session_running) {
        log_session_operation("NAT-PMP setting cached (session not running)", true);
        return true;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            libtorrent::settings_pack settings;
            settings.set_bool(libtorrent::settings_pack::enable_natpmp, enable);
            static_cast<libtorrent::session*>(_session_ptr)->apply_settings(settings);
            
            log_session_operation("NAT-PMP port mapping " + String(enable ? "enabled" : "disabled"));
            return true;
        } catch (const std::exception& e) {
            handle_session_error(std::string("enable_natpmp_port_mapping"), e);
            return false;
        }
#endif
    } else {
        simulate_session_operation("enable_natpmp_port_mapping");
        return true;
    }
    return false;
}

Dictionary TorrentSession::get_port_mapping_status() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    Dictionary status;
    
    status["upnp_enabled"] = _upnp_enabled;
    status["natpmp_enabled"] = _natpmp_enabled;
    
    if (!_session_running) {
        status["upnp_status"] = "not_running";
        status["natpmp_status"] = "not_running";
        return status;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            // In real implementation, would query actual port mapping status
            status["upnp_status"] = _upnp_enabled ? "enabled" : "disabled";
            status["natpmp_status"] = _natpmp_enabled ? "enabled" : "disabled";
            status["external_ip"] = ""; // Would contain discovered external IP
            status["mapped_ports"] = Array(); // Would contain actually mapped ports
            
            log_session_operation("Port mapping status retrieved");
        } catch (const std::exception& e) {
            handle_session_error(std::string("get_port_mapping_status"), e);
            status["upnp_status"] = "error";
            status["natpmp_status"] = "error";
        }
#endif
    } else {
        status["upnp_status"] = _upnp_enabled ? "stub_enabled" : "stub_disabled";
        status["natpmp_status"] = _natpmp_enabled ? "stub_enabled" : "stub_disabled";
        status["external_ip"] = "192.168.1.100"; // Stub IP
        status["mapped_ports"] = Array();
    }
    
    return status;
}

// IPv6 Support
void TorrentSession::enable_ipv6(bool enable) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    _ipv6_enabled = enable;
    
    if (!_session_running) {
        log_session_operation("IPv6 setting cached (session not running)", true);
        return;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            // Reconfigure listen interfaces to add/remove IPv6
            set_listen_port_range(_listen_port_min, _listen_port_max);
            log_session_operation("IPv6 " + String(enable ? "enabled" : "disabled"));
        } catch (const std::exception& e) {
            handle_session_error(std::string("enable_ipv6"), e);
        }
#endif
    } else {
        simulate_session_operation("enable_ipv6");
    }
}

bool TorrentSession::is_ipv6_enabled() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    return _ipv6_enabled;
}

// Network Diagnostics
Dictionary TorrentSession::run_network_diagnostics() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    Dictionary diagnostics;
    
    diagnostics["timestamp"] = Time::get_singleton()->get_ticks_msec();
    diagnostics["session_running"] = _session_running;
    
    // Test port accessibility
    Array port_tests;
    for (int port = _listen_port_min; port <= _listen_port_max; port++) {
        Dictionary port_test;
        port_test["port"] = port;
        port_test["accessible"] = test_port_accessibility(port);
        port_tests.append(port_test);
    }
    diagnostics["port_tests"] = port_tests;
    
    // Network interface information
    diagnostics["bound_interface"] = _bound_interface;
    diagnostics["ipv6_enabled"] = _ipv6_enabled;
    
    // DHT diagnostics
    if (_session_running) {
        diagnostics["dht_state"] = get_dht_state();
        diagnostics["network_status"] = get_network_status();
        diagnostics["port_mapping"] = get_port_mapping_status();
    }
    
    if (!_is_stub_mode) {
        log_session_operation("Network diagnostics completed");
    } else {
        diagnostics["mode"] = "stub";
        simulate_session_operation("run_network_diagnostics");
    }
    
    return diagnostics;
}

Array TorrentSession::get_network_interfaces() {
    Array interfaces;
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            // In real implementation, would enumerate actual network interfaces
            // For now, return common interface patterns
            Dictionary localhost;
            localhost["name"] = "localhost";
            localhost["ip"] = "127.0.0.1";
            localhost["ipv6"] = "::1";
            localhost["type"] = "loopback";
            interfaces.append(localhost);
            
            Dictionary any_ipv4;
            any_ipv4["name"] = "any_ipv4";
            any_ipv4["ip"] = "0.0.0.0";
            any_ipv4["type"] = "any";
            interfaces.append(any_ipv4);
            
            if (_ipv6_enabled) {
                Dictionary any_ipv6;
                any_ipv6["name"] = "any_ipv6";
                any_ipv6["ip"] = "::";
                any_ipv6["type"] = "any";
                interfaces.append(any_ipv6);
            }
            
            log_session_operation("Network interfaces enumerated");
        } catch (const std::exception& e) {
            handle_session_error(std::string("get_network_interfaces"), e);
        }
#endif
    } else {
        // Stub interfaces
        Dictionary stub_interface;
        stub_interface["name"] = "stub_interface";
        stub_interface["ip"] = "192.168.1.100";
        stub_interface["ipv6"] = _ipv6_enabled ? "2001:db8::1" : "";
        stub_interface["type"] = "stub";
        interfaces.append(stub_interface);
    }
    
    return interfaces;
}

// Simplified implementations for remaining methods...
Ref<TorrentHandle> TorrentSession::add_torrent_file(PackedByteArray torrent_data, String save_path) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=red]Session not running, cannot add torrent[/color]");
        return Ref<TorrentHandle>();
    }
    
    // Create dummy handle for both modes for now
    Ref<TorrentHandle> torrent_handle;
    torrent_handle.instantiate();
    
    if (!_is_stub_mode) {
        log_session_operation("Real torrent file add (implementation pending)");
    } else {
        simulate_session_operation("add_torrent_file");
    }
    
    return torrent_handle;
}

Ref<TorrentHandle> TorrentSession::add_magnet_uri(String magnet_uri, String save_path) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=red]Session not running, cannot add magnet[/color]");
        return Ref<TorrentHandle>();
    }
    
    // Create dummy handle for both modes for now
    Ref<TorrentHandle> torrent_handle;
    torrent_handle.instantiate();
    
    if (!_is_stub_mode) {
        log_session_operation("Real magnet URI add (implementation pending)");
    } else {
        simulate_session_operation("add_magnet_uri");
    }
    
    return torrent_handle;
}

bool TorrentSession::remove_torrent(Ref<TorrentHandle> handle, bool delete_files) {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (!_session_running) {
        UtilityFunctions::print_rich("[color=yellow]Session not running, cannot remove torrent[/color]");
        return false;
    }
    
    if (handle.is_null()) {
        UtilityFunctions::print_rich("[color=yellow]Invalid torrent handle[/color]");
        return false;
    }
    
    if (!_is_stub_mode) {
        log_session_operation("Real torrent remove (implementation pending)");
    } else {
        simulate_session_operation("remove_torrent");
    }
    
    return true;
}

Dictionary TorrentSession::get_session_stats() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (!_session_running) {
        return Dictionary();
    }
    
    if (!_is_stub_mode) {
        try {
#ifndef TORRENT_STUB_MODE
            libtorrent::session* session = static_cast<libtorrent::session*>(_session_ptr);
            if (!session) {
                return Dictionary();
            }
            
            // Request session statistics
            session->post_session_stats();
            
            // Wait for the session_stats_alert (with timeout)
            const auto timeout = std::chrono::milliseconds(1000); // 1 second timeout
            auto start_time = std::chrono::steady_clock::now();
            
            std::vector<libtorrent::alert*> alerts;
            libtorrent::session_stats_alert* stats_alert = nullptr;
            
            while (std::chrono::steady_clock::now() - start_time < timeout) {
                session->pop_alerts(&alerts);
                
                for (auto* alert : alerts) {
                    if (auto* sa = libtorrent::alert_cast<libtorrent::session_stats_alert>(alert)) {
                        stats_alert = sa;
                        break;
                    }
                }
                
                if (stats_alert) break;
                
                // Brief sleep to avoid busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (!stats_alert) {
                // Timeout - return basic info
                Dictionary stats;
                stats["mode"] = "real";
                stats["error"] = "Statistics timeout";
                return stats;
            }
            
            // Get the metrics mapping
            auto metrics = libtorrent::session_stats_metrics();
            auto counters = stats_alert->counters();
            
            Dictionary stats;
            stats["mode"] = "real";
            stats["timestamp"] = Time::get_singleton()->get_ticks_msec();
            
            // Helper lambda to safely get metric value by name
            auto get_metric_value = [&](const char* name) -> int64_t {
                for (const auto& metric : metrics) {
                    if (std::string(metric.name) == name) {
                        if (metric.value_index < static_cast<int>(counters.size())) {
                            return counters[metric.value_index];
                        }
                    }
                }
                return 0;
            };
            
            // Extract key statistics - Download/Upload rates and totals
            stats["download_rate"] = get_metric_value("net.recv_bytes");
            stats["upload_rate"] = get_metric_value("net.sent_bytes");
            stats["total_download"] = get_metric_value("net.recv_ip_overhead_bytes") + 
                                    get_metric_value("net.recv_payload_bytes");
            stats["total_upload"] = get_metric_value("net.sent_ip_overhead_bytes") + 
                                  get_metric_value("net.sent_payload_bytes");
            
            // Peer statistics
            stats["num_peers"] = get_metric_value("peer.num_peers_connected");
            stats["num_unchoked"] = get_metric_value("peer.num_peers_up_unchoked");
            stats["num_peers_half_open"] = get_metric_value("peer.num_peers_half_open");
            
            // DHT statistics
            stats["dht_nodes"] = get_metric_value("dht.dht_nodes");
            stats["dht_node_cache"] = get_metric_value("dht.dht_node_cache");
            stats["dht_torrents"] = get_metric_value("dht.dht_torrents");
            
            // Torrent statistics
            stats["num_torrents"] = get_metric_value("ses.num_torrents");
            stats["num_downloading"] = get_metric_value("ses.num_downloading_torrents");
            stats["num_seeding"] = get_metric_value("ses.num_seeding_torrents");
            stats["num_checking"] = get_metric_value("ses.num_checking_torrents");
            stats["num_stopped"] = get_metric_value("ses.num_stopped_torrents");
            
            // Disk I/O statistics
            stats["disk_read_cache_size"] = get_metric_value("disk.read_cache_size");
            stats["disk_write_cache_size"] = get_metric_value("disk.write_cache_size");
            stats["disk_blocks_read"] = get_metric_value("disk.num_blocks_read");
            stats["disk_blocks_written"] = get_metric_value("disk.num_blocks_written");
            
            // Network statistics
            stats["num_tcp_peers"] = get_metric_value("net.num_tcp_peers");
            stats["num_incoming_connections"] = get_metric_value("net.num_incoming_connections");
            stats["optimistic_unchokes"] = get_metric_value("peer.optimistic_unchoke_counter");
            
            // Tracker statistics
            stats["num_tracker_announces"] = get_metric_value("tracker.num_tracker_announces");
            stats["num_tracker_scrapes"] = get_metric_value("tracker.num_tracker_scrapes");
            
            // Additional useful metrics
            stats["has_incoming_connections"] = get_metric_value("net.has_incoming_connections");
            stats["utp_stats"] = get_metric_value("utp.num_utp_connected");
            
            log_session_operation("Session statistics retrieved successfully");
            return stats;
#endif
        } catch (const std::exception& e) {
            handle_session_error("get_session_stats", e);
            Dictionary error_stats;
            error_stats["mode"] = "real";
            error_stats["error"] = String("Exception: ") + e.what();
            return error_stats;
        }
        
        // Fallback if TORRENT_STUB_MODE is defined
        Dictionary stats;
        stats["mode"] = "real";
        stats["implementation"] = "pending";
        return stats;
    } else {
        return create_stub_stats();
    }
}

Array TorrentSession::get_alerts() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (!_session_running) {
        return Array();
    }
    
    if (!_is_stub_mode) {
        try {
#ifndef TORRENT_STUB_MODE
            libtorrent::session* session = static_cast<libtorrent::session*>(_session_ptr);
            if (!session) {
                return Array();
            }
            
            // Get alerts from libtorrent
            std::vector<libtorrent::alert*> alerts;
            session->pop_alerts(&alerts);
            
            Array result;
            for (auto* alert : alerts) {
                if (alert) {
                    Dictionary alert_dict = convert_alert_to_dictionary(alert);
                    if (!alert_dict.is_empty()) {
                        result.append(alert_dict);
                    }
                }
            }
            
            if (result.size() > 0) {
                log_session_operation("Retrieved " + String::num_int64(result.size()) + " alerts from libtorrent");
            }
            
            return result;
#endif
        } catch (const std::exception& e) {
            handle_session_error("get_alerts", e);
            return Array();
        }
        
        // Fallback if TORRENT_STUB_MODE is defined
        return Array();
    } else {
        return create_stub_alerts();
    }
}

void TorrentSession::clear_alerts() {
    std::lock_guard<std::mutex> lock(_session_mutex);
    
    if (!_session_running) {
        return;
    }
    
    if (!_is_stub_mode) {
        try {
#ifndef TORRENT_STUB_MODE
            libtorrent::session* session = static_cast<libtorrent::session*>(_session_ptr);
            if (session) {
                // Clear any remaining alerts from libtorrent
                std::vector<libtorrent::alert*> alerts;
                session->pop_alerts(&alerts);
                // Alerts are automatically cleaned up by libtorrent
                
                if (alerts.size() > 0) {
                    log_session_operation("Cleared " + String::num_int64(alerts.size()) + " alerts from libtorrent");
                }
            }
#endif
        } catch (const std::exception& e) {
            handle_session_error("clear_alerts", e);
        }
    } else {
        // Stub mode - nothing to clear
        simulate_session_operation("clear_alerts");
    }
}

// Resource management methods
void TorrentSession::cleanup_session() {
    if (_session_ptr && !_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        delete static_cast<libtorrent::session*>(_session_ptr);
#endif
        _session_ptr = nullptr;
    }
}

void TorrentSession::initialize_session_resources() {
    _initialization_error = false;
    // Additional resource initialization can be added here
}

// Settings management
void* TorrentSession::create_default_settings() {
#ifndef TORRENT_STUB_MODE
    if (!_is_stub_mode) {
        libtorrent::settings_pack* settings = new libtorrent::settings_pack();
        
        // Basic settings
        settings->set_str(libtorrent::settings_pack::user_agent, "Godot-Torrent/1.0.0");
        
        // Network interface configuration
        std::string listen_interfaces;
        std::string base_interface = _bound_interface.is_empty() ? "0.0.0.0" : _bound_interface.utf8().get_data();
        
        // IPv4 interface
        listen_interfaces = base_interface + ":" + std::to_string(_listen_port_min);
        if (_listen_port_min != _listen_port_max) {
            listen_interfaces += "-" + std::to_string(_listen_port_max);
        }
        
        // IPv6 interface (if enabled)
        if (_ipv6_enabled) {
            std::string ipv6_interface = _bound_interface.is_empty() ? "[::]" : "[" + _bound_interface.utf8().get_data() + "]";
            listen_interfaces += "," + ipv6_interface + ":" + std::to_string(_listen_port_min);
            if (_listen_port_min != _listen_port_max) {
                listen_interfaces += "-" + std::to_string(_listen_port_max);
            }
        }
        
        settings->set_str(libtorrent::settings_pack::listen_interfaces, listen_interfaces);
        
        // DHT configuration
        settings->set_bool(libtorrent::settings_pack::enable_dht, _dht_enabled);
        
        // Enable local peer discovery
        settings->set_bool(libtorrent::settings_pack::enable_lsd, true);
        
        // Port mapping configuration
        settings->set_bool(libtorrent::settings_pack::enable_upnp, _upnp_enabled);
        settings->set_bool(libtorrent::settings_pack::enable_natpmp, _natpmp_enabled);
        
        // Apply cached limits
        if (_download_rate_limit > 0) {
            settings->set_int(libtorrent::settings_pack::download_rate_limit, _download_rate_limit);
        }
        if (_upload_rate_limit > 0) {
            settings->set_int(libtorrent::settings_pack::upload_rate_limit, _upload_rate_limit);
        }
        
        // Alert settings for network events
        settings->set_int(libtorrent::settings_pack::alert_mask, 
            libtorrent::alert::error_notification |
            libtorrent::alert::status_notification |
            libtorrent::alert::storage_notification |
            libtorrent::alert::port_mapping_notification |
            libtorrent::alert::dht_notification);
        
        return settings;
    }
#endif
    return nullptr;
}

void* TorrentSession::dictionary_to_settings_pack(Dictionary settings) {
#ifndef TORRENT_STUB_MODE
    if (!_is_stub_mode) {
        libtorrent::settings_pack* lt_settings = static_cast<libtorrent::settings_pack*>(create_default_settings());
        if (!lt_settings) return nullptr;
        
        // Override with custom settings
        for (int i = 0; i < settings.keys().size(); i++) {
            String key = settings.keys()[i];
            Variant value = settings[key];
            
            // Handle common settings
            if (key == "user_agent") {
                lt_settings->set_str(libtorrent::settings_pack::user_agent, value.operator String().utf8().get_data());
            } else if (key == "listen_interfaces") {
                lt_settings->set_str(libtorrent::settings_pack::listen_interfaces, value.operator String().utf8().get_data());
            } else if (key == "enable_dht") {
                lt_settings->set_bool(libtorrent::settings_pack::enable_dht, value.operator bool());
            } else if (key == "enable_lsd") {
                lt_settings->set_bool(libtorrent::settings_pack::enable_lsd, value.operator bool());
            } else if (key == "enable_upnp") {
                lt_settings->set_bool(libtorrent::settings_pack::enable_upnp, value.operator bool());
            } else if (key == "enable_natpmp") {
                lt_settings->set_bool(libtorrent::settings_pack::enable_natpmp, value.operator bool());
            } else if (key == "download_rate_limit") {
                lt_settings->set_int(libtorrent::settings_pack::download_rate_limit, value.operator int());
            } else if (key == "upload_rate_limit") {
                lt_settings->set_int(libtorrent::settings_pack::upload_rate_limit, value.operator int());
            } else if (key == "bound_interface") {
                _bound_interface = value.operator String();
                // Regenerate listen interfaces with new binding
                // This would require recreating the settings
            } else if (key == "ipv6_enabled") {
                _ipv6_enabled = value.operator bool();
                // Regenerate listen interfaces with IPv6 setting
            }
        }
        
        return lt_settings;
    }
#endif
    return nullptr;
}

// Stub mode helpers
void TorrentSession::apply_stub_settings(Dictionary settings) {
    if (settings.has("download_rate_limit")) {
        _download_rate_limit = settings["download_rate_limit"];
    }
    if (settings.has("upload_rate_limit")) {
        _upload_rate_limit = settings["upload_rate_limit"];
    }
    if (settings.has("enable_dht")) {
        _dht_enabled = settings["enable_dht"];
    }
    if (settings.has("enable_upnp")) {
        _upnp_enabled = settings["enable_upnp"];
    }
    if (settings.has("enable_natpmp")) {
        _natpmp_enabled = settings["enable_natpmp"];
    }
    if (settings.has("ipv6_enabled")) {
        _ipv6_enabled = settings["ipv6_enabled"];
    }
    if (settings.has("bound_interface")) {
        _bound_interface = settings["bound_interface"];
    }
}

Dictionary TorrentSession::create_stub_default_settings() {
    Dictionary settings;
    settings["user_agent"] = "Godot-Torrent/1.0.0-stub";
    settings["enable_dht"] = true;
    settings["enable_lsd"] = true;
    settings["enable_upnp"] = true;
    settings["enable_natpmp"] = true;
    settings["ipv6_enabled"] = true;
    settings["download_rate_limit"] = 0;
    settings["upload_rate_limit"] = 0;
    settings["listen_port_min"] = 6881;
    settings["listen_port_max"] = 6889;
    settings["bound_interface"] = "";
    settings["mode"] = "stub";
    return settings;
}

void TorrentSession::simulate_session_operation(const String& operation) {
    UtilityFunctions::print("STUB SESSION: " + operation + " (real implementation pending)");
}

Dictionary TorrentSession::create_stub_stats() {
    Dictionary stats;
    stats["mode"] = "stub";
    stats["timestamp"] = Time::get_singleton()->get_ticks_msec();
    
    // Download/Upload rates and totals
    stats["download_rate"] = 0;
    stats["upload_rate"] = 0;
    stats["total_download"] = 0;
    stats["total_upload"] = 0;
    
    // Peer statistics
    stats["num_peers"] = 0;
    stats["num_unchoked"] = 0;
    stats["num_peers_half_open"] = 0;
    
    // DHT statistics
    stats["dht_nodes"] = 0;
    stats["dht_node_cache"] = 0;
    stats["dht_torrents"] = 0;
    
    // Torrent statistics
    stats["num_torrents"] = 0;
    stats["num_downloading"] = 0;
    stats["num_seeding"] = 0;
    stats["num_checking"] = 0;
    stats["num_stopped"] = 0;
    
    // Disk I/O statistics
    stats["disk_read_cache_size"] = 0;
    stats["disk_write_cache_size"] = 0;
    stats["disk_blocks_read"] = 0;
    stats["disk_blocks_written"] = 0;
    
    // Network statistics
    stats["num_tcp_peers"] = 0;
    stats["num_incoming_connections"] = 0;
    stats["optimistic_unchokes"] = 0;
    
    // Tracker statistics
    stats["num_tracker_announces"] = 0;
    stats["num_tracker_scrapes"] = 0;
    
    // Additional useful metrics
    stats["has_incoming_connections"] = 0;
    stats["utp_stats"] = 0;
    
    return stats;
}

Array TorrentSession::create_stub_alerts() {
    Array alerts;
    
    // Create some sample alerts for demonstration
    static int alert_counter = 0;
    alert_counter++;
    
    // Only create alerts occasionally to avoid spam
    if (alert_counter % 5 == 0) {
        Dictionary sample_alert;
        sample_alert["timestamp"] = Time::get_singleton()->get_ticks_msec() * 1000000; // Convert to nanoseconds
        sample_alert["type"] = 42; // Arbitrary type for stub
        sample_alert["category"] = 64; // status category
        sample_alert["type_name"] = "stub_status";
        sample_alert["category_name"] = "status";
        sample_alert["message"] = "STUB: Sample status alert message";
        sample_alert["what"] = "stub_alert";
        sample_alert["info_hash"] = "STUB1234567890abcdef1234567890abcdef12345678";
        
        alerts.append(sample_alert);
    }
    
    return alerts;
}

// Error handling
void TorrentSession::handle_session_error(const std::string& operation, const std::exception& e) {
    String error_msg = "Session error in " + String(operation.c_str()) + ": " + String(e.what());
    UtilityFunctions::print_rich("[color=red]" + error_msg + "[/color]");
    _initialization_error = true;
}

void TorrentSession::log_session_operation(const String& operation, bool success) {
    String mode_prefix = _is_stub_mode ? "STUB SESSION" : "REAL SESSION";
    
    if (success) {
        UtilityFunctions::print(mode_prefix + ": " + operation);
    } else {
        UtilityFunctions::print_rich("[color=yellow]" + mode_prefix + ": " + operation + "[/color]");
    }
}

// Settings validation
bool TorrentSession::validate_settings_dictionary(const Dictionary& settings) {
    // Basic validation - can be extended
    if (settings.has("download_rate_limit")) {
        Variant limit = settings["download_rate_limit"];
        if (limit.get_type() != Variant::INT || int(limit) < 0) {
            return false;
        }
    }
    
    if (settings.has("upload_rate_limit")) {
        Variant limit = settings["upload_rate_limit"];
        if (limit.get_type() != Variant::INT || int(limit) < 0) {
            return false;
        }
    }
    
    return true;
}

// Alert conversion implementation
Dictionary TorrentSession::convert_alert_to_dictionary(void* alert_ptr) {
    Dictionary result;
    
    if (!alert_ptr) {
        return result;
    }
    
#ifndef TORRENT_STUB_MODE
    try {
        libtorrent::alert* alert = static_cast<libtorrent::alert*>(alert_ptr);
        
        // Basic alert information
        result["timestamp"] = alert->timestamp().time_since_epoch().count();
        result["type"] = alert->type();
        result["category"] = int(alert->category());
        result["type_name"] = get_alert_type_name(alert->type());
        result["category_name"] = get_alert_category_name(int(alert->category()));
        result["message"] = String(alert->message().c_str());
        result["what"] = String(alert->what());
        
        // Handle specific alert types for additional information
        switch (alert->type()) {
            case libtorrent::torrent_added_alert::alert_type:
            {
                auto* ta = libtorrent::alert_cast<libtorrent::torrent_added_alert>(alert);
                if (ta) {
                    result["info_hash"] = String(libtorrent::aux::to_hex(ta->handle.info_hash()).c_str());
                    result["torrent_name"] = String(ta->handle.status().name.c_str());
                }
                break;
            }
            
            case libtorrent::torrent_removed_alert::alert_type:
            {
                auto* tr = libtorrent::alert_cast<libtorrent::torrent_removed_alert>(alert);
                if (tr) {
                    result["info_hash"] = String(libtorrent::aux::to_hex(tr->info_hash).c_str());
                }
                break;
            }
            
            case libtorrent::state_changed_alert::alert_type:
            {
                auto* sc = libtorrent::alert_cast<libtorrent::state_changed_alert>(alert);
                if (sc) {
                    result["info_hash"] = String(libtorrent::aux::to_hex(sc->handle.info_hash()).c_str());
                    result["prev_state"] = int(sc->prev_state);
                    result["state"] = int(sc->state);
                }
                break;
            }
            
            case libtorrent::tracker_error_alert::alert_type:
            {
                auto* te = libtorrent::alert_cast<libtorrent::tracker_error_alert>(alert);
                if (te) {
                    result["info_hash"] = String(libtorrent::aux::to_hex(te->handle.info_hash()).c_str());
                    result["url"] = String(te->tracker_url().c_str());
                    result["error_code"] = te->error.value();
                    result["error_message"] = String(te->error.message().c_str());
                }
                break;
            }
            
            case libtorrent::tracker_warning_alert::alert_type:
            {
                auto* tw = libtorrent::alert_cast<libtorrent::tracker_warning_alert>(alert);
                if (tw) {
                    result["info_hash"] = String(libtorrent::aux::to_hex(tw->handle.info_hash()).c_str());
                    result["url"] = String(tw->tracker_url().c_str());
                    result["warning_message"] = String(tw->warning_message.c_str());
                }
                break;
            }
            
            case libtorrent::peer_connect_alert::alert_type:
            {
                auto* pc = libtorrent::alert_cast<libtorrent::peer_connect_alert>(alert);
                if (pc) {
                    result["info_hash"] = String(libtorrent::aux::to_hex(pc->handle.info_hash()).c_str());
                    result["peer_ip"] = String(pc->endpoint.address().to_string().c_str());
                    result["peer_port"] = pc->endpoint.port();
                    result["peer_id"] = String(libtorrent::aux::to_hex(pc->pid).c_str());
                }
                break;
            }
            
            case libtorrent::peer_disconnected_alert::alert_type:
            {
                auto* pd = libtorrent::alert_cast<libtorrent::peer_disconnected_alert>(alert);
                if (pd) {
                    result["info_hash"] = String(libtorrent::aux::to_hex(pd->handle.info_hash()).c_str());
                    result["peer_ip"] = String(pd->endpoint.address().to_string().c_str());
                    result["peer_port"] = pd->endpoint.port();
                    result["peer_id"] = String(libtorrent::aux::to_hex(pd->pid).c_str());
                    result["error_code"] = pd->error.value();
                    result["reason"] = int(pd->reason);
                }
                break;
            }
            
            case libtorrent::file_completed_alert::alert_type:
            {
                auto* fc = libtorrent::alert_cast<libtorrent::file_completed_alert>(alert);
                if (fc) {
                    result["info_hash"] = String(libtorrent::aux::to_hex(fc->handle.info_hash()).c_str());
                    result["file_index"] = fc->index;
                }
                break;
            }
            
            case libtorrent::performance_alert::alert_type:
            {
                auto* pa = libtorrent::alert_cast<libtorrent::performance_alert>(alert);
                if (pa) {
                    result["info_hash"] = String(libtorrent::aux::to_hex(pa->handle.info_hash()).c_str());
                    result["warning_code"] = int(pa->warning_code);
                }
                break;
            }
            
            default:
                // For unknown alert types, just include basic information
                break;
        }
        
    } catch (const std::exception& e) {
        // If conversion fails, return basic error information
        result["error"] = "Alert conversion failed: " + String(e.what());
        result["type"] = -1;
        result["category"] = 0;
        result["message"] = "Alert conversion error";
    }
#endif
    
    return result;
}

String TorrentSession::get_alert_type_name(int alert_type) {
#ifndef TORRENT_STUB_MODE
    // Map common alert types to readable names
    switch (alert_type) {
        case libtorrent::torrent_added_alert::alert_type: return "torrent_added";
        case libtorrent::torrent_removed_alert::alert_type: return "torrent_removed";
        case libtorrent::state_changed_alert::alert_type: return "state_changed";
        case libtorrent::tracker_error_alert::alert_type: return "tracker_error";
        case libtorrent::tracker_warning_alert::alert_type: return "tracker_warning";
        case libtorrent::tracker_announce_alert::alert_type: return "tracker_announce";
        case libtorrent::tracker_reply_alert::alert_type: return "tracker_reply";
        case libtorrent::peer_connect_alert::alert_type: return "peer_connect";
        case libtorrent::peer_disconnected_alert::alert_type: return "peer_disconnected";
        case libtorrent::file_completed_alert::alert_type: return "file_completed";
        case libtorrent::performance_alert::alert_type: return "performance_warning";
        case libtorrent::stats_alert::alert_type: return "stats";
        case libtorrent::session_stats_alert::alert_type: return "session_stats";
        case libtorrent::dht_announce_alert::alert_type: return "dht_announce";
        case libtorrent::dht_get_peers_alert::alert_type: return "dht_get_peers";
        default: return "unknown_" + String::num_int64(alert_type);
    }
#else
    return "stub_alert_" + String::num_int64(alert_type);
#endif
}

String TorrentSession::get_alert_category_name(int category_mask) {
#ifndef TORRENT_STUB_MODE
    Array categories;
    
    if (category_mask & int(libtorrent::alert_category::error)) categories.append("error");
    if (category_mask & int(libtorrent::alert_category::peer)) categories.append("peer");
    if (category_mask & int(libtorrent::alert_category::port_mapping)) categories.append("port_mapping");
    if (category_mask & int(libtorrent::alert_category::storage)) categories.append("storage");
    if (category_mask & int(libtorrent::alert_category::tracker)) categories.append("tracker");
    if (category_mask & int(libtorrent::alert_category::connect)) categories.append("connect");
    if (category_mask & int(libtorrent::alert_category::status)) categories.append("status");
    if (category_mask & int(libtorrent::alert_category::performance_warning)) categories.append("performance_warning");
    if (category_mask & int(libtorrent::alert_category::dht)) categories.append("dht");
    if (category_mask & int(libtorrent::alert_category::stats)) categories.append("stats");
    
    if (categories.size() == 0) {
        return "unknown";
    } else if (categories.size() == 1) {
        return String(categories[0]);
    } else {
        String result = String(categories[0]);
        for (int i = 1; i < categories.size(); i++) {
            result += "|" + String(categories[i]);
        }
        return result;
    }
#else
    return "stub_category_" + String::num_int64(category_mask);
#endif
}