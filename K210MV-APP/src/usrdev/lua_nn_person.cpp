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
DEV_HANDLE hSysNNPersonHandle = NULL;

#define OBJECT (0)
#define HUMAN (1)
#define DIST_MAX (160000.)
#define DIST_THRESH (4096.)

#define N_PERSON_ANCHOR 15
static float person_anchors[][4] = {
    {7.500000, 7.500000, 6.444802, 17.378922},
    {7.500000, 7.500000, 27.070000, 32.411266},
    {7.499999, 7.500000, 59.323946, 145.040344},
    {7.500000, 7.500000, 10.376238, 12.129096},
    {7.500000, 7.500000, 78.554626, 86.740006},
    {7.500000, 7.500000, 15.523774, 41.475006},
    {7.500000, 7.500000, 186.322494, 216.810502},
    {7.500000, 7.499999, 42.645004, 54.964464},
    {7.500000, 7.500000, 4.783142, 9.161744},
    {7.500000, 7.500000, 2.699996, 4.678788},
    {7.500000, 7.500000, 37.044770, 98.219254},
    {7.500000, 7.500000, 23.190628, 64.775010},
    {7.500000, 7.500000, 16.653870, 20.397190},
    {7.500000, 7.500000, 9.910644, 27.630906},
    {7.500000, 7.500000, 106.485000, 175.425660}};


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
extern void init_tracker(tracker_t *tracker, obj_t *objects, uint16_t n_object);
extern void update_tracker(tracker_t *tracker,
                           float *measurements[],
                           uint16_t n_measurement,
                           uint8_t uiclass,
                           float squared_dist_thresh);
static uint16_t n_obj = 10;
static obj_t objects[10];
static tracker_t tracker;
//----------------------------------------------------------------------------------
static int net_nn_person_tostring(FUN_HANDLE hFunHandle) {

	const char *szNetName = getNetworkName(hFunHandle);
	const char *szModName = getModuleName(hFunHandle);
	char szinfo[256] = { 0 };
	snprintf(szinfo, sizeof(szinfo), "NN ModName:%s, NetName:%s", szModName,
			szNetName);
	clear_toret(hFunHandle);
	setstring_toret(hFunHandle, szinfo, strlen(szinfo));
	return 1;
}

static int net_nn_person_init(FUN_HANDLE hFunHandle) {

	memset(&tracker,0,sizeof(tracker));
	init_tracker(&tracker, objects, n_obj);

	return 0;
}
static int net_nn_person_forward(FUN_HANDLE hFunHandle) {

	IMG_HANDLE arg_other = getarg_toimage(hFunHandle, 2);

	float threshold = getarg_tonumber(hFunHandle, 3,0.7,true);
	float nms_value = getarg_tonumber(hFunHandle, 4, 0.4,true);

	float *output = NULL;
	size_t output_size = 0;
	//printf("KPU_Run \r\n");
	const char *szNetName = getNetworkName(hFunHandle);
	assert(szNetName != NULL);

	assert(KPU_Run(szNetName, arg_other) == 0);

	int ret = KPU_GetOutput(szNetName, 0, &output, output_size);

	if (ret == 0) {

		static uint16_t n_box_limit = 10;
		static float *boxes[10];
		//-------------------------------------------
		memset(&tracker,0,sizeof(tracker));
		memset(boxes,0,sizeof(boxes));
		init_tracker(&tracker, objects, n_obj);
		//-------------------------------------------
		uint16_t n_result = get_boxes(boxes, n_box_limit, output, 15, 20, person_anchors, N_PERSON_ANCHOR, 0.7, 0.15);
		update_tracker(&tracker, boxes, n_result, HUMAN, DIST_THRESH);



		if (tracker.max_n_object <= 0) {
			return 0;
		}

		clear_toret(hFunHandle);
		settable_toret(hFunHandle);

		//-----------------------------------------------------------------------------
		bool bFind = false;
		for (int i = 0; i < tracker.max_n_object; i++) {
			if (tracker.objects[i].is_valid == 0) {
				continue;
			}
			bFind = true;
			break;
		}
		if ( !bFind ){
			return 0;
		}
		settable_enter(hFunHandle, "boxs");
        int number = 0;
		for (int i = 0; i < tracker.max_n_object; i++) {
			obj_t *obj = &(tracker.objects[i]);

			if (!obj->is_valid) {
				continue;
			}
			settable_enter(hFunHandle, number + 1);

			float half_w = obj->w / 2;
			float half_h = obj->h / 2;
			uint32_t x1 = obj->cx - half_w;
			uint32_t y1 = obj->cy - half_h;
			uint32_t x2 = obj->cx + half_w;
			uint32_t y2 = obj->cy + half_h;

			if (x1 <= 0)
				x1 = 1;
			if (x2 >= 319)
				x2 = 318;
			if (y1 <= 0)
				y1 = 1;
			if (y2 >= 239)
				y2 = 238;
			setinteger_totable(hFunHandle, 1, x1);
			setinteger_totable(hFunHandle, 2, y1);
			setinteger_totable(hFunHandle, 3, x2 - x1);
			setinteger_totable(hFunHandle, 4, y2 - y1);
			settable_exit(hFunHandle);
			number++;
		}
		settable_exit(hFunHandle);
		//-----------------------------------------------------------------------------
		setinteger_totable(hFunHandle, "number", number);
		//-----------------------------------------------------------------------------

		settable_enter(hFunHandle, "class");
		for (int i = 0; i < tracker.max_n_object; i++) {
			setinteger_totable(hFunHandle, i + 1, tracker.objects[i].uiclass);
		}
		settable_exit(hFunHandle);

		return 1;


	}
	return 0;
}

