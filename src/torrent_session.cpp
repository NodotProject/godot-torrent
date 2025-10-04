#include "torrent_session.h"
#include "torrent_handle.h"
#include "torrent_status.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/time.hpp>

#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/session_stats.hpp>
#include <libtorrent/hex.hpp>

#include <vector>
#include <string>

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

    ClassDB::bind_method(D_METHOD("bind_network_interface", "interface_ip"), &TorrentSession::bind_network_interface, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("get_listening_ports"), &TorrentSession::get_listening_ports);
    ClassDB::bind_method(D_METHOD("get_network_status"), &TorrentSession::get_network_status);

    ClassDB::bind_method(D_METHOD("enable_upnp_port_mapping", "enable"), &TorrentSession::enable_upnp_port_mapping, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("enable_natpmp_port_mapping", "enable"), &TorrentSession::enable_natpmp_port_mapping, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("get_port_mapping_status"), &TorrentSession::get_port_mapping_status);

    ClassDB::bind_method(D_METHOD("enable_ipv6", "enable"), &TorrentSession::enable_ipv6, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("is_ipv6_enabled"), &TorrentSession::is_ipv6_enabled);

    ClassDB::bind_method(D_METHOD("run_network_diagnostics"), &TorrentSession::run_network_diagnostics);
    ClassDB::bind_method(D_METHOD("get_network_interfaces"), &TorrentSession::get_network_interfaces);

    ClassDB::bind_method(D_METHOD("add_torrent_file", "torrent_data", "save_path"), &TorrentSession::add_torrent_file);
    ClassDB::bind_method(D_METHOD("add_magnet_uri", "magnet_uri", "save_path"), &TorrentSession::add_magnet_uri);
    ClassDB::bind_method(D_METHOD("remove_torrent", "handle", "delete_files"), &TorrentSession::remove_torrent, DEFVAL(false));

    ClassDB::bind_method(D_METHOD("get_session_stats"), &TorrentSession::get_session_stats);
    ClassDB::bind_method(D_METHOD("get_alerts"), &TorrentSession::get_alerts);
    ClassDB::bind_method(D_METHOD("clear_alerts"), &TorrentSession::clear_alerts);
}

TorrentSession::TorrentSession() : _session(nullptr) {
}

TorrentSession::~TorrentSession() {
    stop_session();
}

bool TorrentSession::start_session() {
    if (_session) {
        return true;
    }

    try {
        libtorrent::settings_pack settings;
        settings.set_str(libtorrent::settings_pack::user_agent, "Godot-Torrent/1.0.0");
        settings.set_str(libtorrent::settings_pack::listen_interfaces, "0.0.0.0:6881");
        settings.set_bool(libtorrent::settings_pack::enable_dht, true);
        settings.set_bool(libtorrent::settings_pack::enable_lsd, true);
        settings.set_bool(libtorrent::settings_pack::enable_upnp, true);
        settings.set_bool(libtorrent::settings_pack::enable_natpmp, true);
        settings.set_int(libtorrent::settings_pack::alert_mask,
            libtorrent::alert::error_notification |
            libtorrent::alert::status_notification |
            libtorrent::alert::storage_notification);

        _session = new libtorrent::session(settings);
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to start session: " + String(e.what()));
        return false;
    }
}

bool TorrentSession::start_session_with_settings(Dictionary settings) {
    if (_session) {
        return true;
    }

    try {
        libtorrent::settings_pack lt_settings;
        lt_settings.set_str(libtorrent::settings_pack::user_agent, "Godot-Torrent/1.0.0");
        apply_dictionary_settings(settings);

        _session = new libtorrent::session(lt_settings);
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to start session with settings: " + String(e.what()));
        return false;
    }
}

void TorrentSession::stop_session() {
    if (_session) {
        delete _session;
        _session = nullptr;
    }
}

bool TorrentSession::is_running() const {
    return _session != nullptr;
}

void TorrentSession::set_download_rate_limit(int bytes_per_second) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_int(libtorrent::settings_pack::download_rate_limit, bytes_per_second);
        _session->apply_settings(settings);
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set download rate limit: " + String(e.what()));
    }
}

void TorrentSession::set_upload_rate_limit(int bytes_per_second) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_int(libtorrent::settings_pack::upload_rate_limit, bytes_per_second);
        _session->apply_settings(settings);
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set upload rate limit: " + String(e.what()));
    }
}

void TorrentSession::set_listen_port(int port) {
    set_listen_port_range(port, port);
}

void TorrentSession::set_listen_port_range(int min_port, int max_port) {
    if (!_session) return;

    try {
        std::string listen_interfaces = "0.0.0.0:" + std::to_string(min_port);
        if (min_port != max_port) {
            listen_interfaces += "-" + std::to_string(max_port);
        }

        libtorrent::settings_pack settings;
        settings.set_str(libtorrent::settings_pack::listen_interfaces, listen_interfaces);
        _session->apply_settings(settings);
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set listen port range: " + String(e.what()));
    }
}

bool TorrentSession::is_dht_running() {
    if (!_session) return false;

    try {
        return _session->is_dht_running();
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to check DHT status: " + String(e.what()));
        return false;
    }
}

