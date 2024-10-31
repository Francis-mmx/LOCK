#include "ui/includes.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "face_detect.h"
#include "video_photo.h"

/************************人脸图片显示函数接口*********************************************
 * 函数接口规则说明:
 *  该函数通过传入的人脸起始坐标和结束坐标,以及5个人脸关键点(分别为左眼[0]、右眼[1]、鼻子[2]、左嘴角[3]、右嘴角[4])
 *  进行计算,得出图片显示坐标与大小
 *  \param : box_face 人脸框,包含起始坐标和结束坐标
 *  \param : alignment_temp 5个关键点坐标
 *  \return : 返回图片结构体  (wear_sucess:表示显示成功与否 1成功 0失败)
 *  注: 实际坐标值有些许误差,可在函数中进行坐标位置的微调校准
*******************************************************************************************/
struct photo_icon_t *wear_one_hat(const char *name, BBox *box_face, Point_t *alignment_temp)
{
    int border_width = 0;
    int border_height = 0;
    int x1_trans = 0;
    int y1_trans = 0;
    int left, top = 0;
    int i = 0;
    struct photo_icon_t *icon = NULL;
    icon = get_photo_icon(name);
    if (!icon) {
        return NULL;
    }
    //1.计算脸框坐标
    border_width = box_face->x2 - box_face->x1;
    border_height = box_face->y2 - box_face->y1;
#if (IMAGE_270 == 1)
    //横转竖(270)
    x1_trans = box_face->y1;
    y1_trans = YUV_DATA_WIDTH - (box_face->x1 + border_width);
    left = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x1_trans / 10000;
    top = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y1_trans / 10000 + 30;

    //2.设置图片显示属性
    icon->width = border_width;
    icon->height = border_height;
    icon->left = left - icon->height + 4;
    icon->top = top;
    icon->wear_sucess = 1;
    if (icon->top < 0 || icon->top + border_width > LCD_WIDTH || \
        icon->left < 0 || icon->left + border_height > LCD_HEIGHT) {
        icon->width = 0;
        icon->height = 0;
        icon->left = 0;
        icon->top = 0;
        icon->wear_sucess = 0;
        log_w("maozi invaild!!!\n");
    }
#endif

#if (IMAGE_90 == 1)
    //横转竖(90)
    x1_trans = YUV_DATA_HEIGHT - (box_face->y1 + border_height);
    y1_trans = box_face->x1;
    left = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x1_trans / 10000;
    top = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y1_trans / 10000 + 30;

    //2.设置图片显示属性
    icon->width = border_width;
    icon->height = border_height;
    icon->left = left + icon->height - 4;
    icon->top = top;
    icon->wear_sucess = 1;
    if (icon->top < 0 || icon->top + border_width > LCD_WIDTH || \
        icon->left < 0 || icon->left + border_height > LCD_HEIGHT) {
        icon->width = 0;
        icon->height = 0;
        icon->left = 0;
        icon->top = 0;
        icon->wear_sucess = 0;
        log_w("maozi invaild!!!\n");
    }
#endif
    return icon;
}
struct photo_icon_t *wear_one_glass(const char *name, BBox *box_face, Point_t *alignment_face)
{
    int border_width = 0;
    int border_height = 0;
    int x_trans = 0;
    int y_trans = 0;
    int left0, top0 = 0;
    int left1, top1 = 0;

    struct photo_icon_t *icon_maozi = NULL;
    struct photo_icon_t *icon = NULL;
    icon = get_photo_icon(name);
    if (!icon) {
        return NULL;
    }

    //1.计算脸框坐标
    border_width = box_face->x2 - box_face->x1;
    border_height = box_face->y2 - box_face->y1;
#if (IMAGE_270 == 1)
    //横转竖(270)
    //左眼
    x_trans = alignment_face[0].y;
    y_trans = YUV_DATA_WIDTH - alignment_face[0].x;
    left0 = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top0 = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;
    //右眼
    x_trans = alignment_face[1].y;
    y_trans = YUV_DATA_WIDTH - alignment_face[1].x;
    left1 = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top1 = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;

    //2.设置图片显示属性
    icon->width = border_width;
    icon->height = border_height / 3;  //认为眼镜高度占脸高度1/3
    icon->left = left1 - icon->height / 3;
    icon->top = top1 - (icon->width - (top0 - top1)) / 2;
    icon->wear_sucess = 1;
#endif

#if (IMAGE_90 == 1)
    //横转竖(90)
    //左眼
    x_trans = YUV_DATA_HEIGHT - alignment_face[0].y;
    y_trans = alignment_face[0].x;
    left0 = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top0 = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;
    //右眼
    x_trans = YUV_DATA_HEIGHT - alignment_face[1].y;
    y_trans = alignment_face[1].x;
    left1 = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top1 = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;

    //2.设置图片显示属性
    icon->width = border_width;
    icon->height = border_height / 3;  //认为眼镜高度占脸高度1/3
    icon->left = left1 - icon->height * 2 / 3;
    icon->top = top0 - (icon->width - (top1 - top0)) / 2;
    icon->wear_sucess = 1;
#endif

    icon_maozi = get_photo_icon("maozi");
    if (!icon_maozi->wear_sucess) {
        icon->wear_sucess = 0;
    }

