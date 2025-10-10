#include "torrent_session.h"
#include "torrent_handle.h"
#include "torrent_status.h"
#include "torrent_error.h"
#include "torrent_logger.h"
#include "torrent_key_pair.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>

#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/session_stats.hpp>
#include <libtorrent/hex.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/bdecode.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/session_handle.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/address.hpp>
#include <libtorrent/create_torrent.hpp>

#include <chrono>
#include <libtorrent/file_storage.hpp>
#include <libtorrent/aux_/ed25519.hpp>

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

    ClassDB::bind_method(D_METHOD("set_max_connections", "limit"), &TorrentSession::set_max_connections);
    ClassDB::bind_method(D_METHOD("set_max_uploads", "limit"), &TorrentSession::set_max_uploads);
    ClassDB::bind_method(D_METHOD("set_max_half_open_connections", "limit"), &TorrentSession::set_max_half_open_connections);

    ClassDB::bind_method(D_METHOD("set_encryption_policy", "policy"), &TorrentSession::set_encryption_policy);
    ClassDB::bind_method(D_METHOD("set_prefer_encrypted", "prefer"), &TorrentSession::set_prefer_encrypted);

    ClassDB::bind_method(D_METHOD("is_dht_running"), &TorrentSession::is_dht_running);
    ClassDB::bind_method(D_METHOD("start_dht"), &TorrentSession::start_dht);
    ClassDB::bind_method(D_METHOD("stop_dht"), &TorrentSession::stop_dht);
    ClassDB::bind_method(D_METHOD("get_dht_state"), &TorrentSession::get_dht_state);
    ClassDB::bind_method(D_METHOD("set_dht_bootstrap_nodes", "nodes"), &TorrentSession::set_dht_bootstrap_nodes);
    ClassDB::bind_method(D_METHOD("add_dht_node", "host", "port"), &TorrentSession::add_dht_node);
    ClassDB::bind_method(D_METHOD("save_dht_state"), &TorrentSession::save_dht_state);
    ClassDB::bind_method(D_METHOD("load_dht_state", "dht_data"), &TorrentSession::load_dht_state);

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
    ClassDB::bind_method(D_METHOD("add_torrent_file_with_resume", "torrent_data", "save_path", "resume_data"), &TorrentSession::add_torrent_file_with_resume);
    ClassDB::bind_method(D_METHOD("add_magnet_uri", "magnet_uri", "save_path"), &TorrentSession::add_magnet_uri);
    ClassDB::bind_method(D_METHOD("add_magnet_uri_with_resume", "magnet_uri", "save_path", "resume_data"), &TorrentSession::add_magnet_uri_with_resume);
    ClassDB::bind_method(D_METHOD("remove_torrent", "handle", "delete_files"), &TorrentSession::remove_torrent, DEFVAL(false));

    ClassDB::bind_method(D_METHOD("create_torrent_from_path", "path", "piece_size", "flags", "comment", "creator"), &TorrentSession::create_torrent_from_path, DEFVAL(0), DEFVAL(0), DEFVAL(""), DEFVAL(""));
    ClassDB::bind_method(D_METHOD("dht_put_mutable_item", "keypair", "value", "salt"), &TorrentSession::dht_put_mutable_item, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("dht_get_mutable_item", "public_key", "salt"), &TorrentSession::dht_get_mutable_item, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("add_mutable_torrent", "keypair", "save_path", "initial_torrent_data"), &TorrentSession::add_mutable_torrent, DEFVAL(PackedByteArray()));
    ClassDB::bind_method(D_METHOD("subscribe_mutable_torrent", "public_key", "save_path"), &TorrentSession::subscribe_mutable_torrent);
    ClassDB::bind_method(D_METHOD("publish_mutable_torrent_update", "public_key", "new_torrent_data"), &TorrentSession::publish_mutable_torrent_update);
    ClassDB::bind_method(D_METHOD("check_mutable_torrent_for_updates", "public_key"), &TorrentSession::check_mutable_torrent_for_updates);

    ClassDB::bind_method(D_METHOD("get_session_stats"), &TorrentSession::get_session_stats);
    ClassDB::bind_method(D_METHOD("get_alerts"), &TorrentSession::get_alerts);
    ClassDB::bind_method(D_METHOD("clear_alerts"), &TorrentSession::clear_alerts);
    ClassDB::bind_method(D_METHOD("post_torrent_updates"), &TorrentSession::post_torrent_updates);

    ADD_SIGNAL(MethodInfo("metadata_received", PropertyInfo(Variant::STRING, "info_hash")));

    ClassDB::bind_method(D_METHOD("save_state"), &TorrentSession::save_state);
    ClassDB::bind_method(D_METHOD("load_state", "state_data"), &TorrentSession::load_state);

    ClassDB::bind_method(D_METHOD("set_ip_filter_enabled", "enabled"), &TorrentSession::set_ip_filter_enabled);
    ClassDB::bind_method(D_METHOD("add_ip_filter_rule", "ip_range", "blocked"), &TorrentSession::add_ip_filter_rule);
    ClassDB::bind_method(D_METHOD("clear_ip_filter"), &TorrentSession::clear_ip_filter);

    ClassDB::bind_method(D_METHOD("set_cache_size", "size_mb"), &TorrentSession::set_cache_size);
    ClassDB::bind_method(D_METHOD("set_cache_expiry", "seconds"), &TorrentSession::set_cache_expiry);

    ClassDB::bind_method(D_METHOD("set_logger", "logger"), &TorrentSession::set_logger);
    ClassDB::bind_method(D_METHOD("get_logger"), &TorrentSession::get_logger);
    ClassDB::bind_method(D_METHOD("enable_logging", "enabled"), &TorrentSession::enable_logging);
    ClassDB::bind_method(D_METHOD("set_log_level", "level"), &TorrentSession::set_log_level);
}

