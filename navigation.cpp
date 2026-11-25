#include "navigation.h"
#include "uav.h"

// ---------- GPSNavigation ----------
GPSNavigation::GPSNavigation(const UAV* vehicle) : vehicle(vehicle) {}

Position GPSNavigation::get_position() const {
    return { 55.75, 37.61, vehicle ? vehicle->altitude() : -1.0 };
}
bool GPSNavigation::is_valid() const { return satellites >= 4 && valid; }
QString GPSNavigation::system_name() const { return "GPS"; }
void GPSNavigation::set_valid(bool value) { valid = value; }
void GPSNavigation::set_satellites(int count) { satellites = count; }

// ---------- InertialNavigation ----------
InertialNavigation::InertialNavigation(const UAV* vehicle) : vehicle(vehicle) {}

Position InertialNavigation::get_position() const {
    return { 55.75, 37.61, vehicle ? vehicle->altitude() + drift : -1.0 };
}
bool InertialNavigation::is_valid() const { return vehicle != nullptr; }
QString InertialNavigation::system_name() const { return "INS"; }

// ---------- VisualNavigation ----------
VisualNavigation::VisualNavigation(const UAV* vehicle) : vehicle(vehicle) {}

Position VisualNavigation::get_position() const {
    return { 55.75, 37.61, vehicle ? vehicle->altitude() : -1.0 };
}
bool VisualNavigation::is_valid() const { return target_tracked; }
QString VisualNavigation::system_name() const { return "Visual"; }
void VisualNavigation::set_target_tracked(bool value) { target_tracked = value; }

// ---------- NavigationManager ----------
NavigationManager::NavigationManager(NavigationSystem* gps,
                                     NavigationSystem* inertial,
                                     NavigationSystem* visual,
                                     QObject* parent)
    : QObject(parent), gps(gps), inertial(inertial), visual(visual) {
}

void NavigationManager::update() {
    NavigationSystem* selected = nullptr;
    for (NavigationSystem* candidate : { gps, inertial, visual }) {
        if (candidate != nullptr && candidate->is_valid()) {
            selected = candidate;
            break;
        }
    }
    if (selected != active_system) {
        active_system = selected;
        emit active_system_changed(selected ? selected->system_name() : QString("none"));
    }
}

Position NavigationManager::get_position() const {
    return active_system ? active_system->get_position() : Position{ 0, 0, -1.0 };
}
bool NavigationManager::is_valid() const {
    return active_system != nullptr && active_system->is_valid();
}
QString NavigationManager::system_name() const {
    return active_system ? active_system->system_name() : QString("none");
}
