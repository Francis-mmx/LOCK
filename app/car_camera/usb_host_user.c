#include "asm/debug.h"
#include "os/os_api.h"
#include "ch9.h"
#include "usb_audio.h"
#include "usb_host_user.h"

/*
 *                                 注意 ！
 *  API的声明以及说明在usb_host_user.h，请打开此头文件查看接口使用说明
 */

#define print_line(_fmt,...) \
    printf("%s()@%d\n"#_fmt"\n", __func__, __LINE__, ##__VA_ARGS__)

#define print_line_lite(_fmt,...) \
    printf(_fmt"\n", ##__VA_ARGS__)

#if 0
    #define drv_info(fmt, ...)          print_line_lite(fmt, ##__VA_ARGS__)
#else
    #define drv_info(fmt, ...)
#endif
#if 0
    #define drv_dbg(fmt, ...)           print_line_lite(fmt, ##__VA_ARGS__)
#else
    #define drv_dbg(fmt, ...)
#endif
#if 1
    #define drv_err(fmt, ...)           print_line(fmt, ##__VA_ARGS__)
#else
    #define drv_err(fmt, ...)
#endif
#if 0
    #define drv_printf_buf(buf, len)    put_buf((buf), (len))
#else
    #define drv_printf_buf(buf, len)
#endif


#define retval_check(ret, out) \
    if ((int)(ret) < 0) { \
        drv_err("ret %d", (ret)); \
        if ((out)) { \
            goto __exit_fail; \
        } \
    }

#define my_abs(a)    ((a) > 0 ? (a) : (-(a)))

struct usb_mic_host
{
    u8 host_epin;
    u8 epin;
    u32 rxmaxp;
    u32 interval;
    u8 *ep_buffer;

    u8 ctl_itf;  //control interface number
    u8 mic_itf;  //stream interface number of mic

    u8 i_t_id;  //input terminal index
    u8 f_u_id;  //feature unit index
    u8 o_t_id;  //output terminal index

    u32 cur_sr;  //当前采样率
    u32 *sr_tbl;  //采样率表
    u8 channels;
    u8 bitwidth;

    u16 vol_res;
    u16 vol_min;
    u16 vol_max;
    u16 vol_cur;

    u16 vol_l_res;
    u16 vol_l_min;
    u16 vol_l_max;
    u16 vol_l_cur;

    u16 vol_r_res;
    u16 vol_r_min;
    u16 vol_r_max;
    u16 vol_r_cur;

    u32 mute_en : 1;
    u32 vol_en : 1;
    u32 agc_en : 1;
    u32 mute_l_en : 1;
    u32 vol_l_en : 1;
    u32 agc_l_en : 1;
    u32 mute_r_en : 1;
    u32 vol_r_en : 1;
    u32 agc_r_en : 1;
    u32 num_sr : 8;  //总采样率数
    u32 sr_ctrl_en : 1;  //mic是否开启了采样率控制
    u32 mute : 1;
    u32 mute_l : 1;
    u32 mute_r : 1;
    u32 agc : 1;
    u32 agc_l : 1;
    u32 agc_r : 1;
    u32 isopen : 1;
    u32 init_done : 1;
    u32 attr_change : 1;
    u32 reserved : 5;

    void *hdl;
    void (*recv_handler)(u8 *buf, u32 len);
};
static struct usb_mic_host *mic_host;

struct usb_speaker_host
{
    u8 host_epout;
    u8 epout;
    u32 txmaxp;
    u32 interval;
    u8 *ep_buffer;

    u8 ctl_itf;  //control interface number
    u8 spk_itf;  //stream interface number of spk

    u8 i_t_id;  //input terminal index
    u8 f_u_id;  //feature unit index
    u8 o_t_id;  //output terminal index

    u32 cur_sr;  //当前采样率
    u32 *sr_tbl;  //采样率表
    u32 channels;
    u32 bitwidth;

    u32 vol_res;
    u32 vol_min;
    u32 vol_max;
    u32 vol_cur;

    u32 vol_l_res;
    u32 vol_l_min;
    u32 vol_l_max;
    u32 vol_l_cur;

    u32 vol_r_res;
    u32 vol_r_min;
    u32 vol_r_max;
    u32 vol_r_cur;

    u32 mute_en : 1;
    u32 vol_en : 1;
    u32 mute_l_en : 1;
    u32 vol_l_en : 1;
    u32 mute_r_en : 1;
    u32 vol_r_en : 1;
    u32 num_sr : 8;  //总采样率数
    u32 sr_ctrl_en : 1;  //spk是否开启了采样率控制
    u32 mute : 1;
    u32 mute_l : 1;
    u32 mute_r : 1;
    u32 is_open : 1;
    u32 init_done : 1;
    u32 attr_change : 1;
    u32 reserved : 11;

    void *hdl;
    u32(*get_stream_data)(u8 *buf, u32 len);
    u32 last_len;
};
static struct usb_speaker_host *spk_host;

static u8 audio_ep_idx;
static u8 *audio_ep_buffer;

static OS_MUTEX mutex;
static int usb_user_mutex_init()
{
    os_mutex_create(&mutex);
    return 0;
}
late_initcall(usb_user_mutex_init);

static s32 user_usb_interface_init(void *hdl, int usb_id);
static s32 user_usb_interface_uninit(void *hdl, int usb_id);
static void user_usb_ep_isr(void *hdl, u32 intrrx, u32 intrtx);

static s32 __usb_host_mic_get_mute();
static s32 __usb_host_mic_set_mute();
static s32 __usb_host_mic_get_all_volume();
static s32 __usb_host_mic_get_cur_volume();
static s32 __usb_host_mic_set_cur_volume();
static s32 __usb_host_mic_get_agc();
static s32 __usb_host_mic_set_agc();

static s32 __usb_host_speaker_get_mute();
static s32 __usb_host_speaker_set_mute();
static s32 __usb_host_speaker_get_all_volume();
static s32 __usb_host_speaker_get_cur_volume();
static s32 __usb_host_speaker_set_cur_volume();

s32 user_usb_other_interface_descriptor_parser(void *hdl, u8 *pBuf)
{
    int len = 0;
    int cur_len;
    int cur_type;
    int cur_sub_type;
    int bControlSize;
    u16 wTotalLength = 0;
    u32 wMaxPacketSize = 0;
    u32 AlternateSetting = 0;
    u8 ItfSubClass = 0;
    u8 i_t_id = -1;
    u8 f_u_id = -1;
    u8 o_t_id = -1;
    u8 cur_itf_num = 0;
    u8 terminal_link = 0;
    u8 mic_valid_alt = 0;
    u8 spk_valid_alt = 0;
    struct usb_endpoint_descriptor end_desc;
    u8 *feature_desc = NULL;

    //由于接入的usb设备除了库内支持的驱动类以外，可能有不止一个额外的接口，
    //所以这个函数可能会重入多次，注意资源不要重复分配，变量不要重复赋值
    if(!mic_host)
    {
        mic_host = zalloc(sizeof(struct usb_mic_host));
        if(!mic_host)
        {
            return -DEV_ERR_INVALID_BUF;
        }
        mic_host->i_t_id = -2;
        mic_host->f_u_id = -2;
        mic_host->o_t_id = -2;
    }
    if(!spk_host)
    {
        spk_host = zalloc(sizeof(struct usb_speaker_host));
        if(!spk_host)
        {
            free(mic_host);
            mic_host = NULL;
            return -DEV_ERR_INVALID_BUF;
        }
        spk_host->i_t_id = -2;
        spk_host->f_u_id = -2;
        spk_host->o_t_id = -2;
    }

    os_mutex_pend(&mutex, 0);

    while(1)
    {
        cur_len = pBuf[len];
        cur_type = pBuf[len + 1];
        cur_sub_type = pBuf[len + 2];

        if(!cur_len)
        {
            break;
        }

        if(cur_type == USB_DT_INTERFACE_ASSOCIATION)
        {
            //  composite device
            drv_dbg("USB_DT_INTERFACE_ASSOCIATION");
            drv_printf_buf(pBuf + len, cur_len);
        }
        else if(cur_type == USB_DT_INTERFACE)
        {
            if(cur_len != USB_DT_INTERFACE_SIZE)
            {
                len = -USB_DT_INTERFACE;
                goto __exit_fail;
            }
            if(pBuf[len + 5] != USB_CLASS_AUDIO)
            {
                break;
            }
            drv_dbg("USB_DT_INTERFACE");
            drv_printf_buf(pBuf + len, cur_len);

            if(pBuf[len + 6] == USB_SUBCLASS_AUDIOCONTROL)
            {
                ItfSubClass = USB_SUBCLASS_AUDIOCONTROL;
                mic_host->ctl_itf = pBuf[len + 2];
                spk_host->ctl_itf = pBuf[len + 2];
                i_t_id = -1;
                f_u_id = -1;
                o_t_id = -1;
            }
            else if(pBuf[len + 6] == USB_SUBCLASS_AUDIOSTREAMING)
            {
                ItfSubClass = USB_SUBCLASS_AUDIOSTREAMING;
            }
            else
            {
                drv_err("invalid audio subclass");
                len = -USB_DT_INTERFACE;
                goto __exit_fail;
            }
            cur_itf_num = pBuf[len + 2];
            AlternateSetting = pBuf[len + 3];
            drv_dbg("AlternateSetting:%d", AlternateSetting);
        }
        else if(cur_type == USB_DT_CS_INTERFACE)
        {
            if(ItfSubClass == USB_SUBCLASS_AUDIOCONTROL)
            {
                if(cur_sub_type == UAC_HEADER)
                {
                    wTotalLength = pBuf[len + 6] << 8 | pBuf[len + 5];
                    drv_dbg("UAC_HEADER: version %04x, TotalLength %d",
                            pBuf[len + 4] << 8 | pBuf[len + 3],
                            wTotalLength);
                    drv_printf_buf(pBuf + len, cur_len);
                }
                else if(cur_sub_type == UAC_INPUT_TERMINAL)
                {
                    u16 terminal_type = pBuf[len + 5] << 8 | pBuf[len + 4];
                    u16 ch = pBuf[len + 7];
                    i_t_id = pBuf[len + 3];
                    switch(terminal_type)
                    {
                        case UAC_INPUT_TERMINAL_MICROPHONE:
                        case UAC_INPUT_TERMINAL_DESKTOP_MICROPHONE:
                        case UAC_INPUT_TERMINAL_PERSONAL_MICROPHONE:
                            mic_host->i_t_id = i_t_id;
                            break;
                        default:
                            break;
                    }
                    drv_dbg("INPUT_TERMINAL: %d 0x%x %d", i_t_id, terminal_type, ch);
                    drv_printf_buf(pBuf + len, cur_len);
                }
                else if(cur_sub_type == UAC_FEATURE_UNIT)
                {
                    f_u_id = pBuf[len + 3];
                    u8 src_id = pBuf[len + 4];
                    bControlSize = pBuf[len + 5];
                    feature_desc = pBuf + len;
                    if(mic_host->i_t_id == i_t_id && src_id == i_t_id)
                    {
                        mic_host->f_u_id = f_u_id;
                        mic_host->mute_en = !!(feature_desc[6] & 0x1);
                        mic_host->vol_en = !!(feature_desc[6] & 0x2);
                        mic_host->agc_en = !!(feature_desc[6] & 0x40);
                        if(cur_len == (7 + (2 + 1) * bControlSize))
                        {
                            mic_host->mute_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x1);
                            mic_host->vol_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x2);
                            mic_host->agc_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x40);

                            mic_host->mute_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x1);
                            mic_host->vol_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x2);
                            mic_host->agc_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x40);
                        }
                    }
                    drv_dbg("FEATURE_UNIT: %d %d %d", f_u_id, src_id, bControlSize);
                    drv_printf_buf(pBuf + len, cur_len);
                }
                else if(cur_sub_type == UAC_SELECTOR_UNIT)
                {
                    u8 sel_id = pBuf[len + 3];
                    u8 src_id = pBuf[len + 5];
                    drv_dbg("SELECTOR_UNIT: %d %d", sel_id, src_id);
                    drv_printf_buf(pBuf + len, cur_len);
                }
                else if(cur_sub_type == UAC_OUTPUT_TERMINAL)
                {
                    o_t_id = pBuf[len + 3];
                    u8 src_id = pBuf[len + 7];
                    u16 terminal_type = pBuf[len + 5] << 8 | pBuf[len + 4];
                    if(mic_host->i_t_id == i_t_id)
                    {
                        mic_host->o_t_id = o_t_id;
                    }
                    switch(terminal_type)
                    {
                        case UAC_OUTPUT_TERMINAL_SPEAKER:
                        case UAC_OUTPUT_TERMINAL_HEADPHONES:
                        case UAC_OUTPUT_TERMINAL_DESKTOP_SPEAKER:
                        case UAC_OUTPUT_TERMINAL_ROOM_SPEAKER:
                        case UAC_OUTPUT_TERMINAL_COMMUNICATION_SPEAKER:
                            spk_host->i_t_id = i_t_id;
                            spk_host->f_u_id = f_u_id;
                            spk_host->o_t_id = o_t_id;
                            if(feature_desc && (feature_desc[4] == i_t_id))
                            {
                                spk_host->mute_en = !!(feature_desc[6] & 0x01);
                                spk_host->vol_en = !!(feature_desc[6] & 0x02);
                                spk_host->mute_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x01);
                                spk_host->vol_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x02);
                                if(feature_desc[0] == (7 + (2 + 1) * bControlSize))
                                {
                                    spk_host->mute_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x01);
                                    spk_host->vol_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x02);
                                }
                            }
                            break;
                        default:
                            break;
                    }
                    drv_dbg("OUTPUT_TERMINAL: %d %d %x", o_t_id, src_id, terminal_type);
                    drv_printf_buf(pBuf + len, cur_len);
                }
                else
                {
                    drv_dbg("unreconized audio control desc");
                    drv_printf_buf(pBuf + len, cur_len);
                }
            }
            else if(ItfSubClass == USB_SUBCLASS_AUDIOSTREAMING)
            {
                if(cur_sub_type == UAC_AS_GENERAL)
                {
                    terminal_link = pBuf[len + 3];
                    u16 type = pBuf[len + 6] << 8 | pBuf[len + 5];
                    drv_dbg("UAC_AS_GENERAL: %d %04x", terminal_link, type);
                    drv_printf_buf(pBuf + len, cur_len);
                    if(terminal_link == mic_host->o_t_id)
                    {
                        //为了简化流程，只取第1个mic stream interface alternate
                        if(!mic_valid_alt)
                        {
                            mic_host->mic_itf = cur_itf_num;
                            mic_valid_alt = AlternateSetting;
                        }
                    }
                    else if(terminal_link == spk_host->i_t_id)
                    {
                        if(!spk_valid_alt)
                        {
                            spk_host->spk_itf = cur_itf_num;
                            spk_valid_alt = AlternateSetting;
                        }
                    }
                }
                else if(cur_sub_type == UAC_FORMAT_TYPE)
                {
                    if(pBuf[len + 3] != UAC_FORMAT_TYPE_I_PCM)
                    {
                        len = 0;
                        goto __exit_fail;
                    }
                    drv_dbg("UAC_FORMAT_TYPE: ch %d bw %d num_sr %d", pBuf[len + 4], pBuf[len + 6], pBuf[len + 7]);
                    drv_printf_buf(pBuf + len, cur_len);
                    if(terminal_link == mic_host->o_t_id && mic_valid_alt == AlternateSetting)
                    {
                        mic_host->channels = pBuf[len + 4];
                        mic_host->bitwidth = pBuf[len + 6];
                        mic_host->num_sr = pBuf[len + 7];
                        mic_host->sr_tbl = zalloc(4 * mic_host->num_sr);
                        if(!mic_host->sr_tbl)
                        {
                            len = -DEV_ERR_INVALID_BUF;
                            goto __exit_fail;
                        }
                        for(int i = 0; i < mic_host->num_sr; i++)
                        {
                            memcpy(&mic_host->sr_tbl[i], &pBuf[len + 8 + i * 3], 3);
                            drv_dbg("mic sr%d: %d", i, mic_host->sr_tbl[i]);
                        }
                        mic_host->cur_sr = mic_host->sr_tbl[0];
                        drv_dbg("mic cur sr: %d", mic_host->cur_sr);
                    }
                    else if(terminal_link == spk_host->i_t_id && spk_valid_alt == AlternateSetting)
                    {
                        spk_host->channels = pBuf[len + 4];
                        spk_host->bitwidth = pBuf[len + 6];
                        spk_host->num_sr = pBuf[len + 7];
                        spk_host->sr_tbl = zalloc(4 * spk_host->num_sr);
                        if(!spk_host->sr_tbl)
                        {
                            len = -DEV_ERR_INVALID_BUF;
                            goto __exit_fail;
                        }
                        for(int i = 0; i < spk_host->num_sr; i++)
                        {
                            memcpy(&spk_host->sr_tbl[i], &pBuf[len + 8 + i * 3], 3);
                            drv_dbg("spk sr%d: %d", i, spk_host->sr_tbl[i]);
                        }
                        spk_host->cur_sr = spk_host->sr_tbl[0];
                        drv_dbg("spk cur sr: %d", spk_host->cur_sr);
                    }
                }
                else
                {
                    drv_dbg("unreconized audio stream desc");
                    drv_printf_buf(pBuf + len, cur_len);
                }
            }
        }
        else if(cur_type == USB_DT_ENDPOINT)
        {
            memcpy(&end_desc, pBuf + len, USB_DT_ENDPOINT_SIZE);
            if(end_desc.bDescriptorType != USB_DT_ENDPOINT ||
                                           end_desc.bLength < USB_DT_ENDPOINT_SIZE)
            {

                drv_err("invalid descriptor for endpoint bDescriptorType = %d bLength= %d",
                        end_desc.bDescriptorType, end_desc.bLength);
                len = -USB_DT_ENDPOINT;
                goto __exit_fail;
            }
            drv_dbg("USB_DT_ENDPOINT");
            drv_printf_buf(pBuf + len, cur_len);
            if(cur_itf_num == mic_host->mic_itf && mic_valid_alt == AlternateSetting)
            {
                if((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_ISOC)
                {
                    if(end_desc.bEndpointAddress & USB_DIR_IN)
                    {
                        mic_host->epin = end_desc.bEndpointAddress & 0xf;
                        mic_host->rxmaxp = end_desc.wMaxPacketSize;
                        mic_host->interval = end_desc.bInterval;
                        drv_dbg("mic ep%d maxpktsize %d interval %d", mic_host->epin, mic_host->rxmaxp, mic_host->interval);
                    }
                }
            }
            else if(cur_itf_num == spk_host->spk_itf && spk_valid_alt == AlternateSetting)
            {
                if((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_ISOC)
                {
                    if((end_desc.bEndpointAddress & 0x80) == 0)
                    {
                        spk_host->epout = end_desc.bEndpointAddress & 0xf;
                        spk_host->txmaxp = end_desc.wMaxPacketSize;
                        spk_host->interval = end_desc.bInterval;
                        drv_dbg("spk ep%d maxpktsize %d interval %d", spk_host->epout, spk_host->txmaxp, spk_host->interval);
                    }
                }
            }
        }
        else if(cur_type == USB_DT_CS_ENDPOINT)
        {
            drv_dbg("USB_DT_CS_ENDPOINT");
            drv_printf_buf(pBuf + len, cur_len);
            if(cur_itf_num == mic_host->mic_itf && mic_valid_alt == AlternateSetting)
            {
                mic_host->sr_ctrl_en = !!(pBuf[len + 3] & 0x01);
            }
            else if(cur_itf_num == spk_host->spk_itf && spk_valid_alt == AlternateSetting)
            {
                spk_host->sr_ctrl_en = !!(pBuf[len + 3] & 0x01);
            }
        }
        else
        {
            break;
        }

        len += cur_len;
    }

    if(len > 0)
    {
        //drv_dbg(">>> set usb user ops <<<");
        struct usb_host_user_ops ops = {0};
        ops.mount = user_usb_interface_init;
        ops.unmount = user_usb_interface_uninit;
        ops.isr = user_usb_ep_isr;
        user_usb_host_set_ops(&ops);

        mic_host->hdl = hdl;
        spk_host->hdl = hdl;
    }

    os_mutex_post(&mutex);
    return len;

__exit_fail:
    if(mic_host)
    {
        if(mic_host->sr_tbl)
        {
            free(mic_host->sr_tbl);
        }
        free(mic_host);
        mic_host = NULL;
    }
    if(spk_host)
    {
        if(spk_host->sr_tbl)
        {
            free(spk_host->sr_tbl);
        }
        free(spk_host);
        spk_host = NULL;
    }
    os_mutex_post(&mutex);
    return len;
}

