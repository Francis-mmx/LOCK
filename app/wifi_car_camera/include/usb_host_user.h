#ifndef __USB_HOST_USER_H__
#define __USB_HOST_USER_H__

#include "system/includes.h"

#ifdef __cplusplus
extern "C" {
#endif



/**@defgroup usb_host_user_api
 * @{
 * @brief usb host 用户扩展接口
 * @details 以下接口是usb库提供给用户添加额外的usb设备类驱动，额外添加的usb
 *          类可以与usb库中原先的mass storage类，usb video类，hub类驱动复合，
 *          也可以单独使用。例如用这套接口实现了usb mic驱动，接入type-C耳机，
 *          由于库中没有匹配的usb类驱动，则usb mic驱动单独工作。
 */

/**@brief   注册(*mount)(), (*unmount)(), (*isr)()回调函数
 * @param   *ops  接口的函数指针集合
 * @return  none
 * @note    函数指针说明：
 *          -# (*mount)()在公共枚举流程结束后执行，用来实现class-specified初始化
 *          时的请求，端点初始化等
 *          -# (*unmount)()在usb卸载过程中执行，用来停止数据流，释放descriptor parser
 *          和init()过程中分配的资源
 *          -# (*isr)()在usb中断服务函数中执行，用来处理端点触发的事件
 */
extern void user_usb_host_set_ops(struct usb_host_user_ops *ops);
/**@brief   取代库里面的配置+接口+端点描述符解析
 * @param   *hdl  usb句柄
 * @param   *pBuf  传入完整的配置+接口+端点描述符
 * @param   total_len  描述符的总长
 * @return  执行结果
 *          - >= 0  成功
 *          - <  0  失败
 * @note    WEEK函数，库里面有实现的话就调用库的描述符解析，库外实现就取代库
 *          的解析。注意，库外实现该函数会使库内所有usb设备类驱动失效，用户
 *          需要在库外实现所有希望用到的usb设备类驱动。如果只是希望在库原有
 *          的usb设备类驱动基础上增加支持额外的驱动，请不要在库外实现该函数，
 *          实现另一个描述符解析函数
 *          user_usb_other_interface_descriptor_parser()
 */
extern s32 user_usb_descriptor_parser(void *hdl, u8 *pBuf, u32 total_len);
/**@brief   实现除mass storage，video，hub，wifi类以外的描述符解析
 * @param   *hdl  usb句柄
 * @param   *pBuf  从IAD描述符/接口描述符开始的描述符起始地址
 * @return  返回值含义：
 *          - > 0  成功解析到感兴趣的usb设备类驱动描述符的长度，库将会把描述符
 *             数据流长度减去返回的长度，继续解析剩下的描述符数据流
 *          - = 0  没有从描述符流中获取到感兴趣的设备类，库将会继续下一轮描述符
 *             解析，直至描述符数据流结束
 *          - < 0  解析过程中发现描述符不符合格式标准或者资源分配失败，返回<0会
 *             认为整个usb启动失败，继而卸载其他成功打开的设备类
 * @note    WEEK函数，不实现会直接调用库里面的同名函数，库函数返回0。该函数的
 *          目的在于实现额外的usb设备类驱动，会与库里面原有的设备类驱动一同运
 *          作，如果库里面没有匹配的设备类驱动，库外的驱动亦可独立运行
 */
extern s32 user_usb_other_interface_descriptor_parser(void *hdl, u8 *pBuf);
/**@brief   控制传输请求，包含了setup阶段，data阶段和stage阶段
 * @param   *hdl  usb句柄
 * @param   request  usb请求的参数字段
 * @param   requesttype  usb请求的参数字段
 * @param   value  usb请求的参数字段
 * @param   index  usb请求的参数字段
 * @param   *data  data阶段的数据，大于ep0 maxpktsize会分包发送
 * @param   size  usb请求的参数字段，对应wLength，数据阶段的数据长度
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
extern s32 user_usb_control_msg(void *hdl, u8 request, u8 requesttype, u16 value, u16 index, void *data, u16 size);
/**@brief   分配端点号和端点buffer，包含了IN和OUT方向，所以buffer需要分成两半使用
 * @param   *hdl  usb句柄
 * @param   type  usb设备类，例如USB_CLASS_VIDEO
 * @param   *ep  用来返回分配到的端点号，如果返回失败，不会改动指针指向的变量的值
 * @return  分配到的ep buffer，如果失败就会返回NULL
 * @note
 */
extern void *user_usb_alloc_ep_buffer(void *hdl, u32 type, u8 *ep);
/**@brief   释放端点号和端点buffer，buf需要填user_usb_alloc_ep_buffer()返回的地址
 * @param   *hdl  usb句柄
 * @param   *buf  分配到的ep buffer
 * @param   ep  分配到的端点号
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
extern int user_usb_free_ep_buffer(void *hdl, u8 *buf, u8 ep);
/**@brief   初始化端点
 * @param   *hdl  usb句柄
 * @param   host_ep  分配到的端点号，如果是IN端点，需要host_ep | USB_DIR_IN
 * @param   xfer_type  端点类型，如USB_ENDPOINT_XFER_BULK
 * @param   target_ep  从描述符中获取到的设备类的端点号，
 * @param   ie  中断使能
 * @param   interval  端点取数时间间隔
 * @param   *ptr  端点dma buffer地址，应该填user_usb_alloc_ep_buffer()分配出来
 *          的buffer，或由此分半的buffer，不能填地址在ddr的全局数组或局部数组
 * @param   maxpktsize  端点最大包长
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
extern s32 usb_h_ep_config(void *hdl, u32 host_ep, u32 xfer_type, u32 target_ep, u32 ie, u32 interval, u8 *ptr, u32 maxpktsize);
/**@brief   端点发送函数(for interrupt)
 * @param   *hdl  usb句柄
 * @param   host_ep  分配到的端点号
 * @param   txmaxp  端点最大包长
 * @param   target_ep  从描述符中获取到的设备类的端点号
 * @param   *ptr  发送的数据
 * @param   len  数据长度
 * @param   xfer_type  端点类型，如USB_ENDPOINT_XFER_BULK
 * @param   kstart  启动一次传输
 * @return  执行结果
 *          - >= 0  实际发送长度
 *          - < 0  传输错误
 * @note
 */
extern s32 usb_h_ep_write_async(void *hdl, u8 host_ep, u16 txmaxp, u8 target_ep, const u8 *ptr, u32 len, u32 xfer_type, u32 kstart);
/**@brief   端点发送函数(not for interrupt)
 * @param   *hdl  usb句柄
 * @param   host_ep  分配到的端点号
 * @param   txmaxp  端点最大包长
 * @param   target_ep  从描述符中获取到的设备类的端点号
 * @param   *ptr  发送的数据
 * @param   len  数据长度
 * @param   xfer_type  端点类型，如USB_ENDPOINT_XFER_BULK
 * @return  执行结果
 *          - >= 0  实际发送长度
 *          - < 0  传输错误
 * @note    未实现，请勿使用！！！！
 */
extern s32 usb_h_ep_write(void *hdl, u8 host_ep, u16 txmaxp, u8 target_ep, const u8 *ptr, u32 len, u32 xfer_type);
/**@brief   端点接收函数(for interrupt)
 * @param   *hdl  usb句柄
 * @param   host_ep  分配到的端点号
 * @param   rxmaxp  端点最大包长
 * @param   target_ep  从描述符中获取到的设备类的端点号
 * @param   *ptr  接收的数据
 * @param   len  数据长度
 * @param   xfer_type  端点类型，如USB_ENDPOINT_XFER_BULK
 * @param   kstart  启动一次传输
 * @return  执行结果
 *          - >= 0  实际发送长度
 *          - < 0  传输错误
 * @note
 */
extern s32 usb_h_ep_read_async(void *hdl, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *ptr, u32 len, u32 xfer_type, u32 kstart);
/**@brief   端点接收函数(not for interrupt)
 * @param   *hdl  usb句柄
 * @param   host_ep  分配到的端点号
 * @param   rxmaxp  端点最大包长
 * @param   target_ep  从描述符中获取到的设备类的端点号
 * @param   *ptr  发送的数据
 * @param   len  数据长度
 * @param   xfer_type  端点类型，如USB_ENDPOINT_XFER_BULK
 * @return  执行结果
 *          - >= 0  实际接收长度
 *          - < 0  传输错误
 * @note    未实现，请勿使用！！！！
 */
extern s32 usb_h_ep_read(void *hdl, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *ptr, u32 len, u32 xfer_type);
/**@brief   端点tx中断使能
 * @param   *hdl  usb句柄
 * @param   ep  端点号
 * @return  none
 * @note
 */
extern void usb_set_intr_txe(void *hdl, u32 ep);
/**@brief   端点tx中断禁用
 * @param   *hdl  usb句柄
 * @param   ep  端点号
 * @return  none
 * @note
 */
extern void usb_clr_intr_txe(void *hdl, u32 ep);
/**@brief   端点rx中断使能
 * @param   *hdl  usb句柄
 * @param   ep  端点号
 * @return  none
 * @note
 */
extern void usb_set_intr_rxe(void *hdl, u32 ep);
/**@brief   端点rx中断禁用
 * @param   *hdl  usb句柄
 * @param   ep  端点号
 * @return  none
 * @note
 */
extern void usb_clr_intr_rxe(void *hdl, u32 ep);
/**@brief   注销端点
 * @param   *hdl  usb句柄
 * @param   ep  端点号
 * @return  none
 * @note
 */
extern void usb_h_ep_uninit(void *hdl, u32 ep);
/**@brief   强制让usb phy复位，使从机重新挂载
 * @param   *hdl  usb句柄
 * @return  none
 * @note
 */
extern s32 user_usb_host_force_reset(void *hdl);
/**@} usb_host_user_api*/



/**@defgroup usb_mic_host_api
 * @{
 * @brief usb mic host 驱动接口
 * @details 以下接口是使用“usb库用户扩展接口”实现的usb mic host驱动，sdk的
 *          audio模块可以不用了解usb mic驱动的实现细节，直接调用这些接口实
 *          现音频的播放，录音，透传等功能
 */

/**@struct  host_mic_ops
 * @brief   usb mic函数指针信息结构体
 */
struct host_mic_ops {
    void (*recv_handler)(u8 *, u32);  ///<mic端点接收中断数据回调函数
};
/**@struct  host_mic_attr
 * @brief   usb mic属性信息结构体
 */
struct host_mic_attr {
    u8 vol;                         ///<音量：0 - 100
    u8 ch;                          ///<通道数
    u8 bitwidth;                    ///<位宽
    u8 mute;                        ///<静音
    u32 sr;                         ///<采样率
};
/**@brief   打开usb mic（启动数据流）
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_mic_open(void);
/**@brief   关闭usb mic（停止数据流）
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_mic_close(void);
/**@brief   判断usb mic是否在线
 * @return  是否在线
 *          - = 1  在线
 *          - = 0  不在线
 * @note
 */
u8 usb_host_mic_online(void);
/**@brief   获取usb mic属性
 * @param   *attr  usb mic属性
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_mic_get_attr(struct host_mic_attr *attr);
/**@brief   设置usb mic属性
 * @param   *attr  usb mic属性
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_mic_set_attr(struct host_mic_attr *attr);
/**@brief   注册usb mic的回调函数接口
 * @param   *ops 回调函数指针集合
 * @return  none
 * @note    (*recv_handler)(u8 *, u32)  注册该函数获取usb mic的数据
 */
void usb_host_mic_set_ops(struct host_mic_ops *ops);
/**@brief   设置usb mic的音量
 * @param   vol  音量：0 - 100
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_mic_set_volume(u16 vol);
/**@brief   设置usb mic静音
 * @param   mute 是否静音
 *          - 1 静音
 *          - 0 不静音
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_mic_set_mute(u8 mute);
/**@} usb_mic_host_api*/



/**@defgroup usb_speaker_host_api
 * @{
 * @brief usb speaker host 驱动接口
 * @details 以下接口是使用“usb库用户扩展接口”实现的usb speaker host驱动，sdk的
 *          audio模块可以不用了解usb speaker驱动的实现细节，直接调用这些接口实
 *          现音频的播放，录音，透传等功能
 */

/**@struct  host_speaker_ops
 * @brief   usb speaker函数指针信息结构体
 */
struct host_speaker_ops {
    u32(*get_stream_data)(u8 *, u32);   ///<获取要给speaker发送的数据
};
/**@struct  host_speaker_attr
 * @brief   usb speaker属性信息结构体
 */
struct host_speaker_attr {
    u8 vol_l;                       ///<音量：0 - 100
    u8 vol_r;                       ///<音量：0 - 100
    u8 ch;                          ///<通道数
    u8 bitwidth;                    ///<位宽
    u8 mute;                        ///<静音
    u32 sr;                         ///<采样率
};
/**@brief   打开usb speaker（打开数据流）
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_speaker_open(void);
/**@brief   关闭usb speaker（关闭数据流）
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_speaker_close(void);
/**@brief   判断usb speaker是否在线
 * @return  是否在线
 *          - = 1  在线
 *          - = 0  不在线
 * @note
 */
u8 usb_host_speaker_online(void);
/**@brief   获取usb speaker的属性
 * @param   *attr usb speaker属性
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_speaker_get_attr(struct host_speaker_attr *attr);
/**@brief   设置usb speaker的属性
 * @param   *attr usb speaker属性
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_speaker_set_attr(struct host_speaker_attr *attr);
/**@brief   设置usb speaker回调函数接口
 * @param   *ops 回调函数指针集合
 * @return  none
 * @note    (*get_stream_data)(u8 *buf, u32 len) 注册该函数输送usb speaker数据
 */
void usb_host_speaker_set_ops(struct host_speaker_ops *ops);
/**@brief   设置usb speaker音量
 * @param   vol_l 左声道音量：0 - 100
 * @param   vol_r 右声道音量：0 - 100
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_speaker_set_volume(u16 vol_l, u16 vol_r);
/**@brief   设置usb speaker静音
 * @param   mute 静音
 *          - 1 静音
 *          - 0 不静音
 * @return  执行结果
 *          - = 0  成功
 *          - != 0  失败
 * @note
 */
s32 usb_host_speaker_set_mute(u8 mute);
/**@} usb_speaker_host_api*/

/**@brief   强制让usb phy复位，使从机重新挂载，不用传入hdl
 * @return  none
 * @note
 */
s32 usb_host_force_reset_module(void);

#ifdef __cplusplus
}
#endif

#endif  //__USB_HOST_USER_H__
