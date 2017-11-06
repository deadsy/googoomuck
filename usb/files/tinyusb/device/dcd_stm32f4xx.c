//-----------------------------------------------------------------------------
/*

Device Controller Driver

*/
//-----------------------------------------------------------------------------

#include "dcd.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

tusb_error_t dcd_init(void) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

void dcd_isr(uint8_t coreid) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
}

//-----------------------------------------------------------------------------
// Controller API

#if 0
void dcd_controller_connect(uint8_t coreid);
void dcd_controller_disconnect(uint8_t coreid);
#endif

void dcd_controller_set_address(uint8_t coreid, uint8_t dev_addr) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
}

void dcd_controller_set_configuration(uint8_t coreid) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
}

//-----------------------------------------------------------------------------
// PIPE API
tusb_error_t dcd_pipe_control_xfer(uint8_t coreid, tusb_direction_t dir, uint8_t * p_buffer, uint16_t length, bool int_on_complete) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

void dcd_pipe_control_stall(uint8_t coreid) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
}

endpoint_handle_t dcd_pipe_open(uint8_t coreid, tusb_descriptor_endpoint_t const *p_endpoint_desc, uint8_t class_code) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return (endpoint_handle_t) {
	.coreid = 0,.index = 0,.class_code = 0};
}

tusb_error_t dcd_pipe_queue_xfer(endpoint_handle_t edpt_hdl, uint8_t * buffer, uint16_t total_bytes) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

tusb_error_t dcd_pipe_xfer(endpoint_handle_t edpt_hdl, uint8_t * buffer, uint16_t total_bytes, bool int_on_complete) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

tusb_error_t dcd_pipe_stall(endpoint_handle_t edpt_hdl) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

bool dcd_pipe_is_busy(endpoint_handle_t edpt_hdl) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return 0;
}

tusb_error_t dcd_pipe_clear_stall(uint8_t coreid, uint8_t edpt_addr) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

//-----------------------------------------------------------------------------