static s32 user_usb_interface_init(void *hdl, int usb_id)
{
    s32 ret = 0;
    u8 mute = 0;
    drv_dbg("init interface");

    os_mutex_pend(&mutex, 0);

    audio_ep_buffer = user_usb_alloc_ep_buffer(hdl, USB_CLASS_AUDIO, &audio_ep_idx);
    if(!audio_ep_buffer)
    {
        ret = -DEV_ERR_INVALID_BUF;
        goto __exit_fail;
    }
    //drv_dbg("user usb alloc ep%d buffer %x", audio_ep_idx, audio_ep_buffer);

    if(mic_host->mic_itf)
    {
        mic_host->host_epin = audio_ep_idx;
        mic_host->ep_buffer = &audio_ep_buffer[0];

        ret = __usb_host_mic_get_mute();
        retval_check(ret, 0);

        ret = __usb_host_mic_get_all_volume();
        retval_check(ret, 0);

        ret = __usb_host_mic_set_agc();
        retval_check(ret, 0);

        //set interface to 0
        ret = user_usb_control_msg(hdl, 0x0b, 0x01, 0, mic_host->mic_itf, NULL, 0);
        retval_check(ret, 1);

        mic_host->init_done = 1;

        struct sys_event e = {0};
        e.type = SYS_DEVICE_EVENT;
        e.arg = "usb mic";
        e.u.dev.event = DEVICE_EVENT_IN;
        sys_event_notify(&e);
    }

    if(spk_host->spk_itf)
    {
        spk_host->host_epout = audio_ep_idx;
        spk_host->ep_buffer = &audio_ep_buffer[512];

        ret = __usb_host_speaker_get_mute();
        retval_check(ret, 0);

        ret = __usb_host_speaker_get_all_volume();
        retval_check(ret, 0);

        //set interface to 0
        ret = user_usb_control_msg(hdl, 0x0b, 0x01, 0, spk_host->spk_itf, NULL, 0);
        retval_check(ret, 1);

        spk_host->init_done = 1;

        struct sys_event e = {0};
        e.type = SYS_DEVICE_EVENT;
        e.arg = "usb speaker";
        e.u.dev.event = DEVICE_EVENT_IN;
        sys_event_notify(&e);
    }

__exit_fail:
    os_mutex_post(&mutex);
    return ret;
}

