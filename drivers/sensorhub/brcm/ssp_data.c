/*
 *  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */
#include "ssp.h"
#include <linux/math64.h>
#include <linux/sched.h>

/* SSP -> AP Instruction */
#define MSG2AP_INST_BYPASS_DATA			0x37
#define MSG2AP_INST_LIBRARY_DATA		0x01
#define MSG2AP_INST_DEBUG_DATA			0x03
#define MSG2AP_INST_BIG_DATA			0x04
#define MSG2AP_INST_META_DATA			0x05
#define MSG2AP_INST_TIME_SYNC			0x06
#define MSG2AP_INST_RESET			0x07
#define MSG2AP_INST_GYRO_CAL			0x08

#define U64_MS2NS 1000000ULL
#define U64_US2NS 1000ULL
#define U64_MS2US 1000ULL
#define MS_IDENTIFIER 1000000000U

/*************************************************************************/
/* SSP parsing the dataframe                                             */
/*************************************************************************/
static void get_timestamp(struct ssp_data *data, char *pchRcvDataFrame,
		int *iDataIdx, struct sensor_value *sensorsdata,
		struct ssp_time_diff *sensortime, int iSensorData)
{
	unsigned int deltaTimeUs = 0;
	u64 deltaTimeNs = 0;

	memset(&deltaTimeUs, 0, 4);
	memcpy(&deltaTimeUs, pchRcvDataFrame + *iDataIdx, 4);

	if (deltaTimeUs > MS_IDENTIFIER) {
		//We condsider, unit is ms (MS->NS)
		deltaTimeNs = ((u64) (deltaTimeUs % MS_IDENTIFIER)) * U64_MS2NS;
	} else {
		deltaTimeNs = (((u64) deltaTimeUs) * U64_US2NS);//US->NS
	}

	if (sensortime->batch_mode == BATCH_MODE_RUN) {
		// BATCHING MODE
		data->lastTimestamp[iSensorData] += deltaTimeNs;
	} else {
		// NORMAL MODE

		// CAMERA SYNC MODE
		if (data->cameraGyroSyncMode && iSensorData == GYROSCOPE_SENSOR) {
			if (deltaTimeNs == 1000ULL || data->lastTimestamp[iSensorData] == 0ULL) {
				//eltaTimeNs = 0ULL;
				data->lastTimestamp[iSensorData] = data->timestamp;
				deltaTimeNs = 0ULL;
			} else {
				if (data->timestamp < data->lastTimestamp[iSensorData]) {
					deltaTimeNs = 0ULL;
				} else {
					deltaTimeNs = data->timestamp - data->lastTimestamp[iSensorData];
				}
			}

			if (deltaTimeNs == 0ULL) {
				// Don't report when time is 0.
				data->skipEventReport = true;
			} else if (deltaTimeNs > (data->adDelayBuf[iSensorData] * 18ULL / 10ULL)) {
				int cnt = 0;
				int i = 0;
				cnt = deltaTimeNs / (data->adDelayBuf[iSensorData]);

				for (i = 0; i < cnt; i++) {
					data->lastTimestamp[iSensorData] += data->adDelayBuf[iSensorData];
					sensorsdata->timestamp = data->lastTimestamp[iSensorData];
					data->report_sensor_data[iSensorData](data, sensorsdata);
					deltaTimeNs -= data->adDelayBuf[iSensorData];
				}

				// mod is calculated automatically.
				if (deltaTimeNs > (data->adDelayBuf[iSensorData] / 2ULL)) {
					data->lastTimestamp[iSensorData] += deltaTimeNs;
					sensorsdata->timestamp = data->lastTimestamp[iSensorData];
					data->report_sensor_data[iSensorData](data, sensorsdata);

					data->skipEventReport = true;
				}
				deltaTimeNs = 0ULL;
			}
			else if (deltaTimeNs < (data->adDelayBuf[iSensorData] / 2ULL)) {
				data->skipEventReport = true;
				deltaTimeNs = 0ULL;
			}
			data->lastTimestamp[iSensorData] += deltaTimeNs;

		} else {
			// 80ms is magic number. reset time base.
			if (deltaTimeNs == 0ULL || deltaTimeNs == 1000ULL || deltaTimeNs == 80000ULL) {
				data->lastTimestamp[iSensorData] = data->timestamp - 15000000ULL;
				deltaTimeNs = 0ULL;
			}

			if (data->report_mode[iSensorData] == REPORT_MODE_ON_CHANGE) {
				data->lastTimestamp[iSensorData] = data->timestamp;
			} else {
				data->lastTimestamp[iSensorData] += deltaTimeNs;
			}
		}
	}
	sensorsdata->timestamp = data->lastTimestamp[iSensorData];
	*iDataIdx += 4;
}