TorrentSession::TorrentSession() : _session(nullptr), _update_check_interval_seconds(300) {
    _last_update_check = std::chrono::steady_clock::now();
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

        // Listen on all interfaces (0.0.0.0) with random port for incoming connections
        settings.set_str(libtorrent::settings_pack::listen_interfaces, "0.0.0.0:6881,[::]:6881");

        // Don't enable DHT by default to avoid blocking during startup
        // User should call start_dht() explicitly
        settings.set_bool(libtorrent::settings_pack::enable_dht, false);
        settings.set_bool(libtorrent::settings_pack::enable_lsd, true);
        settings.set_bool(libtorrent::settings_pack::enable_upnp, true);
        settings.set_bool(libtorrent::settings_pack::enable_natpmp, true);

        settings.set_int(libtorrent::settings_pack::alert_mask,
            libtorrent::alert::error_notification |
            libtorrent::alert::status_notification |
            libtorrent::alert::storage_notification |
            libtorrent::alert::tracker_notification |
            libtorrent::alert::session_log_notification);

        // Increase alert queue size to prevent blocking
        settings.set_int(libtorrent::settings_pack::alert_queue_size, 10000);

        // Connection settings for better peer discovery
        settings.set_int(libtorrent::settings_pack::connections_limit, 200);
        settings.set_int(libtorrent::settings_pack::active_downloads, 3);
        settings.set_int(libtorrent::settings_pack::active_seeds, 5);
        settings.set_int(libtorrent::settings_pack::active_limit, 15);

        // Fast shutdown settings (don't wait for trackers)
        settings.set_int(libtorrent::settings_pack::stop_tracker_timeout, 1);
        settings.set_int(libtorrent::settings_pack::auto_scrape_interval, 1800);
        settings.set_int(libtorrent::settings_pack::auto_scrape_min_interval, 900);

        _session = new libtorrent::session(settings);
        return true;
    } catch (const std::exception& e) {
        report_error("start_session", String("Failed to start session: ") + e.what());
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
        try {
            // Proper libtorrent shutdown sequence:
            // 1. Pause session to stop all activity
            _session->pause();

            // 2. Remove all torrents immediately without waiting for trackers
            std::vector<libtorrent::torrent_handle> torrents = _session->get_torrents();
            for (auto& handle : torrents) {
                try {
                    // Remove without announcing to trackers (avoid network delays)
                    _session->remove_torrent(handle,
                        libtorrent::session_handle::delete_partfile); // Removed libtorrent::session::delete_files
                } catch (...) {
                    // Ignore errors removing individual torrents
                }
            }

            // 3. Apply settings to speed up shutdown
            libtorrent::settings_pack shutdown_settings;
            // Don't wait for tracker announces on shutdown
            shutdown_settings.set_int(libtorrent::settings_pack::stop_tracker_timeout, 0);
            shutdown_settings.set_bool(libtorrent::settings_pack::announce_to_all_trackers, false);
            shutdown_settings.set_bool(libtorrent::settings_pack::announce_to_all_tiers, false);
            _session->apply_settings(shutdown_settings);

            // 4. Call abort() to get a session_proxy for async shutdown
            // 5. Delete the session (non-blocking)
            // 6. session_proxy destructor will block until shutdown completes
            //    (but should be fast now that trackers are skipped)
            auto proxy = _session->abort();
            delete _session;
            _session = nullptr;
            // proxy destructor blocks here until session is fully shut down
        } catch (const std::exception& e) {
            UtilityFunctions::push_error("Error during session shutdown: " + String(e.what()));
            // Force cleanup even on error
            try {
                delete _session;
            } catch (...) {}
            _session = nullptr;
        }
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

void TorrentSession::set_max_connections(int limit) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_int(libtorrent::settings_pack::connections_limit, limit);
        _session->apply_settings(settings);
        UtilityFunctions::print("Max connections set to: " + String::num(limit));
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set max connections: " + String(e.what()));
    }
}

void TorrentSession::set_max_uploads(int limit) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_int(libtorrent::settings_pack::unchoke_slots_limit, limit);
        _session->apply_settings(settings);
        UtilityFunctions::print("Max uploads set to: " + String::num(limit));
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set max uploads: " + String(e.what()));
    }
}

void TorrentSession::set_max_half_open_connections(int limit) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_int(libtorrent::settings_pack::half_open_limit, limit);
        _session->apply_settings(settings);
        UtilityFunctions::print("Max half-open connections set to: " + String::num(limit));
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set max half-open connections: " + String(e.what()));
    }
}

void TorrentSession::set_encryption_policy(int policy) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;

        // 0=disabled, 1=enabled, 2=forced
        switch (policy) {
            case 0: // Disabled
                settings.set_int(libtorrent::settings_pack::out_enc_policy, libtorrent::settings_pack::pe_disabled);
                settings.set_int(libtorrent::settings_pack::in_enc_policy, libtorrent::settings_pack::pe_disabled);
                UtilityFunctions::print("Encryption policy: Disabled");
                break;
            case 1: // Enabled (prefer plaintext)
                settings.set_int(libtorrent::settings_pack::out_enc_policy, libtorrent::settings_pack::pe_enabled);
                settings.set_int(libtorrent::settings_pack::in_enc_policy, libtorrent::settings_pack::pe_enabled);
                UtilityFunctions::print("Encryption policy: Enabled");
                break;
            case 2: // Forced (only encrypted)
                settings.set_int(libtorrent::settings_pack::out_enc_policy, libtorrent::settings_pack::pe_forced);
                settings.set_int(libtorrent::settings_pack::in_enc_policy, libtorrent::settings_pack::pe_forced);
                UtilityFunctions::print("Encryption policy: Forced");
                break;
            default:
                UtilityFunctions::push_error("Invalid encryption policy. Use 0=disabled, 1=enabled, 2=forced");
                return;
        }

        _session->apply_settings(settings);
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set encryption policy: " + String(e.what()));
    }
}

void TorrentSession::set_prefer_encrypted(bool prefer) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        if (prefer) {
            settings.set_bool(libtorrent::settings_pack::prefer_rc4, true);
            UtilityFunctions::print("Prefer encrypted connections: enabled");
        } else {
            settings.set_bool(libtorrent::settings_pack::prefer_rc4, false);
            UtilityFunctions::print("Prefer encrypted connections: disabled");
        }
        _session->apply_settings(settings);
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set prefer encrypted: " + String(e.what()));
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

        // Set DHT bootstrap nodes for better connectivity
        settings.set_str(libtorrent::settings_pack::dht_bootstrap_nodes,
            "dht.transmissionbt.com:6881,"
            "router.bittorrent.com:6881,"
            "router.utorrent.com:6881,"
            "dht.libtorrent.org:25401");

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
        if (nodes.size() == 0) {
            return;
        }

        for (int i = 0; i < nodes.size(); i++) {
            String node = nodes[i];
            // Expected format: "host:port"
            int colon_pos = node.find(":");
            if (colon_pos > 0) {
                String host = node.substr(0, colon_pos);
                String port_str = node.substr(colon_pos + 1);
                int port = port_str.to_int();
                if (port > 0) {
                    add_dht_node(host, port);
                }
            }
        }
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set DHT bootstrap nodes: " + String(e.what()));
    }
}

