#ifndef COMMAND_H
#define COMMAND_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/ias.h>
#include <stdio.h>


enum Commands{
	NO_CMD = 0,
	UPDATE_CMD = 1,
};

enum CmdFeedbacks{
	NO_FEEDBACK = 0,
	OTA_UPDATE_READY = 1,
	OTA_UPDATE_NOT_READY = 2,
};

extern uint8_t cmd;
ssize_t write_cmd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags);


extern uint8_t cmd_feedback ;
extern uint8_t cmd_feedback_notify_enabled;
void cmd_feedback_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
ssize_t read_cmd_feedback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			    void *buf, uint16_t len, uint16_t offset);

void send_feedback(enum CmdFeedbacks cmd_fb, struct bt_gatt_attr *cmd_feedback_attr);
#endif