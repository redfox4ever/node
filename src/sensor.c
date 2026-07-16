#include <sensor.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(my_module, LOG_LEVEL_INF);

const struct device *apds9960 = DEVICE_DT_GET_ANY(avago_apds9960);
struct sensor_value apds9960_light, apds9960_prox, apds9960_red, apds9960_green, apds9960_blue;

#define SENSOR_WORKQ_STACK_SIZE 4096 
#define SENSOR_WORKQ_PRIO 5
struct k_work_q sensor_workq;
struct k_work_delayable sensor_work;

K_THREAD_STACK_DEFINE(sensor_workq_stack, SENSOR_WORKQ_STACK_SIZE);

static void sensor_work_handler(struct k_work *work) {
    int err = 0;

    sensor_sample_fetch(apds9960);

    if (0 != err) {
        LOG_ERR("Failed to fetch sensor reading: %d", err);
        return;
    }


    sensor_channel_get(apds9960, SENSOR_CHAN_LIGHT, &apds9960_light);
    err = sensor_channel_get(apds9960, SENSOR_CHAN_PROX, &apds9960_prox);
    sensor_channel_get(apds9960, SENSOR_CHAN_RED, &apds9960_red);
    sensor_channel_get(apds9960, SENSOR_CHAN_GREEN, &apds9960_green);
    sensor_channel_get(apds9960, SENSOR_CHAN_BLUE, &apds9960_blue);

    if (0 != err) {
        LOG_ERR("Failed to get sensor value: %d", err);
        return;
    }

    LOG_INF("Proximity reading: %d", apds9960_prox.val1);
    err = k_work_reschedule_for_queue(	&sensor_workq,
                                        &sensor_work,
                                        K_MSEC(1000) );

}

void sensor_init(void) {
    printk("APDS9960 is ready\n");
    if (!device_is_ready(apds9960)) {
        printk("APDS9960 not ready\n");
        return ;
    }
    printk("APDS9960 is ready\n");
    k_work_queue_start(&sensor_workq,
                       sensor_workq_stack,
                       K_THREAD_STACK_SIZEOF(sensor_workq_stack),
                       SENSOR_WORKQ_PRIO,
                       NULL);
    k_work_init_delayable(&sensor_work, sensor_work_handler);

    int err = 0;

    sensor_sample_fetch(apds9960);

    if (0 != err) {
        LOG_ERR("Failed to fetch sensor reading: %d", err);
        return;
    }


    sensor_channel_get(apds9960, SENSOR_CHAN_LIGHT, &apds9960_light);
    err = sensor_channel_get(apds9960, SENSOR_CHAN_PROX, &apds9960_prox);
    sensor_channel_get(apds9960, SENSOR_CHAN_RED, &apds9960_red);
    sensor_channel_get(apds9960, SENSOR_CHAN_GREEN, &apds9960_green);
    sensor_channel_get(apds9960, SENSOR_CHAN_BLUE, &apds9960_blue);

    if (0 != err) {
        LOG_ERR("Failed to get sensor value: %d", err);
        return;
    }

    LOG_INF("Proximity reading: %d", apds9960_prox.val1);

    k_work_schedule_for_queue(&sensor_workq, &sensor_work, K_MSEC(0));
}