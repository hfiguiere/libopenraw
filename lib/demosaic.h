


#ifndef __DEMOSAIC_H_
#define __DEMOSAIC_H_


void
bimedian_demosaic (uint16_t *src, uint32_t src_x, uint32_t src_y, 
		   or_cfa_pattern pattern, uint8_t *dst);

#endif
