#ifndef TORRENT_SESSION_STUB_H
#define TORRENT_SESSION_STUB_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <memory>

using namespace godot;

class TorrentHandle;

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
    bool _session_running;
    
    // Stub methods (will be replaced with real libtorrent functionality)
    void create_default_settings();
    void dictionary_to_settings_pack(Dictionary settings);
};

#endif // TORRENT_SESSION_STUB_H