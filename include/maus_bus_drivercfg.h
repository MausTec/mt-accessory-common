#ifndef __mt_accessory_common__maus_bus_driver_h
#define __mt_accessory_common__maus_bus_driver_h

#ifdef __cplusplus
extern "C" {
#endif

#include "cJSON.h"

struct maus_bus_driver_functions;

/**
 * @brief Linked list of driver action invocations, which reference a functinon and pass arguments.
 *
 */
typedef struct maus_bus_driver_actions {
    cJSON* args;
    struct maus_bus_driver_actions* next;
    struct maus_bus_driver_functions* fn;
    char function[];
} maus_bus_driver_actions_t;

/**
 * @brief Linked list of driver event registrations, which reference an action list.
 *
 */
typedef struct maus_bus_driver_events {
    struct maus_bus_driver_actions* actions;
    struct maus_bus_driver_events* next;
    char event_name[];
} maus_bus_driver_events_t;

/**
 * @brief Linked list of function definitions loaded into a particular driver.
 *
 */
typedef struct maus_bus_driver_functions {
    struct maus_bus_driver_actions* actions;
    struct maus_bus_driver_functions* next;
    char function_name[];
} maus_bus_driver_functions_t;

/**
 * @brief Linked list of variables loaded into a particular driver.
 *
 */
typedef struct maus_bus_driver_variables {
    int value;
    struct maus_bus_driver_variables* next;
    char variable_name[];
} maus_bus_driver_variables_t;

/**
 * @brief Match struct containing parameters to automatically load a driver given an accessory
 * descriptor.
 *
 */
typedef struct maus_bus_driver_match {
    int vid;
    int pid;
    char serial[];
} maus_bus_driver_match_t;

/**
 * @brief Base driver installation, which contains all the relevant function definitions, actions,
 * and config.
 *
 */
typedef struct maus_bus_driver {
    maus_bus_driver_events_t* events;
    maus_bus_driver_functions_t* functions;
    maus_bus_driver_match_t* match;
    maus_bus_driver_variables_t* variables;
    cJSON* config;
    char display_name[];
} maus_bus_drivercfg_t;

typedef void (*maus_bus_driver_system_function_t)(maus_bus_drivercfg_t* driver, cJSON* args);

// Data Definitions
void maus_bus_driver_load(maus_bus_drivercfg_t** driver, cJSON* root);
void maus_bus_driver_unload(maus_bus_drivercfg_t* driver);
void maus_bus_driver_register_system_function(
    const char* fn_name, maus_bus_driver_system_function_t fn
);

maus_bus_driver_functions_t*
maus_bus_driver_define_function(maus_bus_drivercfg_t* driver, const char* fn_name);

maus_bus_driver_actions_t* maus_bus_driver_define_function_action(
    maus_bus_driver_functions_t* function, const char* callee_name, cJSON* args
);

maus_bus_driver_events_t*
maus_bus_driver_define_event(maus_bus_drivercfg_t* driver, const char* evt_name);

maus_bus_driver_actions_t* maus_bus_driver_define_event_action(
    maus_bus_driver_events_t* events, const char* callee_name, cJSON* args
);

// Runtime Invocations
void maus_bus_driver_function_call(maus_bus_drivercfg_t* driver, const char* fn_name, cJSON* args);
void maus_bus_driver_get_variable(maus_bus_drivercfg_t* driver, const char* var_name, int* value);
void maus_bus_driver_set_variable(maus_bus_drivercfg_t* driver, const char* var_name, int value);
void maus_bus_driver_action_invoke(maus_bus_drivercfg_t* driver, maus_bus_driver_actions_t* action);
void maus_bus_driver_event_invoke(maus_bus_drivercfg_t* driver, const char* event, int arg);

/**
 * @brief This invokes any matching event across all drivers registered.
 *
 * @param event
 * @param arg
 */
void maus_bus_invoke_all_events(const char* event, int arg);

// Driver Search and Invocations
maus_bus_drivercfg_t* maus_bus_find_driver(maus_bus_driver_match_t* match);

typedef void (*maus_bus_driver_enumeration_cb_t)(maus_bus_drivercfg_t* driver);
size_t maus_bus_enumerate_drivers(maus_bus_driver_enumeration_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif
