#ifndef FIRMWARE_UPDATE_H
#define FIRMWARE_UPDATE_H
#include <zephyr/storage/flash_map.h>
int copy_update_partition();
void swap_A_B_partitions();

#endif