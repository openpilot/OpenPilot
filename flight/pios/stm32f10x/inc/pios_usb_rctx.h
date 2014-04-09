#ifndef PIOS_USB_RCTX_H
#define PIOS_USB_RCTX_H

extern uint32_t pios_usb_rctx_id;

extern void PIOS_USB_RCTX_Update(uint32_t usbrctx_id, const uint16_t channel[], const int16_t channel_min[], const int16_t channel_max[], uint8_t num_channels);

#endif /* PIOS_USB_RCTX_H */
