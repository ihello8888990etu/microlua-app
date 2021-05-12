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

uint8_t P_IO[13];


DEV_HANDLE hUsrGPIOHandle = NULL;

void P_CTRL(uint8_t pio,bool status) {
	//printf("pio %d\r\n",pio);
	assert( pio < 13 );
	if (status) {
		//fpioa_set_io_pull(LED_B_PIN, FPIOA_PULL_DOWN);
		//gpiohs_set_drive_mode(LED_B_IO, GPIO_DM_OUTPUT);
		gpiohs_set_pin(P_IO[pio], GPIO_PV_HIGH);
	} else {

		//fpioa_set_io_pull(LED_B_PIN, FPIOA_PULL_NONE);
		//gpiohs_set_drive_mode(LED_B_IO, GPIO_DM_INPUT);
		gpiohs_set_pin(P_IO[pio], GPIO_PV_LOW);
	}
}
static int lua_p_ctrl(FUN_HANDLE hFunHandle) {
	uint8_t pio = getarg_tointeger(hFunHandle,2);
	//printf("lua_p_ctrl %d\r\n",pio);
	bool status = getarg_toboolean(hFunHandle,3);
	P_CTRL(pio,status);
	return 0;
}
static int openai_gpio_new(FUN_HANDLE hFunHandle){
	const uint8_t P0_PIN = 20;
	const uint8_t P1_PIN = 19;
	const uint8_t P2_PIN = 18;
	const uint8_t P3_PIN = 17;
	const uint8_t P4_PIN = 15;
	const uint8_t P5_PIN = 14;
	const uint8_t P6_PIN = 13;
	const uint8_t P7_PIN = 26;
	const uint8_t P8_PIN = 25;
	const uint8_t P9_PIN = 24;
	const uint8_t P10_PIN = 23;
	const uint8_t P11_PIN = 22;
	const uint8_t P12_PIN = 21;

	for (int i = 0; i < 13; i++) {
		//printf("ddd i = %d\r\n",i);
		P_IO[i] = AssignGPIOHS();
		//printf("eee i = %d %d\r\n",i,P_IO[i]);
	}

	fpioa_set_function(P0_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[0]));
	fpioa_set_function(P1_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[1]));
	fpioa_set_function(P2_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[2]));
	fpioa_set_function(P3_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[3]));
	fpioa_set_function(P4_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[4]));
	fpioa_set_function(P5_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[5]));
	fpioa_set_function(P6_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[6]));
	fpioa_set_function(P7_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[7]));
	fpioa_set_function(P8_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[8]));
	fpioa_set_function(P9_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[9]));
	fpioa_set_function(P10_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[10]));
	fpioa_set_function(P11_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[11]));
	fpioa_set_function(P12_PIN, (fpioa_function_t) (FUNC_GPIOHS0 + P_IO[12]));

	for (int k = 0; k < 13; k++) {
		//printf("----------------\r\n");
		//printf("fff k = %d\r\n",k);
		gpiohs_set_drive_mode(P_IO[k], GPIO_DM_OUTPUT);
	}
	return 0;
}
void openai_gpio_init(){


	hUsrGPIOHandle = addModule("openaigpio", NULL,openai_gpio_new,NULL);
	assert(hUsrGPIOHandle != NULL);
	addFuntion(hUsrGPIOHandle,"ctrl",lua_p_ctrl);

}

#pragma pack(pop)
