#ifndef TORRENT_RESULT_H
#define TORRENT_RESULT_H

#include "torrent_error.h"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

/**
 * TorrentResult - Result wrapper for operations that can fail
 *
 * Provides a standardized way to return either:
 * - A successful result with optional value
 * - An error with detailed information
 *
 * Usage in GDScript:
 *   var result = session.add_torrent_file(data, path)
 *   if result.is_ok():
 *       var handle = result.get_value()
 *   else:
 *       var error = result.get_error()
 *       print("Error: ", error.get_message())
 */
class TorrentResult : public RefCounted {
    GDCLASS(TorrentResult, RefCounted)

protected:
    static void _bind_methods();

public:
    TorrentResult();
    ~TorrentResult();

    // Create success result
    static Ref<TorrentResult> ok(const Variant& value = Variant());

    // Create error result
    static Ref<TorrentResult> error(Ref<TorrentError> error);
    static Ref<TorrentResult> error_code(TorrentError::Code code, const String& message = "", const String& context = "");

    // Status checking
    bool is_ok() const;
    bool is_error() const;

    // Value retrieval
    Variant get_value() const;
    Ref<TorrentError> get_error() const;

    // Convenience methods
    Dictionary to_dict() const;
    String to_string() const;

    // Unwrap (returns value or throws error to console)
    Variant unwrap();
    Variant unwrap_or(const Variant& default_value);

private:
    bool _ok;
    Variant _value;
    Ref<TorrentError> _error;
};

#endif // TORRENT_RESULT_H