void TorrentSession::add_dht_node(String host, int port) {
    if (!_session) return;

    try {
        _session->add_dht_node(std::make_pair(host.utf8().get_data(), port));
        UtilityFunctions::print("Added DHT node: " + host + ":" + String::num(port));
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to add DHT node: " + String(e.what()));
    }
}

PackedByteArray TorrentSession::save_dht_state() {
    PackedByteArray dht_data;

    if (!_session) {
        UtilityFunctions::push_error("Cannot save DHT state: Session not running");
        return dht_data;
    }

    try {
        libtorrent::entry state;
        _session->save_state(state, libtorrent::session::save_dht_state);

        std::vector<char> buffer;
        libtorrent::bencode(std::back_inserter(buffer), state);

        dht_data.resize(buffer.size());
        memcpy(dht_data.ptrw(), buffer.data(), buffer.size());

        UtilityFunctions::print("DHT state saved: " + String::num(dht_data.size()) + " bytes");
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to save DHT state: " + String(e.what()));
    }

    return dht_data;
}

bool TorrentSession::load_dht_state(PackedByteArray dht_data) {
    if (!_session) {
        UtilityFunctions::push_error("Cannot load DHT state: Session not running");
        return false;
    }

    if (dht_data.size() == 0) {
        UtilityFunctions::push_error("Cannot load DHT state: Empty DHT data");
        return false;
    }

    try {
        const char* data_ptr = reinterpret_cast<const char*>(dht_data.ptr());
        libtorrent::bdecode_node node;
        libtorrent::error_code ec;

        libtorrent::bdecode(data_ptr, data_ptr + dht_data.size(), node, ec);

        if (ec) {
            UtilityFunctions::push_error("Failed to decode DHT state: " + String(ec.message().c_str()));
            return false;
        }

        _session->load_state(node, libtorrent::session::save_dht_state);

        UtilityFunctions::print("DHT state loaded successfully");
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to load DHT state: " + String(e.what()));
        return false;
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
        report_error("add_torrent_file", "Session not running");
        return Ref<TorrentHandle>();
    }

    // Validate save_path
    if (save_path.is_empty()) {
        report_error("add_torrent_file", "Save path cannot be empty");
        return Ref<TorrentHandle>();
    }

    // Check for invalid path patterns
    if (save_path.contains("..") || save_path.contains("//")) {
        report_error("add_torrent_file", "Invalid save_path: contains '..' or '//' patterns");
        return Ref<TorrentHandle>();
    }

    try {
        const char* data_ptr = reinterpret_cast<const char*>(torrent_data.ptr());

        libtorrent::error_code ec;
        auto torrent_info = std::make_shared<libtorrent::torrent_info>(
            data_ptr,
            torrent_data.size(),
            ec
        );

        if (ec) {
            report_libtorrent_error("add_torrent_file", ec.value(), ec.message().c_str());
            return Ref<TorrentHandle>();
        }

        libtorrent::add_torrent_params params;
        params.ti = torrent_info;
        params.save_path = save_path.utf8().get_data();

        libtorrent::torrent_handle lt_handle = _session->add_torrent(params, ec);

        if (ec) {
            report_libtorrent_error("add_torrent_file", ec.value(), ec.message().c_str());
            return Ref<TorrentHandle>();
        }

        Ref<TorrentHandle> handle;
        handle.instantiate();

        libtorrent::torrent_handle* handle_copy = new libtorrent::torrent_handle(lt_handle);
        Dictionary handle_data;
        handle_data["libtorrent_ptr"] = (uint64_t)handle_copy;
        handle->_set_internal_handle(handle_data);
        handle->_set_parent_session(_session);

        return handle;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Exception adding torrent: " + String(e.what()));
        return Ref<TorrentHandle>();
    }
}

Ref<TorrentHandle> TorrentSession::add_torrent_file_with_resume(PackedByteArray torrent_data, String save_path, PackedByteArray resume_data) {
    if (!_session) {
        UtilityFunctions::push_error("Session not running");
        return Ref<TorrentHandle>();
    }

    // Validate save_path
    if (save_path.is_empty()) {
        UtilityFunctions::push_error("Save path cannot be empty");
        return Ref<TorrentHandle>();
    }

    // Check for invalid path patterns
    if (save_path.contains("..") || save_path.contains("//")) {
        UtilityFunctions::push_error("Invalid save_path: contains invalid characters");
        return Ref<TorrentHandle>();
    }

    try {
        const char* data_ptr = reinterpret_cast<const char*>(torrent_data.ptr());

        libtorrent::error_code ec;
        auto torrent_info = std::make_shared<libtorrent::torrent_info>(
            data_ptr,
            torrent_data.size(),
            ec
        );

        if (ec) {
            UtilityFunctions::push_error("Failed to parse torrent: " + String(ec.message().c_str()));
            return Ref<TorrentHandle>();
        }

        libtorrent::add_torrent_params params;
        params.ti = torrent_info;
        params.save_path = save_path.utf8().get_data();

        // Load resume data if provided
        if (resume_data.size() > 0) {
            const char* resume_ptr = reinterpret_cast<const char*>(resume_data.ptr());
            libtorrent::span<char const> resume_span(resume_ptr, resume_data.size());
            libtorrent::error_code resume_ec;

            params = libtorrent::read_resume_data(resume_span, resume_ec);
            if (resume_ec) {
                UtilityFunctions::push_warning("Failed to parse resume data: " + String(resume_ec.message().c_str()));
            } else {
                UtilityFunctions::print("Resume data loaded successfully");
            }

            // Re-apply torrent info and save path after reading resume data
            params.ti = torrent_info;
            params.save_path = save_path.utf8().get_data();
        }

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
        handle->_set_parent_session(_session);

        return handle;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Exception adding torrent: " + String(e.what()));
        return Ref<TorrentHandle>();
    }
}

