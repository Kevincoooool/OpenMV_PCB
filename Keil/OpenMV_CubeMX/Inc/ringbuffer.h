/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

/* Includes ------------------------------------------------------------------*/
#include "bsp_mcu.h"

typedef struct ring_buffer 
{
	uint16_t read_mirror : 1;
	uint16_t read_index : 15;
	uint16_t write_mirror : 1;
	uint16_t write_index : 15;
	uint16_t buffer_size;
	uint8_t *buffer_ptr;
} ring_buffer_t;

enum ringbuffer_state
{
    RINGBUFFER_EMPTY,
    RINGBUFFER_FULL,
    /* half full is neither full nor empty */
    RINGBUFFER_HALFFULL,
};

void ringbuffer_init(ring_buffer_t *rb , uint8_t *pool , uint16_t size);
void ringbuffer_reset(ring_buffer_t *rb);
uint16_t ringbuffer_data_len(ring_buffer_t *rb);

uint16_t ringbuffer_put(ring_buffer_t *rb, const uint8_t *ptr, uint16_t length);
uint16_t ringbuffer_put_force(ring_buffer_t *rb, const uint8_t *ptr, uint16_t length);
uint16_t ringbuffer_putchar(ring_buffer_t *rb, const uint8_t ch);
uint16_t ringbuffer_putchar_force(ring_buffer_t *rb, const uint8_t ch);

uint16_t ringbuffer_get(ring_buffer_t *rb, uint8_t *ptr, uint16_t length);
uint16_t ringbuffer_getchar(ring_buffer_t *rb, uint8_t *ch);

__inline uint16_t rt_ringbuffer_get_size(ring_buffer_t *rb)
{
    return rb->buffer_size;
}

/** return the size of empty space in rb */
#define ringbuffer_space_len(rb) ((rb)->buffer_size - ringbuffer_data_len(rb))


#endif /* __RING_BUFFER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
