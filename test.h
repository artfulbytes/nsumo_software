#ifndef TEST_H
#define TEST_H

#if BUILD_MCU
void test_dimming_led(void);
void test_leds(void);
void test_run_motors(void);
void test_adc(void);
void test_range_sensors(void);
void test_ir_receiver(void);
void test_state_machine_ir(void);
void test_vl53l0x(void);
void test_vl53l0x_multiple(void);
void test_vl53l0x_multiple_time(void);
void test_qre1113(void);
void test_qre1113_time(void);
void test_enemy_detection(void);
void test_gpio_input(void);
#endif
void test_line_detection(void);
void test_drive_and_line_detect(void);
void test_rotate_and_enemy_detect(void);

#endif /* TEST_H */