static void get_3axis_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 6);
	*iDataIdx += 6;
}

static void get_uncalib_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 12);
	*iDataIdx += 12;
}

static void get_geomagnetic_uncaldata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 12);
	*iDataIdx += 12;
}

static void get_geomagnetic_rawdata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 6);
	*iDataIdx += 6;
}

static void get_geomagnetic_caldata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 7);
	*iDataIdx += 7;
}

static void get_rot_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 17);
	*iDataIdx += 17;
}

static void get_step_det_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 1);
	*iDataIdx += 1;
}

static void get_light_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 10);
	*iDataIdx += 10;
}

#ifdef CONFIG_SENSORS_SSP_IRDATA_FOR_CAMERA
static void get_light_ir_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 12);
	*iDataIdx += 12;
}
#endif

static void get_pressure_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	s16 temperature = 0;
	memcpy(&sensorsdata->pressure[0], pchRcvDataFrame + *iDataIdx, 4);
	memcpy(&temperature, pchRcvDataFrame + *iDataIdx + 4, 2);
	sensorsdata->pressure[1] = temperature;
	*iDataIdx += 6;
}

static void get_gesture_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 20);
	*iDataIdx += 20;
}

static void get_proximity_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
#if defined(CONFIG_SENSORS_SSP_TMG399x)
	memset(&sensorsdata->prox[0], 0, 1);
	memcpy(&sensorsdata->prox[0], pchRcvDataFrame + *iDataIdx, 2);
	*iDataIdx += 2;
#else	//CONFIG_SENSORS_SSP_TMD4903, CONFIG_SENSORS_SSP_TMD3782
	memset(&sensorsdata->prox[0], 0, 2);
	memcpy(&sensorsdata->prox[0], pchRcvDataFrame + *iDataIdx, 1);
	memcpy(&sensorsdata->prox[1], pchRcvDataFrame + *iDataIdx + 1, 2);
	*iDataIdx += 3;
#endif
}

static void get_proximity_rawdata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
#if defined(CONFIG_SENSORS_SSP_TMG399x)
	memcpy(&sensorsdata->prox[0], pchRcvDataFrame + *iDataIdx, 1);
	*iDataIdx += 1;
#else	//CONFIG_SENSORS_SSP_TMD4903, CONFIG_SENSORS_SSP_TMD3782
	memcpy(&sensorsdata->prox[0], pchRcvDataFrame + *iDataIdx, 2);
	*iDataIdx += 2;
#endif
}

#ifdef CONFIG_SENSORS_SSP_SX9306
static void get_grip_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 9);
		*iDataIdx += 9;
}
#endif

static void get_temp_humidity_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memset(&sensorsdata->data[2], 0, 2);
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 5);
	*iDataIdx += 5;
}

static void get_sig_motion_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 1);
	*iDataIdx += 1;
}

static void get_step_cnt_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(&sensorsdata->step_diff, pchRcvDataFrame + *iDataIdx, 4);
	*iDataIdx += 4;
}

static void get_shake_cam_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 1);
	*iDataIdx += 1;
}

static void get_tilt_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 1);
	*iDataIdx += 1;
}

static void get_pickup_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 1);
	*iDataIdx += 1;
}

int handle_big_data(struct ssp_data *data, char *pchRcvDataFrame, int *pDataIdx) {
	u8 bigType = 0;
	struct ssp_big *big = kzalloc(sizeof(*big), GFP_KERNEL);
	big->data = data;
	bigType = pchRcvDataFrame[(*pDataIdx)++];
	memcpy(&big->length, pchRcvDataFrame + *pDataIdx, 4);
	*pDataIdx += 4;
	memcpy(&big->addr, pchRcvDataFrame + *pDataIdx, 4);
	*pDataIdx += 4;

	if (bigType >= BIG_TYPE_MAX) {
		kfree(big);
		return FAIL;
	}

	INIT_WORK(&big->work, data->ssp_big_task[bigType]);
	queue_work(data->debug_wq, &big->work);
	return SUCCESS;
}

