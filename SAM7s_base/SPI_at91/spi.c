#include "spi.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

static xSemaphoreHandle spiLock;

void init_spi_lock()
{
    vSemaphoreCreateBinary(spiLock);
}
void lock_spi()
{
    xSemaphoreTake(spiLock, portMAX_DELAY);
}

void unlock_spi()
{
    xSemaphoreGive(spiLock);
}
