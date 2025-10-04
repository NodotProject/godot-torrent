#ifndef PEER_INFO_H
#define PEER_INFO_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <memory>

using namespace godot;

namespace libtorrent {
    struct peer_info;
}

class PeerInfo : public RefCounted {
    GDCLASS(PeerInfo, RefCounted)

protected:
    static void _bind_methods();

public:
    PeerInfo();
    ~PeerInfo();

    // Basic peer information
    String get_ip() const;
    int get_port() const;
    String get_client() const;
    String get_peer_id() const;

    // Connection information
    String get_connection_type() const;
    bool is_seed() const;
    bool is_local() const;

    // Transfer information
    int get_download_rate() const;
    int get_upload_rate() const;
    int64_t get_total_download() const;
    int64_t get_total_upload() const;

    // Progress information
    float get_progress() const;
    int get_pieces_downloaded() const;

    // Timing information
    int get_last_request() const;
    int get_last_active() const;

    // Queue information
    int get_download_queue_length() const;
    int get_upload_queue_length() const;

    // Flags
    bool is_interesting() const;
    bool is_choked() const;
    bool is_remote_interested() const;
    bool is_remote_choked() const;

    // Country and location (if available)
    String get_country() const;

    // Internal methods for libtorrent integration
    void _set_internal_info(std::shared_ptr<libtorrent::peer_info> info);
    Dictionary get_peer_dictionary() const;

private:
    std::shared_ptr<libtorrent::peer_info> _peer_info;

    // Helper methods
    String parse_client_string(const std::string& peer_id) const;
    String connection_type_to_string(int type) const;
};

#endif // PEER_INFO_H