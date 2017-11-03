//-----------------------------------------------------------------------------
/*

*/
//-----------------------------------------------------------------------------

#include "hcd.h"

//-----------------------------------------------------------------------------
// USBH-HCD API

tusb_error_t hcd_init(void) {
	return TUSB_ERROR_NONE;
}

void hcd_isr(uint8_t hostid) {
}

//-----------------------------------------------------------------------------
// PIPE API

tusb_error_t hcd_pipe_control_open(uint8_t dev_addr, uint8_t max_packet_size) {
	return TUSB_ERROR_NONE;
}

tusb_error_t hcd_pipe_control_xfer(uint8_t dev_addr, tusb_control_request_t const *p_request, uint8_t data[]) {
	return TUSB_ERROR_NONE;
}

tusb_error_t hcd_pipe_control_close(uint8_t dev_addr) {
	return TUSB_ERROR_NONE;
}

pipe_handle_t hcd_pipe_open(uint8_t dev_addr, tusb_descriptor_endpoint_t const *endpoint_desc, uint8_t class_code) {
	return (pipe_handle_t) {
	.dev_addr = 0,.xfer_type = 0,.index = 0};
}

tusb_error_t hcd_pipe_queue_xfer(pipe_handle_t pipe_hdl, uint8_t buffer[], uint16_t total_bytes) {
	return TUSB_ERROR_NONE;
}

tusb_error_t hcd_pipe_xfer(pipe_handle_t pipe_hdl, uint8_t buffer[], uint16_t total_bytes, bool int_on_complete) {
	return TUSB_ERROR_NONE;
}

tusb_error_t hcd_pipe_close(pipe_handle_t pipe_hdl) {
	return TUSB_ERROR_NONE;
}

bool hcd_pipe_is_busy(pipe_handle_t pipe_hdl) {
	return 0;
}

bool hcd_pipe_is_stalled(pipe_handle_t pipe_hdl) {
	return 0;
}

uint8_t hcd_pipe_get_endpoint_addr(pipe_handle_t pipe_hdl) {
	return 0;
}

tusb_error_t hcd_pipe_clear_stall(pipe_handle_t pipe_hdl) {
	return TUSB_ERROR_NONE;
}

#if 0
bool hcd_pipe_is_error(pipe_handle_t pipe_hdl) {
	return 0;
}
#endif

//-----------------------------------------------------------------------------
// PORT API

bool hcd_port_connect_status(uint8_t hostid) {
	return 0;
}

void hcd_port_reset(uint8_t hostid) {
}

tusb_speed_t hcd_port_speed_get(uint8_t hostid) {
	return (tusb_speed_t) TUSB_SPEED_FULL;
}

void hcd_port_unplug(uint8_t hostid) {
}

//-----------------------------------------------------------------------------
