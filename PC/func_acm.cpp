#include "func_acm.h"

// Устройство
void get_device_param(uint8_t par) {}
void exec_cmd(uint8_t cmd) {}
void exec_cmd_adc_test(uint8_t cmd_data) {}
void get_alg_param(uint8_t par) {}
void set_leak_level(uint8_t leakLevel) {}
int  set_temp_mode(uint8_t tempMode) { return 1; }
void set_calc_mode(uint8_t calcMode) {}

// Настройки
void set_alg_settings() {}
void set_work_settings() {}
int  set_test_settings() { return 1; }
void set_link_settings() {}
int  set_laser_settings(uint8_t temp) { return 1; }
void set_dist_settings() {}
void set_gps_settings() {}

// Чтение настроек
void get_settings() {}
void exec_settings_cmd(uint8_t cmd, uint8_t type) {}

// Служебные функции
void make_dev_get_param_packet(uint8_t par) {}
void make_alg_cmd_packet(uint8_t cmd) {}
void make_alg_cmd_packet_ext(uint8_t cmd, uint8_t cmd_data) {}
void make_alg_set_leak_level_packet() {}
void make_alg_set_temp_mode_packet() {}
void make_alg_set_calc_mode_packet() {}
void make_alg_get_param_packet(uint8_t par) {}
void make_set_settings_packet(){}
void make_get_settings_packet(uint8_t par) {}
void make_settings_cmd_packet(uint8_t cmd, uint8_t par){}
