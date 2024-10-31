#ifndef UI_PAINT_H
#define UI_PAINT_H

#include "ui/ui_core.h"
#include "rect.h"
#include "stdlib.h"

/*
 * 画笔形状
 */
enum brush {
    BRUSH_SQUARE,
    BRUSH_CIRCLE,
};

/*
 * 画笔
 */
struct brush_set {
    enum brush shape;
    u32 color;
    u32 size;
};

/*
 * @brief   :设置画笔
 *
 * @pertain :用户接口
 *
 * @param   :shape -> 画笔形状
 * @param   :size  -> 画笔大小
 * @param   :color -> 画笔颜色
 *
 * @returns :返回0成功
 */
int ui_paint_brush_set(enum brush shape, u32 size, u32 color);

/*
 * @brief   :在特定layout上绘制一个点
 *
 * @pertain :用户接口
 *
 * @param   :lay_id -> layout的id号
 * @param   :x      -> 点的中心绝对坐标x
 * @param   :y      -> 点的中心绝对坐标y
 *
 * @returns :返回0成功
 */
int ui_paint_dot(int lay_id, int x, int y);

/*
 * @brief   :在特定layout上绘制一条直线
 *
 * @pertain :用户接口
 *
 * @param   :lay_id  -> layout的id号
 * @param   :x_start -> 起始点的中心绝对坐标x
 * @param   :y_start -> 起始点的中心绝对坐标y
 * @param   :x_end   -> 结束点的中心绝对坐标x
 * @param   :y_end   -> 结束点的中心绝对坐标y
 *
 * @returns :返回0成功
 */
int ui_paint_line(int lay_id, int x_start, int y_start, int x_end, int y_end);

/*
 * @brief   :在特定layout上绘制一堆点
 *
 * @pertain :用户接口
 *
 * @param   :lay_id -> layout的id号
 * @param   :array  -> 中心点的绝对坐标集合
 * @param   :num    -> 点集合里存放的点数
 *
 * @returns :返回0成功
 */
int ui_paint_custom(int lay_id, struct position *array, int num);

/*
 * @brief   :在特定layout上绘制一堆点并且点与点之间相互直线连接
 *
 * @pertain :用户接口
 *
 * @param   :lay_id -> layout的id号
 * @param   :array  -> 中心点的绝对坐标集合
 * @param   :num    -> 点集合里存放的点数
 *
 * @returns :返回0成功
 */
int ui_paint_connect_dots(int lay_id, struct position *array, int num);

/*
 * @brief   :在特定layout上绘制一个圆
 *
 * @pertain :用户接口
 *
 * @param   :lay_id -> layout的id号
 * @param   :x0     -> 中心点绝对坐标x
 * @param   :y0     -> 中心点绝对坐标y
 * @param   :r      -> 圆半径
 *
 * @returns :返回0成功
 */
int ui_paint_circle(int lay_id, int x0, int y0, int r);

/*
 * @brief   :在特定layout上绘制一个矩形，定位点在矩形左上角
 *
 * @pertain :用户接口
 *
 * @param   :lay_id -> layout的id号
 * @param   :x      -> 左上角定位点绝对坐标x
 * @param   :y      -> 左上角定位点绝对坐标y
 * @param   :width  -> 矩形宽
 * @param   :height -> 矩形高
 *
 * @returns :返回0成功
 */
int ui_paint_rectangle(int lay_id, int x, int y, int width, int height);

/*
 * @brief   :在特定layout上绘制一个矩形，定位点在矩形中心
 *
 * @pertain :用户接口
 *
 * @param   :lay_id -> layout的id号
 * @param   :x      -> 中心定位点绝对坐标x
 * @param   :y      -> 中心定位点绝对坐标y
 * @param   :width  -> 矩形宽
 * @param   :height -> 矩形高
 *
 * @returns :返回0成功
 */
int ui_paint_rectangle_centre(int lay_id, int x, int y, int width, int height);

/*
 * @brief   :在特定layout上绘制一个等边三角形，定位点在三角形中心
 *
 * @pertain :用户接口
 *
 * @param   :lay_id -> layout的id号
 * @param   :x      -> 中心定位点绝对坐标x
 * @param   :y      -> 中心定位点绝对坐标y
 * @param   :r      -> 三角形中心点到三个顶点的距离
 *
 * @returns :返回0成功
 */
int ui_paint_triangle_centre(int lay_id, int x, int y, int r);

#endif
