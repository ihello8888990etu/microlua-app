/*
 * modules.h
 *
 *  Created on: 2020年8月25日
 *      Author: Administrator
 */

#ifndef MODULES_H_
#define MODULES_H_
#include<stdio.h>
#define DEV_HANDLE void*
#define FUN_HANDLE void*
typedef enum {
	AIRV_BITK210 = 0,
	OPENAI_K210,
	MICRO_CAM
}BoardType;
//----------------------------------------------------------------------
bool SysInit(BoardType type);
int  SysRun();
//-----------------------------------------------------------------------
typedef int (*module_Function)(FUN_HANDLE hFunHandle);
//-----------------------------------------------------------------------
int modules_assert(FUN_HANDLE hFunHandle,const char *szMsg);

#define LUA_ASSERT(hHandle,cond, msg)               \
    do {                                                \
        if ((cond) == 0) {                              \
        	return modules_assert(hHandle, msg);        \
        }                                               \
    } while(0)

DEV_HANDLE addModule(const char *szModule,module_Function tostring_Fun,void *usrData);
int addFuntion(DEV_HANDLE hHandle, const char *szName, module_Function funtion);
int addEnum(DEV_HANDLE hHandle, unsigned int id,const char *szName);
void SendModuleEvent(DEV_HANDLE hHandle);
//------------------------------------------------------------------------
const char *getModuleName(FUN_HANDLE hFunHandle);
void *getUsrData(FUN_HANDLE hFunHandle);
//------------------------------------------------------------------------
bool   arg_isnone(FUN_HANDLE hFunHandle, int idx);
bool   arg_isnoneornil(FUN_HANDLE hFunHandle, int idx);
bool   arg_isnumber(FUN_HANDLE hFunHandle, int idx);
bool   arg_isinteger(FUN_HANDLE hFunHandle, int idx);
bool   arg_isstring(FUN_HANDLE hFunHandle, int idx);
//--------------------------------------------------------------------
double arg_tonumber(FUN_HANDLE hFunHandle, int idx);
int arg_tointeger(FUN_HANDLE hFunHandle, int idx);
bool arg_toboolean(FUN_HANDLE hFunHandle, int idx);
//-------------------------------------------------------------------
double arg_tonumber_default(FUN_HANDLE hFunHandle, int idx,double val);
int arg_tointeger_default(FUN_HANDLE hFunHandle, int idx,int val);
bool arg_toboolean_default(FUN_HANDLE hFunHandle, int idx,bool val);
//------------------------------------------------------------------
const char* arg_tostring(FUN_HANDLE hFunHandle, int idx, size_t &len);
const void *arg_toimage(FUN_HANDLE hFunHandle, int idx, int &w,int &h,int &bpp);
int arg_tointegerarry(FUN_HANDLE hFunHandle, int inx, int *array,
		size_t arraysizes);
int arg_tofloatarry(FUN_HANDLE hFunHandle, int inx, float *array,
		size_t arraysizes);
int arg_tocallback(FUN_HANDLE hFunHandle, int inx);
//-------------------------------------------------------------------------
void pushstart(FUN_HANDLE hFunHandle);
void pushnumber_ret(FUN_HANDLE hFunHandle, double);
void pushinteger_ret(FUN_HANDLE hFunHandle, int);
void pushboolean_ret(FUN_HANDLE hFunHandle, bool);
void pushstring_ret(FUN_HANDLE hFunHandle, const char*, size_t len);
void pushintegerarry_ret(FUN_HANDLE hFunHandle,int *array, size_t len );
void pushfloatarry_ret(FUN_HANDLE hFunHandle, float *array,size_t len );
//------------------------------------------------------------------------



#endif /* MODULES_H_ */