    return icon;
}
struct photo_icon_t *wear_one_nose(const char *name, BBox *box_face, Point_t *alignment_face)
{
    int border_width = 0;
    int border_height = 0;
    int x_trans = 0;
    int y_trans = 0;
    int left, top = 0;

    struct photo_icon_t *icon_glass = NULL;
    struct photo_icon_t *icon = NULL;
    icon = get_photo_icon(name);
    if (!icon) {
        return NULL;
    }

    //1.计算脸框坐标
    border_width = box_face->x2 - box_face->x1;
    border_height = box_face->y2 - box_face->y1;
#if (IMAGE_270 == 1)
    //横转竖(270)
    y_trans = alignment_face[2].y;
    y_trans = YUV_DATA_WIDTH - alignment_face[2].x;
    left = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;

    //2.设置图片显示属性
    icon->width = border_width / 6;
    icon->height = border_height / 6;  //认为高度占脸高度1/6
    icon->left = left - icon->height / 2;
    icon->top = top - icon->width  / 2;
    icon->wear_sucess = 1;
#endif

#if (IMAGE_90 == 1)
    //横转竖(90)
    x_trans = YUV_DATA_HEIGHT - alignment_face[2].y;
    y_trans = alignment_face[2].x;
    left = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;

    //2.设置图片显示属性
    icon->width = border_width / 6;
    icon->height = border_height / 6;  //认为高度占脸高度1/6
    icon->left = left - icon->height / 2;
    icon->top = top - icon->width  / 2;
    icon->wear_sucess = 1;
#endif

    icon_glass = get_photo_icon("glass");
    if (!icon_glass->wear_sucess) {
        icon->wear_sucess = 0;
    }

    return icon;
}

struct photo_icon_t *wear_one_mouth(const char *name, BBox *box_face, Point_t *alignment_face)
{
    int border_width = 0;
    int border_height = 0;
    int x_trans = 0;
    int y_trans = 0;
    int left0, top0 = 0;
    int left1, top1 = 0;

    struct photo_icon_t *icon_nose = NULL;
    struct photo_icon_t *icon = NULL;
    icon = get_photo_icon(name);
    if (!icon) {
        return NULL;
    }

    //1.计算脸框坐标
    border_width = box_face->x2 - box_face->x1;
    border_height = box_face->y2 - box_face->y1;
#if (IMAGE_270 == 1)
    //横转竖(270)
    //左嘴角
    x_trans = alignment_face[3].y;
    y_trans = YUV_DATA_WIDTH - alignment_face[3].x;
    left0 = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top0 = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;
    //右嘴角
    x_trans = alignment_face[4].y;
    y_trans = YUV_DATA_WIDTH - alignment_face[4].x;
    left1 = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top1 = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;

    //2.设置图片显示属性
    icon->width = border_width / 3; //认为嘴宽占脸宽1/3
    icon->height = icon->width / 2;
    icon->left = left1 - icon->height / 2;
    icon->top = top1 - (icon->width - (top0 - top1)) / 2;
    icon->wear_sucess = 1;
#endif
#if (IMAGE_90 == 1)
    //横转竖(90)
    //左嘴角
    x_trans = YUV_DATA_HEIGHT - alignment_face[3].y;
    y_trans = alignment_face[3].x;
    left0 = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top0 = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;
    //右嘴角
    x_trans = YUV_DATA_HEIGHT - alignment_face[4].y;
    y_trans = alignment_face[4].x;
    left1 = LCD_HEIGHT * 10000 / YUV_DATA_HEIGHT * x_trans / 10000;
    top1 = LCD_WIDTH * 10000 / YUV_DATA_WIDTH * y_trans / 10000;

    //2.设置图片显示属性
    icon->width = border_width / 3; //认为嘴宽占脸宽1/3
    icon->height = icon->width / 2;
    icon->left = left1 - icon->height / 2;
    icon->top = top0 - (icon->width - (top1 - top0)) / 2;
    icon->wear_sucess = 1;
#endif
    icon_nose = get_photo_icon("nose");
    if (!icon_nose->wear_sucess) {
        icon->wear_sucess = 0;
    }

    return icon;
}
#ifdef FACE_DECT_ENABLE
/*****************以下注册人脸相关图片***************************
 * 成员说明:
 * name : 图片名称 (唯一)
 * sticker_path : 贴纸路径 (用于拍照图片的合成)
 * id : UI控件ID
 * put_image : 显示函数指针
/**************************************************************/
REGISTER_FACE_ICON(FACE_HAT)
.name = "maozi",
 .sticker_path = "mnt/spiflash/audlogo/sdmaozi.jpg",
  .id = TPH_FACE_HAT,
   .put_image = wear_one_hat,
};
REGISTER_FACE_ICON(FACE_GLASS)
.name = "glass",
 .sticker_path = "mnt/spiflash/audlogo/yanjing.jpg",
  .id = TPH_FACE_EYE,
   .put_image = wear_one_glass,
};
REGISTER_FACE_ICON(FACE_NOSE)
.name = "nose",
 .sticker_path = "mnt/spiflash/audlogo/nose.jpg",
  .id = TPH_FACE_NOSE,
   .put_image = wear_one_nose,
};
REGISTER_FACE_ICON(FACE_MOUTH)
.name = "mouth",
 .sticker_path = "mnt/spiflash/audlogo/mouth.jpg",
  .id = TPH_FACE_MOUTH,
   .put_image = wear_one_mouth,
};
#endif
