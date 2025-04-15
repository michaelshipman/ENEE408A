/*
 * sensor_log.h
 *
 *  Created on: Apr 9, 2025
 *      Author: steph
 */

#ifndef SENSOR_LOG_H
#define SENSOR_LOG_H

#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint32_t timestamp;     // milliseconds since boot
    float temperature;      // degrees Celsius
    float pressure;         // hPa or millibar?
    float humidity;         // percent
} SensorData;



static inline void format_csv_line(char *buffer, size_t size, const SensorData *data) {
    snprintf(buffer, size, "%lu,%.2f,%.2f,%.2f\r\n",
             data->timestamp,
             data->temperature,
             data->pressure,
             data->humidity);
}

#endif // SENSOR_LOG_H
