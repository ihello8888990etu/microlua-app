/*!
 *
 *  \brief     The user device  extension API.
 *  \details   this file is part of the microlua ai project..
 *  \author    Hbs
 *  \version   1.1
 *  \date      2020-2020
 *  \copyright the GNU Affero General Public License
 */
#ifndef MODULES_H_
#define MODULES_H_
#include<stdio.h>

#define MICROLUA_API_MAJOR 1
#define MICROLUA_API_MINOR 8
#define MICROLUA_API_PATCH 0

/**
 * 设备句柄
 */
#define DEV_HANDLE void*

/**
 * 函数句柄
 */
#define FUN_HANDLE void*

/**
 * 图像句柄
 */
#define IMG_HANDLE void*


#define LAYER_HANDLE void*
#define MAX_OUTCNT 10
typedef struct {
	uint32_t obj_number;
	struct {
		uint32_t x1;
		uint32_t y1;
		uint32_t x2;
		uint32_t y2;
		uint32_t class_id;
		float prob;
	} obj[MAX_OUTCNT];
} __attribute__((packed, aligned(4))) obj_info_t;


/**
 *板子类型
 *支持不同类型开发板
 */
typedef enum {
	OPENAI_K210 = 0,     /**< enum value MICROLUA AI V2.6以上开发板 */
}BoardType;

/**
 * 系统初始化
 * @param type 系统内置板子类型
 * @param debug_tx 调试TX针脚
 * @param debug_rx 调试RX针脚
 * @return true成功
 */
bool SysInit(BoardType type,uint8_t debug_tx,uint8_t debug_rx);

/**
 * 系统初始化
 * @param szLua 板子配置lua脚本
 * @param iLen  脚本长度
 * @param debug_tx 调试TX针脚
 * @param debug_rx 调试RX针脚
 * @return true成功
 */
bool SysInit(const char *szLua,unsigned int iLen,uint8_t debug_tx,uint8_t debug_rx);

/**
 * 系统运行
 */
int  SysRun();

#define ASSIGN_ERR 0xff
/**
 * 分配GPIOHSIO num
 * @return 系统分配的numIO 失败返回ASSIGN_ERR
 */
uint8_t AssignGPIOHS();

/**
 * 分配GPIO num
 * @return 系统分配的numIO 失败返回255
 */
uint8_t AssignGPIO();

//-----------------------------------------------------------------------
typedef int (*module_Function)(FUN_HANDLE hFunHandle);
//-----------------------------------------------------------------------
typedef int (*tablecallback_Function)(FUN_HANDLE hFunHandle,int inx,void *usrParam);
//-----------------------------------------------------------------------

int lua_module_assert(FUN_HANDLE hFunHandle,const char *szMsg);

/**
 * LUA 断言宏,断言信息会在集成开发环境 MICROLUA IDE显示
 * @param hHandle 函数句柄
 * @param 断言条件
 * @param msg 断言消息
 */
#define LUA_ASSERT(_hHandle_,_cond_, _msg_)               \
    do {                                                \
        if ((_cond_) == 0) {                              \
        	return lua_module_assert(_hHandle_, _msg_);        \
        }                                               \
    } while(0)
//----------------------------------------------------------------------


//----------------------------------------------------------------------
/**
 * 加一个用户驱动模块
 *  @param szModule 模块名称
 *  @param tostring_Fun 打印输出信息函数
 *  @param init_Fun 初始化函数
 *  @param usrData 用户自定义数据
 *  @return 用户模块句柄
 */
DEV_HANDLE addModule(const char *szModule,module_Function tostring_Fun,module_Function init_Fun,void *usrData);

/**
 * 加一个NN网络处理模块
 *  @param szModule 模块名称
 *  @param tostring_Fun 打印输出信息函数
 *  @param init_Fun 初始化函数
 *  @param usrData 用户自定义数据
 *  @return 网络模块句柄
 */
DEV_HANDLE addNNModule(const char *szModule,module_Function tostring_Fun,module_Function init_Fun,void *usrData);

/**
 * 加一个函数到模块
 *  @param hHandle 模块句柄
 *  @param szName 函数名称
 *  @param funtion 用户函数
 *  @return 0是成功，负数是失败
 */
int addFuntion(DEV_HANDLE hHandle, const char *szName, module_Function funtion);

/**
 * 加一个枚举到模块
 *  @param hHandle 模块句柄
 *  @param id 枚举值
 *  @param szName 枚举名称
 *  @return 0是成功，负数是失败
 */
int addEnum(DEV_HANDLE hHandle, unsigned int id,const char *szName);

/**
 * 发送一个事件到模块，用户可以有自己的回调函数，事件发生会调用
 *  @param hHandle 模块句柄
 */
void SendModuleEvent(DEV_HANDLE hHandle);

//------------------------------------------------------------------------
/**
 * 获取模块名字
 * @param hHandle 模块句柄
 * @return 返回模块名称
 */
const char *getModuleName(FUN_HANDLE hFunHandle);
//------------------------------------------------------------------------
/**
 * 获取网络名字，只限于NN驱动，其他驱动不支持
 * @param hHandle 模块句柄
 * @return 返回模块名称
 */
const char *getNetworkName(FUN_HANDLE hFunHandle);
//------------------------------------------------------------------------
/**
 * 获取模块用户数据
 * @param hHandle 模块句柄
 * @return 返回模块用户数据
 */
void *getUsrData(FUN_HANDLE hFunHandle);