static s32 user_usb_interface_uninit(void *hdl, int usb_id)
{
    int ret = 0;
    drv_dbg("uninit interface");

    if(mic_host)
    {
        usb_host_mic_close();

        os_mutex_pend(&mutex, 0);
        if(mic_host->sr_tbl)
        {
            free(mic_host->sr_tbl);
            mic_host->sr_tbl = NULL;
        }
        free(mic_host);
        mic_host = NULL;

        struct sys_event e = {0};
        e.type = SYS_DEVICE_EVENT;
        e.arg = "usb mic";
        e.u.dev.event = DEVICE_EVENT_OUT;
        sys_event_notify(&e);
        os_mutex_post(&mutex);
    }

    if(spk_host)
    {
        usb_host_speaker_close();

        os_mutex_pend(&mutex, 0);
        if(spk_host->sr_tbl)
        {
            free(spk_host->sr_tbl);
            spk_host->sr_tbl = NULL;
        }
        free(spk_host);
        spk_host = NULL;

        struct sys_event e = {0};
        e.type = SYS_DEVICE_EVENT;
        e.arg = "usb speaker";
        e.u.dev.event = DEVICE_EVENT_OUT;
        sys_event_notify(&e);
        os_mutex_post(&mutex);
    }

    if(audio_ep_buffer)
    {
        user_usb_free_ep_buffer(hdl, audio_ep_buffer, audio_ep_idx);
        audio_ep_buffer = NULL;
        audio_ep_idx = 0;
    }

    return 0;
}