void module_nn_person_init() {

	//strncpy(lua_nn_facedetect.szModName,)
	hSysNNPersonHandle = addNNModule("person",
			net_nn_person_tostring, net_nn_person_init,NULL);
	assert(hSysNNPersonHandle != NULL);
	addFuntion(hSysNNPersonHandle, "forward", net_nn_person_forward);
}
//-------------------------------------------------------------------------------

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

static float overlap(float *x1, float *w1, float *x2, float *w2)
{
    float l1 = *x1 - *w1 / 2;
    float l2 = *x2 - *w2 / 2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = *x1 + *w1 / 2;
    float r2 = *x2 + *w2 / 2;
    float right = r1 < r2 ? r1 : r2;

    return right - left;
}

static float box_intersection(float *a, float *b)
{
    float w = overlap(get_x(a), get_w(a), get_x(b), get_w(b));
    float h = overlap(get_y(a), get_h(a), get_y(b), get_h(b));

    if (w < 0 || h < 0)
        return 0;
    return w * h;
}

static float box_union( float *a,  float *b)
{
    float i = box_intersection(a, b);
    float u = *get_w(a) * *get_h(a) + *get_w(b) * *get_h(b) - i;

    return u;
}

static float box_iou( float *a,  float *b)
{
    float i = box_intersection(a, b);
    float u = *get_w(a) * *get_h(a) + *get_w(b) * *get_h(b) - i;

    return i / u;
}

static float sigmoid(const float x)
{
    return 1. / (1. + expf(-x));
}

static uint16_t get_candidates( float *candidates[],
                                float logits[],
                               const uint16_t h,
                               const uint16_t w,
                               const uint16_t n_anchor,
                               const float score_thresh)
{
    uint16_t n_candidate = 0;
    for (uint16_t n = 0; n < n_anchor; ++n)
    {
        float *p = logits + n * h * w * 5;
        for (uint16_t loc_h = 0; loc_h < h; ++loc_h)
        {
            for (uint16_t loc_w = 0; loc_w < w; ++loc_w, ++p)
            {
                if (*p < 5) // should take sigmoid and < score_thresh
                    continue;
                candidates[n_candidate++] = p;
            }
        }
    }
    return n_candidate;
}

static int candidate_cmp_fn(const void *a, const void *b)
{
    return (**(float **)b - **(float **)a);
}
static void sort_candidates(float *candidates[], const uint16_t n_candidate)
{
    qsort(candidates, n_candidate, sizeof(float *), candidate_cmp_fn);
}

static void disable_non_max(float *const candidates[], const uint16_t n_candidate, float thresh)
{
    for (uint16_t c = 0; c < n_candidate; ++c)
    {
        if (*candidates[c] == 0)
            continue;
        for (uint16_t r = c + 1; r < n_candidate; ++r)
        {
            if (*candidates[r] == 0)
                continue;
            if (box_iou(candidates[c], candidates[r]) > thresh)
                *candidates[r] = 0;
        }
    }
}

