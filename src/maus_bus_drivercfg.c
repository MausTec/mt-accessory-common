#include "maus_bus_drivercfg.h"
#include <string.h>

#include "esp_log.h"
static const char* TAG = "MB_DRIVER_DEBUG_REMOVE_ME";
static const char* DEFAULT_DISPLAY_NAME = "<untitled>";

void maus_bus_driver_load_match(maus_bus_drivercfg_t* driver, cJSON* match_json) {
    const char* serial_str = NULL;

    cJSON* serial = cJSON_GetObjectItem(match_json, "serial");
    if (serial != NULL) serial_str = serial->valuestring;

    maus_bus_driver_match_t* match = (maus_bus_driver_match_t*)malloc(
        sizeof(maus_bus_driver_match_t) + (serial_str != NULL ? strlen(serial_str) : 0) + 1
    );

    if (match == NULL) return;

    if (serial_str != NULL)
        strcpy(match->serial, serial_str);
    else
        match->serial[0] = '\0';

    cJSON* pid = cJSON_GetObjectItem(match_json, "pid");
    if (pid != NULL) match->pid = pid->valueint;

    cJSON* vid = cJSON_GetObjectItem(match_json, "vid");
    if (vid != NULL) match->vid = vid->valueint;

    driver->match = match;

    ESP_LOGI(
        TAG,
        "-> Define Match: vid=%d, pid=%d, serial=%s",
        driver->match->vid,
        driver->match->pid,
        driver->match->serial
    );
}

void maus_bus_driver_load_actions(maus_bus_driver_actions_t** first, cJSON* actions) {
    maus_bus_driver_actions_t* action = NULL;
    maus_bus_driver_actions_t* head = NULL;
    maus_bus_driver_actions_t* ptr = NULL;

    ESP_LOGI(TAG, "--> Define all Actions.");
    if (actions == NULL || first == NULL) return;

    if (cJSON_IsArray(actions)) {
        cJSON* action_json = NULL;
        cJSON_ArrayForEach(action_json, actions) {
            maus_bus_driver_load_actions(first, action_json);
        }
        return;
    }

    if (cJSON_IsObject(actions)) {
        cJSON* action_json = NULL;
        cJSON_ArrayForEach(action_json, actions) {
            if (action_json->string == NULL) break;

            action = malloc(sizeof(maus_bus_driver_actions_t) + strlen(action_json->string) + 1);
            if (action == NULL) break;

            strcpy(action->function, action_json->string);
            action->args = cJSON_DetachItemFromObject(actions, action_json->string);
            action->next = NULL;

            if (head == NULL)
                head = action;
            else {
                for (ptr = head; ptr->next != NULL; ptr = ptr->next)
                    continue;

                ptr->next = action;
            }
            ESP_LOGI(TAG, "--> Loaded action to call %s with args", action->function);
        }
    }

    else if (cJSON_IsString(actions)) {
        action = malloc(sizeof(maus_bus_driver_actions_t) + strlen(actions->valuestring) + 1);
        if (action == NULL) return;

        strcpy(action->function, actions->valuestring);
        action->args = NULL;
        action->next = NULL;

        ESP_LOGI(TAG, "--> Loaded action to call %s", action->function);
    }

    // Finally, attach the action.
    if (action != NULL) {
        if (*first == NULL)
            *first = head;
        else {
            for (ptr = *first; ptr->next != NULL; ptr = ptr->next)
                ;
            ptr->next = head;
        }
    }
}

void maus_bus_driver_load_functions(maus_bus_drivercfg_t* driver, cJSON* functions_json) {
    cJSON* fn_json = NULL;
    maus_bus_driver_functions_t* ptr = NULL;
    maus_bus_driver_functions_t* function = NULL;

    ESP_LOGI(TAG, "-> Define all Functions: %s", functions_json->string);

    if (driver == NULL || functions_json == NULL) return;
    if (!cJSON_IsObject(functions_json)) return;

    ptr = driver->functions;

    cJSON_ArrayForEach(fn_json, functions_json) {
        if (fn_json->string == NULL) continue;
        ESP_LOGI(TAG, "--> Found fn: %s", fn_json->string);

        function = (maus_bus_driver_functions_t*)malloc(
            sizeof(maus_bus_driver_functions_t) + strlen(fn_json->string) + 1
        );

        if (function == NULL) break;
        strcpy(function->function_name, fn_json->string);
        function->next = NULL;
        function->actions = NULL;

        maus_bus_driver_load_actions(&function->actions, fn_json);

        // Register:
        if (ptr == NULL)
            driver->functions = function;
        else
            ptr->next = function;

        ptr = function;

        ESP_LOGI(TAG, "--> Define function: %s", function->function_name);
    }
}

