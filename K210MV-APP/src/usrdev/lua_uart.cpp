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

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "modules.h"
#include "uart.h"
#include <assert.h>

typedef struct lua_uart_obj {
	int uartid;
	int baudrate;
	int bits;
	int parity;
	int stop;
	char revbuf[1024];
	int revcnt;
} lua_uart_t;

DEV_HANDLE hDevUartHandle ;

lua_uart_t uart_data;

static int lua_uart_tostring(FUN_HANDLE hFunHandle) {
	lua_uart_t *arg_uart = (lua_uart_t*) getUsrData(hFunHandle);
	char szinfo[256] = { 0 };
	snprintf(szinfo, sizeof(szinfo),
			"id:%d baudrate:%d bits:%d parity:%d stop:%d \n",
			arg_uart->uartid, arg_uart->baudrate, arg_uart->bits,
			arg_uart->parity, arg_uart->stop);
	clear_toret(hFunHandle);
	setstring_toret(hFunHandle, szinfo,strlen(szinfo));
	return 1;
}

int uart_recv_irq(void *ctx) {

	SendModuleEvent(hDevUartHandle);
	return 0;
}
static int lua_uart_init(FUN_HANDLE hFunHandle) {
	lua_uart_t *arg_uart = (lua_uart_t*) getUsrData(hFunHandle);
	arg_uart->baudrate = getarg_tointeger(hFunHandle, 2, 9600);
	arg_uart->bits = getarg_tointeger(hFunHandle, 3, 8);
	LUA_ASSERT(hFunHandle,
			(7 == arg_uart->bits || 8 == arg_uart->bits || 9 == arg_uart->bits),
			"bits must be equal to 7,8,9!");
	arg_uart->parity = getarg_tointeger(hFunHandle, 4, 0);
	LUA_ASSERT(hFunHandle, (0 == arg_uart->parity || 1 == arg_uart->parity),
			"parity must be equal to 0,1!");
	arg_uart->stop = getarg_tointeger(hFunHandle, 5, 1);
	LUA_ASSERT(hFunHandle, (2 == arg_uart->stop || 1 == arg_uart->stop),
			"stop must be equal to 1,2!");
	assert(arg_uart->uartid == UART_DEVICE_1);

	uart_configure(UART_DEVICE_1, arg_uart->baudrate,
			(uart_bitwidth_t) arg_uart->bits, (uart_stopbit_t) arg_uart->stop,
			(uart_parity_t) arg_uart->parity);
	getarg_tocallback(hFunHandle, 2);
	uart_set_receive_trigger(UART_DEVICE_1, UART_RECEIVE_FIFO_4); //UART_RECEIVE_FIFO_8);

	return 0;
}

static int lua_uart_deinit(FUN_HANDLE hFunHandle) {
	lua_uart_t *arg_uart = (lua_uart_t*) getUsrData(hFunHandle);
	assert(arg_uart->uartid == UART_DEVICE_1);
	uart_irq_unregister(UART_DEVICE_1, UART_RECEIVE);
	return 0;
}
static int lua_uart_read(FUN_HANDLE hFunHandle) {
	lua_uart_t *arg_uart = (lua_uart_t*) getUsrData(hFunHandle);

	int nbytes = getarg_tointeger(hFunHandle, 2, -1,true);
	LUA_ASSERT(hFunHandle, (nbytes <= sizeof(arg_uart->revbuf) && nbytes > 0), //sizeof(arg_uart->revbuf is 1024
			" nbytes are invalid !");

	int ret = 0;
	//uint64_t t_start = GetSysTick();
	arg_uart->revcnt = 0;

	if (nbytes < 0) {
		nbytes = sizeof(arg_uart->revbuf);
	}

	ret = uart_receive_data(UART_DEVICE_1, (char*) arg_uart->revbuf, nbytes);
	if (ret <= 0) {
		return 0;
	}
	clear_toret(hFunHandle);
	setstring_toret(hFunHandle, (const char*) arg_uart->revbuf, ret);
	return 1;
}

static int lua_uart_write(FUN_HANDLE hFunHandle) {
	lua_uart_t *arg_uart = (lua_uart_t*) getUsrData(hFunHandle);
	int ret = 0;
	if (arg_isinteger(hFunHandle, 2)) {
		int numbers = getarg_tointeger(hFunHandle, 2);
		LUA_ASSERT(hFunHandle, (numbers <= 255 && numbers >= 0),
				"numbers are invalid !");
		ret = uart_send_data(UART_DEVICE_1, (const char*) &numbers, 1);
		clear_toret(hFunHandle);
		setinteger_toret(hFunHandle, ret);
		return 1;
	}else
	if (arg_isstring(hFunHandle, 2)) {
		size_t len = 0;
		const char *buf = getarg_tostring(hFunHandle, 2, len);
		assert(buf != NULL);
		assert(len > 0);
		ret = uart_send_data(UART_DEVICE_1, (const char*) buf, len);
		clear_toret(hFunHandle);
		setinteger_toret(hFunHandle, ret);
	}else{
		LUA_ASSERT(hFunHandle, 0,
						"arg #2  invalid !");
	}
	return 1;
}
static int lua_uart_any(FUN_HANDLE hFunHandle) {
	lua_uart_t *arg_uart = (lua_uart_t*) getUsrData(hFunHandle);

	int ret = 0;
	do {
		ret = uart_receive_data(UART_DEVICE_1, (char*) arg_uart->revbuf,
				sizeof(arg_uart->revbuf));
	} while (ret > 0);
	clear_toret(hFunHandle);
	setstring_toret(hFunHandle, (const char*) arg_uart->revbuf, ret);
	return 1;
}
void uartmodule_uart_init(){
	char tmp[32];
	//------------------------------------------------------------------------
	uart_init(UART_DEVICE_1);
	memset(&uart_data, 0, sizeof(lua_uart_t));
	hDevUartHandle = NULL;
	uart_data.uartid = UART_DEVICE_1;
	uart_irq_register(UART_DEVICE_1, UART_RECEIVE,
			(plic_irq_callback_t) uart_recv_irq, &uart_data.uartid, 2);
	snprintf(tmp, sizeof(tmp), "usr-uart%d", 1);
	hDevUartHandle = addModule(tmp, lua_uart_tostring,NULL, &uart_data);
	assert(hDevUartHandle != NULL);
	addFuntion(hDevUartHandle, "init", lua_uart_init);
	addFuntion(hDevUartHandle, "deinit", lua_uart_deinit);
	addFuntion(hDevUartHandle, "read", lua_uart_read);
	addFuntion(hDevUartHandle, "write", lua_uart_write);
	addFuntion(hDevUartHandle, "any", lua_uart_any);

}
#pragma pack(pop)
