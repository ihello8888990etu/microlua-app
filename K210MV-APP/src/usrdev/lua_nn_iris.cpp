/*
 * lua_nn_class.c
 *
 *  Created on: 2019年3月30日
 *      Author: Administrator
 */
/*
 ** $Id: loslib.c,v 1.65.1.1 2017/04/19 17:29:57 roberto Exp $
 ** Standard Operating System library
 ** See Copyright Notice in lua.h
 */
#pragma pack(push)
#pragma pack(1)

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <modules.h>
#include <assert.h>

DEV_HANDLE hUsrNNIrisHandle = NULL;
typedef struct {
    int test;
}lua_nn_iris_t;
lua_nn_iris_t lua_nn_iris;
//const char *labels[] = { "setosa", "versicolor", "virginica" };
//const char *labels[] = { "Silky", "versicolor", "virginica" };
static int net_nn_iris_tostring(FUN_HANDLE hFunHandle) {
	lua_nn_iris_t *self = (lua_nn_iris_t*) getUsrData(hFunHandle);
	const char *szNetName = getNetworkName(hFunHandle);
	const char *szModName = getModuleName(hFunHandle);
	char szinfo[256] = { 0 };
	snprintf(szinfo, sizeof(szinfo), "NN ModName:%s, NetName:%s test:%d", szModName,
			szNetName,self->test);
	clear_toret(hFunHandle);
	setstring_toret(hFunHandle, szinfo, strlen(szinfo));
	return 1;
}
static int net_nn_iris_forward(FUN_HANDLE hFunHandle) {
	lua_nn_iris_t *self = (lua_nn_iris_t*) getUsrData(hFunHandle);
	size_t len = 0;
	const char *netstr = getarg_tostring(hFunHandle, 2, len);
	assert(len > 0);
	float *aa = (float*) netstr;
	float *output = NULL;
	printf("net_nn_iris_forward len:%d %f %f %f %f\r\n", len, aa[0], aa[1],
			aa[2], aa[3]);
	size_t output_size = 0;
	const char *szNetName = getNetworkName(hFunHandle);
	assert(KPU_Run(szNetName, (uint8_t* )netstr, len) == 0);
	int ret = KPU_GetOutput(szNetName, 0, &output, output_size);
	//printf("face cnt:%d\r\n", cnt);
	if (ret == 0) {
		printf("iris cnt:%d\r\n", output_size);
		clear_toret(hFunHandle);
		assert(output != NULL);
		assert(output_size > 0);
		settable_toret(hFunHandle);
		for (int i = 0; i < output_size / 4; i++) {
			setfloat_totable(hFunHandle, i + 1, output[i]);
		}
		return 1;
	}
	return 0;
}
void module_nn_iris_init() {

	hUsrNNIrisHandle = addNNModule("iris", net_nn_iris_tostring, (module_Function)NULL,&lua_nn_iris);
	assert(hUsrNNIrisHandle != NULL);
	addFuntion(hUsrNNIrisHandle, "forward", net_nn_iris_forward);
}

#pragma pack(pop)
