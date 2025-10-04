#include "torrent_info.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/announce_entry.hpp>
#include <sstream>
#include <iomanip>

using namespace godot;

void TorrentInfo::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_name"), &TorrentInfo::get_name);
    ClassDB::bind_method(D_METHOD("get_total_size"), &TorrentInfo::get_total_size);
    ClassDB::bind_method(D_METHOD("get_comment"), &TorrentInfo::get_comment);
    ClassDB::bind_method(D_METHOD("get_creator"), &TorrentInfo::get_creator);
    ClassDB::bind_method(D_METHOD("get_creation_date"), &TorrentInfo::get_creation_date);
    ClassDB::bind_method(D_METHOD("get_info_hash"), &TorrentInfo::get_info_hash);
    ClassDB::bind_method(D_METHOD("get_info_hash_v2"), &TorrentInfo::get_info_hash_v2);
    
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

TorrentInfo::TorrentInfo() : _torrent_info(nullptr) {
}

TorrentInfo::~TorrentInfo() {
}

String TorrentInfo::get_name() const {
    if (!_torrent_info) {
        return "";
    }
    return String(_torrent_info->name().c_str());
}

int64_t TorrentInfo::get_total_size() const {
    if (!_torrent_info) {
        return 0;
    }
    return _torrent_info->total_size();
}

String TorrentInfo::get_comment() const {
    if (!_torrent_info) {
        return "";
    }
    return String(_torrent_info->comment().c_str());
}

String TorrentInfo::get_creator() const {
    if (!_torrent_info) {
        return "";
    }
    return String(_torrent_info->creator().c_str());
}

int64_t TorrentInfo::get_creation_date() const {
    if (!_torrent_info) {
        return 0;
    }
    return _torrent_info->creation_date();
}

String TorrentInfo::get_info_hash() const {
    if (!_torrent_info) {
        return "";
    }

    libtorrent::sha1_hash hash = _torrent_info->info_hash();
    std::stringstream ss;
    ss << hash;
    return String(ss.str().c_str());
}

String TorrentInfo::get_info_hash_v2() const {
    if (!_torrent_info) {
        return "";
    }

    // v2 torrents are not supported in older libtorrent versions
    // This feature requires libtorrent 2.0+
    return "";
}

int TorrentInfo::get_file_count() const {
    if (!_torrent_info) {
        return 0;
    }
    return _torrent_info->num_files();
}

Dictionary TorrentInfo::get_file_at(int index) const {
    Dictionary file_info;

    if (!_torrent_info || index < 0 || index >= _torrent_info->num_files()) {
        return file_info;
    }

    libtorrent::file_storage const& fs = _torrent_info->files();

    file_info["path"] = String(fs.file_path(libtorrent::file_index_t(index)).c_str());
    file_info["size"] = fs.file_size(libtorrent::file_index_t(index));
    file_info["offset"] = fs.file_offset(libtorrent::file_index_t(index));
    file_info["pad_file"] = fs.pad_file_at(libtorrent::file_index_t(index));

    // Extract file flags
    auto flags = fs.file_flags(libtorrent::file_index_t(index));
    file_info["hidden"] = static_cast<bool>(flags & libtorrent::file_storage::flag_hidden);
    file_info["executable"] = static_cast<bool>(flags & libtorrent::file_storage::flag_executable);
    file_info["symlink"] = static_cast<bool>(flags & libtorrent::file_storage::flag_symlink);

    return file_info;
}

String TorrentInfo::get_file_path_at(int index) const {
    if (!_torrent_info || index < 0 || index >= _torrent_info->num_files()) {
        return "";
    }

    libtorrent::file_storage const& fs = _torrent_info->files();
    return String(fs.file_path(libtorrent::file_index_t(index)).c_str());
}

int64_t TorrentInfo::get_file_size_at(int index) const {
    if (!_torrent_info || index < 0 || index >= _torrent_info->num_files()) {
        return 0;
    }

    libtorrent::file_storage const& fs = _torrent_info->files();
    return fs.file_size(libtorrent::file_index_t(index));
}

Array TorrentInfo::get_files() const {
    Array files;

    if (!_torrent_info) {
        return files;
    }

    int num_files = _torrent_info->num_files();
    for (int i = 0; i < num_files; i++) {
        files.append(get_file_at(i));
    }

    return files;
}

int TorrentInfo::get_piece_count() const {
    if (!_torrent_info) {
        return 0;
    }
    return _torrent_info->num_pieces();
}

int TorrentInfo::get_piece_size() const {
    if (!_torrent_info) {
        return 0;
    }
    return _torrent_info->piece_length();
}

int TorrentInfo::get_piece_size_at(int index) const {
    if (!_torrent_info || index < 0 || index >= _torrent_info->num_pieces()) {
        return 0;
    }

    // The last piece might be smaller
    int piece_size = _torrent_info->piece_size(libtorrent::piece_index_t(index));
    return piece_size;
}

Array TorrentInfo::get_trackers() const {
    Array trackers;

    if (!_torrent_info) {
        return trackers;
    }

    std::vector<libtorrent::announce_entry> const& announce_list = _torrent_info->trackers();
    for (auto const& entry : announce_list) {
        Dictionary tracker;
        tracker["url"] = String(entry.url.c_str());
        tracker["tier"] = entry.tier;
        tracker["fail_limit"] = entry.fail_limit;
        tracker["source"] = entry.source;
        trackers.append(tracker);
    }

    return trackers;
}

Array TorrentInfo::get_web_seeds() const {
    Array web_seeds;

    if (!_torrent_info) {
        return web_seeds;
    }

    std::vector<libtorrent::web_seed_entry> const& seeds = _torrent_info->web_seeds();
    for (auto const& seed : seeds) {
        Dictionary web_seed;
        web_seed["url"] = String(seed.url.c_str());
        web_seed["type"] = seed.type;  // 0 = BEP 17 (url-seed), 1 = BEP 19 (http-seed)
        web_seeds.append(web_seed);
    }

    return web_seeds;
}

bool TorrentInfo::is_valid() const {
    return _torrent_info && _torrent_info->is_valid();
}

bool TorrentInfo::is_private() const {
    if (!_torrent_info) {
        return false;
    }
    return _torrent_info->priv();
}

void TorrentInfo::_set_internal_info(std::shared_ptr<libtorrent::torrent_info> info) {
    _torrent_info = info;
}

std::shared_ptr<libtorrent::torrent_info> TorrentInfo::_get_internal_info() const {
    return _torrent_info;
}