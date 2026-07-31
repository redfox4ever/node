
// TODO: add atoms to track connected / disconnected states, and call start_advertising when you dc
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/ias.h>
#include <sensor.h>




/* Custom Service Variables */
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static const struct bt_uuid_128 vnd_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_SERVICE_VAL);

static const struct bt_uuid_128 vnd_enc_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

static const struct bt_uuid_128 vnd_auth_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef2));






// message characteristic 
static const struct bt_uuid_128 vnd_msg_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef6));
#define MSG_MAX_LEN 32
char msg[MSG_MAX_LEN + 1] = {0};

static ssize_t write_msg(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > MSG_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}



// sensor notification -------------------------------------------------------->
static const struct bt_uuid_128 vnd_sensor_uuid = BT_UUID_INIT_128(
BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef5));
static int16_t sensor_value = 0; 
static uint8_t sensor_notify_enabled;
static void sensor_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	sensor_notify_enabled = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

int read_my_sensor(){
	return ++sensor_value;
}

static ssize_t read_sensor(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			    void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				  &sensor_value, sizeof(sensor_value));
}
// sensor notification -------------------------------------------------------->


// command -------------------------------------------------------------->



// command characteristic 
static const struct bt_uuid_128 vnd_cmd_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef7));
enum Commands{
	NO_CMD = 0,
	UPDATE = 1,
};
uint8_t cmd = NO_CMD;
static ssize_t write_cmd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(cmd)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);

    if (cmd != NO_CMD && cmd != UPDATE) {
        cmd = NO_CMD;  // reject unrecognized values
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }

	return len;
}

// command --------------------------------------------------------------/


// command feedback notification -------------------------------------------------------------->


static const struct bt_uuid_128 vnd_cmd_feedback_uuid = BT_UUID_INIT_128(
BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef8));
enum CmdFeedbacks{
	NO_FEEDBACK = 0,
	OTA_UPDATE_READY = 1,
	OTA_UPDATE_NOT_READY = 2,
};
uint8_t cmd_feedback = NO_FEEDBACK;
static uint8_t cmd_feedback_notify_enabled;
static void cmd_feedback_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	cmd_feedback_notify_enabled = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static ssize_t read_cmd_feedback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			    void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				  &cmd_feedback, sizeof(sensor_value));
}

// command feedback notification --------------------------------------------------------------/

#define VND_MAX_LEN 20
// #define BT_HR_HEARTRATE_DEFAULT_MIN 90U
// #define BT_HR_HEARTRATE_DEFAULT_MAX 160U

static uint8_t vnd_value[VND_MAX_LEN + 1] = { 'V', 'e', 'n', 'd', 'o', 'r'};
static uint8_t vnd_auth_value[VND_MAX_LEN + 1] = { 'V', 'e', 'n', 'd', 'o', 'r'};
static uint8_t vnd_wwr_value[VND_MAX_LEN + 1] = { 'V', 'e', 'n', 'd', 'o', 'r' };

static ssize_t read_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 strlen(value));
}

static ssize_t write_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > VND_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

static uint8_t simulate_vnd;
static uint8_t indicating;
static struct bt_gatt_indicate_params ind_params;

static void vnd_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	simulate_vnd = (value == BT_GATT_CCC_INDICATE) ? 1 : 0;
}

static void indicate_cb(struct bt_conn *conn,
			struct bt_gatt_indicate_params *params, uint8_t err)
{
	printk("Indication %s\n", err != 0U ? "fail" : "success");
}

static void indicate_destroy(struct bt_gatt_indicate_params *params)
{
	printk("Indication complete\n");
	indicating = 0U;
}

#define VND_LONG_MAX_LEN 74
static uint8_t vnd_long_value[VND_LONG_MAX_LEN + 1] = {
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '1',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '2',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '3',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '4',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '5',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '6',
		  '.', ' ' };

static ssize_t write_long_vnd(struct bt_conn *conn,
			      const struct bt_gatt_attr *attr, const void *buf,
			      uint16_t len, uint16_t offset, uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (flags & BT_GATT_WRITE_FLAG_PREPARE) {
		return 0;
	}

	if (offset + len > VND_LONG_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

static const struct bt_uuid_128 vnd_long_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef3));

static struct bt_gatt_cep vnd_long_cep = {
	.properties = BT_GATT_CEP_RELIABLE_WRITE,
};

