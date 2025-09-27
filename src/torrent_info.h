#ifndef TORRENT_INFO_H
#define TORRENT_INFO_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class TorrentInfo : public RefCounted {
    GDCLASS(TorrentInfo, RefCounted)

protected:
    static void _bind_methods();

public:
    TorrentInfo();
    ~TorrentInfo();
    
    // Basic information
    String get_name() const;
    int64_t get_total_size() const;
    String get_comment() const;
    String get_creator() const;
    int64_t get_creation_date() const;
    String get_info_hash() const;
    
    // File information
    int get_file_count() const;
    Dictionary get_file_at(int index) const;
    String get_file_path_at(int index) const;
    int64_t get_file_size_at(int index) const;
    Array get_files() const;
    
    // Piece information
    int get_piece_count() const;
    int get_piece_size() const;
    int get_piece_size_at(int index) const;
    
    // Tracker information
    Array get_trackers() const;
    
    // Web seed information
    Array get_web_seeds() const;
    
    // Validation
    bool is_valid() const;
    bool is_private() const;

    // STUB: Internal methods for future libtorrent integration
    void _set_internal_info(void* info);

private:
    bool _valid;
};

#endif // TORRENT_INFO_H