/*!
 *
 *  \brief     The user device  extension API.
 *  \details   this file is part of the microlua ai project..
 *  \author    Hbs
 *  \version   1.1
 *  \date      2020-2020
 *  \copyright the GNU Affero General Public License
 */
#pragma pack(push)
#pragma pack(1)




//#include "lprefix.h"

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "modules.h"
#include "gpiohs.h"
#include "fpioa.h"
#include "assert.h"

DEV_HANDLE hUsrLEDIRHandle = NULL;
uint8_t LED_IR_IO  = 0;

void LED_IR(bool status) {
	if (status) {
		gpiohs_set_pin(LED_IR_IO, GPIO_PV_HIGH );
	} else {
		gpiohs_set_pin(LED_IR_IO, GPIO_PV_LOW);
	}
}
static int lua_ir(FUN_HANDLE hFunHandle) {
	bool status = getarg_toboolean(hFunHandle,2);
	LED_IR(status);
	return 0;
}
void openai_ledir_init(){
	const uint8_t LED_IR_PIN = 12;
	LED_IR_IO = AssignGPIOHS();
	fpioa_set_function(LED_IR_PIN,
			(fpioa_function_t) (FUNC_GPIOHS0 + LED_IR_IO));
	gpiohs_set_drive_mode(LED_IR_IO, GPIO_DM_OUTPUT);
	LED_IR(false);

	hUsrLEDIRHandle = addModule("ledir", NULL,NULL,NULL);
	assert(hUsrLEDIRHandle != NULL);
	addFuntion(hUsrLEDIRHandle,"ir",lua_ir);
}

#pragma pack(pop)
