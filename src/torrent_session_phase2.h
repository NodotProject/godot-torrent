#ifndef TORRENT_SESSION_PHASE2_H
#define TORRENT_SESSION_PHASE2_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <memory>
#include <vector>

using namespace godot;

class TorrentHandle;

// Phase 2: Transitional implementation showing libtorrent integration readiness
// This demonstrates the architectural changes needed for real libtorrent integration
// while maintaining compatibility with existing tests and functionality
class TorrentSession : public RefCounted {
    GDCLASS(TorrentSession, RefCounted)

protected:
    static void _bind_methods();

public:
    TorrentSession();
    ~TorrentSession();
    
    // Basic session management
    bool start_session();
    bool start_session_with_settings(Dictionary settings);
    void stop_session();
    bool is_running() const;
    
    // Configuration
    void set_download_rate_limit(int bytes_per_second);
    void set_upload_rate_limit(int bytes_per_second);
    void set_listen_port(int port);
    void set_listen_port_range(int min_port, int max_port);
    
    // DHT
    bool is_dht_running();
    void start_dht();
    void stop_dht();
    
    // Torrent management
    Ref<TorrentHandle> add_torrent_file(PackedByteArray torrent_data, String save_path);
    Ref<TorrentHandle> add_magnet_uri(String magnet_uri, String save_path);
    bool remove_torrent(Ref<TorrentHandle> handle, bool delete_files = false);
    
    // Statistics
    Dictionary get_session_stats();
    
    // Alert handling
    Array get_alerts();
    void clear_alerts();

private:
    // Phase 2: Enhanced state management ready for libtorrent integration
    bool _session_running;
    bool _dht_running;
    int _download_rate_limit;
    int _upload_rate_limit;
    int _listen_port_min;
    int _listen_port_max;
    
    // Phase 2: Prepare structures for libtorrent integration  
    Dictionary _session_settings;
    Array _pending_alerts;  // Use Godot Array instead of std::vector
    
    // Phase 2: Ready for libtorrent session object
    // std::unique_ptr<libtorrent::session> _lt_session;  // Will be enabled in Phase 3
    
    // Helper methods (Phase 2: enhanced with proper structure)
    Dictionary create_default_settings();
    void apply_settings_dictionary(Dictionary settings);
    void generate_status_alert(const String& message, const String& type = "info");
    void simulate_network_operations();
};

#endif // TORRENT_SESSION_PHASE2_H