static void user_usb_ep_isr(void *hdl, u32 intrrx, u32 intrtx)
{
    s32 len;
    u8 iso_buf[192];
    u8 status;
    s32 _len;

    if(mic_host && mic_host->init_done)
    {
        if(intrrx & BIT(mic_host->host_epin))
        {
            len = usb_h_ep_read_async(hdl,
                                      mic_host->host_epin,
                                      mic_host->rxmaxp,
                                      mic_host->epin,
                                      iso_buf,
                                      sizeof(iso_buf),
                                      USB_ENDPOINT_XFER_ISOC,
                                      0);
            if(len > 0)
            {
                if(mic_host->recv_handler)
                {
                    mic_host->recv_handler(iso_buf, len);
                }
            }
            //触发下一次接收
            usb_h_ep_read_async(hdl,
                                mic_host->host_epin,
                                mic_host->rxmaxp,
                                mic_host->epin,
                                NULL,
                                0,
                                USB_ENDPOINT_XFER_ISOC,
                                1);
        }
    }

    if(spk_host && spk_host->init_done)
    {
        if(intrtx & BIT(spk_host->host_epout))
        {
            //检查上一次发送结果的状态
            len = usb_h_ep_write_async(hdl,
                                       spk_host->host_epout,
                                       spk_host->txmaxp,
                                       spk_host->epout,
                                       NULL,
                                       0,
                                       USB_ENDPOINT_XFER_ISOC,
                                       0);
            status = 0;
            if(len == -DEV_ERR_TIMEOUT)     //for bulk: NAK_Timeout, for interrupt: IncompTx
            {
                status = 1;
            }
            else if(len == -DEV_ERR_RXSTALL)       //a STALL handshake is received, not for iso mode
            {
                status = 2;
            }
            else if(len == -DEV_ERR_CMD_TIMEOUT)       //3 attemps to send but no handshake packet, just be valid only in bulk or interrupt mode
            {
                status = 3;
            }
            else if(len == -DEV_ERR_INUSE)       //FIFONotEmpty
            {
                status = 4;
            }
            else if(len == 0)       //succeed
            {
            }

            if(status == 0 || status == 1 || status == 3)
            {
                _len = 0;
                if(spk_host->get_stream_data)
                {
                    _len = sizeof(iso_buf) > spk_host->txmaxp ? spk_host->txmaxp : sizeof(iso_buf);
                    len = spk_host->channels * spk_host->cur_sr * (spk_host->bitwidth / 8);
                    len = (len / 1000) + (len % 1000 ? (spk_host->channels * (spk_host->bitwidth / 8)) : 0);
                    if(_len > len)
                    {
                        _len = len;
                    }
                    _len = spk_host->get_stream_data(iso_buf, _len);
                }
                len = usb_h_ep_write_async(hdl,
                                           spk_host->host_epout,
                                           spk_host->txmaxp,
                                           spk_host->epout,
                                           iso_buf,
                                           _len,
                                           USB_ENDPOINT_XFER_ISOC,
                                           1);
                spk_host->last_len = len;
            }
            else if(status == 2)
            {

            }
            else if(status == 4)
            {
                len = usb_h_ep_write_async(hdl,
                                           spk_host->host_epout,
                                           spk_host->txmaxp,
                                           spk_host->epout,
                                           NULL,
                                           spk_host->last_len,
                                           USB_ENDPOINT_XFER_ISOC,
                                           1);
            }
        }
    }
}

s32 usb_host_mic_open(void)
{
    int ret = 0;
    if(!mic_host)
    {
        return -DEV_ERR_OFFLINE;
    }
    drv_err("usb mic open");
    os_mutex_pend(&mutex, 0);

    if(!mic_host->init_done)
    {
        ret = -DEV_ERR_NOT_EXIST;
        goto __exit_fail;
    }

    if(mic_host->isopen == 0)
    {
        if(mic_host->attr_change)
        {
            ret = __usb_host_mic_set_mute();
            retval_check(ret, 0);
            ret = __usb_host_mic_set_cur_volume();
            retval_check(ret, 0);
            mic_host->attr_change = 0;
        }
        //set interface to 1
        ret = user_usb_control_msg(mic_host->hdl, 0x0b, 0x01, 1, mic_host->mic_itf, NULL, 0);
        retval_check(ret, 1);

        if(mic_host->sr_ctrl_en)
        {
            //采样率设置
            ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x22, 0x0100, mic_host->epin | USB_DIR_IN, &mic_host->cur_sr, 3);
            retval_check(ret, 0);
        }

        usb_h_ep_config(mic_host->hdl,
                        mic_host->host_epin | USB_DIR_IN,
                        USB_ENDPOINT_XFER_ISOC,
                        mic_host->epin,
                        1,
                        mic_host->interval,
                        mic_host->ep_buffer,
                        mic_host->rxmaxp);
        //触发端点接收
        usb_h_ep_read_async(mic_host->hdl,
                            mic_host->host_epin,
                            mic_host->rxmaxp,
                            mic_host->epin,
                            NULL,
                            0,
                            USB_ENDPOINT_XFER_ISOC,
                            1);
        mic_host->isopen = 1;
    }