static uint16_t copy_boxes(float *boxes[], uint16_t n_box_limit, float *const candidates[], uint16_t n_candidate)
{
    uint16_t box_count = 0;
    for (uint16_t c = 0; c < n_candidate; ++c)
    {
        if (box_count == n_box_limit)
            return n_box_limit;
        if (*candidates[c] != 0)
        {
            boxes[box_count++] = candidates[c];
        }
    }
    return box_count;
}
static uint16_t non_max_suppression(float *boxes[],
                                    const uint16_t n_box_limit,
                                    float * candidates[],
                                    const uint16_t n_candidate,
                                    const uint16_t h,
                                    const uint16_t w,
                                    const float thresh)
{
    sort_candidates(candidates, n_candidate);
    disable_non_max(candidates, n_candidate, thresh);
    uint16_t n_result = copy_boxes(boxes, n_box_limit, candidates, n_candidate);

    return n_result;
}

static void decode_ccwh(float *logits,
                        float *candidates[],
                        const uint16_t n_candidate,
                        const float anchors[][4],
                        const uint16_t n_anchor,
                        const uint16_t h,
                        const uint16_t w)
{
    for (uint16_t c = 0; c < n_candidate; ++c)
    {
        float *px = get_x(candidates[c]);
        float *py = get_y(candidates[c]);
        float *pw = get_w(candidates[c]);
        float *ph = get_h(candidates[c]);
        uint16_t anchor_index = (candidates[c] - logits) / (h * w * 5);
        uint16_t shift_x = (candidates[c] - logits) % w;
        uint16_t shift_y = (candidates[c] - logits) % (h * w) / w;
        shift_x *= 16;
        shift_y *= 16;
        *px = (*px - 3) * anchors[anchor_index][2] + anchors[anchor_index][0] + shift_x;
        *py = (*py - 3) * anchors[anchor_index][3] + anchors[anchor_index][1] + shift_y;
        *pw = expf(*pw - 3) * anchors[anchor_index][2];
        *ph = expf(*ph - 3) * anchors[anchor_index][3];
    }
}

uint16_t get_boxes(float *boxes[],
                          uint16_t n_box_limit,
                          float *logits,
                          uint16_t h,
                          uint16_t w,
                          float anchors[][4],
                          uint16_t n_anchor,
                          const float score_thresh,
                          const float nms_thresh)
{
    static float *candidates[H * W * 15] = {NULL};
    memset(candidates,0,sizeof(candidates));
    uint16_t n_candidate = get_candidates(candidates, logits, h, w, n_anchor, score_thresh);
    decode_ccwh(logits, candidates, n_candidate, anchors, n_anchor, h, w);
    uint16_t n_result = non_max_suppression(boxes, n_box_limit, candidates, n_candidate, h, w, nms_thresh);
    return n_result;
}
// test

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

static float overlap_8(const float *x1, const float *w1, const float *x2, const float *w2)
{
    float l1 = *x1 - *w1 / 2;
    float l2 = *x2 - *w2 / 2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = *x1 + *w1 / 2;
    float r2 = *x2 + *w2 / 2;
    float right = r1 < r2 ? r1 : r2;

    return right - left;
}

static float box_intersection_8( float *a,  float *b)
{
    const float w = overlap_8(get_x_8(a), get_w_8(a), get_x_8(b), get_w_8(b));
    const float h = overlap_8(get_y_8(a), get_h_8(a), get_y_8(b), get_h_8(b));

    if (w < 0 || h < 0)
        return 0;
    return w * h;
}

static float box_union_8( float *a,  float *b)
{
    float i = box_intersection_8(a, b);
    float u = *get_w_8(a) * *get_h_8(a) + *get_w_8(b) * *get_h_8(b) - i;

    return u;
}

static float box_iou_8( float *a,  float *b)
{
    float i = box_intersection_8(a, b);
    float u = *get_w_8(a) * *get_h_8(a) + *get_w_8(b) * *get_h_8(b) - i;

    return i / u;
}

/*
static float sigmoid(const float x)
{
    return 1. / (1. + expf(-x));
}
*/

