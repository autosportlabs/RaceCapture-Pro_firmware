#ifndef _ITM_H_
#define _ITM_H_


/* Enable the ITM serial port */
static inline void itm_init(void)
{
    DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY |
                  DBGMCU_CR_DBG_STOP |
                  DBGMCU_CR_DBG_SLEEP |
                  DBGMCU_CR_TRACE_IOEN;
}

#endif