void TorrentSession::start_dht() {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_bool(libtorrent::settings_pack::enable_dht, true);
        _session->apply_settings(settings);
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to start DHT: " + String(e.what()));
    }
}

void TorrentSession::stop_dht() {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_bool(libtorrent::settings_pack::enable_dht, false);
        _session->apply_settings(settings);
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to stop DHT: " + String(e.what()));
    }
}

Dictionary TorrentSession::get_dht_state() {
    Dictionary state;

    if (!_session) {
        state["running"] = false;
        state["nodes"] = 0;
        return state;
    }

    try {
        state["running"] = _session->is_dht_running();
        // Note: DHT node count requires parsing session stats
        state["nodes"] = 0; // Would need to implement via get_session_stats
        return state;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to get DHT state: " + String(e.what()));
        return state;
    }
}

void TorrentSession::set_dht_bootstrap_nodes(Array nodes) {
    if (!_session) return;

    try {
        // libtorrent expects bootstrap nodes to be added via DHT routing table
        // This requires direct DHT API access which is not exposed in simple settings
        UtilityFunctions::push_warning("DHT bootstrap nodes configuration not yet implemented");
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set DHT bootstrap nodes: " + String(e.what()));
    }
}

bool TorrentSession::bind_network_interface(String interface_ip) {
    if (!_session) return false;

    try {
        std::string listen_interfaces = interface_ip.is_empty() ?
            "0.0.0.0:6881" :
            interface_ip.utf8().get_data() + std::string(":6881");

        libtorrent::settings_pack settings;
        settings.set_str(libtorrent::settings_pack::listen_interfaces, listen_interfaces);
        _session->apply_settings(settings);
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to bind network interface: " + String(e.what()));
        return false;
    }
}

Array TorrentSession::get_listening_ports() {
    Array ports;

    if (!_session) return ports;

    try {
        // libtorrent doesn't expose listening ports directly via simple API
        // Would need to parse alerts or use advanced session API
        UtilityFunctions::push_warning("get_listening_ports not fully implemented");
        return ports;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to get listening ports: " + String(e.what()));
        return ports;
    }
}

Dictionary TorrentSession::get_network_status() {
    Dictionary status;

    if (!_session) {
        status["running"] = false;
        return status;
    }

    try {
        status["running"] = true;
        // Network status would come from session stats
        return status;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to get network status: " + String(e.what()));
        return status;
    }
}

bool TorrentSession::enable_upnp_port_mapping(bool enable) {
    if (!_session) return false;

    try {
        libtorrent::settings_pack settings;
        settings.set_bool(libtorrent::settings_pack::enable_upnp, enable);
        _session->apply_settings(settings);
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to configure UPnP: " + String(e.what()));
        return false;
    }
}

bool TorrentSession::enable_natpmp_port_mapping(bool enable) {
    if (!_session) return false;

    try {
        libtorrent::settings_pack settings;
        settings.set_bool(libtorrent::settings_pack::enable_natpmp, enable);
        _session->apply_settings(settings);
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to configure NAT-PMP: " + String(e.what()));
        return false;
    }
}

Dictionary TorrentSession::get_port_mapping_status() {
    Dictionary status;

    if (!_session) return status;

    try {
        // Port mapping status would come from alerts
        status["upnp_enabled"] = true; // Would need to track via settings
        status["natpmp_enabled"] = true;
        return status;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to get port mapping status: " + String(e.what()));
        return status;
    }
}

void TorrentSession::enable_ipv6(bool enable) {
    if (!_session) return;

    try {
        // IPv6 is controlled via listen_interfaces setting
        std::string listen_interfaces = enable ?
            "0.0.0.0:6881,[::]:6881" :
            "0.0.0.0:6881";

        libtorrent::settings_pack settings;
        settings.set_str(libtorrent::settings_pack::listen_interfaces, listen_interfaces);
        _session->apply_settings(settings);
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to configure IPv6: " + String(e.what()));
    }
}

bool TorrentSession::is_ipv6_enabled() {
    if (!_session) return false;

    try {
        // Would need to parse current listen_interfaces setting
        return false; // Not implemented
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to check IPv6 status: " + String(e.what()));
        return false;
    }
}

Dictionary TorrentSession::run_network_diagnostics() {
    Dictionary diagnostics;

    if (!_session) {
        diagnostics["error"] = "Session not running";
        return diagnostics;
    }

    diagnostics["session_running"] = true;
    diagnostics["dht_state"] = get_dht_state();
    return diagnostics;
}

Array TorrentSession::get_network_interfaces() {
    Array interfaces;
    // This would require system-level network enumeration
    // Not directly exposed by libtorrent
    return interfaces;
}