static uint16_t get_candidates_8(float *candidates[],
                                 float logits[],
                                 const uint16_t h,
                                 const uint16_t w,
                                 const uint16_t n_anchor,
                                 const float score_thresh)
{
    uint16_t n_candidate = 0;
    for (uint16_t n = 0; n < n_anchor; ++n)
    {
        float *p = logits + n * h * w * 5;
        for (uint16_t loc_h = 0; loc_h < h; ++loc_h)
        {
            for (uint16_t loc_w = 0; loc_w < w; ++loc_w, ++p)
            {
                if (*p < 4.2) // should take sigmoid and < score_thresh
                    continue;
                candidates[n_candidate++] = p;
            }
        }
    }
    return n_candidate;
}

static int candidate_cmp_fn_8(const void *a, const void *b)
{
    return (**(float **)b - **(float **)a);
}
static void sort_candidates_8( float *candidates[], const uint16_t n_candidate)
{
    qsort(candidates, n_candidate, sizeof(float *), candidate_cmp_fn_8);
}

static void disable_non_max_8(float *const candidates[], const uint16_t n_candidate, float thresh)
{
    for (uint16_t c = 0; c < n_candidate; ++c)
    {
        if (*candidates[c] == 0)
            continue;
        for (uint16_t r = c + 1; r < n_candidate; ++r)
        {
            if (*candidates[r] == 0)
                continue;
            if (box_iou_8(candidates[c], candidates[r]) > thresh)
                *candidates[r] = 0;
        }
    }
}

static uint16_t copy_boxes_8(float *boxes[], uint16_t n_box_limit, float *const candidates[], uint16_t n_candidate)
{
    uint16_t box_count = 0;
    for (uint16_t c = 0; c < n_candidate; ++c)
    {
        if (box_count == n_box_limit)
            return n_box_limit;
        if (*candidates[c] != 0)
        {
            boxes[box_count++] = candidates[c];
        }
    }
    return box_count;
}

static uint16_t non_max_suppression_8(float **boxes,
                                      const uint16_t n_box_limit,
                                      float **candidates,
                                      const uint16_t n_candidate,
                                      const uint16_t h,
                                      const uint16_t w,
                                      const float thresh)
{
    sort_candidates_8(candidates, n_candidate);
    disable_non_max_8(candidates, n_candidate, thresh);
    uint16_t n_result = copy_boxes_8(boxes, n_box_limit, candidates, n_candidate);

    return n_result;
}

static void decode_ccwh_8(float *logits,
                          float *candidates[],
                          const uint16_t n_candidate,
                          const float anchors[][4],
                          const uint16_t n_anchor,
                          const uint16_t h,
                          const uint16_t w)
{
    for (uint16_t c = 0; c < n_candidate; ++c)
    {
        float *px = get_x_8(candidates[c]);
        float *py = get_y_8(candidates[c]);
        float *pw = get_w_8(candidates[c]);
        float *ph = get_h_8(candidates[c]);
        uint16_t anchor_index = (candidates[c] - logits) / (h * w * 5);
        uint16_t shift_x = (candidates[c] - logits) % w;
        uint16_t shift_y = (candidates[c] - logits) % (h * w) / w;
        shift_x *= 8;
        shift_y *= 8;
        *px = (*px - 3) * anchors[anchor_index][2] + anchors[anchor_index][0] + shift_x;
        *py = (*py - 3) * anchors[anchor_index][3] + anchors[anchor_index][1] + shift_y;
        *pw = expf(*pw - 3) * anchors[anchor_index][2];
        *ph = expf(*ph - 3) * anchors[anchor_index][3];
    }
}

static uint16_t get_boxes_8(float *boxes[],
                            uint16_t n_box_limit,
                            float *logits,
                            uint16_t h,
                            uint16_t w,
                            float anchors[][4],
                            uint16_t n_anchor,
                            const float score_thresh,
                            const float nms_thresh)
{
    static float *candidates[H_8 * W_8 * 9] = {NULL};
    uint16_t n_candidate = get_candidates_8(candidates, logits, h, w, n_anchor, score_thresh);
    decode_ccwh_8(logits, candidates, n_candidate, anchors, n_anchor, h, w);
    uint16_t n_result = non_max_suppression_8(boxes, n_box_limit, candidates, n_candidate, h, w, nms_thresh);
    return n_result;
}

//#define OBJECT (0)
//#define HUMAN (1)
//#define DIST_MAX (160000.)
//#define DIST_THRESH (4096.)

#define STATE_NEW (0)
#define STATE_TRACKING (1)
#define STATE_UNTRACKED (2)

#define STATE_OWNED (10)
#define STATE_DISCARDED (11)
#define STATE_NOT_OWNED (12)
#define STATE_PICKED (13)

