#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include <vector>
#include <memory>

using namespace godot;

class AlertManager : public RefCounted {
    GDCLASS(AlertManager, RefCounted)

protected:
    static void _bind_methods();

public:
    AlertManager();
    ~AlertManager();
    
    // Alert processing (STUB: will use libtorrent alerts when implemented)
    void process_alerts(const std::vector<void*>& alerts);
    Array get_alerts();
    Array get_alerts_by_type(int alert_type);
    void clear_alerts();
    
    // Alert filtering
    void set_alert_mask(int mask);
    int get_alert_mask() const;
    
    // Alert categories (convenience methods)
    void enable_error_alerts(bool enabled);
    void enable_status_alerts(bool enabled);
    void enable_progress_alerts(bool enabled);
    void enable_peer_alerts(bool enabled);
    void enable_storage_alerts(bool enabled);
    void enable_tracker_alerts(bool enabled);
    void enable_dht_alerts(bool enabled);
    
    // Signal-like functionality
    Array get_torrent_alerts() const;
    Array get_peer_alerts() const;
    Array get_tracker_alerts() const;
    Array get_error_alerts() const;

private:
    Array _alerts;
    int _alert_mask;
    
    Dictionary _convert_alert_to_dictionary(void* alert);
    String _get_alert_type_name(int type);
};

#endif // ALERT_MANAGER_H