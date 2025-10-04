#ifndef TORRENT_SESSION_H
#define TORRENT_SESSION_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <memory>

using namespace godot;

class TorrentHandle;

namespace libtorrent {
    class session;
}

/**
 * TorrentSession - Pure wrapper around libtorrent::session
 *
 * Direct delegation to libtorrent with minimal Godot integration layer.
 */
class TorrentSession : public RefCounted {
    GDCLASS(TorrentSession, RefCounted)

protected:
    static void _bind_methods();

public:
    TorrentSession();
    ~TorrentSession();

    // Session lifecycle management
    bool start_session();
    bool start_session_with_settings(Dictionary settings);
    void stop_session();
    bool is_running() const;

    // Configuration management
    void set_download_rate_limit(int bytes_per_second);
    void set_upload_rate_limit(int bytes_per_second);
    void set_listen_port(int port);
    void set_listen_port_range(int min_port, int max_port);

    // DHT management
    bool is_dht_running();
    void start_dht();
    void stop_dht();
    Dictionary get_dht_state();
    void set_dht_bootstrap_nodes(Array nodes);

    // Network interface and port management
    bool bind_network_interface(String interface_ip = "");
    Array get_listening_ports();
    Dictionary get_network_status();

    // UPnP/NAT-PMP port mapping
    bool enable_upnp_port_mapping(bool enable = true);
    bool enable_natpmp_port_mapping(bool enable = true);
    Dictionary get_port_mapping_status();

    // IPv6 support
    void enable_ipv6(bool enable = true);
    bool is_ipv6_enabled();

    // Network diagnostics
    Dictionary run_network_diagnostics();
    Array get_network_interfaces();

    // Torrent operations
    Ref<TorrentHandle> add_torrent_file(PackedByteArray torrent_data, String save_path);
    Ref<TorrentHandle> add_magnet_uri(String magnet_uri, String save_path);
    bool remove_torrent(Ref<TorrentHandle> handle, bool delete_files = false);

    // Statistics and monitoring
    Dictionary get_session_stats();

    // Alert system
    Array get_alerts();
    void clear_alerts();
    void post_torrent_updates();

private:
    // The actual libtorrent session
    libtorrent::session* _session;

    // Helper to convert Dictionary to libtorrent settings_pack
    void apply_dictionary_settings(Dictionary settings);
};

#endif // TORRENT_SESSION_H