#define STATE_NORMAL (20)
#define STATE_HERO (21)
#define STATE_LITTERER (22)

#define LIFE_INIT (16)

//#define DELTA_T (0.5)
#define P_INIT (5)
#define P_P_NOISE (0.5)
#define P_L_NOISE (0.05)
#define P_V_NOISE (0.5)




static void init_object(obj_t *object)
{
    object->is_valid = 0;
}

void init_tracker(tracker_t *tracker, obj_t *objects, uint16_t n_object)
{
    tracker->objects = objects;
    tracker->max_n_object = n_object;
    tracker->n_object = 0;
    tracker->new_object_id = 1;
    for (uint16_t i = 0; i < n_object; ++i)
        init_object(&objects[i]);
}

static uint8_t _add_object(tracker_t *tracker, float *measurement, uint8_t uiclass, clock_t cur_time)
{
    uint8_t ok = 0;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        if (!tracker->objects[i].is_valid)
        {
            obj_t *obj = &(tracker->objects[i]);
            obj->cx = *get_x(measurement);
            obj->cy = *get_y(measurement);
            obj->w = *get_w(measurement);
            obj->h = *get_h(measurement);
            obj->vx = 0.;
            obj->vy = 0.;
            obj->time_stamp = cur_time;

            obj->is_valid = 1;
            obj->id = tracker->new_object_id;
            obj->uiclass = uiclass;
            obj->track_state = STATE_NEW;
            obj->interaction_state = STATE_NORMAL;
            obj->belongs_to = 255; // actually max uint8
            obj->life = LIFE_INIT;
            obj->updated_this_run = 1;

            obj->pcx = P_INIT;
            obj->pcy = P_INIT;
            obj->pw = P_INIT;
            obj->ph = P_INIT;
            obj->pvx = P_INIT;
            obj->pvy = P_INIT;

            ok = 1;
            ++tracker->new_object_id;
            ++tracker->n_object;
            break;
        }
    }
    return ok;
}

static float _get_squared_distance(float xa, float ya, float xb, float yb)
{
    float dist_x = xb - xa;
    float dist_y = yb - ya;
    dist_x *= dist_x;
    dist_y *= dist_y;
    float squared_dist = dist_x + dist_y;
    return squared_dist;
}

static uint8_t _get_closest_given_measurement(tracker_t *tracker,
                                              float *measurement,
                                              uint8_t target_class,
                                              float dist_thresh,
                                              uint8_t *min_index,
                                              clock_t cur_time)
{
    clock_t elapsed = clock() - cur_time;
    float t_diff = elapsed;
    uint8_t found = 0;
    float min_dist = DIST_MAX;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *obj = &(tracker->objects[i]);
        if (!obj->is_valid || obj->uiclass != target_class || obj->updated_this_run)
            continue;

        float predicted_cx = obj->cx + t_diff * obj->vx;
        float predicted_cy = obj->cy + t_diff * obj->vy;
        float squared_dist = _get_squared_distance(predicted_cx, predicted_cy,
                                                   *get_x(measurement), *get_y(measurement));

        if (squared_dist < dist_thresh && squared_dist < min_dist)
        {
            min_dist = squared_dist;
            *min_index = i;
            found = 1;
        }
    }
    return found;
}

static uint8_t _get_closest_given_object(tracker_t *tracker, obj_t *obj, uint8_t target_class, float dist_thresh, uint16_t *min_index, float *confidence)
{
    uint8_t found = 0;
    float min_dist = DIST_MAX;
    float dist_sum = 0.0;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *target_obj = &(tracker->objects[i]);
        if (!target_obj->is_valid || target_obj->uiclass != target_class)
            continue;

        float squared_dist = _get_squared_distance(target_obj->cx, target_obj->cy, obj->cx, obj->cy);
        dist_sum += squared_dist;

        if (squared_dist < dist_thresh && squared_dist < min_dist)//target_obj->w * target_obj->h && squared_dist < min_dist)
        {
            min_dist = squared_dist;
            *min_index = i;
            found = 1;
        }
    }
    // the confidence
    if(found && confidence){
        if(min_dist == dist_sum)
            *confidence = 1.;
        else
            *confidence = 1. - min_dist / dist_sum;
    }
    return found;
}