__exit_fail:
    os_mutex_post(&mutex);
    return ret;
}

s32 usb_host_mic_close(void)
{
    int ret = 0;
    if(!mic_host)
    {
        return -DEV_ERR_OFFLINE;
    }
    drv_err("usb mic close");
    os_mutex_pend(&mutex, 0);

    if(!mic_host->init_done)
    {
        ret = -DEV_ERR_NOT_EXIST;
        goto __exit_fail;
    }

    if(mic_host->isopen)
    {
        usb_h_ep_uninit(mic_host->hdl, mic_host->host_epin | USB_DIR_IN);
        //set interface to 0
        ret = user_usb_control_msg(mic_host->hdl, 0x0b, 0x01, 0, mic_host->mic_itf, NULL, 0);
        retval_check(ret, 0);
        mic_host->isopen = 0;
    }

__exit_fail:
    os_mutex_post(&mutex);
    return 0;
}

u8 usb_host_mic_online(void)
{
    if(mic_host && mic_host->init_done)
    {
        return 1;
    }
    return 0;
}

void usb_host_mic_set_ops(struct host_mic_ops *ops)
{
    if(mic_host)
    {
        mic_host->recv_handler = ops->recv_handler;
    }
}

static u16 mic_vol_convert(u16 v, u16 min, u16 max, u8 dir)
{
    u16 retval = 0;
    s16 reg1;
    s16 reg2;

    if(dir ==  0)
    {
        if(v > 100)
        {
            v = 100;
        }
        reg1 = my_abs((s16)max - (s16)min);
        retval = (s16)min + (s16)(reg1 * v / 100);
    }
    else
    {
        if((s16)v > (s16)max)
        {
            v = max;
        }
        if((s16)v < (s16)min)
        {
            v = min;
        }
        reg1 = my_abs((s16)max - (s16)min);
        reg2 = my_abs((s16)v - (s16)min);
        retval = reg2 * 100 / reg1;
    }
    return retval;
}

static s32 __usb_host_mic_get_mute()
{
    s32 ret;
    u8 mute = 0;

    if(mic_host->mute_en)
    {
        mute = 0;
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0100, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        if(mute)
        {
            mic_host->mute = 1;
            drv_dbg("mic is mute");
        }
        else
        {
            mic_host->mute = 0;
            drv_dbg("mic is unmute");
        }
    }
    if(mic_host->mute_l_en)
    {
        mute = 0;
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0101, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        if(mute)
        {
            mic_host->mute_l = 1;
            drv_dbg("mic_l is mute");
        }
        else
        {
            mic_host->mute_l = 0;
            drv_dbg("mic_l is unmute");
        }
    }
    if(mic_host->mute_r_en)
    {
        mute = 0;
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0102, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        if(mute)
        {
            mic_host->mute_r = 1;
            drv_dbg("mic_r is mute");
        }
        else
        {
            mic_host->mute_r = 0;
            drv_dbg("mic_r is unmute");
        }
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_set_mute()
{
    s32 ret = 0;
    u8 mute = 0;

    if(mic_host->mute_en)
    {
        mute = mic_host->mute;
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0100, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        drv_err("set mute to %d", mute);
    }
    if(mic_host->mute_l_en)
    {
        mute = mic_host->mute_l;
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0101, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        drv_err("set mute_l to %d", mute);
    }
    if(mic_host->mute_r_en)
    {
        mute = mic_host->mute_r;
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0102, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        drv_err("set mute_r to %d", mute);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_get_all_volume()
{
    s32 ret = 0;

    if(mic_host->vol_en)
    {
        //GET CUR
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0200, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_cur, 2);
        retval_check(ret, 0);
        //GET MIN
        ret = user_usb_control_msg(mic_host->hdl, 0x82, 0xa1, 0x0200, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_min, 2);
        retval_check(ret, 0);
        //GET MAX
        ret = user_usb_control_msg(mic_host->hdl, 0x83, 0xa1, 0x0200, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_max, 2);
        retval_check(ret, 0);
        //GET RES
        ret = user_usb_control_msg(mic_host->hdl, 0x84, 0xa1, 0x0200, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_res, 2);
        retval_check(ret, 0);

        drv_dbg("vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                mic_host->vol_min,
                mic_host->vol_max,
                mic_host->vol_res,
                mic_host->vol_cur);
    }
    if(mic_host->vol_l_en)
    {
        //GET CUR
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0201, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_l_cur, 2);
        retval_check(ret, 0);
        //GET MIN
        ret = user_usb_control_msg(mic_host->hdl, 0x82, 0xa1, 0x0201, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_l_min, 2);
        retval_check(ret, 0);
        //GET MAX
        ret = user_usb_control_msg(mic_host->hdl, 0x83, 0xa1, 0x0201, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_l_max, 2);
        retval_check(ret, 0);
        //GET RES
        ret = user_usb_control_msg(mic_host->hdl, 0x84, 0xa1, 0x0201, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_l_res, 2);
        retval_check(ret, 0);

        drv_dbg("vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                mic_host->vol_l_min,
                mic_host->vol_l_max,
                mic_host->vol_l_res,
                mic_host->vol_l_cur);
    }
    if(mic_host->vol_r_en)
    {
        //GET CUR
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0202, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_r_cur, 2);
        retval_check(ret, 0);
        //GET MIN
        ret = user_usb_control_msg(mic_host->hdl, 0x82, 0xa1, 0x0202, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_r_min, 2);
        retval_check(ret, 0);
        //GET MAX
        ret = user_usb_control_msg(mic_host->hdl, 0x83, 0xa1, 0x0202, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_r_max, 2);
        retval_check(ret, 0);
        //GET RES
        ret = user_usb_control_msg(mic_host->hdl, 0x84, 0xa1, 0x0202, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_r_res, 2);
        retval_check(ret, 0);

        drv_dbg("vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                mic_host->vol_r_min,
                mic_host->vol_r_max,
                mic_host->vol_r_res,
                mic_host->vol_r_cur);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_get_cur_volume()
{
    s32 ret = 0;

    if(mic_host->vol_en)
    {
        //GET CUR
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0200, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_cur, 2);
        retval_check(ret, 0);

        drv_dbg("get vol cur %04x", mic_host->vol_cur);
    }
    if(mic_host->vol_l_en)
    {
        //GET CUR
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0201, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_l_cur, 2);
        retval_check(ret, 0);

        drv_dbg("get vol_l cur %04x", mic_host->vol_l_cur);
    }
    if(mic_host->vol_r_en)
    {
        //GET CUR
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0202, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_r_cur, 2);
        retval_check(ret, 0);

        drv_dbg("get vol_r cur %04x", mic_host->vol_r_cur);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_set_cur_volume()
{
    s32 ret = 0;

    if(mic_host->vol_en)
    {
        //SET CUR
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0200, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_cur, 2);
        retval_check(ret, 0);
        drv_dbg("set vol to 0x%04x", mic_host->vol_cur);
    }
    if(mic_host->vol_l_en)
    {
        //SET CUR
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0201, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_l_cur, 2);
        retval_check(ret, 0);
        drv_dbg("set vol_l to 0x%04x", mic_host->vol_l_cur);
    }
    if(mic_host->vol_r_en)
    {
        //SET CUR
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0202, mic_host->f_u_id << 8 | mic_host->ctl_itf, &mic_host->vol_r_cur, 2);
        retval_check(ret, 0);
        drv_dbg("set vol_r to 0x%04x", mic_host->vol_r_cur);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_get_agc()
{
    int ret = 0;
    u8 agc;

    if(mic_host->agc_en)
    {
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0700, mic_host->ctl_itf, &agc, 0);
        retval_check(ret, 0);
        mic_host->agc = agc;
    }
    if(mic_host->agc_l_en)
    {
        ret = user_usb_control_msg(mic_host->hdl, 0x81, 0xa1, 0x0701, mic_host->ctl_itf, &agc, 0);
        retval_check(ret, 0);
        mic_host->agc_l = agc;
    }
    if(mic_host->agc_r_en)
    {
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0702, mic_host->ctl_itf, &agc, 0);
        retval_check(ret, 0);
        mic_host->agc_r = agc;
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_set_agc()
{
    int ret = 0;
    u8 agc;

    if(mic_host->agc_en)
    {
        agc = mic_host->agc;
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0700, mic_host->ctl_itf, &agc, 0);
        retval_check(ret, 0);
    }
    if(mic_host->agc_l_en)
    {
        agc = mic_host->agc_l;
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0701, mic_host->ctl_itf, &agc, 0);
        retval_check(ret, 0);
    }
    if(mic_host->agc_r_en)
    {
        agc = mic_host->agc_r;
        ret = user_usb_control_msg(mic_host->hdl, 0x01, 0x21, 0x0702, mic_host->ctl_itf, &agc, 0);
        retval_check(ret, 0);
    }

