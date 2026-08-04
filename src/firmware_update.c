#include <firmware_update.h>


LOG_MODULE_REGISTER(update_firmware, LOG_LEVEL_INF);


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
bool payload_size_flag = false;

bool is_payload_size_flag_up(){
    if(payload_size_flag == true){
        payload_size_flag = false;
        return true;
    }
    return false;
}

ssize_t write_payload_size(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
    printf("Receiving a new payload size.\n"); // INF
    printf("Offset: %i, Lenght: %i.\n", offset, len); // INF
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(payload_size)) {
        printf("Data len + offset is too big for [payload_size] ."); // ERR
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
    printf("[payload_size] : %i.\n", payload_size); // INF
    if (payload_size <= 0) {
        printf("[payload_size] is too small."); // ERR
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }

    if(payload_size_flag == true)    printf("[payload_flag] is already true, meaning it hasn't"
                                "been correctly reset."); // ERR
    payload_size_flag = true;

	return len;
}


struct PayloadChunk payload_chunk;
struct PayloadChunkWrapper
{
	int32_t index;
	char data[CHUNK_DATA_SIZE];
    uint8_t len;
    bool payload_chunk_flag;
} ;

static struct PayloadChunkWrapper payload_chunk_wrapper = {.payload_chunk_flag = false};

bool is_payload_chunk_flag_up(){
    if(payload_chunk_wrapper.payload_chunk_flag == true){
        payload_chunk_wrapper.payload_chunk_flag = false;
        return true;
    }
    return false;
}

ssize_t write_payload_chunk(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
    printf("Receiving a new payload chunk.\n"); // INF
    printf("Offset: %i, Lenght: %i.\n", offset, len); // INF
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(payload_chunk)) {
        printf("Data len + offset is too big for [payload_chunk] ."); // ERR
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
    printf("[payload_chunk] index: %i, len: %i.\n", payload_chunk.index, payload_chunk.len); // INF

    payload_chunk_wrapper.index = payload_chunk.index;
    memcpy(payload_chunk_wrapper.data, payload_chunk.data, CHUNK_DATA_SIZE);
    payload_chunk_wrapper.len = payload_chunk.len;

    if(payload_chunk_wrapper.payload_chunk_flag == true)    printf("[payload_chunk_flag] is already true, meaning it hasn't"
                                "been correctly reset."); // ERR
    payload_chunk_wrapper.payload_chunk_flag = true;

	return len;
}


// payload chunk feedback notification -------------------------------------------------------------->

enum PayloadChunkFeedback{
	NO_FEEDBACK = 0,
	CHUNK_MISSED = 1,
	CHUNK_ACCEPTED = 2,
};
static uint8_t payload_chunk_feedback = NO_FEEDBACK;
static uint8_t payload_chunk_feedback_notify_enabled;

void payload_chunk_feedback_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	payload_chunk_feedback_notify_enabled = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

//TODO: check if payload_chunk_feedback_notify_enabled is true and do that for other notifications as well
void send_payload_chunk_feedback(enum PayloadChunkFeedback pc_fb, struct bt_gatt_attr *pc_feedback_attr){
    payload_chunk_feedback = pc_fb;
	bt_gatt_notify(NULL, pc_feedback_attr,  &payload_chunk_feedback, sizeof(payload_chunk_feedback));
}


void transfer_firmware(struct bt_gatt_attr *pc_feedback_attr){

    const struct flash_area *image_update_partition;
    int err;
    err = flash_area_open(PARTITION_ID(image_update_partition), &image_update_partition);
    if (err)
    {
        printf("failed to open image_update_partition during firmware transfer.\n");
        return;
    }
    else
    {
        printf("image_update_partition is open.\n");
    }
    flash_area_erase(image_update_partition, 0, PARTITION_SIZE(image_update_partition));

    // get the payload size
    while(true){
        if(is_payload_size_flag_up()){

            printf("[payload_size] has been received. %d", payload_size); // INF

            // Setup
            int32_t chunks_count = payload_size / CHUNK_DATA_SIZE + ((payload_size % CHUNK_DATA_SIZE ) == 0 ? 0 : 1);
            int32_t expected_chunk_index = 0;

            while(expected_chunk_index < chunks_count){
                // waiting for the chunk to arrive
                while (true)
                {
                    if(is_payload_chunk_flag_up()){
                        if(payload_chunk_wrapper.index == expected_chunk_index){
                            flash_area_write(image_update_partition, expected_chunk_index * CHUNK_DATA_SIZE,
                                             payload_chunk_wrapper.data, payload_chunk_wrapper.len);
                            send_payload_chunk_feedback(CHUNK_ACCEPTED, pc_feedback_attr);
                            expected_chunk_index++;
                        }
                        else{
                            send_payload_chunk_feedback(CHUNK_MISSED, pc_feedback_attr);
                        }
                        break;
                    }
                }
                
            }
            break;
        }
    }
    flash_area_close(image_update_partition);
    
}