static void _predict(obj_t *obj, float t_diff)
{
    obj->cx += obj->vx * t_diff;
    obj->cy += obj->vy * t_diff;

    obj->pcx += P_P_NOISE;
    obj->pcy += P_P_NOISE;
    obj->ph += P_L_NOISE;
    obj->pw += P_L_NOISE;
    obj->pvx += P_V_NOISE;
    obj->pvy += P_V_NOISE;
}

static void _update_object(obj_t *obj, float *measurement, clock_t cur_time)
{
    static clock_t elapsed;
    static float t_diff;
    static float conf;
    static float x_mea;
    static float y_mea;
    static float w_mea;
    static float h_mea;
    static float vx_mea;
    static float vy_mea;
    static float r;
    static float x_kg;
    static float y_kg;
    static float w_kg;
    static float h_kg;
    static float vx_kg;
    static float vy_kg;

    elapsed = cur_time - obj->time_stamp;
    t_diff = elapsed;
    // 3
    conf = sigmoid(*measurement - 3); // NOTE
    x_mea = *get_x(measurement);
    y_mea = *get_y(measurement);
    w_mea = *get_w(measurement);
    h_mea = *get_h(measurement);
    vx_mea = (x_mea - obj->cx) / t_diff;
    vy_mea = (y_mea - obj->cy) / t_diff;

    // predict moved here
    _predict(obj, t_diff);

    // 4
    r = 1. - conf;
    x_kg = obj->pcx / (obj->pcx + r);
    y_kg = obj->pcy / (obj->pcy + r);
    w_kg = obj->pw / (obj->pw + r);
    h_kg = obj->ph / (obj->ph + r);
    vx_kg = obj->pvx / (obj->pvx + r);
    vy_kg = obj->pvy / (obj->pvy + r);

    // 5
    obj->cx += x_kg * (x_mea - obj->cx);
    obj->cy += y_kg * (y_mea - obj->cy);
    obj->w += w_kg * (w_mea - obj->w);
    obj->h += h_kg * (h_mea - obj->h);
    obj->vx += vx_kg * (vx_mea - obj->vx);
    obj->vy += vy_kg * (vy_mea - obj->vy);

    // 6
    obj->pcx *= (1. - x_kg);
    obj->pcy *= (1. - y_kg);
    obj->pw *= (1. - w_kg);
    obj->ph *= (1. - h_kg);
    obj->pvx *= (1. - vx_kg);
    obj->pvy *= (1. - vy_kg);

    // update
    obj->time_stamp = cur_time;
    obj->life = LIFE_INIT;
    obj->updated_this_run = 1;
}

static void _add_or_update_closest(tracker_t *tracker,
                                   float *measurement,
                                   uint8_t i_class,
                                   float dist_thresh,
                                   clock_t cur_time)
{
    uint8_t obj_index = 0;
    uint8_t found = _get_closest_given_measurement(tracker, measurement, i_class, dist_thresh, &obj_index, cur_time);
    if (!found)
        _add_object(tracker, measurement, i_class, cur_time); // may check ok or not
    else
    {
        _update_object(&(tracker->objects[obj_index]), measurement, cur_time);
    }
}

static void _obj_state_normal_handler(tracker_t *tracker, float dist_thresh, obj_t *obj)
{
    uint16_t belongs_to_index = 0;
    float confidence;
    uint8_t found = _get_closest_given_object(tracker, obj, HUMAN, dist_thresh, &belongs_to_index, &confidence);
    if (found)
    {
        obj->belongs_to = belongs_to_index;
        obj->interaction_state = STATE_OWNED;
        // add confidence
        tracker->objects[belongs_to_index].prob_is_litterer = confidence;
    }
    else
    {
        obj->interaction_state = STATE_NOT_OWNED;
    }
}
static void _obj_state_owned_handler(tracker_t *tracker, float dist_thresh, obj_t *obj)
{
    obj_t *owner = &(tracker->objects[obj->belongs_to]);
    float squared_dist = _get_squared_distance(owner->cx, owner->cy, obj->cx, obj->cy);
    if (squared_dist >= DIST_THRESH) // owner->w * owner->h)
    {
        owner->interaction_state = STATE_LITTERER;
        obj->interaction_state = STATE_DISCARDED;
    }
}
static void _obj_state_discarded_handler(tracker_t *tracker, float dist_thresh, obj_t *obj)
{
    obj_t *owner = &(tracker->objects[obj->belongs_to]);
    float squared_dist = _get_squared_distance(owner->cx, owner->cy, obj->cx, obj->cy);
    if (squared_dist < DIST_THRESH - 100) //owner->w * owner->h)
    {
        if (owner->interaction_state == STATE_LITTERER)
        {
            owner->interaction_state = STATE_NORMAL;
        }
    }
}



