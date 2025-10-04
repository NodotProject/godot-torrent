#ifndef TORRENT_ERROR_H
#define TORRENT_ERROR_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

/**
 * TorrentError - Standardized error reporting for godot-torrent
 *
 * Provides consistent error handling across all torrent operations with:
 * - Error codes and categories
 * - Human-readable error messages
 * - Mapping from libtorrent error_code to GDScript format
 * - Recovery information
 */
class TorrentError : public RefCounted {
    GDCLASS(TorrentError, RefCounted)

public:
    // Error Categories
    enum Category {
        NONE = 0,
        SESSION_ERROR = 1,
        TORRENT_ERROR = 2,
        NETWORK_ERROR = 3,
        STORAGE_ERROR = 4,
        PARSE_ERROR = 5,
        VALIDATION_ERROR = 6,
        TRACKER_ERROR = 7,
        DHT_ERROR = 8,
        PEER_ERROR = 9,
        INTERNAL_ERROR = 10
    };

    // Specific Error Codes
    enum Code {
        // Success
        OK = 0,

        // Session errors (100-199)
        SESSION_NOT_RUNNING = 100,
        SESSION_ALREADY_RUNNING = 101,
        SESSION_START_FAILED = 102,
        SESSION_STOP_FAILED = 103,

        // Torrent errors (200-299)
        INVALID_TORRENT_HANDLE = 200,
        INVALID_TORRENT_FILE = 201,
        INVALID_MAGNET_URI = 202,
        TORRENT_ADD_FAILED = 203,
        TORRENT_REMOVE_FAILED = 204,
        TORRENT_NOT_FOUND = 205,

        // Network errors (300-399)
        NETWORK_INIT_FAILED = 300,
        PORT_BINDING_FAILED = 301,
        CONNECTION_FAILED = 302,
        TIMEOUT = 303,

        // Storage errors (400-499)
        INVALID_PATH = 400,
        PATH_NOT_FOUND = 401,
        PERMISSION_DENIED = 402,
        DISK_FULL = 403,
        STORAGE_MOVE_FAILED = 404,
        FILE_RENAME_FAILED = 405,

        // Parse errors (500-599)
        BENCODE_PARSE_ERROR = 500,
        TORRENT_INFO_PARSE_ERROR = 501,
        RESUME_DATA_PARSE_ERROR = 502,
        DHT_STATE_PARSE_ERROR = 503,

        // Validation errors (600-699)
        INVALID_PARAMETER = 600,
        EMPTY_SAVE_PATH = 601,
        INVALID_PIECE_INDEX = 602,
        INVALID_FILE_INDEX = 603,
        INVALID_PRIORITY = 604,
        INVALID_URL = 605,

        // Tracker errors (700-799)
        TRACKER_ANNOUNCE_FAILED = 700,
        TRACKER_SCRAPE_FAILED = 701,
        TRACKER_INVALID_RESPONSE = 702,

        // DHT errors (800-899)
        DHT_START_FAILED = 800,
        DHT_BOOTSTRAP_FAILED = 801,

        // Peer errors (900-999)
        PEER_CONNECTION_FAILED = 900,
        PEER_BANNED = 901,

        // Internal errors (1000+)
        INTERNAL_EXCEPTION = 1000,
        UNKNOWN_ERROR = 1001
    };

protected:
    static void _bind_methods();

public:
    TorrentError();
    ~TorrentError();

    // Create error from code
    static Ref<TorrentError> create(Code code, const String& message = "", const String& context = "");

    // Create error from libtorrent error_code
    static Ref<TorrentError> from_libtorrent_error(int ec_value, const String& ec_message, const String& context = "");

    // Getters
    int get_code() const;
    int get_category() const;
    String get_message() const;
    String get_context() const;
    bool is_error() const;
    bool is_recoverable() const;

    // Convert to Dictionary for GDScript
    Dictionary to_dict() const;

    // Static helper to get category from code
    static Category get_category_for_code(Code code);
    static String get_category_name(Category category);
    static String get_default_message(Code code);

private:
    Code _code;
    Category _category;
    String _message;
    String _context;
    bool _recoverable;

    void _init_error(Code code, const String& message, const String& context);
};

// Enum registration for GDScript
VARIANT_ENUM_CAST(TorrentError::Category);
VARIANT_ENUM_CAST(TorrentError::Code);

#endif // TORRENT_ERROR_H
