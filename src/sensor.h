#ifndef SENSOR_H
#define SENSOR_H

void sensor_init(void);

extern struct sensor_value apds9960_light, apds9960_prox, apds9960_red, apds9960_green, apds9960_blue;
extern struct k_work_q sensor_workq;
extern struct k_work_delayable sensor_work;
#endif