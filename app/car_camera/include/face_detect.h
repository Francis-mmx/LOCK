#ifndef __CHILD_FACE_DETECT_H_
#define __CHILD_FACE_DETECT_H_

#include "system/includes.h"



typedef int8_t q7_t;
typedef int16_t q15_t;
typedef int32_t q31_t;
typedef float f32;
typedef unsigned short u16;
typedef struct {
    q15_t w;
    q15_t h;
    q15_t c;
    q7_t *pixel;
} frame;

typedef struct {
    q15_t x1;
    q15_t y1;
    q15_t x2;
    q15_t y2;
    q15_t offset_x1;
    q15_t offset_y1;
    q15_t offset_x2;
    q15_t offset_y2;
} BBox;

typedef struct {
    q15_t x;
    q15_t y;
} Point_t;

typedef struct {
    int fast_mode;
    int num_r;
    int num_o;
    int i_w[2];
    int i_h[2];
    int scale[2];
    int thresh[3];
    int smile_thresh;
    q15_t *confidence_;
    BBox *bounding_box ;
    Point_t *alignment_temp;

    frame *img;
    frame *img_temp;
    frame *score ;
    frame *location;
    frame *keypoint;

    BBox *bound_temp;
    q15_t *conf_temp;
    q7_t *smile_idx;

} face_detect_f;


/******* 人脸识别数据源尺寸 (勿动!) ******/
#define YUV_DATA_WIDTH       320
#define YUV_DATA_HEIGHT      240
/****************************************/


/******* 屏幕实际尺寸 ******/
#define LCD_WIDTH       480
#define LCD_HEIGHT      272
#define IMAGE_90        1
#define IMAGE_270       0

struct photo_icon_t {
    const char *name;
    const char *sticker_path; //拍照贴纸路径
    int id;
    int left;
    int top;
    int width;
    int height;
    u8 wear_sucess; //1:表示佩戴成功 0:表示佩戴失败
    void *(*put_image)(const char *name, BBox *bounding_box, Point_t *alignment_temp);

};
extern struct photo_icon_t face_icon_begin[];
extern struct photo_icon_t face_icon_end[];

#define REGISTER_FACE_ICON(faceIcon) \
	static struct photo_icon_t faceIcon sec(.face_icon) = {

#define list_for_each_face_icon(c) \
	for (c=face_icon_begin; c<face_icon_end; c++)

static inline struct photo_icon_t *get_photo_icon(const char *name)
{
    struct photo_icon_t *c = NULL;
    list_for_each_face_icon(c) {
        if (!strcmp(c->name, name)) {
            return c;
        }
    }
    return NULL;
}
static inline int reset_photo_icons(void)
{
    struct photo_icon_t *c = NULL;
    list_for_each_face_icon(c) {
        c->left = 0;
        c->top = 0;
        c->width = 0;
        c->height = 0;
        c->wear_sucess = 0;
    }

    return 0;
}

int face_detect_process(frame *image, face_detect_f *e, q15_t *confidence_, BBox *bounding_box, Point_t *alignment_temp);

int face_detect_free(face_detect_f *e);

void  face_detect_init(int fast_m, int num_r, int num_o, int *thresh, int smile_thresh, face_detect_f *e);


void smile_face_process(int global_count, frame *image, face_detect_f *e, q15_t *confidence_, BBox *bounding_box, Point_t *alignment_temp, q7_t *idx_smile);


#endif

