#include <firmware_update.h>


LOG_MODULE_REGISTER(firmware_update, LOG_LEVEL_INF);


#define TRANSFER_BUFF_SIZE 256
int copy_update_partition(){
    const struct flash_area *image_update_partition;
    const struct flash_area *slot1_partition;


    int err;
    err = flash_area_open(PARTITION_ID(image_update_partition), &image_update_partition);
    if (err)
    {
        printf("failed to open image_update_partition\n");
        return 1;
    }
    else
    {
        printf("image_update_partition is open.\n");
    }
    
    err = flash_area_open(PARTITION_ID(slot1_partition), &slot1_partition);

        if (err)
    {
        printf("failed to open slot1_partition\n");
        return 1;
    }
    else
    {
        printf("slot1_partition is open.\n");
    }

    flash_area_erase(slot1_partition, 0, PARTITION_SIZE(slot1_partition));

    char transfer_buff[TRANSFER_BUFF_SIZE];
    int upper_limit = MIN(PARTITION_SIZE(image_update_partition), PARTITION_SIZE(slot1_partition));
    int pointer = 0;
    while(pointer < upper_limit){
        int chunk_size = MIN(TRANSFER_BUFF_SIZE, upper_limit - pointer);
        flash_area_read(image_update_partition, pointer, transfer_buff, chunk_size);
        flash_area_write(slot1_partition, pointer, transfer_buff, chunk_size);
        pointer += chunk_size;
    }
    


    flash_area_close(slot1_partition);
    flash_area_close(image_update_partition);
    return 0;
}
void swap_A_B_partitions(){
    boot_request_upgrade(BOOT_UPGRADE_TEST);
    k_sleep(K_SECONDS(5));
    sys_reboot(SYS_REBOOT_WARM);
}




int32_t payload_size;
bool payload_flag = false;

bool is_payload_flag_up(){
    if(payload_flag == true){
        payload_flag = false;
        return true;
    }
    return false;
}

ssize_t write_payload_size(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
    LOG_INF("Receiving a new payload size.\n");
    LOG_INF("Offset: %i, Lenght: %i.\n", offset, len);
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(payload_size)) {
        LOG_ERR("Data len + offset is too big for [payload_size] .");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
    LOG_INF("[payload_size] : %i.\n", payload_size);
    if (payload_size <= 0) {
        LOG_ERR("[payload_size] is too small.");
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }

    if(payload_flag == true)    LOG_ERR("[payload_flag] is already true, meaning it hasn't"
                                "been correctly reset.");
    payload_flag = true;

	return len;
}




void transfer_firmware(){

    // get the payload size
    while(true){
        if(is_payload_flag_up()){
            LOG_INF("LOG_INF output [payload_size] has been received. %d\n", payload_size);
            printf(" printf output [payload_size] has been received. %d\n", payload_size);
        }
    }
    
}