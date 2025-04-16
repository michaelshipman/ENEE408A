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

#include "MS8607.h"

#define MILLI_G_TO_MS2 0.0098067 ///< Scalar to convert milli-gs to m/s^2
#define DEGREE_SCALE 0.01        ///< To convert the degree values

typedef struct sensor_data {
	uint32_t timestamp;     // milliseconds since boot
	uint32_t pressure;
	int32_t temperature;
	int32_t rel_humidity;
	float yaw,     ///< Yaw in Degrees
			pitch,     ///< Pitch in Degrees
			roll;      ///< Roll in Degrees
	float x_accel, ///< The X acceleration value in m/s^2
			y_accel,   ///< The Y acceleration value in m/s^2
			z_accel;   ///< The Z acceleration value in m/s^2
} sensor_data_t;

static inline void format_csv_line(char *buffer, size_t size,
		const sensor_data_t *data) {
	snprintf(buffer, size,
			"%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%ld,%lu,%ld\r\n",
			data->timestamp, data->roll, data->pitch, data->yaw, data->x_accel,
			data->y_accel, data->z_accel, data->temperature, data->pressure,
			data->rel_humidity);
}

extern UART_HandleTypeDef hlpuart1;

static inline void get_bno_data(sensor_data_t *data) {

	uint8_t bno_buffer[20] = { 0 };

	while (bno_buffer[0] != 0xAA)
		HAL_UART_Receive(&hlpuart1, bno_buffer, 1, 1000);

	while (bno_buffer[1] != 0xAA)
		HAL_UART_Receive(&hlpuart1, bno_buffer + 1, 1, 1000);

	HAL_UART_Receive(&hlpuart1, bno_buffer + 2, 17, 1000);

	if (bno_buffer[0] == 0xAA && bno_buffer[1] == 0xAA) {
		//got data!!!

		uint8_t sum = 0;
		// get checksum ready
		for (uint8_t i = 2; i < 17; i++) {
			sum += bno_buffer[i];
		}

		if (sum != bno_buffer[18]) {
			// data not valid
			;
		}

		// The data comes in endian'd, this solves it so it works on all platforms
		int16_t buffer_16[6];

		for (uint8_t i = 0; i < 6; i++) {

			buffer_16[i] = (bno_buffer[1 + (i * 2)]);
			buffer_16[i] += (bno_buffer[1 + (i * 2) + 1] << 8);
		}
		data->yaw = (float) buffer_16[0] * DEGREE_SCALE;
		data->pitch = (float) buffer_16[1] * DEGREE_SCALE;
		data->roll = (float) buffer_16[2] * DEGREE_SCALE;

		data->x_accel = (float) buffer_16[3] * MILLI_G_TO_MS2;
		data->y_accel = (float) buffer_16[4] * MILLI_G_TO_MS2;
		data->z_accel = (float) buffer_16[5] * MILLI_G_TO_MS2;
	}
}

#endif // SENSOR_LOG_H
