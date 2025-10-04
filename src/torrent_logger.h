#ifndef TORRENT_LOGGER_H
#define TORRENT_LOGGER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <memory>
#include <mutex>

using namespace godot;

/**
 * TorrentLogger - Centralized logging for godot-torrent
 *
 * Integrates with libtorrent logging and provides:
 * - Log level filtering (error, warning, info, debug, trace)
 * - Category filtering (session, torrent, peer, tracker, dht, etc.)
 * - Forward logs to Godot console
 * - Optional file output
 * - Performance-conscious (minimal overhead when disabled)
 */
class TorrentLogger : public RefCounted {
    GDCLASS(TorrentLogger, RefCounted)

public:
    // Log levels (matching typical log severity levels)
    enum LogLevel {
        NONE = 0,     // Logging disabled
        ERROR = 1,    // Errors only
        WARNING = 2,  // Warnings and errors
        INFO = 3,     // Info, warnings, and errors
        DEBUG = 4,    // Debug info + above
        TRACE = 5     // Verbose trace logging + above
    };

    // Log categories for filtering
    enum LogCategory {
        ALL = 0,
        SESSION = 1,
        TORRENT = 2,
        PEER = 3,
        TRACKER = 4,
        DHT = 5,
        PORT_MAPPING = 6,
        STORAGE = 7,
        PERFORMANCE = 8,
        ALERT = 9
    };

protected:
    static void _bind_methods();

public:
    TorrentLogger();
    ~TorrentLogger();

    // Logging control
    void enable_logging(bool enabled);
    bool is_logging_enabled() const;

    void set_log_level(LogLevel level);
    LogLevel get_log_level() const;

    // Category filtering
    void enable_category(LogCategory category, bool enabled);
    bool is_category_enabled(LogCategory category) const;
    void enable_all_categories();
    void disable_all_categories();

    // File logging
    void set_log_file(const String& file_path);
    void close_log_file();
    bool is_log_file_enabled() const;

    // Logging methods
    void log_error(const String& message, const String& category = "");
    void log_warning(const String& message, const String& category = "");
    void log_info(const String& message, const String& category = "");
    void log_debug(const String& message, const String& category = "");
    void log_trace(const String& message, const String& category = "");

    // Log a message with explicit level
    void log(LogLevel level, const String& message, const String& category = "");

    // Internal: Process libtorrent alert for logging
    void process_libtorrent_alert(int alert_type, const String& alert_message);

    // Statistics
    Dictionary get_log_stats() const;
    void reset_log_stats();

private:
    bool _enabled;
    LogLevel _log_level;
    bool _category_filters[10]; // One for each LogCategory

    // File logging
    Ref<FileAccess> _log_file;
    String _log_file_path;
    bool _file_logging_enabled;

    // Statistics
    int _log_count_error;
    int _log_count_warning;
    int _log_count_info;
    int _log_count_debug;
    int _log_count_trace;

    // Thread safety for file writes
    mutable std::mutex _log_mutex;

    // Helper methods
    bool should_log(LogLevel level, const String& category) const;
    String format_log_message(LogLevel level, const String& message, const String& category) const;
    String get_level_name(LogLevel level) const;
    String get_timestamp() const;
    void write_to_console(LogLevel level, const String& formatted_message);
    void write_to_file(const String& formatted_message);

    // Map libtorrent alert types to log levels and categories
    LogLevel get_alert_log_level(int alert_type) const;
    String get_alert_category(int alert_type) const;
};

// Enum registration for GDScript
VARIANT_ENUM_CAST(TorrentLogger::LogLevel);
VARIANT_ENUM_CAST(TorrentLogger::LogCategory);

#endif // TORRENT_LOGGER_H
