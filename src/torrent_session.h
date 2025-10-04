#ifndef TORRENT_SESSION_H
#define TORRENT_SESSION_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <memory>
#include <mutex>

using namespace godot;

class TorrentHandle;

// Forward declaration for libtorrent session
namespace libtorrent {
    class session;
}

/**
 * TorrentSession - Real libtorrent::session integration
 * 
 * This implementation replaces the Phase 2 stub with actual libtorrent::session
 * functionality. It gracefully handles both real libtorrent and stub mode.
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
    bool test_port_accessibility(int port);
    
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

private:
    // Session object (using void* for stub compatibility)
    void* _session_ptr;
    
    // Session state tracking
    bool _session_running;
    bool _initialization_error;
    bool _is_stub_mode;
    
    // Configuration cache (for stub mode and settings persistence)
    int _download_rate_limit;
    int _upload_rate_limit;
    int _listen_port_min;
    int _listen_port_max;
    bool _dht_enabled;
    bool _ipv6_enabled;
    bool _upnp_enabled;
    bool _natpmp_enabled;
    String _bound_interface;
    Array _dht_bootstrap_nodes;
    
    // Settings management
    Dictionary _current_settings;
    
    // Torrent handle storage
    Array _active_torrents;
    
    // Thread safety
    mutable std::mutex _session_mutex;
    
    // Resource management
    void cleanup_session();
    void initialize_session_resources();
    
    // Settings management (implementation specific)
    void* create_default_settings();
    void* dictionary_to_settings_pack(Dictionary settings);
    void apply_stub_settings(Dictionary settings);
    Dictionary create_stub_default_settings();
    
    // Error handling
    void handle_session_error(const std::string& operation, const std::exception& e);
    void log_session_operation(const String& operation, bool success = true);
    
    // Settings validation
    bool validate_settings_dictionary(const Dictionary& settings);
    
    // Stub mode helpers
    void simulate_session_operation(const String& operation);
    Dictionary create_stub_stats();
    Array create_stub_alerts();
    
    // Alert conversion
    Dictionary convert_alert_to_dictionary(void* alert);
    String get_alert_type_name(int alert_type);
    String get_alert_category_name(int category_mask);
    
    // Mode detection
    void detect_build_mode();
    
    // Torrent management helpers
    bool validate_save_path(const String& path);
    void configure_add_torrent_params(void* params_ptr);
    void add_torrent_to_storage(Ref<TorrentHandle> handle, const void* lt_handle);
    bool remove_torrent_from_storage(Ref<TorrentHandle> handle);
};

#endif // TORRENT_SESSION_H