__exit_fail:
    return ret;
}

s32 usb_host_mic_get_attr(struct host_mic_attr *attr)
{
    u32 vol = 0;
    u8 cnt = 0;
    if(!mic_host)
    {
        return -1;
    }
    os_mutex_pend(&mutex, 0);
    if(mic_host->vol_en)
    {
        vol = mic_vol_convert(mic_host->vol_cur,
                              mic_host->vol_min,
                              mic_host->vol_max,
                              1);
    }
    else
    {
        if(mic_host->vol_l_en)
        {
            vol += mic_vol_convert(mic_host->vol_l_cur,
                                   mic_host->vol_l_min,
                                   mic_host->vol_l_max,
                                   1);
            cnt++;
        }
        if(mic_host->vol_r_en)
        {
            vol += mic_vol_convert(mic_host->vol_r_cur,
                                   mic_host->vol_r_min,
                                   mic_host->vol_r_max,
                                   1);
            cnt++;
        }
        vol /= cnt;
    }
    attr->vol = vol;
    attr->ch = mic_host->channels;
    attr->bitwidth = mic_host->bitwidth;
    attr->sr = mic_host->cur_sr;
    attr->mute = mic_host->mute;
__exit_fail:
    os_mutex_post(&mutex);
    return 0;
}

s32 usb_host_mic_set_attr(struct host_mic_attr *attr)
{
    u32 vol;
    u32 cnt = 0;

    if(!mic_host)
    {
        return -1;
    }
    os_mutex_pend(&mutex, 0);
    if(mic_host->vol_en)
    {
        vol = mic_vol_convert(attr->vol,
                              mic_host->vol_min,
                              mic_host->vol_max,
                              0);
        mic_host->vol_cur = vol;
    }
    if(mic_host->vol_l_en)
    {
        vol = mic_vol_convert(attr->vol,
                              mic_host->vol_l_min,
                              mic_host->vol_l_max,
                              0);
        mic_host->vol_l_cur = vol;
    }
    if(mic_host->vol_r_en)
    {
        vol = mic_vol_convert(attr->vol,
                              mic_host->vol_r_min,
                              mic_host->vol_r_max,
                              0);
        mic_host->vol_r_cur = vol;
    }
    if(mic_host->sr_ctrl_en)
    {
        for(int i = 0; i < mic_host->num_sr; i++)
        {
            if(attr->sr == mic_host->sr_tbl[i])
            {
                mic_host->cur_sr = attr->sr;
            }
        }
    }
    if(mic_host->mute_en)
    {
        mic_host->mute = attr->mute;
    }
    if(mic_host->mute_l_en)
    {
        mic_host->mute_l = attr->mute;
    }
    if(mic_host->mute_r_en)
    {
        mic_host->mute_r = attr->mute;
    }
    mic_host->attr_change = 1;

__exit_fail:
    os_mutex_post(&mutex);
    return 0;
}

s32 usb_host_mic_set_volume(u16 vol)
{
    int ret = 0;
    if(!mic_host)
    {
        return -1;
    }
    os_mutex_pend(&mutex, 0);
    if(mic_host->vol_en)
    {
        mic_host->vol_cur = mic_vol_convert(vol, mic_host->vol_min, mic_host->vol_max, 0);
    }
    if(mic_host->vol_l_en)
    {
        mic_host->vol_l_cur = mic_vol_convert(vol, mic_host->vol_l_min, mic_host->vol_l_max, 0);
    }
    if(mic_host->vol_r_en)
    {
        mic_host->vol_r_cur = mic_vol_convert(vol, mic_host->vol_r_min, mic_host->vol_r_max, 0);
    }
    ret = __usb_host_mic_set_cur_volume();
    retval_check(ret, 0);
__exit_fail:
    os_mutex_post(&mutex);
    return ret;
}

s32 usb_host_mic_set_mute(u8 mute)
{
    int ret = 0;
    if(!mic_host)
    {
        return -1;
    }
    mute = !!mute;
    drv_dbg("set mute %d", mute);
    os_mutex_pend(&mutex, 0);
    if(mic_host->mute_en)
    {
        mic_host->mute = mute;
    }
    if(mic_host->mute_l_en)
    {
        mic_host->mute_l = mute;
    }
    if(mic_host->mute_r_en)
    {
        mic_host->mute_r = mute;
    }
    ret = __usb_host_mic_set_mute();
    retval_check(ret, 0);
__exit_fail:
    os_mutex_post(&mutex);
    return ret;
}

s32 usb_host_speaker_open(void)
{
    int ret = 0;
    u8 mute = 0;
    u32 vol = 0;
    void *fd = NULL;
    if(!spk_host)
    {
        return -DEV_ERR_OFFLINE;
    }
    drv_err("usb speaker open");
    os_mutex_pend(&mutex, 0);

    if(!spk_host->init_done)
    {
        ret = -DEV_ERR_NOT_EXIST;
        goto __exit_fail;
    }

    if(!spk_host->is_open)
    {
        if(spk_host->attr_change)
        {
            ret = __usb_host_speaker_set_mute();
            retval_check(ret, 0);
            ret = __usb_host_speaker_set_cur_volume();
            retval_check(ret, 0);
            spk_host->attr_change = 0;
        }
        fd = dev_open("uvc", (void *)0);
        if(fd)
        {
            struct usb_device_id id;
            dev_ioctl(fd, USBIOC_GET_DEVICE_ID, (u32)(&id));
            if(id.vendor == 0x4C4A)
            {
                printf("AC53 uac spk control open %x", id.product);
                s32 user_host_speaker_control(void *hdl);
                user_host_speaker_control(spk_host->hdl);
            }
            dev_close(fd);
        }
        //set interface to 1
        ret = user_usb_control_msg(spk_host->hdl, 0x0b, 0x01, 0x0001, spk_host->spk_itf, NULL, 0);
        retval_check(ret, 1);

        if(spk_host->sr_ctrl_en)
        {
            //采样率设置
            ret = user_usb_control_msg(spk_host->hdl, 0x01, 0x22, 0x0100, spk_host->epout | USB_DIR_OUT, &spk_host->cur_sr, 3);
            retval_check(ret, 0);
        }

        usb_h_ep_config(spk_host->hdl,
                        spk_host->host_epout,
                        USB_ENDPOINT_XFER_ISOC,
                        spk_host->epout,
                        1,
                        spk_host->interval,
                        spk_host->ep_buffer,
                        spk_host->txmaxp);
        usb_h_ep_write_async(spk_host->hdl,
                             spk_host->host_epout,
                             spk_host->txmaxp,
                             spk_host->epout,
                             NULL,
                             0,
                             USB_ENDPOINT_XFER_ISOC,
                             1);
        spk_host->is_open = 1;
    }

__exit_fail:
    os_mutex_post(&mutex);
    return ret;
}