int parse_dataframe(struct ssp_data *data, char *pchRcvDataFrame, int iLength)
{
	int iDataIdx, iSensorData;
	u16 length = 0;
	struct sensor_value sensorsdata;
	struct ssp_time_diff sensortime;
	s16 caldata[3] = { 0, };

	sensortime.time_diff = 0;
	data->uIrqCnt++;

	for (iDataIdx = 0; iDataIdx < iLength;) {
		switch (pchRcvDataFrame[iDataIdx++]) {
		case MSG2AP_INST_BYPASS_DATA:
			iSensorData = pchRcvDataFrame[iDataIdx++];
			if ((iSensorData < 0) || (iSensorData >= SENSOR_MAX)) {
				pr_err("[SSP]: %s - Mcu data frame1 error %d\n", __func__,
						iSensorData);
				return ERROR;
			}

			memcpy(&length, pchRcvDataFrame + iDataIdx, 2);
			iDataIdx += 2;
			sensortime.batch_count = sensortime.batch_count_fixed = length;
			sensortime.batch_mode = length > 1 ? BATCH_MODE_RUN : BATCH_MODE_NONE;

			do {
				data->get_sensor_data[iSensorData](pchRcvDataFrame, &iDataIdx, &sensorsdata);
				data->skipEventReport = false;
				get_timestamp(data, pchRcvDataFrame, &iDataIdx, &sensorsdata, &sensortime, iSensorData);
				if (data->skipEventReport == false) {
					data->report_sensor_data[iSensorData](data, &sensorsdata);
				}

				sensortime.batch_count--;
			} while ((sensortime.batch_count > 0) && (iDataIdx < iLength));

			if (sensortime.batch_count > 0)
				pr_err("[SSP]: %s batch count error (%d)\n", __func__, sensortime.batch_count);

			//data->lastTimestamp[iSensorData] = data->timestamp;
			data->reportedData[iSensorData] = true;
			break;
		case MSG2AP_INST_DEBUG_DATA:
			iSensorData = print_mcu_debug(pchRcvDataFrame, &iDataIdx, iLength);
			if (iSensorData) {
				pr_err("[SSP]: %s - Mcu data frame3 error %d\n", __func__,
						iSensorData);
				return ERROR;
			}
			break;
		case MSG2AP_INST_LIBRARY_DATA:
			memcpy(&length, pchRcvDataFrame + iDataIdx, 2);
			iDataIdx += 2;
			ssp_sensorhub_handle_data(data, pchRcvDataFrame, iDataIdx,
					iDataIdx + length);
			iDataIdx += length;
			break;
		case MSG2AP_INST_BIG_DATA:
			handle_big_data(data, pchRcvDataFrame, &iDataIdx);
			break;
		case MSG2AP_INST_META_DATA:
			sensorsdata.meta_data.what = pchRcvDataFrame[iDataIdx++];
			sensorsdata.meta_data.sensor = pchRcvDataFrame[iDataIdx++];
			report_meta_data(data, &sensorsdata);
			break;
		case MSG2AP_INST_TIME_SYNC:
			data->bTimeSyncing = true;
			break;
		case MSG2AP_INST_GYRO_CAL:
			pr_info("[SSP]: %s - Gyro caldata received from MCU\n",  __func__);
			memcpy(caldata, pchRcvDataFrame + iDataIdx, sizeof(caldata));
			wake_lock(&data->ssp_wake_lock);
			save_gyro_caldata(data, caldata);
			wake_unlock(&data->ssp_wake_lock);
			iDataIdx += sizeof(caldata);
			break;
		}
	}

	return SUCCESS;
}