static const struct bt_uuid_128 vnd_write_cmd_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef4));


static ssize_t write_without_rsp_vnd(struct bt_conn *conn,
				     const struct bt_gatt_attr *attr,
				     const void *buf, uint16_t len, uint16_t offset,
				     uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (!(flags & BT_GATT_WRITE_FLAG_CMD)) {
		/* Write Request received. Reject it since this Characteristic
		 * only accepts Write Without Response.
		 */
		return BT_GATT_ERR(BT_ATT_ERR_WRITE_REQ_REJECTED);
	}

	if (offset + len > VND_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

/* Vendor Primary Service Declaration */
BT_GATT_SERVICE_DEFINE(vnd_svc,
	BT_GATT_PRIMARY_SERVICE(&vnd_uuid),
	BT_GATT_CHARACTERISTIC(&vnd_enc_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |
			       BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_READ_ENCRYPT |
			       BT_GATT_PERM_WRITE_ENCRYPT,
			       read_vnd, write_vnd, vnd_value),
	BT_GATT_CCC(vnd_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	BT_GATT_CHARACTERISTIC(&vnd_auth_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ_AUTHEN |
			       BT_GATT_PERM_WRITE_AUTHEN,
			       read_vnd, write_vnd, vnd_auth_value),
	BT_GATT_CHARACTERISTIC(&vnd_long_uuid.uuid, BT_GATT_CHRC_READ |
			       BT_GATT_CHRC_WRITE | BT_GATT_CHRC_EXT_PROP,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE |
			       BT_GATT_PERM_PREPARE_WRITE,
			       read_vnd, write_long_vnd, &vnd_long_value),
	BT_GATT_CEP(&vnd_long_cep),
	BT_GATT_CHARACTERISTIC(&vnd_write_cmd_uuid.uuid,
			       BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_WRITE, NULL,
			       write_without_rsp_vnd, &vnd_wwr_value),
	// sensor ------------------------------------------------------>
	BT_GATT_CHARACTERISTIC(&vnd_sensor_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
			       read_sensor, NULL, NULL),
	BT_GATT_CCC(sensor_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
		// sensor ------------------------------------------------------>
	
	// message --------------------------->
	BT_GATT_CHARACTERISTIC(&vnd_msg_uuid.uuid,
							BT_GATT_CHRC_WRITE,
							BT_GATT_PERM_WRITE,
							NULL, write_msg, &msg),
	// message --------------------------->

	// command --------------------------->
	BT_GATT_CHARACTERISTIC(&vnd_cmd_uuid.uuid,
							BT_GATT_CHRC_WRITE,
							BT_GATT_PERM_WRITE,
							NULL, write_cmd, &cmd)
	// command --------------------------->

	// command feedback ------------------------------------------------------>
	BT_GATT_CHARACTERISTIC(&vnd_cmd_feedback_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
			       read_sensor, NULL, NULL),
	BT_GATT_CCC(cmd_feedback_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	// command feedback------------------------------------------------------>
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	/*
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_HRS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_CTS_VAL)),
	*/
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {
	.att_mtu_updated = mtu_updated
};

enum{
	CONNECTED_FLAG,
	DISCONNECTED_FLAG,
	NUMBER_OF_CONNECTION_FLAGS
};

ATOMIC_DEFINE(connection_flag_bits, NUMBER_OF_CONNECTION_FLAGS);

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
	} else {
		printk("Connected\n");
		atomic_set_bit(connection_flag_bits, CONNECTED_FLAG);
	}
}
void start_advertising(){

	int	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}
	printk("Advertising successfully started\n");
}
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
	atomic_set_bit(connection_flag_bits, DISCONNECTED_FLAG);
}

static void alert_stop(void)
{
	printk("Alert stopped\n");
}

static void alert_start(void)
{
	printk("Mild alert started\n");
}

static void alert_high_start(void)
{
	printk("High alert started\n");
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

BT_IAS_CB_DEFINE(ias_callbacks) = {
	.no_alert = alert_stop,
	.mild_alert = alert_start,
	.high_alert = alert_high_start,
};

static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	printk("Passkey for %s: %06u\n", bt_conn_dst_str(conn), passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
	printk("Pairing cancelled: %s\n", bt_conn_dst_str(conn));
}

static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
};


#include <firmware_update.h>
int main(void)
{
	/*
    int erro;
	const struct flash_area* slot1_partition;
    erro = flash_area_open(PARTITION_ID(slot1_partition), &slot1_partition);
	const struct flash_area *image_update_partition;
    erro = flash_area_open(PARTITION_ID(image_update_partition), &image_update_partition);
	flash_area_erase(image_update_partition, 0, PARTITION_SIZE(image_update_partition));
	flash_area_write(image_update_partition, 0, "hello, world", sizeof("hello, world"));
	*/

	//copy_update_partition();
	
	/*
	char *message = malloc(32);
	flash_area_read(slot1_partition, 0, message, strlen("hello, world"));
	message[strlen("hello, world")] = '\0';
	printf("Message is : %s /////////////////////////////", message);
	*/

	//swap_A_B_partitions();

	/*
	    if (!device_is_ready(apds9960)) {
        printk("APDS9960 not ready\n");
        return 0;
    }
		*/
	printf("getting started ...");

	struct bt_gatt_attr *vnd_ind_attr;
	char str[BT_UUID_STR_LEN];
	int err;

	// sensor ------------>
	struct bt_gatt_attr *sensor_attr;


	err = bt_enable(NULL);
	if (err != 0) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}
	sensor_attr = bt_gatt_find_by_uuid(vnd_svc.attrs, vnd_svc.attr_count,
				    &vnd_sensor_uuid.uuid);
	printk("Sensor attr %p\n", sensor_attr);
	// sensor ------------>

	struct bt_gatt_attr *msg_attr = bt_gatt_find_by_uuid(vnd_svc.attrs, vnd_svc.attr_count,
				    &vnd_msg_uuid.uuid);
	printk("Sensor attr %p\n", msg_attr);
	bt_ready();


	bt_gatt_cb_register(&gatt_callbacks);
	bt_conn_auth_cb_register(&auth_cb_display);

	vnd_ind_attr = bt_gatt_find_by_uuid(vnd_svc.attrs, vnd_svc.attr_count,
					    &vnd_enc_uuid.uuid);
	bt_uuid_to_str(&vnd_enc_uuid.uuid, str, sizeof(str));
	printk("Indicate VND attr %p (UUID %s)\n", vnd_ind_attr, str);

	printf("about to initialize the sensor:\n");
	sensor_init();
	while (true) {
		k_sleep(K_SECONDS(1));
		printf("messga value: %s\n", msg);

		
		///////////////////
		/*
        sensor_sample_fetch(apds9960);
        sensor_channel_get(apds9960, SENSOR_CHAN_LIGHT, &apds9960_light);
        sensor_channel_get(apds9960, SENSOR_CHAN_PROX, &apds9960_prox);
        sensor_channel_get(apds9960, SENSOR_CHAN_RED, &apds9960_red);
        sensor_channel_get(apds9960, SENSOR_CHAN_GREEN, &apds9960_green);
        sensor_channel_get(apds9960, SENSOR_CHAN_BLUE, &apds9960_blue);
		sensor_value = apds9960_prox.val1;

        printk("light=%d prox=%d r=%d g=%d b=%d\n",
               apds9960_light.val1, apds9960_prox.val1, apds9960_red.val1, apds9960_green.val1, apds9960_blue.val1);

        k_sleep(K_MSEC(1000));
		*/
		///////////////////////////////////

		if (atomic_test_and_clear_bit(connection_flag_bits, DISCONNECTED_FLAG))
		{
			printf("TRIED TO ADVERTISE ???????????????????\n");
			start_advertising();
		}

		/* Vendor indication simulation */
		if (simulate_vnd && vnd_ind_attr) {
			if (indicating) {
				continue;
			}

			ind_params.attr = vnd_ind_attr;
			ind_params.func = indicate_cb;
			ind_params.destroy = indicate_destroy;
			ind_params.data = &indicating;
			ind_params.len = sizeof(indicating);

			if (bt_gatt_indicate(NULL, &ind_params) == 0) {
				indicating = 1U;
			}
		}

		sensor_value = (int16_t)apds9960_prox.val1; 

		if (sensor_notify_enabled) {
			bt_gatt_notify(NULL, sensor_attr, &sensor_value, sizeof(sensor_value));
		}
	}
	return 0;
}