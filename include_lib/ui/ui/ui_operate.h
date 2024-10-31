


#ifndef UI_OPERATE_H
#define UI_OPERATE_H

#include "ui/ui_core.h"
#include "rect.h"
#include "stdlib.h"

enum location {
    LOCAT_LEFT_TOP,
    LOCAT_CENTER,
};


/*
 * @brief   :复制一个控件到某个父控件下
 *
 * @pertain :用户接口
 *
 * @param   :src_id     -> 原控件ID
 * @param   :parent_id  -> 要复制到的父控件ID
 * @param   :pos        -> 要复制到的位置绝对坐标
 * @param   :loca       -> 要复制到的坐标点相对于控件的位置
 * @param   :handle     -> 复制出的控件的回调，此处传的handle->id无意义
 * @param   :all        -> 是否要复制该控件及控件下的所有子控件
 *
 * @returns :返回复制后的控件ID
 */
int ui_opt_ctr_copy(int src_id, int parent_id, struct position *pos,
                    enum location loca, struct element_event_handler *handle, char all);

/*
 * @brief   :删除一个控件并隐藏
 *
 * @pertain :用户接口
 *
 * @param   :id -> 控件ID
 *
 * @returns :返回0成功
 */
int ui_opt_ctr_del(int id);

/*
 * @brief   :拖拽一个控件到某个坐标,但不可拖出其父控件范围
 *
 * @pertain :用户接口
 *
 * @param   :id -> 控件ID
 * @param   :h_move  -> 水平拖拽像素点，正数向右，负数向左
 * @param   :v_move  -> 垂直拖拽像素点，正数向下，负数向上
 *
 * @returns :返回0成功
 */
int ui_opt_ctr_move(int id, int h_move, int v_move);

/*
 * @brief   :改变一个控件的层级
 *
 * @pertain :用户接口
 *
 * @param   :id  -> 控件ID
 * @param   :sk  -> 调整方式
 * @param   :cnt -> 调整次数，也就是执行sk的次数
 *
 * @returns :返回0成功
 */
int ui_opt_ctr_skip_floor(int id, enum skip_f sk, u8 cnt);

/*
 * @brief   :指定布局内子控件回滚删除，每次回滚一个
 * @pertain :用户接口
 * @param   :id->layout布局 id
 * @returns :返回0成功
 * */
int ui_opt_scroll_back(int layout_id);

/*@brief    :移动图层
 *@pertain  :用户接口
 *@param    :id  ->图层id
 *@param    :setp->步进
 *@param    :updown->方向 MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT
 注意：使能图 层移动功能，需要将ui_show(id|BIT(31))
 * */
int ui_opt_layer_move(int id, int step, int updown);

int ui_opt_get_pos_by_id(int id, struct position *pos);
#endif





