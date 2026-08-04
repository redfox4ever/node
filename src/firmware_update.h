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
void transfer_firmware();

ssize_t write_payload_size(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags);
extern int32_t payload_size;
extern bool payload_flag;
#endif