static void _obj_state_handler(tracker_t *tracker, float dist_thresh)
{
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *obj = &(tracker->objects[i]);
        if (!obj->is_valid || obj->uiclass == HUMAN)
            continue;

        if (obj->interaction_state == STATE_NORMAL)
        {
            _obj_state_normal_handler(tracker, dist_thresh, obj);
        }
        else if (obj->interaction_state == STATE_OWNED)
        {
            _obj_state_owned_handler(tracker, dist_thresh, obj);
        }
        else if (obj->interaction_state == STATE_DISCARDED)
        {
            _obj_state_discarded_handler(tracker, dist_thresh, obj);
        }
        else if (obj->interaction_state == STATE_NOT_OWNED)
        {
            //_obj_state_not_owned_handler(tracker, dist_thresh, obj);
        }
        else if (obj->interaction_state == STATE_PICKED)
        {
            //_obj_state_picked_handler(tracker, dist_thresh, obj);
        }
        else
            ;
    }
}

static void _handle_post_update(tracker_t *tracker)
{
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *obj = &(tracker->objects[i]);
        if (!obj->is_valid)
            continue;

        if (--obj->life == 0){
            obj->is_valid = 0;
            --tracker->n_object;
        }

        obj->updated_this_run = 0;
    }
}

static void _handle_update(tracker_t *tracker,
                           float *measurements[],
                           uint16_t n_measurement,
                           uint8_t uiclass,
                           float squared_dist_thresh,
                           clock_t cur_time)
{
    for (uint16_t i = 0; i < n_measurement; ++i)
    {
        _add_or_update_closest(tracker, measurements[i], uiclass, squared_dist_thresh, cur_time);
    }
}

void update_tracker(tracker_t *tracker,
                           float *measurements[],
                           uint16_t n_measurement,
                           uint8_t uiclass,
                           float squared_dist_thresh)
{
    clock_t cur_time = clock();
    _handle_update(tracker, measurements, n_measurement, uiclass, squared_dist_thresh, cur_time);
    //_obj_state_handler(tracker, squared_dist_thresh);
    //_handle_post_update(tracker);
}



/* 8 */
static uint8_t _add_object_8(tracker_t *tracker, float *measurement, uint8_t uiclass, clock_t cur_time)
{
    uint8_t ok = 0;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        if (!tracker->objects[i].is_valid)
        {
            obj_t *obj = &(tracker->objects[i]);
            obj->cx = *get_x_8(measurement);
            obj->cy = *get_y_8(measurement);
            obj->w = *get_w_8(measurement);
            obj->h = *get_h_8(measurement);
            obj->vx = 0.;
            obj->vy = 0.;
            obj->time_stamp = cur_time;

            obj->is_valid = 1;
            obj->id = tracker->new_object_id;
            obj->uiclass = uiclass;
            obj->track_state = STATE_NEW;
            obj->interaction_state = STATE_NORMAL;
            obj->belongs_to = 255; // actually max uint8
            obj->life = LIFE_INIT;
            obj->updated_this_run = 1;

            obj->pcx = P_INIT;
            obj->pcy = P_INIT;
            obj->pw = P_INIT;
            obj->ph = P_INIT;
            obj->pvx = P_INIT;
            obj->pvy = P_INIT;

            ok = 1;
            if(++tracker->new_object_id == 255)
                tracker->new_object_id = 1;
            ++tracker->n_object;
            break;
        }
    }
    return ok;
}

/*
static float _get_squared_distance(float xa, float ya, float xb, float yb)
{
    float dist_x = xb - xa;
    float dist_y = yb - ya;
    dist_x *= dist_x;
    dist_y *= dist_y;
    float squared_dist = dist_x + dist_y;
    return squared_dist;
}
*/

