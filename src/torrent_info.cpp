#include "torrent_info.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void TorrentInfo::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_name"), &TorrentInfo::get_name);
    ClassDB::bind_method(D_METHOD("get_total_size"), &TorrentInfo::get_total_size);
    ClassDB::bind_method(D_METHOD("get_comment"), &TorrentInfo::get_comment);
    ClassDB::bind_method(D_METHOD("get_creator"), &TorrentInfo::get_creator);
    ClassDB::bind_method(D_METHOD("get_creation_date"), &TorrentInfo::get_creation_date);
    ClassDB::bind_method(D_METHOD("get_info_hash"), &TorrentInfo::get_info_hash);
    
    ClassDB::bind_method(D_METHOD("get_file_count"), &TorrentInfo::get_file_count);
    ClassDB::bind_method(D_METHOD("get_file_at", "index"), &TorrentInfo::get_file_at);
    ClassDB::bind_method(D_METHOD("get_file_path_at", "index"), &TorrentInfo::get_file_path_at);
    ClassDB::bind_method(D_METHOD("get_file_size_at", "index"), &TorrentInfo::get_file_size_at);
    ClassDB::bind_method(D_METHOD("get_files"), &TorrentInfo::get_files);
    
    ClassDB::bind_method(D_METHOD("get_piece_count"), &TorrentInfo::get_piece_count);
    ClassDB::bind_method(D_METHOD("get_piece_size"), &TorrentInfo::get_piece_size);
    ClassDB::bind_method(D_METHOD("get_piece_size_at", "index"), &TorrentInfo::get_piece_size_at);
    
    ClassDB::bind_method(D_METHOD("get_trackers"), &TorrentInfo::get_trackers);
    ClassDB::bind_method(D_METHOD("get_web_seeds"), &TorrentInfo::get_web_seeds);
    
    ClassDB::bind_method(D_METHOD("is_valid"), &TorrentInfo::is_valid);
    ClassDB::bind_method(D_METHOD("is_private"), &TorrentInfo::is_private);
}

TorrentInfo::TorrentInfo() {
    _valid = false;
}

TorrentInfo::~TorrentInfo() {
}

String TorrentInfo::get_name() const {
    if (!_valid) {
        return "";
    }
    return "STUB: Test Torrent";
}

int64_t TorrentInfo::get_total_size() const {
    if (!_valid) {
        return 0;
    }
    return 1024 * 1024 * 100; // 100MB
}

String TorrentInfo::get_comment() const {
    return "STUB: Test comment";
}

String TorrentInfo::get_creator() const {
    return "STUB: Godot Torrent";
}

int64_t TorrentInfo::get_creation_date() const {
    return 1640995200; // 2022-01-01
}

String TorrentInfo::get_info_hash() const {
    return "STUB: 1234567890abcdef1234567890abcdef12345678";
}

int TorrentInfo::get_file_count() const {
    return 1; // STUB: Single file
}

Dictionary TorrentInfo::get_file_at(int index) const {
    Dictionary file_info;
    if (index == 0) {
        file_info["path"] = "test_file.txt";
        file_info["size"] = get_total_size();
        file_info["offset"] = 0;
    }
    return file_info;
}

String TorrentInfo::get_file_path_at(int index) const {
    if (index == 0) {
        return "test_file.txt";
    }
    return "";
}

int64_t TorrentInfo::get_file_size_at(int index) const {
    if (index == 0) {
        return get_total_size();
    }
    return 0;
}

Array TorrentInfo::get_files() const {
    Array files;
    files.append(get_file_at(0));
    return files;
}

int TorrentInfo::get_piece_count() const {
    return 100; // STUB: 100 pieces
}

int TorrentInfo::get_piece_size() const {
    return 1024 * 1024; // 1MB per piece
}

int TorrentInfo::get_piece_size_at(int index) const {
    return get_piece_size(); // All pieces same size for stub
}

Array TorrentInfo::get_trackers() const {
    Array trackers;
    Dictionary tracker;
    tracker["url"] = "http://tracker.example.com:8080/announce";
    tracker["tier"] = 0;
    tracker["fail_limit"] = 0;
    trackers.append(tracker);
    return trackers;
}

Array TorrentInfo::get_web_seeds() const {
    Array web_seeds;
    return web_seeds; // Empty for stub
}

bool TorrentInfo::is_valid() const {
    return _valid;
}

bool TorrentInfo::is_private() const {
    return false; // STUB: Not private
}

void TorrentInfo::_set_internal_info(void* info) {
    _valid = (info != nullptr);
}