#include "component.h"


void update_vehicles(Component* component, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        VehicleComponent* vehicle = component->vehicle[i];

        if (!vehicle) continue;

        for (int j = 0; j < 10; j++) {
            if (vehicle->lights[j] = -1) continue;

            LightComponent* light = component->light[j];

            if (vehicle->driver == -1) {
                light->brightness = max(0.0, light->brightness - delta_time);
            } else {
                light->brightness = min(0.0, light->brightness - delta_time);
            }
        }
    }
}