s32 usb_host_speaker_close(void)
{
    int ret = 0;
    if(!spk_host)
    {
        return -DEV_ERR_OFFLINE;
    }
    drv_err("usb speaker close");
    os_mutex_pend(&mutex, 0);

    if(!spk_host->init_done)
    {
        ret = -DEV_ERR_NOT_EXIST;
        goto __exit_fail;
    }

    if(spk_host->is_open)
    {
        usb_h_ep_uninit(spk_host->hdl, spk_host->host_epout);
        //set interface to 0
        ret = user_usb_control_msg(spk_host->hdl, 0x0b, 0x01, 0, spk_host->spk_itf, NULL, 0);
        retval_check(ret, 0);
        spk_host->is_open = 0;
    }

__exit_fail:
    os_mutex_post(&mutex);
    return ret;
}

u8 usb_host_speaker_online(void)
{
    if(spk_host && spk_host->init_done)
    {
        return 1;
    }
    return 0;
}

void usb_host_speaker_set_ops(struct host_speaker_ops *ops)
{
    if(spk_host)
    {
        spk_host->get_stream_data = ops->get_stream_data;
    }
}

static u16 speaker_vol_convert(u16 v, u16 min, u16 max, u8 dir)
{
    u16 retval = 0;
    s16 reg1;
    s16 reg2;

    if(dir ==  0)
    {
        if(v > 100)
        {
            v = 100;
        }
        reg1 = my_abs((s16)max - (s16)min);
        retval = (s16)min + (s16)(reg1 * v / 100);
    }
    else
    {
        if((s16)v > (s16)max)
        {
            v = max;
        }
        if((s16)v < (s16)min)
        {
            v = min;
        }
        reg1 = my_abs((s16)max - (s16)min);
        reg2 = my_abs((s16)v - (s16)min);
        retval = reg2 * 100 / reg1;
    }
    return retval;
}

