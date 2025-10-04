#ifndef TORRENT_STATUS_H
#define TORRENT_STATUS_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <memory>
#include <mutex>

using namespace godot;

// Forward declaration for libtorrent torrent_status
namespace libtorrent {
    struct torrent_status;
}

/**
 * TorrentStatus - Real libtorrent::torrent_status integration
 * 
 * This implementation provides real-time torrent status retrieval with
 * actual libtorrent::torrent_status data mapping.
 */
class TorrentStatus : public RefCounted {
    GDCLASS(TorrentStatus, RefCounted)

protected:
    static void _bind_methods();

public:
    TorrentStatus();
    ~TorrentStatus();
    
    // Basic status
    String get_state_string() const;
    int get_state() const;
    bool is_paused() const;
    bool is_finished() const;
    bool is_seeding() const;
    
    // Progress information
    float get_progress() const;
    int64_t get_total_done() const;
    int64_t get_total_size() const;
    int64_t get_total_wanted() const;
    int64_t get_total_wanted_done() const;
    
    // Rate information (bytes per second)
    int get_download_rate() const;
    int get_upload_rate() const;
    int get_download_payload_rate() const;
    int get_upload_payload_rate() const;
    
    // Peer information
    int get_num_peers() const;
    int get_num_seeds() const;
    int get_num_connections() const;
    int get_connections_limit() const;
    
    // Time information (seconds)
    int get_active_time() const;
    int get_seeding_time() const;
    int get_time_since_download() const;
    int get_time_since_upload() const;
    
    // Piece information
    int get_num_pieces() const;
    int get_pieces_downloaded() const;
    
    // Queue information
    int get_queue_position() const;
    
    // Error information
    String get_error() const;
    
    // Additional information
    String get_save_path() const;
    String get_name() const;
    float get_distributed_copies() const;
    
    // Enhanced status information
    int64_t get_all_time_download() const;
    int64_t get_all_time_upload() const;
    float get_availability() const;
    int get_block_size() const;
    int get_list_peers() const;
    int get_list_seeds() const;
    int get_connect_candidates() const;
    int get_downloading_piece_index() const;
    int get_downloading_block_index() const;
    int get_downloading_progress() const;
    int get_downloading_total() const;
    
    // Internal methods for libtorrent integration
    void _set_internal_status(const Variant& status);
    Dictionary get_status_dictionary() const;

private:
    // Status storage (using void* for stub compatibility)
    void* _status_ptr;
    
    // Status state tracking
    bool _is_valid;
    bool _is_stub_mode;
    int64_t _last_update_time;
    
    // Cached status data for performance optimization
    struct CachedStatus {
        String state_string;
        int state;
        bool paused;
        bool finished;
        bool seeding;
        float progress;
        int64_t total_done;
        int64_t total_size;
        int64_t total_wanted;
        int64_t total_wanted_done;
        int download_rate;
        int upload_rate;
        int download_payload_rate;
        int upload_payload_rate;
        int num_peers;
        int num_seeds;
        int num_connections;
        int connections_limit;
        int active_time;
        int seeding_time;
        int time_since_download;
        int time_since_upload;
        int num_pieces;
        int pieces_downloaded;
        int queue_position;
        String error;
        String save_path;
        String name;
        float distributed_copies;
        int64_t all_time_download;
        int64_t all_time_upload;
        float availability;
        int block_size;
        int list_peers;
        int list_seeds;
        int connect_candidates;
        int downloading_piece_index;
        int downloading_block_index;
        int downloading_progress;
        int downloading_total;
    } _cached_status;
    
    // Performance optimization
    static const int64_t CACHE_VALIDITY_MS = 100; // 100ms cache validity
    
    // Thread safety
    mutable std::mutex _status_mutex;
    
    // Status management
    void cleanup_status();
    void detect_build_mode();
    void update_cached_status();
    bool is_cache_valid() const;
    
    // Real status mapping
    void map_libtorrent_status();
    String map_state_to_string(int state) const;
    
    // Stub status simulation
    void create_stub_status();
    
    // Error handling
    void handle_status_error(const std::string& operation, const std::exception& e) const;
    void log_status_operation(const String& operation, bool success = true) const;
    
    // Validation helpers
    bool validate_status() const;
};

#endif // TORRENT_STATUS_H