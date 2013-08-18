#ifndef INFOCAST_SENSORS_H_
#define INFOCAST_SENSORS_H_

#include <stdint.h>

uint8_t get_cpu_temperature();
uint8_t get_wifi_quality();
float get_battery_info();
void get_ip_address(const char* device, uint32_t& ip, unsigned char* mac);

#endif /* INFOCAST_SENSORS_H_ */
