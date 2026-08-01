#include <firmware_update.h>
#include <stdio.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/toolchain.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

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

void transfer_firmware(){

    // get the payload size
    
}