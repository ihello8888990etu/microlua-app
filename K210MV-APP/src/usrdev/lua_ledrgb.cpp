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
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "modules.h"
#include "gpiohs.h"
#include <assert.h>
#include "fpioa.h"

uint8_t LED_R_IO = 0; //= 14;
uint8_t LED_G_IO = 0; //= 15;
uint8_t LED_B_IO = 0; //= 16;

DEV_HANDLE hUsrLEDRGBHandle = NULL;
//static int lua_ledrgb_gc(FUN_HANDLE hFunHandle) {
//	freeHandle(hFunHandle);
//	return 0;
//}

void LED_RED(bool status) {
	if (status) {

		//對方是3.3V，所以不用上拉
		//fpioa_set_io_pull(LED_R_PIN, FPIOA_PULL_NONE);
		//gpiohs_set_drive_mode(LED_R_IO, GPIO_DM_INPUT);
		gpiohs_set_pin(LED_R_IO, GPIO_PV_LOW);
	} else {
		//fpioa_set_io_pull(LED_R_PIN, FPIOA_PULL_DOWN);
		//gpiohs_set_drive_mode(LED_R_IO, GPIO_DM_OUTPUT);
		gpiohs_set_pin(LED_R_IO, GPIO_PV_HIGH);
	}

}
void LED_GREEN(bool status) {
	if (status) {
		//fpioa_set_io_pull(LED_G_PIN, FPIOA_PULL_DOWN);
		//gpiohs_set_drive_mode(LED_G_IO, GPIO_DM_OUTPUT);
		gpiohs_set_pin(LED_G_IO, GPIO_PV_LOW);
	} else {
		//對方是3.3V，所以不用上拉
		//fpioa_set_io_pull(LED_G_PIN, FPIOA_PULL_NONE);
		//gpiohs_set_drive_mode(LED_G_IO, GPIO_DM_INPUT);
		gpiohs_set_pin(LED_G_IO, GPIO_PV_HIGH);
	}
}
void LED_BLUE(bool status) {
	if (status) {
		//fpioa_set_io_pull(LED_B_PIN, FPIOA_PULL_DOWN);
		//gpiohs_set_drive_mode(LED_B_IO, GPIO_DM_OUTPUT);
		gpiohs_set_pin(LED_B_IO, GPIO_PV_LOW);
	} else {
		//對方是3.3V，所以不用上拉
		//fpioa_set_io_pull(LED_B_PIN, FPIOA_PULL_NONE);
		//gpiohs_set_drive_mode(LED_B_IO, GPIO_DM_INPUT);
		gpiohs_set_pin(LED_B_IO, GPIO_PV_HIGH);
	}
}
static int lua_ledrgb_red(FUN_HANDLE hFunHandle) {
	bool status = getarg_toboolean(hFunHandle,2);
	LED_RED(status);
	return 0;
}
static int lua_ledrgb_green(FUN_HANDLE hFunHandle) {
	bool status = getarg_toboolean(hFunHandle,2);

	LED_GREEN(status);
	return 0;
}
static int lua_ledrgb_blue(FUN_HANDLE hFunHandle) {
	bool status = getarg_toboolean(hFunHandle,2);

	LED_BLUE(status);
	return 0;
}

void openai_ledrgb_init(){
	const uint8_t LED_B_PIN = 29;
	const uint8_t LED_G_PIN = 30;
	const uint8_t LED_R_PIN = 31;
	LED_R_IO = AssignGPIOHS(); //14;
	LED_G_IO = AssignGPIOHS(); //15;
	LED_B_IO = AssignGPIOHS(); //16;
	printf("RGB IO %d %d %d \r\n",LED_R_IO,LED_G_IO,LED_B_IO);
	fpioa_set_function(LED_R_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + LED_R_IO));
	fpioa_set_function(LED_G_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + LED_G_IO));
	fpioa_set_function(LED_B_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + LED_B_IO));
	gpiohs_set_drive_mode(LED_R_IO, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(LED_G_IO, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(LED_B_IO, GPIO_DM_OUTPUT);
	LED_RED(false);
	LED_GREEN(false);
	LED_BLUE(false);
	hUsrLEDRGBHandle = addModule("ledrgb", NULL,NULL,NULL);
	assert(hUsrLEDRGBHandle != NULL);
	addFuntion(hUsrLEDRGBHandle,"red",lua_ledrgb_red);
	addFuntion(hUsrLEDRGBHandle,"green",lua_ledrgb_green);
	addFuntion(hUsrLEDRGBHandle,"blue",lua_ledrgb_blue);
}

#pragma pack(pop)
