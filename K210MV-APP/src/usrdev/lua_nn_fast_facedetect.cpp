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

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <modules.h>
#include <assert.h>
#include <sleep.h>

extern int ultra_face_init(int input_width, int input_height,
		float score_threshold, float iou_threshold, int topk);


//----------------------------------------------------------------------------------
#define num_featuremap 4
#define hard_nms 1
#define blending_nms 2 /* mix nms was been proposaled in paper blaze face, aims to minimize the temporal jitter*/

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

typedef struct FaceInfo {
	float x1;
	float y1;
	float x2;
	float y2;
	float score;

} __attribute__((packed, aligned(4))) FaceInfo;

class ultra_face {
public:
	ultra_face(int input_width, int input_length, float score_threshold_,
			float iou_threshold_, int topk_ = -1) {
		score_threshold = score_threshold_;
		iou_threshold = iou_threshold_;
		in_w = input_width;
		in_h = input_length;
		w_h_list = { in_w, in_h };

		image_h = in_h;
		image_w = in_w;
		//printf("0ultra_face\r\n");
		for (auto size : w_h_list) {
			std::vector<float> fm_item;
			for (float stride : strides) {
				fm_item.push_back(ceil(size / stride));
			}
			featuremap_size.push_back(fm_item);
		}
		//sleep(5);
		//printf("1ultra_face\r\n");
		for (auto size : w_h_list) {
			shrinkage_size.push_back(strides);
		}
		//printf("2ultra_face\r\n");
		//sleep(5);
		/* generate prior anchors */
		for (int index = 0; index < num_featuremap; index++) {
			float scale_w = in_w / shrinkage_size[0][index];
			float scale_h = in_h / shrinkage_size[1][index];
			for (int j = 0; j < featuremap_size[1][index]; j++) {
				for (int i = 0; i < featuremap_size[0][index]; i++) {
					float x_center = (i + 0.5) / scale_w;
					float y_center = (j + 0.5) / scale_h;

					for (float k : min_boxes[index]) {
						float w = k / in_w;
						float h = k / in_h;
						priors.push_back(
								{ clip(x_center, 1), clip(y_center, 1), clip(w,
										1), clip(h, 1) });
					}
				}
			}
		}
		//sleep(5);
		//printf("3ultra_face\r\n");
		/* generate prior anchors finished */

		num_anchors = priors.size();
	}

	void generateBBox(float *scores, float *boxes) {
		for (int i = 0; i < num_anchors; i++) {
			if (scores[i * 2 + 1] > score_threshold) {
				FaceInfo rects;
				float x_center = boxes[i * 4] * center_variance * priors[i][2]
						+ priors[i][0];
				float y_center = boxes[i * 4 + 1] * center_variance
						* priors[i][3] + priors[i][1];
				float w = exp(boxes[i * 4 + 2] * size_variance) * priors[i][2];
				float h = exp(boxes[i * 4 + 3] * size_variance) * priors[i][3];

				rects.x1 = clip(x_center - w / 2.0, 1) * image_w;
				rects.y1 = clip(y_center - h / 2.0, 1) * image_h;
				rects.x2 = clip(x_center + w / 2.0, 1) * image_w;
				rects.y2 = clip(y_center + h / 2.0, 1) * image_h;
				rects.score = clip(scores[i * 2 + 1], 1);
				bbox_collection.push_back(rects);
			}
		}
	}

	std::vector<FaceInfo>& nms(int type) {
		auto &input = bbox_collection;
		std::sort(input.begin(), input.end(),
				[](const FaceInfo &a, const FaceInfo &b) {
					return a.score > b.score;
				});

		int box_num = input.size();

		std::vector<int> merged(box_num, 0);

		for (int i = 0; i < box_num; i++) {
			if (merged[i])
				continue;
			std::vector<FaceInfo> buf;

			buf.push_back(input[i]);
			merged[i] = 1;

			float h0 = input[i].y2 - input[i].y1 + 1;
			float w0 = input[i].x2 - input[i].x1 + 1;

			float area0 = h0 * w0;

			for (int j = i + 1; j < box_num; j++) {
				if (merged[j])
					continue;

				float inner_x0 =
						input[i].x1 > input[j].x1 ? input[i].x1 : input[j].x1;
				float inner_y0 =
						input[i].y1 > input[j].y1 ? input[i].y1 : input[j].y1;

				float inner_x1 =
						input[i].x2 < input[j].x2 ? input[i].x2 : input[j].x2;
				float inner_y1 =
						input[i].y2 < input[j].y2 ? input[i].y2 : input[j].y2;

				float inner_h = inner_y1 - inner_y0 + 1;
				float inner_w = inner_x1 - inner_x0 + 1;

				if (inner_h <= 0 || inner_w <= 0)
					continue;

				float inner_area = inner_h * inner_w;

				float h1 = input[j].y2 - input[j].y1 + 1;
				float w1 = input[j].x2 - input[j].x1 + 1;

				float area1 = h1 * w1;

				float score;

				score = inner_area / (area0 + area1 - inner_area);

				if (score > iou_threshold) {
					merged[j] = 1;
					buf.push_back(input[j]);
				}
			}

			switch (type) {
			case hard_nms: {
				output.push_back(buf[0]);
				break;
			}
			case blending_nms: {
				float total = 0;
				for (int i = 0; i < buf.size(); i++) {
					total += exp(buf[i].score);
				}
				FaceInfo rects { };
				for (int i = 0; i < buf.size(); i++) {
					float rate = exp(buf[i].score) / total;
					rects.x1 += buf[i].x1 * rate;
					rects.y1 += buf[i].y1 * rate;
					rects.x2 += buf[i].x2 * rate;
					rects.y2 += buf[i].y2 * rate;
					rects.score += buf[i].score * rate;
				}
				output.push_back(rects);
				break;
			}
			default: {
				printf("wrong type of nms.\n");
				exit(-1);
			}
			}
		}

		return output;
	}

