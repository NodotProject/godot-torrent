#ifndef TORRENT_SESSION_H
#define TORRENT_SESSION_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <memory>
#include <map>

using namespace godot;

class TorrentHandle;
class TorrentLogger;

// Forward declaration not sufficient since we use Ref<TorrentKeyPair> in member struct
// Need complete type definition
#include "torrent_key_pair.h"

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

    // Connection management
    void set_max_connections(int limit);
    void set_max_uploads(int limit);
    void set_max_half_open_connections(int limit);

    // Protocol encryption
    void set_encryption_policy(int policy); // 0=disabled, 1=enabled, 2=forced
    void set_prefer_encrypted(bool prefer);

    // DHT management
    bool is_dht_running();
    void start_dht();
    void stop_dht();
    Dictionary get_dht_state();
    void set_dht_bootstrap_nodes(Array nodes);
    void add_dht_node(String host, int port);
    PackedByteArray save_dht_state();
    bool load_dht_state(PackedByteArray dht_data);

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
    Ref<TorrentHandle> add_torrent_file_with_resume(PackedByteArray torrent_data, String save_path, PackedByteArray resume_data);
    Ref<TorrentHandle> add_magnet_uri(String magnet_uri, String save_path);
    Ref<TorrentHandle> add_magnet_uri_with_resume(String magnet_uri, String save_path, PackedByteArray resume_data);
    bool remove_torrent(Ref<TorrentHandle> handle, bool delete_files = false);

    // Torrent creation
    PackedByteArray create_torrent_from_path(String path, int piece_size = 0, int flags = 0, String comment = "", String creator = "");

    // DHT mutable item operations (for mutable torrents - BEP 46)
    void dht_put_mutable_item(Ref<TorrentKeyPair> keypair, Dictionary value, String salt = "");
    void dht_get_mutable_item(PackedByteArray public_key, String salt = "");

    // Mutable torrent operations
    Ref<TorrentHandle> add_mutable_torrent(Ref<TorrentKeyPair> keypair, String save_path, PackedByteArray initial_torrent_data = PackedByteArray());
    Ref<TorrentHandle> subscribe_mutable_torrent(PackedByteArray public_key, String save_path);
    bool publish_mutable_torrent_update(PackedByteArray public_key, PackedByteArray new_torrent_data);
    void check_mutable_torrent_for_updates(PackedByteArray public_key);

    // Statistics and monitoring
    Dictionary get_session_stats();

    // Alert system
    Array get_alerts();
    void clear_alerts();
    void post_torrent_updates();

    // Session state persistence
    PackedByteArray save_state();
    bool load_state(PackedByteArray state_data);

    // IP filtering
    void set_ip_filter_enabled(bool enabled);
    void add_ip_filter_rule(String ip_range, bool blocked);
    void clear_ip_filter();

    // Disk cache configuration
    void set_cache_size(int size_mb);
    void set_cache_expiry(int seconds);

    // Logging
    void set_logger(Ref<TorrentLogger> logger);
    Ref<TorrentLogger> get_logger() const;
    void enable_logging(bool enabled);
    void set_log_level(int level);

private:
    // The actual libtorrent session
    libtorrent::session* _session;

    // Logger
    Ref<TorrentLogger> _logger;

    // Mutable torrent tracking
    struct MutableTorrentInfo {
        Ref<TorrentKeyPair> keypair;
        PackedByteArray public_key;
        int64_t sequence_number;
        bool is_publisher;
        bool auto_update_enabled;
        String save_path;
    };
    std::map<std::string, MutableTorrentInfo> _mutable_torrents;

    // Update checking
    std::chrono::steady_clock::time_point _last_update_check;
    int _update_check_interval_seconds;

    // Helper to convert Dictionary to libtorrent settings_pack
    void apply_dictionary_settings(Dictionary settings);

    // Helpers for mutable torrents
    Dictionary entry_to_dictionary(const void* entry_ptr);
    void* dictionary_to_entry(Dictionary dict);
    void check_mutable_torrent_updates();

    // Error handling helpers
    void report_error(const String& operation, const String& message);
    void report_libtorrent_error(const String& operation, int error_code, const String& error_message);
};

#endif // TORRENT_SESSION_H
