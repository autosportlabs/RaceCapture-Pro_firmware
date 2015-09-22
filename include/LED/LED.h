#ifndef LED_H_
#define LED_H_

#define LED_ERROR 3
int LED_init(void);
void LED_enable(unsigned int Led);
void LED_disable(unsigned int Led);
void LED_toggle(unsigned int Led);

#endif /* LED_H_ */