Ref<TorrentHandle> TorrentSession::add_magnet_uri(String magnet_uri, String save_path) {
    if (!_session) {
        report_error("add_magnet_uri", "Session not running");
        return Ref<TorrentHandle>();
    }

    // Validate save_path
    if (save_path.is_empty()) {
        report_error("add_magnet_uri", "Save path cannot be empty");
        return Ref<TorrentHandle>();
    }

    // Check for invalid path patterns
    if (save_path.contains("..") || save_path.contains("//")) {
        report_error("add_magnet_uri", "Invalid save_path: contains '..' or '//' patterns");
        return Ref<TorrentHandle>();
    }

    try {
        libtorrent::error_code ec;
        libtorrent::add_torrent_params params;

        libtorrent::parse_magnet_uri(magnet_uri.utf8().get_data(), params, ec);

        if (ec) {
            report_libtorrent_error("parse_magnet_uri", ec.value(), ec.message().c_str());
            return Ref<TorrentHandle>();
        }

        // Add some public trackers to increase chances of finding peers
        params.trackers.push_back("udp://tracker.opentrackr.org:1337/announce");
        params.trackers.push_back("udp://tracker.openbittorrent.com:6969/announce");
        params.trackers.push_back("udp://opentracker.i2p.rocks:6969/announce");

        params.save_path = save_path.utf8().get_data();

        libtorrent::torrent_handle lt_handle = _session->add_torrent(params, ec);

        if (ec) {
            report_libtorrent_error("add_magnet", ec.value(), ec.message().c_str());
            return Ref<TorrentHandle>();
        }

        Ref<TorrentHandle> handle;
        handle.instantiate();

        libtorrent::torrent_handle* handle_copy = new libtorrent::torrent_handle(lt_handle);
        Dictionary handle_data;
        handle_data["libtorrent_ptr"] = (uint64_t)handle_copy;
        handle->_set_internal_handle(handle_data);
        handle->_set_parent_session(_session);

        return handle;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Exception adding magnet: " + String(e.what()));
        return Ref<TorrentHandle>();
    }
}