static uint8_t _get_closest_given_measurement_8(tracker_t *tracker,
                                                float *measurement,
                                                uint8_t target_class,
                                                float dist_thresh,
                                                uint8_t *min_index,
                                                clock_t cur_time)
{
    clock_t elapsed = clock() - cur_time;
    float t_diff = elapsed;
    uint8_t found = 0;
    float min_dist = DIST_MAX;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *obj = &(tracker->objects[i]);
        if (!obj->is_valid || obj->uiclass != target_class || obj->updated_this_run)
            continue;

        float predicted_cx = obj->cx + t_diff * obj->vx;
        float predicted_cy = obj->cy + t_diff * obj->vy;
        float squared_dist = _get_squared_distance(predicted_cx, predicted_cy,
                                                   *get_x_8(measurement), *get_y_8(measurement));

        if (squared_dist < dist_thresh && squared_dist < min_dist)
        {
            min_dist = squared_dist;
            *min_index = i;
            found = 1;
        }
    }
    return found;
}



static void _update_object_8(obj_t *obj, float *measurement, clock_t cur_time)
{
    static clock_t elapsed;
    static float t_diff;
    static float conf;
    static float x_mea;
    static float y_mea;
    static float w_mea;
    static float h_mea;
    static float vx_mea;
    static float vy_mea;
    static float r;
    static float x_kg;
    static float y_kg;
    static float w_kg;
    static float h_kg;
    static float vx_kg;
    static float vy_kg;

    elapsed = cur_time - obj->time_stamp;
    t_diff = elapsed;
    // 3
    conf = sigmoid(*measurement - 3); // NOTE
    x_mea = *get_x_8(measurement);
    y_mea = *get_y_8(measurement);
    w_mea = *get_w_8(measurement);
    h_mea = *get_h_8(measurement);
    vx_mea = (x_mea - obj->cx) / t_diff;
    vy_mea = (y_mea - obj->cy) / t_diff;

    // predict moved here
    _predict(obj, t_diff);

    // 4
    r = 1. - conf;
    x_kg = obj->pcx / (obj->pcx + r);
    y_kg = obj->pcy / (obj->pcy + r);
    w_kg = obj->pw / (obj->pw + r);
    h_kg = obj->ph / (obj->ph + r);
    vx_kg = obj->pvx / (obj->pvx + r);
    vy_kg = obj->pvy / (obj->pvy + r);

    // 5
    obj->cx += x_kg * (x_mea - obj->cx);
    obj->cy += y_kg * (y_mea - obj->cy);
    obj->w += w_kg * (w_mea - obj->w);
    obj->h += h_kg * (h_mea - obj->h);
    obj->vx += vx_kg * (vx_mea - obj->vx);
    obj->vy += vy_kg * (vy_mea - obj->vy);

    // 6
    obj->pcx *= (1. - x_kg);
    obj->pcy *= (1. - y_kg);
    obj->pw *= (1. - w_kg);
    obj->ph *= (1. - h_kg);
    obj->pvx *= (1. - vx_kg);
    obj->pvy *= (1. - vy_kg);

    // update
    obj->time_stamp = cur_time;
    obj->life = LIFE_INIT;
    obj->updated_this_run = 1;
}

static void _add_or_update_closest_8(tracker_t *tracker,
                                     float *measurement,
                                     uint8_t i_class,
                                     float dist_thresh,
                                     clock_t cur_time)
{
    uint8_t obj_index = 0;
    uint8_t found = _get_closest_given_measurement_8(tracker, measurement, i_class, dist_thresh, &obj_index, cur_time);
    if (!found)
        _add_object_8(tracker, measurement, i_class, cur_time); // may check ok or not
    else
    {
        _update_object_8(&(tracker->objects[obj_index]), measurement, cur_time);
    }
}

static void _handle_update_8(tracker_t *tracker,
                             float *measurements[],
                             uint16_t n_measurement,
                             uint8_t uiclass,
                             float squared_dist_thresh,
                             clock_t cur_time)
{
    for (uint16_t i = 0; i < n_measurement; ++i)
    {
        _add_or_update_closest_8(tracker, measurements[i], uiclass, squared_dist_thresh, cur_time);
    }
}

static void update_tracker_8(tracker_t *tracker,
                             float *measurements[],
                             uint16_t n_measurement,
                             uint8_t uiclass,
                             float squared_dist_thresh)
{
    clock_t cur_time = clock();
    _handle_update_8(tracker, measurements, n_measurement, uiclass, squared_dist_thresh, cur_time);
    _obj_state_handler(tracker, squared_dist_thresh);
    _handle_post_update(tracker);
}

#pragma pack(pop)