static s32 __usb_host_speaker_get_mute()
{
    s32 ret = 0;
    u8 mute = 0;

    if(spk_host->mute_en)
    {
        mute = 0;
        ret = user_usb_control_msg(spk_host->hdl, 0x81, 0xa1, 0x0100, spk_host->f_u_id << 8 | spk_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        if(mute)
        {
            spk_host->mute = 1;
            drv_dbg("spk is mute");
        }
        else
        {
            spk_host->mute = 0;
            drv_dbg("spk is unmute");
        }
    }
    if(spk_host->mute_l_en)
    {
        mute = 0;
        ret = user_usb_control_msg(spk_host->hdl, 0x81, 0xa1, 0x0101, spk_host->f_u_id << 8 | spk_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        if(mute)
        {
            spk_host->mute_l = 1;
            drv_dbg("spk_l is mute");
        }
        else
        {
            spk_host->mute_l = 0;
            drv_dbg("spk_l is unmute");
        }
    }
    if(spk_host->mute_r_en)
    {
        mute = 0;
        ret = user_usb_control_msg(spk_host->hdl, 0x81, 0xa1, 0x0102, spk_host->f_u_id << 8 | spk_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        if(mute)
        {
            spk_host->mute_r = 1;
            drv_dbg("spk_r is mute");
        }
        else
        {
            spk_host->mute_r = 0;
            drv_dbg("spk_r is unmute");
        }
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_speaker_set_mute()
{
    s32 ret = 0;
    u8 mute = 0;

    if(spk_host->mute_en)
    {
        mute = spk_host->mute;
        ret = user_usb_control_msg(spk_host->hdl, 0x01, 0x21, 0x0100, spk_host->f_u_id << 8 | spk_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        drv_err("set mute to %d", mute);
    }
    if(spk_host->mute_l_en)
    {
        mute = spk_host->mute_l;
        ret = user_usb_control_msg(spk_host->hdl, 0x01, 0x21, 0x0101, spk_host->f_u_id << 8 | spk_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        drv_err("set mute_l to %d", mute);
    }
    if(spk_host->mute_r_en)
    {
        mute = spk_host->mute_r;
        ret = user_usb_control_msg(spk_host->hdl, 0x01, 0x21, 0x0102, spk_host->f_u_id << 8 | spk_host->ctl_itf, &mute, 1);
        retval_check(ret, 0);
        drv_err("set mute_r to %d", mute);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_speaker_get_all_volume()
{
    s32 ret = 0;

    if(spk_host->vol_en)
    {
        //GET CUR
        ret = user_usb_control_msg(spk_host->hdl, 0x81, 0xa1, 0x0200, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_cur, 2);
        retval_check(ret, 0);
        //GET MIN
        ret = user_usb_control_msg(spk_host->hdl, 0x82, 0xa1, 0x0200, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_min, 2);
        retval_check(ret, 0);
        //GET MAX
        ret = user_usb_control_msg(spk_host->hdl, 0x83, 0xa1, 0x0200, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_max, 2);
        retval_check(ret, 0);
        //GET RES
        ret = user_usb_control_msg(spk_host->hdl, 0x84, 0xa1, 0x0200, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_res, 2);
        retval_check(ret, 0);

        drv_dbg("vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                spk_host->vol_min,
                spk_host->vol_max,
                spk_host->vol_res,
                spk_host->vol_cur);
    }
    if(spk_host->vol_l_en)
    {
        //GET CUR
        ret = user_usb_control_msg(spk_host->hdl, 0x81, 0xa1, 0x0201, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_l_cur, 2);
        retval_check(ret, 0);
        //GET MIN
        ret = user_usb_control_msg(spk_host->hdl, 0x82, 0xa1, 0x0201, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_l_min, 2);
        retval_check(ret, 0);
        //GET MAX
        ret = user_usb_control_msg(spk_host->hdl, 0x83, 0xa1, 0x0201, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_l_max, 2);
        retval_check(ret, 0);
        //GET RES
        ret = user_usb_control_msg(spk_host->hdl, 0x84, 0xa1, 0x0201, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_l_res, 2);
        retval_check(ret, 0);

        drv_dbg("vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                spk_host->vol_l_min,
                spk_host->vol_l_max,
                spk_host->vol_l_res,
                spk_host->vol_l_cur);
    }
    if(spk_host->vol_r_en)
    {
        //GET CUR
        ret = user_usb_control_msg(spk_host->hdl, 0x81, 0xa1, 0x0202, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_r_cur, 2);
        retval_check(ret, 0);
        //GET MIN
        ret = user_usb_control_msg(spk_host->hdl, 0x82, 0xa1, 0x0202, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_r_min, 2);
        retval_check(ret, 0);
        //GET MAX
        ret = user_usb_control_msg(spk_host->hdl, 0x83, 0xa1, 0x0202, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_r_max, 2);
        retval_check(ret, 0);
        //GET RES
        ret = user_usb_control_msg(spk_host->hdl, 0x84, 0xa1, 0x0202, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_r_res, 2);
        retval_check(ret, 0);

        drv_dbg("vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                spk_host->vol_r_min,
                spk_host->vol_r_max,
                spk_host->vol_r_res,
                spk_host->vol_r_cur);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_speaker_get_cur_volume()
{
    s32 ret = 0;

    if(spk_host->vol_en)
    {
        //GET CUR
        ret = user_usb_control_msg(spk_host->hdl, 0x81, 0xa1, 0x0200, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_cur, 2);
        retval_check(ret, 0);

        drv_dbg("get vol_cur %04x", spk_host->vol_cur);
    }
    if(spk_host->vol_l_en)
    {
        //GET CUR
        ret = user_usb_control_msg(spk_host->hdl, 0x81, 0xa1, 0x0201, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_l_cur, 2);
        retval_check(ret, 0);

        drv_dbg("get vol_l_cur %04x", spk_host->vol_l_cur);
    }
    if(spk_host->vol_r_en)
    {
        //GET CUR
        ret = user_usb_control_msg(spk_host->hdl, 0x81, 0xa1, 0x0202, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_r_cur, 2);
        retval_check(ret, 0);

        drv_dbg("get vol_r_cur %04x", spk_host->vol_r_cur);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_speaker_set_cur_volume()
{
    s32 ret = 0;

    if(spk_host->vol_en)
    {
        //SET CUR
        ret = user_usb_control_msg(spk_host->hdl, 0x01, 0x21, 0x0200, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_cur, 2);
        retval_check(ret, 0);
        drv_dbg("set vol to 0x%04x", spk_host->vol_cur);
    }
    if(spk_host->vol_l_en)
    {
        //SET CUR
        ret = user_usb_control_msg(spk_host->hdl, 0x01, 0x21, 0x0201, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_l_cur, 2);
        retval_check(ret, 0);
        drv_dbg("set vol_l to 0x%04x", spk_host->vol_l_cur);
    }
    if(spk_host->vol_r_en)
    {
        //SET CUR
        ret = user_usb_control_msg(spk_host->hdl, 0x01, 0x21, 0x0202, spk_host->f_u_id << 8 | spk_host->ctl_itf, &spk_host->vol_r_cur, 2);
        retval_check(ret, 0);
        drv_dbg("set vol_r to 0x%04x", spk_host->vol_r_cur);
    }

__exit_fail:
    return ret;
}

s32 usb_host_speaker_get_attr(struct host_speaker_attr *attr)
{
    u32 vol = 0;
    if(!spk_host)
    {
        return -1;
    }
    os_mutex_pend(&mutex, 0);
    if(spk_host->vol_en)
    {
        vol = speaker_vol_convert(spk_host->vol_cur,
                                  spk_host->vol_min,
                                  spk_host->vol_max,
                                  1);
        attr->vol_l = vol;
        attr->vol_r = vol;
    }
    if(spk_host->vol_l_en)
    {
        vol = speaker_vol_convert(spk_host->vol_l_cur,
                                  spk_host->vol_l_min,
                                  spk_host->vol_l_max,
                                  1);
        attr->vol_l = vol;
    }
    if(spk_host->vol_r_en)
    {
        vol = speaker_vol_convert(spk_host->vol_r_cur,
                                  spk_host->vol_r_min,
                                  spk_host->vol_r_max,
                                  1);
        attr->vol_r = vol;
    }
    attr->ch = spk_host->channels;
    attr->bitwidth = spk_host->bitwidth;
    attr->sr = spk_host->cur_sr;
    attr->mute = spk_host->mute;
__exit_fail:
    os_mutex_post(&mutex);
    return 0;
}

s32 usb_host_speaker_set_attr(struct host_speaker_attr *attr)
{
    u32 vol;
    u32 cnt = 0;

    if(!spk_host)
    {
        return -1;
    }
    os_mutex_pend(&mutex, 0);
    if(spk_host->vol_en)
    {
        vol = (attr->vol_l + attr->vol_r) / 2;
        vol = speaker_vol_convert(vol,
                                  spk_host->vol_min,
                                  spk_host->vol_max,
                                  0);
        spk_host->vol_cur = vol;
    }
    if(spk_host->vol_l_en)
    {
        vol = speaker_vol_convert(attr->vol_l,
                                  spk_host->vol_l_min,
                                  spk_host->vol_l_max,
                                  0);
        spk_host->vol_l_cur = vol;
    }
    if(spk_host->vol_r_en)
    {
        vol = speaker_vol_convert(attr->vol_r,
                                  spk_host->vol_r_min,
                                  spk_host->vol_r_max,
                                  0);
        spk_host->vol_r_cur = vol;
    }
    if(spk_host->sr_ctrl_en)
    {
        for(int i = 0; i < spk_host->num_sr; i++)
        {
            if(attr->sr == spk_host->sr_tbl[i])
            {
                spk_host->cur_sr = attr->sr;
            }
        }
    }
    if(spk_host->mute_en)
    {
        spk_host->mute = attr->mute;
    }
    if(spk_host->mute_l_en)
    {
        spk_host->mute_l = attr->mute;
    }
    if(spk_host->mute_r_en)
    {
        spk_host->mute_r = attr->mute;
    }
    spk_host->attr_change = 1;

__exit_fail:
    os_mutex_post(&mutex);
    return 0;
}

s32 usb_host_speaker_set_volume(u16 vol_l, u16 vol_r)
{
    int ret = 0;
    u32 vol = 0;
    if(!spk_host)
    {
        return -1;
    }
    os_mutex_pend(&mutex, 0);
    if(spk_host->vol_en)
    {
        vol = (vol_l + vol_r) / 2;
        spk_host->vol_cur = speaker_vol_convert(vol, spk_host->vol_min, spk_host->vol_max, 0);
    }
    if(spk_host->vol_l_en)
    {
        spk_host->vol_l_cur = speaker_vol_convert(vol_l, spk_host->vol_l_min, spk_host->vol_l_max, 0);
    }
    if(spk_host->vol_r_en)
    {
        spk_host->vol_r_cur = speaker_vol_convert(vol_r, spk_host->vol_r_min, spk_host->vol_r_max, 0);
    }
    ret = __usb_host_speaker_set_cur_volume();
    retval_check(ret, 0);
__exit_fail:
    os_mutex_post(&mutex);
    return ret;
}

s32 usb_host_speaker_set_mute(u8 mute)
{
    int ret = 0;
    if(!spk_host)
    {
        return -1;
    }
    mute = !!mute;
    drv_dbg("set mute %d", mute);
    os_mutex_pend(&mutex, 0);
    if(spk_host->mute_en)
    {
        spk_host->mute = mute;
    }
    if(spk_host->mute_l_en)
    {
        spk_host->mute_l = mute;
    }
    if(spk_host->mute_r_en)
    {
        spk_host->mute_r = mute;
    }
    ret = __usb_host_speaker_set_mute();
    retval_check(ret, 0);
__exit_fail:
    os_mutex_post(&mutex);
    return ret;
}

s32 usb_host_force_reset_module()
{
    if(mic_host)
    {
        return user_usb_host_force_reset(mic_host->hdl);
    }
    else if(spk_host)
    {
        return user_usb_host_force_reset(spk_host->hdl);
    }
    return -1;
}

