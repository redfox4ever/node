#ifndef FIRMWARE_UPDATE_H
#define FIRMWARE_UPDATE_H
#include <zephyr/storage/flash_map.h>
#include <stdint.h>
#include <zephyr/bluetooth/att.h>
#include <zephyr/bluetooth/gatt.h>
#include <stdio.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/toolchain.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>


int copy_update_partition();
void swap_A_B_partitions();
void transfer_firmware(struct bt_gatt_attr *pc_feedback_attr);

ssize_t write_payload_size(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags);
extern int32_t payload_size;
extern bool payload_size_flag;

#define CHUNK_DATA_SIZE 239
struct PayloadChunk
{
	int32_t index;
	char data[CHUNK_DATA_SIZE];
	uint8_t len;
} __packed;

extern struct PayloadChunk payload_chunk;
ssize_t write_payload_chunk(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags);


void payload_chunk_feedback_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
#endif