void maus_bus_driver_load_variables(maus_bus_drivercfg_t* driver, cJSON* variables_json) {
    cJSON* var_json = NULL;
    maus_bus_driver_variables_t* ptr = NULL;
    maus_bus_driver_variables_t* variable = NULL;

    ESP_LOGI(TAG, "-> Define all Variables: %s", variables_json->string);

    if (driver == NULL || variables_json == NULL) return;
    if (!cJSON_IsObject(variables_json)) return;

    ptr = driver->variables;

    cJSON_ArrayForEach(var_json, variables_json) {
        if (var_json->string == NULL) continue;
        ESP_LOGI(TAG, "--> Found var: %s", var_json->string);

        variable = (maus_bus_driver_variables_t*)malloc(
            sizeof(maus_bus_driver_variables_t) + strlen(var_json->string) + 1
        );

        if (variable == NULL) break;
        strcpy(variable->variable_name, var_json->string);
        variable->next = NULL;
        variable->value = var_json->valueint;

        // Register:
        if (ptr == NULL)
            driver->variables = variable;
        else
            ptr->next = variable;

        ptr = variable;

        ESP_LOGI(TAG, "--> Define variable: %s=%d", variable->variable_name, variable->value);
    }
}

void maus_bus_driver_load_events(maus_bus_drivercfg_t* driver, cJSON* events_json) {
    cJSON* evt_json = NULL;
    maus_bus_driver_events_t* ptr = NULL;
    maus_bus_driver_events_t* event = NULL;

    ESP_LOGI(TAG, "-> Define all Events: %s", events_json->string);

    if (driver == NULL || events_json == NULL) return;
    if (!cJSON_IsObject(events_json)) return;

    ptr = driver->events;

    cJSON_ArrayForEach(evt_json, events_json) {
        if (evt_json->string == NULL) continue;
        ESP_LOGI(TAG, "--> Found evt: %s", evt_json->string);

        event = (maus_bus_driver_events_t*)malloc(
            sizeof(maus_bus_driver_events_t) + strlen(evt_json->string) + 1
        );

        if (event == NULL) break;
        strcpy(event->event_name, evt_json->string);
        event->next = NULL;
        event->actions = NULL;

        maus_bus_driver_load_actions(&event->actions, evt_json);

        // Register:
        if (ptr == NULL)
            driver->events = event;
        else
            ptr->next = event;

        ptr = event;

        ESP_LOGI(TAG, "--> Define event: %s", event->event_name);
    }
}

void maus_bus_driver_load(maus_bus_drivercfg_t** driver, cJSON* root) {
    const char* display_name_str;
    cJSON* display_name = cJSON_GetObjectItem(root, "displayName");

    if (display_name == NULL) {
        display_name_str = DEFAULT_DISPLAY_NAME;
    } else {
        display_name_str = display_name->valuestring;
    }

    *driver =
        (maus_bus_drivercfg_t*)malloc(sizeof(maus_bus_drivercfg_t) + strlen(display_name_str) + 1);

    if (driver == NULL) {
        return;
    }

    strcpy((*driver)->display_name, display_name_str);

    ESP_LOGI(TAG, "Maus-Bus Driver Loading: %s", (*driver)->display_name);

    // Populate Data
    cJSON* match = cJSON_GetObjectItem(root, "match");
    if (match != NULL) maus_bus_driver_load_match(*driver, match);

    cJSON* functions = cJSON_GetObjectItem(root, "functions");
    if (functions != NULL) maus_bus_driver_load_functions(*driver, functions);

    cJSON* variables = cJSON_GetObjectItem(root, "variables");
    if (variables != NULL) maus_bus_driver_load_variables(*driver, variables);

    cJSON* config = cJSON_DetachItemFromObject(root, "config");
    if (config != NULL) (*driver)->config = config;

    cJSON* events = cJSON_GetObjectItem(root, "events");
    if (events != NULL) maus_bus_driver_load_events(*driver, events);

    ESP_LOGI(TAG, "-> Finished.");
}

void maus_bus_driver_event_invoke(maus_bus_drivercfg_t* driver, const char* event, int arg) {
    ESP_LOGI(TAG, "MB Event Invocation: driver=%s, event=%s", driver->display_name, event);
}