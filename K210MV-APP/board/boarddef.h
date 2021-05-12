#ifndef _BOARDDEF_H_
#define _BOARDDEF_H_

#include <incbin.h>

#undef INCBIN_STYLE
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#undef INCBIN_PREFIX
#define INCBIN_PREFIX

//#define OPENAI_V22
#define OPENAI_V26
//#define OPENAI_V27
//--------------------------------------------------------------


#if OPENAI_V22
#define OPENAI_LUA_TX 20
#define OPENAI_LUA_RX 19
#elif   defined OPENAI_V26|| defined OPENAI_V27
#define OPENAI_LUA_TX 6
#define OPENAI_LUA_RX 7
//--------------------------------------------------------------
#else
#error "There is no defined board"
#endif


#endif
