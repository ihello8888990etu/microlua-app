/*!
 *
 *  \brief     The user device  extension API.
 *  \details   this file is part of the microlua ai project..
 *  \author    Hbs
 *  \version   1.1
 *  \date      2020-2020
 *  \copyright the GNU Affero General Public License
 */
#include <bsp.h>
#include <sysctl.h>
#include "fpioa.h"
#include <string.h>
#include "uart.h"
#include "gpio.h"
#include "sysctl.h"
#include "modules.h"
#include <assert.h>


DEV_HANDLE hDevHelloHandle = NULL;
int helloword_fun(FUN_HANDLE hFunHandle) {
    printf("helloword_fun\r\n");
    return 0;
}
int helloword_saveimg(FUN_HANDLE hFunHandle) {
    printf("0helloword_saveimg\r\n");
    //IMG_HANDLE handle = arg_toimage(hFunHandle,2);
    SaveImage(NULL,"0:aa.txt");
    printf("1helloword_saveimg\r\n");
    return 0;
}
int helloword_event(FUN_HANDLE hFunHandle) {
    printf("helloword_fun\r\n");
    SendModuleEvent(hDevHelloHandle);
    return 0;
}

int helloword_tostring(FUN_HANDLE hFunHandle) {
    printf("helloword_tostring\r\n");
    char szinfo[256] = { 0 };
   	snprintf(szinfo, sizeof(szinfo), "%s", getModuleName(hFunHandle));
   	setstring_toret(hFunHandle, szinfo,strlen(szinfo));
    return 1;
}
int helloword_callback(FUN_HANDLE hFunHandle) {
    printf("helloword_callback\r\n");
    getarg_tocallback(hFunHandle,2);
    return 0;
}
void uartmodule_helloword_init() {
	hDevHelloHandle = addModule("hello",helloword_tostring,NULL,NULL);
	assert(hDevHelloHandle != NULL);
	addFuntion(hDevHelloHandle,"helloword",helloword_fun);
	addFuntion(hDevHelloHandle,"saveimg",helloword_saveimg);
	addFuntion(hDevHelloHandle,"sendevent",helloword_event);
	addFuntion(hDevHelloHandle,"callback",helloword_callback);
}

