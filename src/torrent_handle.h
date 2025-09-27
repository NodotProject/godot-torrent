#ifndef TORRENT_HANDLE_H
#define TORRENT_HANDLE_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class TorrentInfo;
class TorrentStatus;
class PeerInfo;

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

    // STUB: Internal methods for future libtorrent integration  
    void _set_internal_handle(const void* handle);
    void* _get_internal_handle() const;

private:
    bool _valid;
    bool _paused; // STUB: track pause state
};

#endif // TORRENT_HANDLE_H