#ifndef LED_DEVICE_H_
#define LED_DEVICE_H_

int LED_device_init(void);
void LED_device_enable(unsigned int Led);
void LED_device_disable(unsigned int Led);
void LED_device_toggle(unsigned int Led);

#endif /* LED_DEVICE_H_ */
