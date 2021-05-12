/*!
 *
 *  \brief     The user device  extension API.
 *  \details   this file is part of the microlua ai project..
 *  \author    Hbs
 *  \version   1.1
 *  \date      2020-2020
 *  \copyright the GNU Affero General Public License
 */

#include <boarddef.h>
#include <string.h>
#include <wdt.h>
#include <bsp.h>
#include <sysctl.h>
#include "uarths.h"
#include "uart.h"
#include "plic.h"
#include "fpioa.h"
#include "assert.h"


extern void uartmodule_helloword_init() ;
extern void module_nn_facedetect_init();
extern void module_nn_yolo_init();
extern void openai_ledir_init();
extern void openai_gpio_init();
extern void openai_ledrgb_init();

#include <iostream>
#include <exception>
using namespace std;
#include <modules.h>


void initCustomDevice()
{
	printf("--initCustomDevice--\r\n");
	openai_ledrgb_init();
	openai_ledir_init();
	openai_gpio_init();
	uartmodule_helloword_init();
	module_nn_facedetect_init();
	module_nn_yolo_init();
}



int main(void) {


    bool bInitRes = false;

#if OPENAI_V22
	uint8_t debug_tx = 19, debug_rx = 20;//这个可以根据需要更
	//uint8_t debug_tx = 20, debug_rx = 19;//这个可以根据需要更
#else
	uint8_t debug_tx = 6, debug_rx = 7;//这个固定不变,V26以后的板子有固定调试串口
#endif

	bInitRes = SysInit(OPENAI_K210,debug_tx,debug_rx);
    assert(bInitRes == true);
	initCustomDevice();
	return SysRun();
}