Ref<TorrentHandle> TorrentSession::add_magnet_uri_with_resume(String magnet_uri, String save_path, PackedByteArray resume_data) {
    if (!_session) {
        UtilityFunctions::push_error("Session not running");
        return Ref<TorrentHandle>();
    }

    // Validate save_path
    if (save_path.is_empty()) {
        UtilityFunctions::push_error("Save path cannot be empty");
        return Ref<TorrentHandle>();
    }

    // Check for invalid path patterns
    if (save_path.contains("..") || save_path.contains("//")) {
        UtilityFunctions::push_error("Invalid save_path: contains invalid characters");
        return Ref<TorrentHandle>();
    }

    try {
        libtorrent::error_code ec;
        libtorrent::add_torrent_params params;

        // Try to load resume data first
        if (resume_data.size() > 0) {
            const char* resume_ptr = reinterpret_cast<const char*>(resume_data.ptr());
            libtorrent::span<char const> resume_span(resume_ptr, resume_data.size());
            libtorrent::error_code resume_ec;

            params = libtorrent::read_resume_data(resume_span, resume_ec);
            if (resume_ec) {
                UtilityFunctions::push_warning("Failed to parse resume data: " + String(resume_ec.message().c_str()));
            } else {
                UtilityFunctions::print("Resume data loaded successfully for magnet");
            }
        }

        // Parse magnet URI (this will override/merge with resume data)
        libtorrent::parse_magnet_uri(magnet_uri.utf8().get_data(), params, ec);

        if (ec) {
            report_libtorrent_error("parse_magnet_uri", ec.value(), ec.message().c_str());
            return Ref<TorrentHandle>();
        }

        params.save_path = save_path.utf8().get_data();

        libtorrent::torrent_handle lt_handle = _session->add_torrent(params, ec);

        if (ec) {
            report_libtorrent_error("add_magnet", ec.value(), ec.message().c_str());
            return Ref<TorrentHandle>();
        }

        Ref<TorrentHandle> handle;
        handle.instantiate();

        libtorrent::torrent_handle* handle_copy = new libtorrent::torrent_handle(lt_handle);
        Dictionary handle_data;
        handle_data["libtorrent_ptr"] = (uint64_t)handle_copy;
        handle->_set_internal_handle(handle_data);
        handle->_set_parent_session(_session);

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

    // Check for mutable torrent updates periodically
    check_mutable_torrent_updates();

    try {
        std::vector<libtorrent::alert*> alerts;
        _session->pop_alerts(&alerts);

        for (auto* alert : alerts) {
            if (alert) {
                Dictionary alert_dict;
                alert_dict["message"] = String(alert->message().c_str());
                alert_dict["type"] = alert->type();
                alert_dict["what"] = String(alert->what());

                // Parse state_update_alert to extract torrent status
                if (auto* status_alert = libtorrent::alert_cast<libtorrent::state_update_alert>(alert)) {
                    Array status_array;
                    for (const auto& status : status_alert->status) {
                        Dictionary status_dict;

                        // Store info_hash as hex string for handle matching
                        status_dict["info_hash"] = String(libtorrent::aux::to_hex(status.info_hash).c_str());
                        status_dict["state"] = (int)status.state;
                        status_dict["paused"] = (bool)(status.flags & libtorrent::torrent_flags::paused);
                        status_dict["has_metadata"] = status.has_metadata;
                        status_dict["progress"] = status.progress;
                        status_dict["download_rate"] = status.download_rate;
                        status_dict["upload_rate"] = status.upload_rate;
                        status_dict["num_peers"] = status.num_peers;
                        status_dict["num_seeds"] = status.num_seeds;
                        status_dict["total_download"] = (int64_t)status.total_download;
                        status_dict["total_upload"] = (int64_t)status.total_upload;
                        status_dict["total_wanted"] = (int64_t)status.total_wanted;
                        status_dict["total_done"] = (int64_t)status.total_done;
                        status_dict["is_finished"] = status.is_finished;
                        status_dict["is_seeding"] = status.is_seeding;

                        status_array.append(status_dict);
                    }
                    alert_dict["torrent_status"] = status_array;
                }

                // Parse save_resume_data_alert to extract resume data
                if (auto* resume_alert = libtorrent::alert_cast<libtorrent::save_resume_data_alert>(alert)) {
                    std::vector<char> buffer = libtorrent::write_resume_data_buf(resume_alert->params);

                    PackedByteArray resume_data;
                    resume_data.resize(buffer.size());
                    memcpy(resume_data.ptrw(), buffer.data(), buffer.size());

                    alert_dict["resume_data"] = resume_data;
                    alert_dict["info_hash"] = String(libtorrent::aux::to_hex(resume_alert->handle.info_hash()).c_str());
                }

                // Parse file_renamed_alert
                if (auto* rename_alert = libtorrent::alert_cast<libtorrent::file_renamed_alert>(alert)) {
                    alert_dict["file_index"] = static_cast<int>(rename_alert->index);
                    alert_dict["new_name"] = String(rename_alert->new_name());
                    alert_dict["info_hash"] = String(libtorrent::aux::to_hex(rename_alert->handle.info_hash()).c_str());
                }

                // Parse file_rename_failed_alert
                if (auto* rename_failed_alert = libtorrent::alert_cast<libtorrent::file_rename_failed_alert>(alert)) {
                    alert_dict["file_index"] = static_cast<int>(rename_failed_alert->index);
                    alert_dict["error"] = String(rename_failed_alert->error.message().c_str());
                    alert_dict["info_hash"] = String(libtorrent::aux::to_hex(rename_failed_alert->handle.info_hash()).c_str());
                }

                // Parse read_piece_alert
                if (auto* read_piece_alert = libtorrent::alert_cast<libtorrent::read_piece_alert>(alert)) {
                    alert_dict["piece_index"] = static_cast<int>(read_piece_alert->piece);
                    alert_dict["info_hash"] = String(libtorrent::aux::to_hex(read_piece_alert->handle.info_hash()).c_str());

                    if (read_piece_alert->ec) {
                        alert_dict["error"] = String(read_piece_alert->ec.message().c_str());
                    } else {
                        // Convert piece data to PackedByteArray
                        PackedByteArray piece_data;
                        piece_data.resize(read_piece_alert->size);
                        memcpy(piece_data.ptrw(), read_piece_alert->buffer.get(), read_piece_alert->size);
                        alert_dict["piece_data"] = piece_data;
                        alert_dict["size"] = read_piece_alert->size;
                    }
                }

                // Parse storage_moved_alert
                if (auto* moved_alert = libtorrent::alert_cast<libtorrent::storage_moved_alert>(alert)) {
                    alert_dict["storage_path"] = String(moved_alert->storage_path());
                    alert_dict["info_hash"] = String(libtorrent::aux::to_hex(moved_alert->handle.info_hash()).c_str());
                }

                // Parse storage_moved_failed_alert
                if (auto* move_failed_alert = libtorrent::alert_cast<libtorrent::storage_moved_failed_alert>(alert)) {
                    alert_dict["error"] = String(move_failed_alert->error.message().c_str());
                    alert_dict["info_hash"] = String(libtorrent::aux::to_hex(move_failed_alert->handle.info_hash()).c_str());
                }

                // Parse tracker_reply_alert
                if (auto* tracker_reply = libtorrent::alert_cast<libtorrent::tracker_reply_alert>(alert)) {
                    alert_dict["tracker_url"] = String(tracker_reply->tracker_url());
                    alert_dict["num_peers"] = tracker_reply->num_peers;
                    alert_dict["info_hash"] = String(libtorrent::aux::to_hex(tracker_reply->handle.info_hash()).c_str());
                }

                // Parse tracker_error_alert
                if (auto* tracker_error = libtorrent::alert_cast<libtorrent::tracker_error_alert>(alert)) {
                    alert_dict["tracker_url"] = String(tracker_error->tracker_url());
                    alert_dict["error"] = String(tracker_error->error.message().c_str());
                    alert_dict["times_in_row"] = tracker_error->times_in_row;
                    alert_dict["status_code"] = tracker_error->status_code;
                    alert_dict["info_hash"] = String(libtorrent::aux::to_hex(tracker_error->handle.info_hash()).c_str());
                }

                // Parse tracker_announce_alert
                if (auto* announce = libtorrent::alert_cast<libtorrent::tracker_announce_alert>(alert)) {
                    alert_dict["tracker_url"] = String(announce->tracker_url());
                    alert_dict["event"] = static_cast<int>(announce->event);
                    alert_dict["info_hash"] = String(libtorrent::aux::to_hex(announce->handle.info_hash()).c_str());
                }

                // Parse dht_mutable_item_alert (BEP 46 support)
                if (auto* dht_alert = libtorrent::alert_cast<libtorrent::dht_mutable_item_alert>(alert)) {
                    // Convert public key to hex string
                    PackedByteArray pk_array;
                    pk_array.resize(32);
                    std::memcpy(pk_array.ptrw(), dht_alert->key.data(), 32);
                    alert_dict["public_key"] = pk_array.hex_encode();
                    alert_dict["public_key_bytes"] = pk_array;

                    // Sequence number
                    alert_dict["sequence"] = static_cast<int64_t>(dht_alert->seq);

                    // Convert entry to Dictionary (simplified - handles common types)
                    Dictionary value_dict;
                    if (dht_alert->item.type() == libtorrent::entry::dictionary_t) {
                        for (auto const& kv : dht_alert->item.dict()) {
                            String key_str(kv.first.c_str());
                            if (kv.second.type() == libtorrent::entry::string_t) {
                                value_dict[key_str] = String(kv.second.string().c_str());
                            } else if (kv.second.type() == libtorrent::entry::int_t) {
                                value_dict[key_str] = static_cast<int64_t>(kv.second.integer());
                            }
                        }
                    }
                    alert_dict["value"] = value_dict;

                    // Salt (if any)
                    alert_dict["salt"] = String(dht_alert->salt.c_str());

                    // Authoritative flag
                    alert_dict["authoritative"] = dht_alert->authoritative;

                    // Check if this is for a tracked mutable torrent and compare sequence numbers
                    std::string pk_str(reinterpret_cast<const char*>(pk_array.ptr()), 32);
                    auto it = _mutable_torrents.find(pk_str);

                    if (it != _mutable_torrents.end()) {
                        auto& info = it->second;

                        // Check for update (new sequence number)
                        if (dht_alert->seq > info.sequence_number) {
                            // Extract new info-hash from value
                            String new_info_hash = "";
                            if (value_dict.has("ih")) {
                                new_info_hash = value_dict["ih"];
                            }

                            // Create custom update alert
                            Dictionary update_alert;
                            update_alert["type"] = "mutable_torrent_update_alert";
                            update_alert["what"] = "torrent_update";
                            update_alert["message"] = "New version of mutable torrent available";
                            update_alert["public_key"] = pk_array.hex_encode();
                            update_alert["public_key_bytes"] = pk_array;
                            update_alert["old_sequence"] = static_cast<int64_t>(info.sequence_number);
                            update_alert["new_sequence"] = static_cast<int64_t>(dht_alert->seq);
                            update_alert["new_info_hash"] = new_info_hash;

                            // Update stored sequence
                            info.sequence_number = dht_alert->seq;

                            // Add the update alert to results
                            result.append(update_alert);

                            if (_logger.is_valid()) {
                                _logger->log_info("Mutable torrent update detected: seq " +
                                    String::num(info.sequence_number - 1) + " -> " + String::num(info.sequence_number), "SESSION");
                            }
                        }
                    }
                }

                // Parse dht_put_alert (confirmation of DHT put operation)
                if (auto* dht_put = libtorrent::alert_cast<libtorrent::dht_put_alert>(alert)) {
                    // Convert public key to hex string
                    PackedByteArray pk_array;
                    pk_array.resize(32);
                    std::memcpy(pk_array.ptrw(), dht_put->public_key.data(), 32);
                    alert_dict["public_key"] = pk_array.hex_encode();
                    alert_dict["public_key_bytes"] = pk_array;

                    // Sequence number
                    alert_dict["sequence"] = static_cast<int64_t>(dht_put->seq);

                    // Salt (if any)
                    alert_dict["salt"] = String(dht_put->salt.c_str());
                }

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

void TorrentSession::post_torrent_updates() {
    if (!_session) return;

    try {
        _session->post_torrent_updates();
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to post torrent updates: " + String(e.what()));
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

PackedByteArray TorrentSession::save_state() {
    PackedByteArray state_data;

    if (!_session) {
        UtilityFunctions::push_error("Cannot save state: Session not running");
        return state_data;
    }

    try {
        libtorrent::entry state;
        _session->save_state(state, libtorrent::session::save_settings |
                                     libtorrent::session::save_dht_state);

        std::vector<char> buffer;
        libtorrent::bencode(std::back_inserter(buffer), state);

        state_data.resize(buffer.size());
        memcpy(state_data.ptrw(), buffer.data(), buffer.size());

        UtilityFunctions::print("Session state saved: " + String::num(state_data.size()) + " bytes");
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to save session state: " + String(e.what()));
    }

    return state_data;
}

bool TorrentSession::load_state(PackedByteArray state_data) {
    if (!_session) {
        UtilityFunctions::push_error("Cannot load state: Session not running");
        return false;
    }

    if (state_data.size() == 0) {
        UtilityFunctions::push_error("Cannot load state: Empty state data");
        return false;
    }

    try {
        const char* data_ptr = reinterpret_cast<const char*>(state_data.ptr());
        libtorrent::bdecode_node node;
        libtorrent::error_code ec;

        libtorrent::bdecode(data_ptr, data_ptr + state_data.size(), node, ec);

        if (ec) {
            UtilityFunctions::push_error("Failed to decode session state: " + String(ec.message().c_str()));
            return false;
        }

        _session->load_state(node, libtorrent::session::save_settings |
                                    libtorrent::session::save_dht_state);

        UtilityFunctions::print("Session state loaded successfully");
        return true;
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to load session state: " + String(e.what()));
        return false;
    }
}

void TorrentSession::set_ip_filter_enabled(bool enabled) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_bool(libtorrent::settings_pack::apply_ip_filter_to_trackers, enabled);
        _session->apply_settings(settings);
        UtilityFunctions::print("IP filter " + String(enabled ? "enabled" : "disabled"));
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set IP filter: " + String(e.what()));
    }
}

void TorrentSession::add_ip_filter_rule(String ip_range, bool blocked) {
    if (!_session) return;

    try {
        // Parse IP range (format: "192.168.1.0-192.168.1.255" or "192.168.1.0")
        int dash_pos = ip_range.find("-");

        libtorrent::ip_filter filter;

        if (dash_pos > 0) {
            String start_ip = ip_range.substr(0, dash_pos).strip_edges();
            String end_ip = ip_range.substr(dash_pos + 1).strip_edges();

            libtorrent::address start_addr = libtorrent::make_address(start_ip.utf8().get_data());
            libtorrent::address end_addr = libtorrent::make_address(end_ip.utf8().get_data());

            filter.add_rule(start_addr, end_addr, blocked ? libtorrent::ip_filter::blocked : 0);
        } else {
            libtorrent::address addr = libtorrent::make_address(ip_range.utf8().get_data());
            filter.add_rule(addr, addr, blocked ? libtorrent::ip_filter::blocked : 0);
        }

        _session->set_ip_filter(filter);
        UtilityFunctions::print("IP filter rule added: " + ip_range + " (" + String(blocked ? "blocked" : "allowed") + ")");
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to add IP filter rule: " + String(e.what()));
    }
}

void TorrentSession::clear_ip_filter() {
    if (!_session) return;

    try {
        libtorrent::ip_filter filter;
        _session->set_ip_filter(filter);
        UtilityFunctions::print("IP filter cleared");
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to clear IP filter: " + String(e.what()));
    }
}

void TorrentSession::set_cache_size(int size_mb) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_int(libtorrent::settings_pack::cache_size, size_mb * 16); // 16 blocks per MB
        _session->apply_settings(settings);
        UtilityFunctions::print("Cache size set to: " + String::num(size_mb) + " MB");
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set cache size: " + String(e.what()));
    }
}

void TorrentSession::set_cache_expiry(int seconds) {
    if (!_session) return;

    try {
        libtorrent::settings_pack settings;
        settings.set_int(libtorrent::settings_pack::cache_expiry, seconds);
        _session->apply_settings(settings);
        UtilityFunctions::print("Cache expiry set to: " + String::num(seconds) + " seconds");
    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Failed to set cache expiry: " + String(e.what()));
    }
}

// Error handling helpers
void TorrentSession::report_error(const String& operation, const String& message) {
    String error_msg = "[TorrentSession::" + operation + "] " + message;
    UtilityFunctions::push_error(error_msg);
}

void TorrentSession::report_libtorrent_error(const String& operation, int error_code, const String& error_message) {
    String error_msg = "[TorrentSession::" + operation + "] libtorrent error " +
                      String::num(error_code) + ": " + error_message;
    UtilityFunctions::push_error(error_msg);

    // Also log to logger if available
    if (_logger.is_valid() && _logger->is_logging_enabled()) {
        _logger->log_error(error_msg, "SESSION");
    }
}

// Logging methods
void TorrentSession::set_logger(Ref<TorrentLogger> logger) {
    _logger = logger;
    if (_logger.is_valid()) {
        _logger->log_info("Logger attached to TorrentSession", "SESSION");
    }
}

Ref<TorrentLogger> TorrentSession::get_logger() const {
    return _logger;
}

void TorrentSession::enable_logging(bool enabled) {
    if (_logger.is_valid()) {
        _logger->enable_logging(enabled);
    } else {
        UtilityFunctions::push_warning("Cannot enable logging: no logger attached");
    }
}

void TorrentSession::set_log_level(int level) {
    if (_logger.is_valid()) {
        _logger->set_log_level(static_cast<TorrentLogger::LogLevel>(level));
    } else {
        UtilityFunctions::push_warning("Cannot set log level: no logger attached");
    }
}
// Mutable Torrent Implementations for TorrentSession
// This file contains the implementation of BEP 46 mutable torrent support

// Torrent Creation
PackedByteArray TorrentSession::create_torrent_from_path(String path, int piece_size, int flags, String comment, String creator) {
    if (!_session) {
        report_error("create_torrent_from_path", "Session not running");
        return PackedByteArray();
    }

    try {
        // Convert Godot path to std::string
        std::string path_str = path.utf8().get_data();

        // Check if path exists
        Ref<DirAccess> dir = DirAccess::open(path);
        Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);

        if (!dir.is_valid() && !file.is_valid()) {
            report_error("create_torrent_from_path", "Path does not exist: " + path);
            return PackedByteArray();
        }

        // Create file_storage
        libtorrent::file_storage fs;

        if (dir.is_valid()) {
            // Directory mode - add all files recursively
            libtorrent::add_files(fs, path_str);
        } else {
            // Single file mode
            int64_t file_size = file->get_length();
            file.unref();
            fs.add_file(path_str, file_size);
        }

        // Create the torrent with piece size (0 = automatic)
        libtorrent::create_torrent ct(fs, piece_size > 0 ? piece_size : 0, flags);

        // Set metadata
        if (!comment.is_empty()) {
            ct.set_comment(comment.utf8().get_data());
        }
        if (!creator.is_empty()) {
            ct.set_creator(creator.utf8().get_data());
        } else {
            ct.set_creator("godot-torrent");
        }

        // Generate piece hashes
        libtorrent::set_piece_hashes(ct, path.get_base_dir().utf8().get_data());

        // Generate the torrent file
        libtorrent::entry e = ct.generate();

        // Bencode to buffer
        std::vector<char> buffer;
        libtorrent::bencode(std::back_inserter(buffer), e);

        // Convert to PackedByteArray
        PackedByteArray result;
        result.resize(buffer.size());
        std::memcpy(result.ptrw(), buffer.data(), buffer.size());

        if (_logger.is_valid()) {
            _logger->log_info("Created torrent from: " + path + " (" + String::num(buffer.size()) + " bytes)", "SESSION");
        }

        return result;

    } catch (const std::exception& e) {
        report_error("create_torrent_from_path", String("Exception: ") + e.what());
        return PackedByteArray();
    }
}

// DHT Mutable Item Operations
void TorrentSession::dht_put_mutable_item(Ref<TorrentKeyPair> keypair, Dictionary value, String salt) {
    if (!_session) {
        report_error("dht_put_mutable_item", "Session not running");
        return;
    }

    if (!keypair.is_valid() || !keypair->can_sign()) {
        report_error("dht_put_mutable_item", "Invalid keypair or cannot sign");
        return;
    }

    try {
        // Convert Dictionary to libtorrent::entry
        libtorrent::entry item;
        for (int i = 0; i < value.size(); i++) {
            auto keys = value.keys();
            Variant key = keys[i];
            Variant val = value[key];

            String key_str = key;
            if (val.get_type() == Variant::STRING) {
                item[key_str.utf8().get_data()] = String(val).utf8().get_data();
            } else if (val.get_type() == Variant::INT) {
                item[key_str.utf8().get_data()] = (int64_t)val;
            }
        }

        // Get the public key
        auto public_key_array = keypair->get_lt_public_key();
        std::array<char, 32> public_key;
        std::memcpy(public_key.data(), public_key_array.data(), 32);

        // Get current sequence number for this keypair
        std::string pk_str(public_key.begin(), public_key.end());
        int64_t seq = 1;
        if (_mutable_torrents.find(pk_str) != _mutable_torrents.end()) {
            seq = _mutable_torrents[pk_str].sequence_number + 1;
        }

        // Create the put callback
        auto cb = [keypair, item, seq](libtorrent::entry& e, std::array<char, 64>& sig,
                                        std::int64_t& sequence, std::string const& s) mutable {
            e = item;
            sequence = seq;

            // Sign the data
            std::vector<char> buf;
            libtorrent::bencode(std::back_inserter(buf), e);

            // Add sequence and salt to signature data
            buf.insert(buf.end(), reinterpret_cast<char*>(&sequence), reinterpret_cast<char*>(&sequence) + 8);
            if (!s.empty()) {
                buf.insert(buf.end(), s.begin(), s.end());
            }

            PackedByteArray data;
            data.resize(buf.size());
            std::memcpy(data.ptrw(), buf.data(), buf.size());

            PackedByteArray signature = keypair->sign(data);
            std::memcpy(sig.data(), signature.ptr(), 64);
        };

        // Put the mutable item
        _session->dht_put_item(public_key, cb, salt.utf8().get_data());

        // Update tracking
        MutableTorrentInfo info;
        info.keypair = keypair;
        info.public_key = keypair->get_public_key();
        info.sequence_number = seq;
        info.is_publisher = true;
        info.auto_update_enabled = false;
        info.save_path = "";
        _mutable_torrents[pk_str] = info;

        if (_logger.is_valid()) {
            _logger->log_info("DHT put mutable item with sequence: " + String::num(seq), "SESSION");
        }

    } catch (const std::exception& e) {
        report_error("dht_put_mutable_item", String("Exception: ") + e.what());
    }
}

void TorrentSession::dht_get_mutable_item(PackedByteArray public_key, String salt) {
    if (!_session) {
        report_error("dht_get_mutable_item", "Session not running");
        return;
    }

    if (public_key.size() != 32) {
        report_error("dht_get_mutable_item", "Public key must be 32 bytes");
        return;
    }

    try {
        // Convert to libtorrent public key
        std::array<char, 32> pk;
        std::memcpy(pk.data(), public_key.ptr(), 32);

        // Get the mutable item
        _session->dht_get_item(pk, salt.utf8().get_data());

        if (_logger.is_valid()) {
            _logger->log_info("DHT get mutable item requested", "SESSION");
        }

        // Result will arrive via dht_mutable_item_alert in get_alerts()

    } catch (const std::exception& e) {
        report_error("dht_get_mutable_item", String("Exception: ") + e.what());
    }
}

// Mutable Torrent Operations
Ref<TorrentHandle> TorrentSession::add_mutable_torrent(Ref<TorrentKeyPair> keypair, String save_path, PackedByteArray initial_torrent_data) {
    if (!_session) {
        report_error("add_mutable_torrent", "Session not running");
        return Ref<TorrentHandle>();
    }

    if (!keypair.is_valid() || !keypair->can_sign()) {
        report_error("add_mutable_torrent", "Invalid keypair or cannot sign");
        return Ref<TorrentHandle>();
    }

    if (save_path.is_empty()) {
        report_error("add_mutable_torrent", "Save path cannot be empty");
        return Ref<TorrentHandle>();
    }

    try {
        // If initial torrent data provided, add it first
        Ref<TorrentHandle> handle;
        if (initial_torrent_data.size() > 0) {
            handle = add_torrent_file(initial_torrent_data, save_path);
            if (!handle.is_valid()) {
                return Ref<TorrentHandle>();
            }
        }

        // Store mutable torrent info
        auto public_key_array = keypair->get_lt_public_key();
        std::string pk_str(public_key_array.begin(), public_key_array.end());

        MutableTorrentInfo info;
        info.keypair = keypair;
        info.public_key = keypair->get_public_key();
        info.sequence_number = 1;
        info.is_publisher = true;
        info.auto_update_enabled = false;
        info.save_path = save_path;
        _mutable_torrents[pk_str] = info;

        if (_logger.is_valid()) {
            _logger->log_info("Added mutable torrent (publisher mode)", "SESSION");
        }

        return handle;

    } catch (const std::exception& e) {
        report_error("add_mutable_torrent", String("Exception: ") + e.what());
        return Ref<TorrentHandle>();
    }
}

Ref<TorrentHandle> TorrentSession::subscribe_mutable_torrent(PackedByteArray public_key, String save_path) {
    if (!_session) {
        report_error("subscribe_mutable_torrent", "Session not running");
        return Ref<TorrentHandle>();
    }

    if (public_key.size() != 32) {
        report_error("subscribe_mutable_torrent", "Public key must be 32 bytes");
        return Ref<TorrentHandle>();
    }

    if (save_path.is_empty()) {
        report_error("subscribe_mutable_torrent", "Save path cannot be empty");
        return Ref<TorrentHandle>();
    }

    try {
        // Store subscription info
        std::string pk_str(reinterpret_cast<const char*>(public_key.ptr()), 32);

        MutableTorrentInfo info;
        info.keypair = Ref<TorrentKeyPair>();  // No keypair for subscribers
        info.public_key = public_key;
        info.sequence_number = 0;
        info.is_publisher = false;
        info.auto_update_enabled = true;  // Enable by default for subscribers
        info.save_path = save_path;
        _mutable_torrents[pk_str] = info;

        // Query DHT for current version
        dht_get_mutable_item(public_key, "");

        if (_logger.is_valid()) {
            _logger->log_info("Subscribed to mutable torrent (consumer mode)", "SESSION");
        }

        // The actual torrent will be added when we receive the DHT response
        // Return null for now - user should listen for alerts
        return Ref<TorrentHandle>();

    } catch (const std::exception& e) {
        report_error("subscribe_mutable_torrent", String("Exception: ") + e.what());
        return Ref<TorrentHandle>();
    }
}

bool TorrentSession::publish_mutable_torrent_update(PackedByteArray public_key, PackedByteArray new_torrent_data) {
    if (!_session) {
        report_error("publish_mutable_torrent_update", "Session not running");
        return false;
    }

    if (public_key.size() != 32) {
        report_error("publish_mutable_torrent_update", "Public key must be 32 bytes");
        return false;
    }

    if (new_torrent_data.size() == 0) {
        report_error("publish_mutable_torrent_update", "Empty torrent data");
        return false;
    }

    try {
        // Find the mutable torrent info by public key
        std::string pk_str(reinterpret_cast<const char*>(public_key.ptr()), 32);
        auto it = _mutable_torrents.find(pk_str);

        if (it == _mutable_torrents.end()) {
            report_error("publish_mutable_torrent_update", "Mutable torrent not found. Use add_mutable_torrent first.");
            return false;
        }

        MutableTorrentInfo& info = it->second;

        if (!info.is_publisher) {
            report_error("publish_mutable_torrent_update", "Not the publisher of this mutable torrent");
            return false;
        }

        if (!info.keypair.is_valid() || !info.keypair->can_sign()) {
            report_error("publish_mutable_torrent_update", "Invalid keypair");
            return false;
        }

        // Parse new torrent to extract info-hash
        const char* data_ptr = reinterpret_cast<const char*>(new_torrent_data.ptr());
        libtorrent::error_code ec;
        auto torrent_info = std::make_shared<libtorrent::torrent_info>(
            data_ptr,
            new_torrent_data.size(),
            ec
        );

        if (ec) {
            report_error("publish_mutable_torrent_update", String("Failed to parse torrent: ") + ec.message().c_str());
            return false;
        }

        // Extract info-hash
        std::string info_hash_hex = libtorrent::aux::to_hex(torrent_info->info_hashes().v1);

        // Create DHT value with new info-hash
        Dictionary value;
        value["ih"] = String(info_hash_hex.c_str());
        value["v"] = 1; // BEP 46 version field

        // Publish the update using dht_put_mutable_item
        dht_put_mutable_item(info.keypair, value, "");

        // Update sequence number in tracking
        info.sequence_number++;

        if (_logger.is_valid()) {
            _logger->log_info("Published mutable torrent update, sequence: " + String::num(info.sequence_number), "SESSION");
        }

        return true;

    } catch (const std::exception& e) {
        report_error("publish_mutable_torrent_update", String("Exception: ") + e.what());
        return false;
    }
}

void TorrentSession::check_mutable_torrent_for_updates(PackedByteArray public_key) {
    if (!_session) {
        report_error("check_mutable_torrent_for_updates", "Session not running");
        return;
    }

    if (public_key.size() != 32) {
        report_error("check_mutable_torrent_for_updates", "Public key must be 32 bytes");
        return;
    }

    // Simply query the DHT for the current value
    dht_get_mutable_item(public_key, "");

    if (_logger.is_valid()) {
        _logger->log_info("Querying DHT for mutable torrent updates", "SESSION");
    }
}

// Helper methods
Dictionary TorrentSession::entry_to_dictionary(const void* entry_ptr) {
    Dictionary dict;
    // This is a simplified conversion - full implementation would handle all entry types
    return dict;
}

void* TorrentSession::dictionary_to_entry(Dictionary dict) {
    // This is a placeholder - full implementation would convert Dictionary to libtorrent::entry
    return nullptr;
}

void TorrentSession::check_mutable_torrent_updates() {
    if (!_session) {
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - _last_update_check).count();

    if (elapsed < _update_check_interval_seconds) {
        return;
    }

    // Check all mutable torrents with auto_update enabled
    for (auto& [pk_str, info] : _mutable_torrents) {
        if (!info.is_publisher && info.auto_update_enabled) {
            // Query DHT for updates
            dht_get_mutable_item(info.public_key, "");
        }
    }

    _last_update_check = now;

    if (_logger.is_valid() && !_mutable_torrents.empty()) {
        _logger->log_info("Checked for mutable torrent updates", "SESSION");
    }
}