//------------------------------------------------------------------------
bool SaveImage(IMG_HANDLE img, const char *path);
//**************************************************************************
//------------------------------------------------------------------------
/**
 * 判断函数参数是否float 或者 double
 * @param szName 模型路径或名字，模型装载来源INCBIN或来源文件系统
 * @return 返回装载结果,0是装载成功
 */
int KPU_Load(const char  *szName);
//------------------------------------------------------------------------

/**
 * 释放模型
 */
void KPU_Free();

//------------------------------------------------------------------------
/**
 * 运行KPU
 * @param hHandle 图像句柄
 * @return 输出状态
 */
int KPU_Run(const char *szName,IMG_HANDLE hHandle,int iReservedAiHead = 0,int iKpuWidth = 320 ,int iKpuHeight = 240);
//------------------------------------------------------------------------
/**
 * 运行KPU
 * @param pData 数据
 * @param len   数据长度
 * @return 输出状态
 */
int KPU_Run(const char *szName,const uint8_t *pData,size_t len);
//------------------------------------------------------------------------
/**
 * 获得KPU输出
 * @param output KPU结果输出
 * @param output_size KPU结果SIZES
 * @return 输出状态
 */
int KPU_GetOutput(const char *szName,uint32_t index,float **output,size_t &output_size);
//------------------------------------------------------------------------
LAYER_HANDLE InitLayer(int width, int height, int channels, int origin_width,
		int origin_height,float *anchor, int anchor_num);
LAYER_HANDLE InitLayer(int width, int height, int channels, int origin_width,
		int origin_height,float *anchor, int anchor_num,int classnum);
void         HaltLayer(LAYER_HANDLE hHandle);
void         RunLayer(LAYER_HANDLE hHandle,float *output, size_t output_size,
		obj_info_t *obj_info);
void  ParamLayer(LAYER_HANDLE hHandle,float threshold, float nms_value);

//************************************************************************
//------------------------------------------------------------------------
bool   arg_isnone(FUN_HANDLE hFunHandle, int idx);
bool   arg_isnoneornil(FUN_HANDLE hFunHandle, int idx);
bool   arg_istable(FUN_HANDLE hFunHandle, int idx);
bool   arg_isnumber(FUN_HANDLE hFunHandle, int idx);
bool   arg_isinteger(FUN_HANDLE hFunHandle, int idx);
bool   arg_isstring(FUN_HANDLE hFunHandle, int idx);
bool   arg_isimage(FUN_HANDLE hFunHandle, int idx);

//------------------------------------------------------------------------
double getarg_tonumber(FUN_HANDLE hFunHandle, int idx,double val = 0.0,bool bDefault = false);
int getarg_tointeger(FUN_HANDLE hFunHandle, int idx,int val = 0,bool bDefault = false);
bool getarg_toboolean(FUN_HANDLE hFunHandle, int idx,bool val = false,bool bDefault = false);
const char* getarg_tostring(FUN_HANDLE hFunHandle, int idx, size_t &len );
IMG_HANDLE getarg_toimage(FUN_HANDLE hFunHandle, int idx);
int getarg_tocallback(FUN_HANDLE hFunHandle, int inx);
//------------------------------------------------------------------------
int gettable_enter(FUN_HANDLE hFunHandle, int idx);
int gettable_enter(FUN_HANDLE hFunHandle, const char *szKey);
void gettable_exit(FUN_HANDLE hFunHandle);
//------------------------------------------------------------------------
float gettable_tofloat(FUN_HANDLE hFunHandle, const char *szKey);
int gettable_tointeger(FUN_HANDLE hFunHandle, const char *szKey);
bool gettable_toboolean(FUN_HANDLE hFunHandle,const char *szKey);
int gettable_tostring(FUN_HANDLE hFunHandle, const char *szKey,char *buf, size_t &len);
//------------------------------------------------------------------------
float gettable_tofloat(FUN_HANDLE hFunHandle, int Key);
int gettable_tointeger(FUN_HANDLE hFunHandle, int Key);
bool gettable_toboolean(FUN_HANDLE hFunHandle, int Key);
int gettable_tostring(FUN_HANDLE hFunHandle, int Key,char *buf, size_t &len );
//************************************************************************
void clear_toret(FUN_HANDLE hFunHandle);

void settable_toret(FUN_HANDLE hFunHandle);
void settable_enter(FUN_HANDLE hFunHandle,const char*key);
void settable_enter(FUN_HANDLE hFunHandle,int key);
void settable_exit(FUN_HANDLE hFunHandle);
//------------------------------------------------------------------------
void setstring_totable(FUN_HANDLE hFunHandle,const char*key, const char*str, size_t len);
void setfloat_totable(FUN_HANDLE hFunHandle,const char*key, float val);
void setboolean_totable(FUN_HANDLE hFunHandle,const char*key, bool val);
void setinteger_totable(FUN_HANDLE hFunHandle,const char*key, int val);
//------------------------------------------------------------------------
void setstring_totable(FUN_HANDLE hFunHandle,int key, const char*str, size_t len);
void setfloat_totable(FUN_HANDLE hFunHandle,int key, float val);
void setboolean_totable(FUN_HANDLE hFunHandle,int key, bool val);
void setinteger_totable(FUN_HANDLE hFunHandle,int key, int val);
//------------------------------------------------------------------------
void setstring_toret(FUN_HANDLE hFunHandle, const char*str, size_t len);
void setfloat_toret(FUN_HANDLE hFunHandle, float val);
void setboolean_toret(FUN_HANDLE hFunHandle, bool val);
void setinteger_toret(FUN_HANDLE hFunHandle, int val);
//------------------------------------------------------------------------

#endif /* MODULES_H_ */