Ref<TorrentHandle> TorrentSession::add_torrent_file(PackedByteArray torrent_data, String save_path) {
    if (!_session) {
        UtilityFunctions::push_error("Session not running");
        return Ref<TorrentHandle>();
    }

    try {
        const char* data_ptr = reinterpret_cast<const char*>(torrent_data.ptr());

        libtorrent::error_code ec;
        auto torrent_info = std::make_shared<libtorrent::torrent_info>(
            libtorrent::span<char const>(data_ptr, torrent_data.size()),
            libtorrent::from_span
        );

        if (ec) {
            UtilityFunctions::push_error("Failed to parse torrent: " + String(ec.message().c_str()));
            return Ref<TorrentHandle>();
        }

        libtorrent::add_torrent_params params;
        params.ti = torrent_info;
        params.save_path = save_path.utf8().get_data();

        libtorrent::torrent_handle lt_handle = _session->add_torrent(params, ec);

        if (ec) {
            UtilityFunctions::push_error("Failed to add torrent: " + String(ec.message().c_str()));
            return Ref<TorrentHandle>();
        }

        Ref<TorrentHandle> handle;
        handle.instantiate();

        libtorrent::torrent_handle* handle_copy = new libtorrent::torrent_handle(lt_handle);
        Dictionary handle_data;
        handle_data["libtorrent_ptr"] = (uint64_t)handle_copy;
        handle->_set_internal_handle(handle_data);

        return handle;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Exception adding torrent: " + String(e.what()));
        return Ref<TorrentHandle>();
    }
}

Ref<TorrentHandle> TorrentSession::add_magnet_uri(String magnet_uri, String save_path) {
    if (!_session) {
        UtilityFunctions::push_error("Session not running");
        return Ref<TorrentHandle>();
    }

    try {
        libtorrent::error_code ec;
        libtorrent::add_torrent_params params;

        libtorrent::parse_magnet_uri(magnet_uri.utf8().get_data(), params, ec);

        if (ec) {
            UtilityFunctions::push_error("Failed to parse magnet: " + String(ec.message().c_str()));
            return Ref<TorrentHandle>();
        }

        params.save_path = save_path.utf8().get_data();

        libtorrent::torrent_handle lt_handle = _session->add_torrent(params, ec);

        if (ec) {
            UtilityFunctions::push_error("Failed to add magnet: " + String(ec.message().c_str()));
            return Ref<TorrentHandle>();
        }

        Ref<TorrentHandle> handle;
        handle.instantiate();

        libtorrent::torrent_handle* handle_copy = new libtorrent::torrent_handle(lt_handle);
        Dictionary handle_data;
        handle_data["libtorrent_ptr"] = (uint64_t)handle_copy;
        handle->_set_internal_handle(handle_data);

        return handle;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Exception adding magnet: " + String(e.what()));
        return Ref<TorrentHandle>();
    }
}

bool TorrentSession::remove_torrent(Ref<TorrentHandle> handle, bool delete_files) {
    if (!_session) {
        UtilityFunctions::push_error("Session not running");
        return false;
    }

    if (handle.is_null() || !handle->is_valid()) {
        UtilityFunctions::push_error("Invalid handle");
        return false;
    }

    try {
        Variant handle_data = handle->_get_internal_handle();
        if (handle_data.get_type() != Variant::DICTIONARY) {
            return false;
        }

        Dictionary data_dict = handle_data;
        if (!data_dict.has("libtorrent_ptr")) {
            return false;
        }

        uint64_t ptr_value = data_dict["libtorrent_ptr"];
        libtorrent::torrent_handle* lt_handle = reinterpret_cast<libtorrent::torrent_handle*>(ptr_value);

        libtorrent::remove_flags_t flags = {};
        if (delete_files) {
            flags |= libtorrent::session::delete_files;
        }

        _session->remove_torrent(*lt_handle, flags);
        handle->_set_internal_handle(Variant());

        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to remove torrent: " + String(e.what()));
        return false;
    }
}

Dictionary TorrentSession::get_session_stats() {
    Dictionary stats;

    if (!_session) return stats;

    try {
        _session->post_session_stats();
        // Would need to wait for session_stats_alert and parse it
        // Simplified version - just return empty for now
        return stats;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to get session stats: " + String(e.what()));
        return stats;
    }
}

Array TorrentSession::get_alerts() {
    Array result;

    if (!_session) return result;

    try {
        std::vector<libtorrent::alert*> alerts;
        _session->pop_alerts(&alerts);

        for (auto* alert : alerts) {
            if (alert) {
                Dictionary alert_dict;
                alert_dict["message"] = String(alert->message().c_str());
                alert_dict["type"] = alert->type();
                alert_dict["what"] = String(alert->what());
                result.append(alert_dict);
            }
        }

        return result;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to get alerts: " + String(e.what()));
        return result;
    }
}

void TorrentSession::clear_alerts() {
    if (!_session) return;

    try {
        std::vector<libtorrent::alert*> alerts;
        _session->pop_alerts(&alerts);
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to clear alerts: " + String(e.what()));
    }
}

void TorrentSession::apply_dictionary_settings(Dictionary settings) {
    if (!_session) return;

    libtorrent::settings_pack lt_settings;

    Array keys = settings.keys();
    for (int i = 0; i < keys.size(); i++) {
        String key = keys[i];
        Variant value = settings[key];

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

    _session->apply_settings(lt_settings);
}