void initialize_function_pointer(struct ssp_data *data)
{
	data->get_sensor_data[ACCELEROMETER_SENSOR] = get_3axis_sensordata;
	data->get_sensor_data[GYROSCOPE_SENSOR] = get_3axis_sensordata;
	data->get_sensor_data[GEOMAGNETIC_UNCALIB_SENSOR] =
		get_geomagnetic_uncaldata;
	data->get_sensor_data[GEOMAGNETIC_RAW] = get_geomagnetic_rawdata;
	data->get_sensor_data[GEOMAGNETIC_SENSOR] =
		get_geomagnetic_caldata;
	data->get_sensor_data[PRESSURE_SENSOR] = get_pressure_sensordata;
	data->get_sensor_data[GESTURE_SENSOR] = get_gesture_sensordata;
	data->get_sensor_data[PROXIMITY_SENSOR] = get_proximity_sensordata;
	data->get_sensor_data[PROXIMITY_RAW] = get_proximity_rawdata;
#ifdef CONFIG_SENSORS_SSP_SX9306
	data->get_sensor_data[GRIP_SENSOR] = get_grip_sensordata;
#endif
	data->get_sensor_data[LIGHT_SENSOR] = get_light_sensordata;
#ifdef CONFIG_SENSORS_SSP_IRDATA_FOR_CAMERA
	data->get_sensor_data[LIGHT_IR_SENSOR] = get_light_ir_sensordata;
#endif
	data->get_sensor_data[TEMPERATURE_HUMIDITY_SENSOR] =
		get_temp_humidity_sensordata;
	data->get_sensor_data[ROTATION_VECTOR] = get_rot_sensordata;
	data->get_sensor_data[GAME_ROTATION_VECTOR] = get_rot_sensordata;
	data->get_sensor_data[STEP_DETECTOR] = get_step_det_sensordata;
	data->get_sensor_data[SIG_MOTION_SENSOR] = get_sig_motion_sensordata;
	data->get_sensor_data[GYRO_UNCALIB_SENSOR] = get_uncalib_sensordata;
	data->get_sensor_data[STEP_COUNTER] = get_step_cnt_sensordata;
	data->get_sensor_data[SHAKE_CAM] = get_shake_cam_sensordata;
#ifdef CONFIG_SENSORS_SSP_INTERRUPT_GYRO_SENSOR
	data->get_sensor_data[INTERRUPT_GYRO_SENSOR] = get_3axis_sensordata;
#endif
	data->get_sensor_data[TILT_DETECTOR] = get_tilt_sensordata;
	data->get_sensor_data[PICKUP_GESTURE] = get_pickup_sensordata;
	data->get_sensor_data[MOTOR_TEST] = get_3axis_sensordata;

	data->report_sensor_data[ACCELEROMETER_SENSOR] = report_acc_data;
	data->report_sensor_data[GYROSCOPE_SENSOR] = report_gyro_data;
	data->report_sensor_data[GEOMAGNETIC_UNCALIB_SENSOR] =
		report_mag_uncaldata;
	data->report_sensor_data[GEOMAGNETIC_RAW] = report_geomagnetic_raw_data;
	data->report_sensor_data[GEOMAGNETIC_SENSOR] =
		report_mag_data;
	data->report_sensor_data[PRESSURE_SENSOR] = report_pressure_data;
	data->report_sensor_data[GESTURE_SENSOR] = report_gesture_data;
	data->report_sensor_data[PROXIMITY_SENSOR] = report_prox_data;
	data->report_sensor_data[PROXIMITY_RAW] = report_prox_raw_data;
#ifdef CONFIG_SENSORS_SSP_SX9306
	data->report_sensor_data[GRIP_SENSOR] = report_grip_data;
#endif
	data->report_sensor_data[LIGHT_SENSOR] = report_light_data;
#ifdef CONFIG_SENSORS_SSP_IRDATA_FOR_CAMERA
	data->report_sensor_data[LIGHT_IR_SENSOR] = report_light_ir_data;
#endif
	data->report_sensor_data[TEMPERATURE_HUMIDITY_SENSOR] =
		report_temp_humidity_data;
	data->report_sensor_data[ROTATION_VECTOR] = report_rot_data;
	data->report_sensor_data[GAME_ROTATION_VECTOR] = report_game_rot_data;
	data->report_sensor_data[STEP_DETECTOR] = report_step_det_data;
	data->report_sensor_data[SIG_MOTION_SENSOR] = report_sig_motion_data;
	data->report_sensor_data[GYRO_UNCALIB_SENSOR] = report_uncalib_gyro_data;
	data->report_sensor_data[STEP_COUNTER] = report_step_cnt_data;
	data->report_sensor_data[SHAKE_CAM] = report_shake_cam_data;
#ifdef CONFIG_SENSORS_SSP_INTERRUPT_GYRO_SENSOR
	data->report_sensor_data[INTERRUPT_GYRO_SENSOR] = report_interrupt_gyro_data;
#endif
	data->report_sensor_data[TILT_DETECTOR] = report_tilt_data;
	data->report_sensor_data[PICKUP_GESTURE] = report_pickup_data;
	data->report_sensor_data[MOTOR_TEST] = report_motor_test_data;

	data->ssp_big_task[BIG_TYPE_DUMP] = ssp_dump_task;
	data->ssp_big_task[BIG_TYPE_READ_LIB] = ssp_read_big_library_task;
	data->ssp_big_task[BIG_TYPE_VOICE_NET] = ssp_send_big_library_task;
	data->ssp_big_task[BIG_TYPE_VOICE_GRAM] = ssp_send_big_library_task;
	data->ssp_big_task[BIG_TYPE_VOICE_PCM] = ssp_pcm_dump_task;
	data->ssp_big_task[BIG_TYPE_TEMP] = ssp_temp_task;
}
