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
#include <modules.h>
#include <math.h>
DEV_HANDLE hSysNNFaceDetectHandle = NULL;

#define ANCHOR_MAX 5
static float faceanchor[ANCHOR_MAX * 2]{1.889,2.5245,  2.9465,3.94056, 3.99987,5.3658, 5.155437,6.92275, 6.718375,9.01025};
//region_layer_init(20, 15, 30, 320, 240,faceanchor,ANCHOR_MAX);

LAYER_HANDLE gFaceLayerHandle = NULL;

//----------------------------------------------------------------------------------
static int net_nn_facedetect_tostring(FUN_HANDLE hFunHandle) {

	const char *szNetName = getNetworkName(hFunHandle);
	const char *szModName = getModuleName(hFunHandle);
	char szinfo[256] = { 0 };
	snprintf(szinfo, sizeof(szinfo), "NN ModName:%s, NetName:%s", szModName,
			szNetName);
	clear_toret(hFunHandle);
	setstring_toret(hFunHandle, szinfo, strlen(szinfo));
	return 1;
}

static int net_nn_facedetect_init(FUN_HANDLE hFunHandle) {

	gettable_enter(hFunHandle, 2);
	int floatnum = gettable_enter(hFunHandle, "anchor");
	LUA_ASSERT(hFunHandle, floatnum <= ANCHOR_MAX * 2,
			"Arg must be less than ANCHOR_MAX*2!");
	for (int i = 0; i < floatnum; i++) {
		faceanchor[i] = gettable_tofloat(hFunHandle, i + 1);
	}
	gettable_exit(hFunHandle);

    int arraynum = floatnum/2;
	if (arraynum > ANCHOR_MAX ) {
		gettable_exit(hFunHandle);
		return 0;
	}

	//float threshold = arg_table_tonumber(hFunHandle, 2, "threshold");
	//float nms_value = arg_table_tonumber(hFunHandle, 2, "nms_value");



	int width = gettable_tointeger(hFunHandle, "width");
	int height = gettable_tointeger(hFunHandle, "height");
	int channels = gettable_tointeger(hFunHandle, "channels");
	int origin_width = gettable_tointeger(hFunHandle, "origin_width");
	int origin_height = gettable_tointeger(hFunHandle, "origin_height");
	gettable_exit(hFunHandle);
	gFaceLayerHandle = InitLayer(width, height, channels,
			origin_width, origin_height, faceanchor, arraynum);
	assert(gFaceLayerHandle != NULL);
	ParamLayer(gFaceLayerHandle,0.7, 0.4);
	return 0;
}

//float *pred_box = NULL, *pred_landm = NULL, *pred_clses = NULL;
//size_t pred_box_size = 0, pred_landm_size = 0, pred_clses_size = 0;
//#define TEST 0

static int net_nn_facedetect_forward(FUN_HANDLE hFunHandle) {

	IMG_HANDLE arg_other = getarg_toimage(hFunHandle, 2);

	float threshold = getarg_tonumber(hFunHandle, 3,0.7,true);
	float nms_value = getarg_tonumber(hFunHandle, 4, 0.4,true);
	obj_info_t draw_boxs;
	memset(&draw_boxs, 0, sizeof(obj_info_t));
	float *output = NULL;
	size_t output_size = 0;
	//printf("KPU_Run \r\n");
	const char *szNetName = getNetworkName(hFunHandle);
	assert(szNetName != NULL);

	assert(KPU_Run(szNetName, arg_other) == 0);


	int ret = KPU_GetOutput(szNetName, 0, &output, output_size);

	if (ret == 0) {

		ParamLayer(gFaceLayerHandle,threshold,nms_value);
		RunLayer(gFaceLayerHandle,output,output_size, &draw_boxs);

		if (draw_boxs.obj_number <= 0) {
			return 0;
		}

		clear_toret(hFunHandle);
		settable_toret(hFunHandle);
		//-----------------------------------------------------------------------------
		setinteger_totable(hFunHandle, "number", draw_boxs.obj_number);
		//-----------------------------------------------------------------------------
		settable_enter(hFunHandle, "boxs");
		for (int i = 0; i < draw_boxs.obj_number; i++) {
			settable_enter(hFunHandle, i + 1);
			setinteger_totable(hFunHandle, 1, draw_boxs.obj[i].x1);
			setinteger_totable(hFunHandle, 2, draw_boxs.obj[i].y1);
			setinteger_totable(hFunHandle, 3, draw_boxs.obj[i].x2-draw_boxs.obj[i].x1);
			setinteger_totable(hFunHandle, 4, draw_boxs.obj[i].y2-draw_boxs.obj[i].y1);
			settable_exit(hFunHandle);
		}
		settable_exit(hFunHandle);
//		printf("number:%d x1:%d y1:%d x2:%d y2:%d\r\n", draw_boxs.obj_number,
//				draw_boxs.obj[0].x1, draw_boxs.obj[0].y1, draw_boxs.obj[0].x2,
//				draw_boxs.obj[0].y2);
		//----------------------------------------------------------------------------
		settable_enter(hFunHandle, "class");
		for (int i = 0; i < draw_boxs.obj_number; i++) {
			setinteger_totable(hFunHandle, i + 1, draw_boxs.obj[i].class_id+1);
		}
		settable_exit(hFunHandle);
		//----------------------------------------------------------------------------
		settable_enter(hFunHandle, "prob");
		for (int i = 0; i < draw_boxs.obj_number; i++) {
			setfloat_totable(hFunHandle, i + 1, draw_boxs.obj[i].prob);
		}
		settable_exit(hFunHandle);
		return 1;


	}
	return 0;
}

void module_nn_facedetect_init() {

	//strncpy(lua_nn_facedetect.szModName,)
	hSysNNFaceDetectHandle = addNNModule("facedetect",
			net_nn_facedetect_tostring, net_nn_facedetect_init,NULL);
	assert(hSysNNFaceDetectHandle != NULL);
	addFuntion(hSysNNFaceDetectHandle, "forward", net_nn_facedetect_forward);
}

#pragma pack(pop)
