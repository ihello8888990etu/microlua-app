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
#include <assert.h>
#include <incbin.h>
#include <modules.h>

DEV_HANDLE hUsrNNBaiduFlowerHandle = NULL;



//const char *labels[] = { "daisy", "dandelion", "roses","sunflowers","tulip" };
static int net_nn_baiduflower_forward(FUN_HANDLE hFunHandle) {

	IMG_HANDLE arg_other = getarg_toimage(hFunHandle, 2);

	float *output = NULL;
	size_t output_size = 0;
	const char *szNetName = getNetworkName(hFunHandle);
	assert(KPU_Run(szNetName, arg_other) == 0);
	int ret = KPU_GetOutput(szNetName, 0, &output, output_size);
	//printf("face cnt:%d\r\n", cnt);
	if (ret == 0) {
		printf("flower cnt:%d\r\n", output_size);
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

static int net_nn_baiduflower_tostring(FUN_HANDLE hFunHandle) {

	const char *szNetName = getNetworkName(hFunHandle);
	const char *szModName = getModuleName(hFunHandle);
	char szinfo[256] = { 0 };
	snprintf(szinfo, sizeof(szinfo), "NN ModName:%s, NetName:%s", szModName,
			szNetName);
	clear_toret(hFunHandle);
	setstring_toret(hFunHandle, szinfo, strlen(szinfo));
	return 1;
}

void module_nn_baiduflower_init() {

	hUsrNNBaiduFlowerHandle = addNNModule("baidu", net_nn_baiduflower_tostring,NULL,
			NULL);
	assert(hUsrNNBaiduFlowerHandle != NULL);
	addFuntion(hUsrNNBaiduFlowerHandle, "forward", net_nn_baiduflower_forward);

}

#pragma pack(pop)
