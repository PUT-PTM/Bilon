
#ifndef	_MILLIS_MICROS_H
#define	_MILLIS_MICROS_H

#include "stm32f4xx.h"

void		init_millis_micros(void);
uint32_t	millis(void);
uint32_t	micros(void);



void	delay(uint32_t delay_ms);
void	delayMicroseconds(uint32_t delay_us);


#endif
