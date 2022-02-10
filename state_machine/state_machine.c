#ifdef BUILD_MCU
#include "state_test.h"
#else // Simulator
#include "microcontroller_c_bindings.h"
#endif

#include "state_common.h"
#include "state_machine.h"
#include "state_search.h"
#include "state_attack.h"
#include "state_retreat.h"
#include "detection_history.h"
#include "drive.h"
#include "enemy_detection.h"
#include "line_detection.h"
#include "trace.h"

typedef struct
{
    detection_history_t history;
    search_state_data_t search_data;
    attack_state_data_t attack_data;
    retreat_state_data_t retreat_data;
} state_machine_data_t;

static const char *main_state_str(main_state_t main_state)
{
    static const char *main_state_map[] = {
        [MAIN_STATE_SEARCH] = "ST_SEARCH",
        [MAIN_STATE_ATTACK] = "ST_ATTACK",
        [MAIN_STATE_RETREAT] = "ST_RETREAT",
        [MAIN_STATE_TEST] = "ST_TEST",
    };
    return main_state_map[main_state];
}

static void init()
{
    enemy_detection_init();
    line_detection_init();
    drive_init();
#ifdef MCU_TEST_STATE
    ir_remote_init();
#endif
}

/**
 * Sets up the state machine data, starts the loop and handles the
 * transitions between the main states.
 *
 * Input data is collected once at the beginning of each loop iteration.
 * Therefore, the state machine and its states must NOT contain any blocking
 * code.
 */
void state_machine_run()
{
    init();

#if BUILD_MCU
    /* This should come after sensors are initilized so line measurements are
     * ready when we start */
    ir_remote_wait_for_start_signal();
#endif

    main_state_t current_state = MAIN_STATE_SEARCH;
    main_state_t next_state = current_state;
    bool entered_new_state = true;
    state_machine_data_t sm_data = { 0 };
    while (1) {
        /* Retrieve the input once every loop iteration */
        const detection_t detection = { .line = line_detection_get(),
                                        .enemy = enemy_detection_get() };
        detection_history_save(&sm_data.history, &detection);

#ifdef MCU_TEST_STATE
        ir_key_t remote_command = ir_remote_get_key();
        if (remote_command != IR_KEY_NONE) {
            if (current_state != MAIN_STATE_TEST) {
                drive_stop();
            }
            current_state = next_state = MAIN_STATE_TEST;
        }
#endif
        switch (current_state) {
        case MAIN_STATE_SEARCH: /* Drive around to find the enemy */
            next_state = main_state_search(&sm_data.search_data, entered_new_state, &detection,
                                           &sm_data.history);
            break;
        case MAIN_STATE_ATTACK: /* Enemy detected so attack it */
        {
            next_state = main_state_attack(&sm_data.attack_data, entered_new_state, &detection);
            break;
        }
        case MAIN_STATE_RETREAT: /* Line detected so drive away from it */
            next_state = main_state_retreat(&sm_data.retreat_data, entered_new_state, &detection);
            break;
        case MAIN_STATE_TEST: /* Control with IR remote */
#ifdef MCU_TEST_STATE
            next_state = main_state_test(remote_command);
#endif
            break;
        }

        if (next_state != current_state) {
            current_state = next_state;
            entered_new_state = true;
            TRACE_NOPREFIX("New state: %s", main_state_str(current_state));
        } else {
            entered_new_state = false;
        }

#ifndef BUILD_MCU
        /* Sleep a bit to offload the host CPU */
        sleep_ms(1);
#endif
    }
}