	void clear() {
		bbox_collection.clear();
		output.clear();
	}

private:
	int image_w;
	int image_h;

	int in_w;
	int in_h;
	int num_anchors;

	float score_threshold;
	float iou_threshold;

	const float mean_vals[3] = { 127, 127, 127 };
	const float norm_vals[3] = { 1.0 / 128, 1.0 / 128, 1.0 / 128 };

	const float center_variance = 0.1;
	const float size_variance = 0.2;
	const std::vector<std::vector<float>> min_boxes = { { 10.0f, 16.0f, 24.0f },
			{ 32.0f, 48.0f }, { 64.0f, 96.0f }, { 128.0f, 192.0f, 256.0f } };
	const std::vector<float> strides = { 8.0, 16.0, 32.0, 64.0 };
	std::vector<std::vector<float>> featuremap_size;
	std::vector<std::vector<float>> shrinkage_size;
	std::vector<int> w_h_list;

	std::vector<std::vector<float>> priors = { };
	std::vector<FaceInfo> bbox_collection, output;
};


static std::unique_ptr<ultra_face> g_detector;
int ultra_face_init(int input_width, int input_height, float score_threshold,
		float iou_threshold, int topk) {
	g_detector = std::make_unique<ultra_face>(input_width, input_height,
			score_threshold, iou_threshold, topk);
	return 0;
}

DEV_HANDLE hUsrNNFastFaceDetectHandle = NULL;

//#define TEST 1
#include <boarddef.h>

class ultra_face;

static int net_nn_fastfacedetect_forward(FUN_HANDLE hFunHandle) {

	IMG_HANDLE arg_other = getarg_toimage(hFunHandle, 2);

	float *boxes = NULL;
	float *scores = NULL;
	size_t output_size = 0;
	const char *szNetName = getNetworkName(hFunHandle);

	assert(KPU_Run(szNetName, arg_other) == 0);

	int ret1 = KPU_GetOutput(szNetName, 0, &boxes, output_size);
	int ret2 = KPU_GetOutput(szNetName, 1, &scores, output_size);
	if (ret1 == 0 && ret2 == 0) {
		g_detector->clear();
		g_detector->generateBBox(scores, boxes);
		std::vector<FaceInfo> output = g_detector->nms(blending_nms);
		int iCnt = output.size();
		if (iCnt <= 0) {
			return 0;
		}
		clear_toret(hFunHandle);
		settable_toret(hFunHandle);
		setinteger_totable(hFunHandle, "number", iCnt);
		settable_enter(hFunHandle, "boxs");
		for (int i = 0; i < iCnt; i++) {
			settable_enter(hFunHandle, i + 1);
			int x1 = output[i].x1;
			int y1 = output[i].y1;
			int x2 = output[i].x2;
			int y2 = output[i].y2;
			if (x1 >= 320)
				x1 = 319;
			if (x2 >= 320)
				x2 = 319;
			if (y1 >= 240)
				y1 = 239;
			if (y2 >= 240)
				y2 = 239;

			setinteger_totable(hFunHandle, 1, x1);
			setinteger_totable(hFunHandle, 2, y1);
			setinteger_totable(hFunHandle, 3, x2-x1);
			setinteger_totable(hFunHandle, 4, y2-y1);
			settable_exit(hFunHandle);
		}
		settable_exit(hFunHandle);

		settable_enter(hFunHandle, "score");
		for (int i = 0; i < iCnt; i++) {
			setfloat_totable(hFunHandle, i + 1, output[i].score);
		}
		settable_exit(hFunHandle);

		return 1;
	}
	return 0;
}

static int net_nn_fastfacedetect_tostring(FUN_HANDLE hFunHandle) {

	const char *szNetName = getNetworkName(hFunHandle);
	const char *szModName = getModuleName(hFunHandle);
	char szinfo[256] = { 0 };
	snprintf(szinfo, sizeof(szinfo), "NN ModName:%s, NetName:%s", szModName,
			szNetName);
	clear_toret(hFunHandle);
	setstring_toret(hFunHandle, szinfo, strlen(szinfo));
	return 1;
}
static int net_nn_fastfacedetect_init(FUN_HANDLE hFunHandle) {


	gettable_enter(hFunHandle, 2);
	int width = gettable_tointeger(hFunHandle, "width");
	int height = gettable_tointeger(hFunHandle, "height");
	float scorethr = gettable_tofloat(hFunHandle, "scorethr");
	float iouthr = gettable_tofloat(hFunHandle, "iouthr");
	int topk = gettable_tointeger(hFunHandle, "topk");
	gettable_exit(hFunHandle);
	printf("width:%d height:%d scorethr:%f iouthr:%f topk:%d\r\n",width,height,scorethr,iouthr,topk);
	ultra_face_init(width, height, scorethr, iouthr, topk);
	return 0;
}
void module_nn_fastfacedetect_init() {

	hUsrNNFastFaceDetectHandle = addNNModule("fastfacedetect",
			net_nn_fastfacedetect_tostring, net_nn_fastfacedetect_init,NULL);
	assert(hUsrNNFastFaceDetectHandle != NULL);
	addFuntion(hUsrNNFastFaceDetectHandle, "forward",
			net_nn_fastfacedetect_forward);
	//ultra_face_init(320, 240, 0.3, 0.05, -1);

}

