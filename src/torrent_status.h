#ifndef TORRENT_STATUS_H
#define TORRENT_STATUS_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

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
    
    // Rate information
    int get_download_rate() const;
    int get_upload_rate() const;
    int get_download_payload_rate() const;
    int get_upload_payload_rate() const;
    
    // Peer information
    int get_num_peers() const;
    int get_num_seeds() const;
    int get_num_connections() const;
    int get_connections_limit() const;
    
    // Time information
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

    // STUB: Internal methods for future libtorrent integration
    void _set_internal_status(const void* status);

private:
    bool _valid;
    int _state;
    bool _paused;
    float _progress;
};

#endif // TORRENT_STATUS_H