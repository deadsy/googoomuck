//-----------------------------------------------------------------------------
/*

Host Controller Driver

*/
//-----------------------------------------------------------------------------

#include "hcd.h"

#include "stm32f4xx_hal_hcd.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_cortex.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

static HCD_HandleTypeDef hhcd;

//-----------------------------------------------------------------------------

// Hook up the OTG full speed interrupt.
// Call back to the HAL with the driver state.
void OTG_FS_IRQHandler(void) {
	HAL_HCD_IRQHandler(&hhcd);
}

//-----------------------------------------------------------------------------

// This is a callback from the HAL_HCD_Init() call to do target specific setup.
void HAL_HCD_MspInit(HCD_HandleTypeDef * hhcd) {
	GPIO_InitTypeDef GPIO_InitStruct;

	// IO Pins used
	// PA9 VBUS_FS
	// PA10 OTG_FS_ID
	// PA11 OTG_FS_DM
	// PA12 OTG_FS_DP
	// PC0 OTG_FS_PowerSwitchOn (0=on,1=off)

	// Configure USB FS GPIOs
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	// Configure DM DP Pins
	GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// This for ID line debug
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// Configure Power Switch Vbus Pin
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	// Enable USB FS Clocks
	__HAL_RCC_USB_OTG_FS_CLK_ENABLE();

	// Set USBFS Interrupt to the lowest priority
	HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0);

	// Enable USBFS Interrupt
	HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

//-----------------------------------------------------------------------------
// Callbacks from the HAL host USB isr.

void HAL_HCD_SOF_Callback(HCD_HandleTypeDef * hhcd) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	//USBH_LL_IncTimer (hhcd->pData);
}

void HAL_HCD_Connect_Callback(HCD_HandleTypeDef * hhcd) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	//USBH_LL_Connect(hhcd->pData);
}

void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef * hhcd) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	//USBH_LL_Disconnect(hhcd->pData);
}

void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef * hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
}

//-----------------------------------------------------------------------------
// USBH-HCD API

tusb_error_t hcd_init(void) {
	HAL_StatusTypeDef rc;

	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);

	// Set the LL driver parameters
	hhcd.Instance = USB_OTG_FS;
	hhcd.Init.Host_channels = 11;
	hhcd.Init.dma_enable = 0;
	hhcd.Init.low_power_enable = 0;
	hhcd.Init.phy_itface = HCD_PHY_EMBEDDED;
	hhcd.Init.Sof_enable = 0;
	hhcd.Init.speed = HCD_SPEED_FULL;
	// Link the driver to the stack
	hhcd.pData = NULL;

	// Initialize the LL Driver
	rc = HAL_HCD_Init(&hhcd);
	return (rc == HAL_OK) ? TUSB_ERROR_NONE : TUSB_ERROR_FAILED;
}

void hcd_isr(uint8_t hostid) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
}

//-----------------------------------------------------------------------------
// PIPE API

tusb_error_t hcd_pipe_control_open(uint8_t dev_addr, uint8_t max_packet_size) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

tusb_error_t hcd_pipe_control_xfer(uint8_t dev_addr, tusb_control_request_t const *p_request, uint8_t data[]) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

tusb_error_t hcd_pipe_control_close(uint8_t dev_addr) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

pipe_handle_t hcd_pipe_open(uint8_t dev_addr, tusb_descriptor_endpoint_t const *endpoint_desc, uint8_t class_code) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return (pipe_handle_t) {
	.dev_addr = 0,.xfer_type = 0,.index = 0};
}

tusb_error_t hcd_pipe_queue_xfer(pipe_handle_t pipe_hdl, uint8_t buffer[], uint16_t total_bytes) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

tusb_error_t hcd_pipe_xfer(pipe_handle_t pipe_hdl, uint8_t buffer[], uint16_t total_bytes, bool int_on_complete) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

tusb_error_t hcd_pipe_close(pipe_handle_t pipe_hdl) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

bool hcd_pipe_is_busy(pipe_handle_t pipe_hdl) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return 0;
}

bool hcd_pipe_is_stalled(pipe_handle_t pipe_hdl) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return 0;
}

uint8_t hcd_pipe_get_endpoint_addr(pipe_handle_t pipe_hdl) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return 0;
}

tusb_error_t hcd_pipe_clear_stall(pipe_handle_t pipe_hdl) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return TUSB_ERROR_NONE;
}

#if 0
bool hcd_pipe_is_error(pipe_handle_t pipe_hdl) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return 0;
}
#endif

//-----------------------------------------------------------------------------
// PORT API

bool hcd_port_connect_status(uint8_t hostid) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return 0;
}

void hcd_port_reset(uint8_t hostid) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
}

tusb_speed_t hcd_port_speed_get(uint8_t hostid) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
	return (tusb_speed_t) TUSB_SPEED_FULL;
}

void hcd_port_unplug(uint8_t hostid) {
	DBG("%s: %s() line %d\r\n", __FILE__, __func__, __LINE__);
}

//-----------------------------------------------------------------------------
