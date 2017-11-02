//-----------------------------------------------------------------------------
/*

Logging Functions

*/
//-----------------------------------------------------------------------------

#include <SEGGER_RTT_Conf.h>
#include <SEGGER_RTT.h>

#include "logging.h"

//-----------------------------------------------------------------------------

int log_init(void) {
	SEGGER_RTT_Init();
	return 0;
}

//-----------------------------------------------------------------------------

int SEGGER_RTT_vprintf(unsigned BufferIndex, const char *sFormat, va_list * pParamList);

void log_printf(char *format_msg, ...) {
	va_list p_args;
	va_start(p_args, format_msg);
	(void)SEGGER_RTT_vprintf(0, format_msg, &p_args);
	va_end(p_args);
}

//-----------------------------------------------------------------------------
