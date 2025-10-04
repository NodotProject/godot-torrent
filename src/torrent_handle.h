#ifndef TORRENT_HANDLE_H
#define TORRENT_HANDLE_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <memory>
#include <mutex>

using namespace godot;

class TorrentInfo;
class TorrentStatus;
class PeerInfo;

// Forward declaration for libtorrent torrent_handle
namespace libtorrent {
    class torrent_handle;
}

/**
 * TorrentHandle - Real libtorrent::torrent_handle integration
 * 
 * This implementation replaces the stub with actual libtorrent::torrent_handle
 * functionality. It gracefully handles both real libtorrent and stub mode.
 */
class TorrentHandle : public RefCounted {
    GDCLASS(TorrentHandle, RefCounted)

protected:
    static void _bind_methods();

public:
    TorrentHandle();
    ~TorrentHandle();
    
    // Basic torrent control
    void pause();
    void resume();
    bool is_paused() const;
    bool is_valid() const;
    
    // Torrent information
    Ref<TorrentInfo> get_torrent_info();
    Ref<TorrentStatus> get_status();
    String get_name() const;
    String get_info_hash() const;
    
    // File and piece management
    void set_piece_priority(int piece_index, int priority);
    int get_piece_priority(int piece_index) const;
    void set_file_priority(int file_index, int priority);
    int get_file_priority(int file_index) const;
    
    // Operations
    void force_recheck();
    void force_reannounce();
    void force_dht_announce();
    void move_storage(String new_path);
    
    // Peer management
    Array get_peer_info();
    
    // Advanced operations
    void scrape_tracker();
    void flush_cache();
    void clear_error();

    // Internal methods for libtorrent integration
    void _set_internal_handle(const Variant& handle);
    Variant _get_internal_handle() const;

private:
    // Handle storage (using void* for stub compatibility)
    void* _handle_ptr;
    
    // Handle state tracking
    bool _is_valid;
    bool _is_stub_mode;
    
    // Stub mode state tracking
    bool _stub_paused;
    String _stub_name;
    String _stub_info_hash;
    
    // Thread safety
    mutable std::mutex _handle_mutex;
    
    // Handle management
    void cleanup_handle();
    void detect_build_mode();
    
    // Error handling
    void handle_operation_error(const std::string& operation, const std::exception& e) const;
    void log_handle_operation(const String& operation, bool success = true) const;
    
    // Stub mode helpers
    void simulate_handle_operation(const String& operation);
    
    // Validation helpers
    bool validate_handle() const;
    bool validate_piece_index(int piece_index) const;
    bool validate_file_index(int file_index) const;
    bool validate_priority(int priority) const;
};

#endif // TORRENT_HANDLE_H