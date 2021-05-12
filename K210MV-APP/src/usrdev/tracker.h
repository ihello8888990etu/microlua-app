/*
 * tracker.h
 *
 *  Created on: 2020年12月30日
 *      Author: Administrator
 */
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <modules.h>
#include <math.h>
#ifndef USRDEV_TRACKER_H_
#define USRDEV_TRACKER_H_


#define OBJECT (0)
#define HUMAN (1)
#define DIST_MAX (160000.)
#define DIST_THRESH (4096.)



typedef struct
{
    uint8_t is_valid;
    float cx;
    float cy;
    float w;
    float h;

    uint8_t id;
    uint8_t uiclass;

    float prob_is_litterer;
    uint8_t track_state;
    uint8_t interaction_state;
    uint8_t belongs_to;
    uint8_t life;
    uint8_t updated_this_run;

    float vx;
    float vy;

    float pcx;
    float pcy;
    float pw;
    float ph;
    float pvx;
    float pvy;

    clock_t time_stamp;
} obj_t;

typedef struct
{
    obj_t *objects;
    uint16_t max_n_object;
    uint16_t n_object;
    uint16_t new_object_id;
} tracker_t;
//----------------------------------------------------
//----------------------------------------------------------------------------------


extern uint16_t get_boxes(float *boxes[],
                          uint16_t n_box_limit,
                          float *logits,
                          uint16_t h,
                          uint16_t w,
                          float anchors[][4],
                          uint16_t n_anchor,
                          const float score_thresh,
                          const float nms_thresh);
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------


extern void init_tracker(tracker_t *tracker, obj_t *objects, uint16_t n_object);
extern void update_tracker(tracker_t *tracker,
                           float *measurements[],
                           uint16_t n_measurement,
                           uint8_t uiclass,
                           float squared_dist_thresh);
//----------------------------------------------------------------------------------
#define H (15)
#define W (20)


inline static float *get_x( float * p)
{
    return p + H * W;
}
inline static float *get_y( float * p)
{
    return p + H * W * 2;
}
inline static float *get_w( float * p)
{
    return p + H * W * 3;
}
inline static float *get_h( float * p)
{
    return p + H * W * 4;
}



//--------------------------------------------------
#define H_8 (30)
#define W_8 (40)

inline static float *get_x_8(float * p)
{
    return p + H_8 * W_8;
}
inline static float *get_y_8(float * p)
{
    return p + H_8 * W_8 * 2;
}
inline static float *get_w_8(float *p)
{
    return p + H_8 * W_8 * 3;
}
inline static float *get_h_8(float *p)
{
    return p + H_8 * W_8 * 4;
}




#endif /* USRDEV_TRACKER_H_ */
