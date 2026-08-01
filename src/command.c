#include <command.h>


// command -------------------------------------------------------------->



// command characteristic 
uint8_t cmd = NO_CMD;
ssize_t write_cmd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
    printf("Attempting to write command. ------------------------------------  \n");
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(cmd)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);

    if (cmd != NO_CMD && cmd != UPDATE_CMD) {
        cmd = NO_CMD;  // reject unrecognized values
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }

	return len;
}

// command --------------------------------------------------------------/


// command feedback notification -------------------------------------------------------------->


uint8_t cmd_feedback = NO_FEEDBACK;
uint8_t cmd_feedback_notify_enabled;
void cmd_feedback_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	cmd_feedback_notify_enabled = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

ssize_t read_cmd_feedback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			    void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				  &cmd_feedback, sizeof(cmd_feedback));
}



void send_feedback(enum CmdFeedbacks cmd_fb, struct bt_gatt_attr *cmd_feedback_attr){
    cmd_feedback = cmd_fb;
	bt_gatt_notify(NULL, cmd_feedback_attr,  &cmd_feedback, sizeof(cmd_feedback));
}
