#include "system/includes.h"
#include "generic/log.h"


extern char __VERSION_BEGIN[];
extern char __VERSION_END[];

const char *sdk_version(void)
{
    return "AC52 SDK on branch [release/AC52N_SDK_V2.0.0]";
}

static int _app_version_check()
{
    char *version;

    printf("================= SDK Version %s ==============\n", sdk_version());
    for(version = __VERSION_BEGIN; version < __VERSION_END;)
    {
        printf("%s\n", version);
        version += strlen(version) + 1;
    }
    puts("=======================================\n");

    return 0;
}

static int app_version_check()
{
    /* lib_version_check(); */

    _app_version_check();

#ifdef VIDEO_REC_SERVER_VERSION
    VERSION_CHECK(video_server, VIDEO_REC_SERVER_VERSION);
#endif

#ifdef VIDEO_DEC_SERVER_VERSION
    VERSION_CHECK(video_dec_server, VIDEO_DEC_SERVER_VERSION);
#endif

#ifdef UI_SERVER_VERSION
    VERSION_CHECK(ui_server, UI_SERVER_VERSION);
#endif

#ifdef AUDIO_SERVER_VERSION
    VERSION_CHECK(audio_server, AUDIO_SERVER_VERSION);
#endif

#ifdef VIDEO_ENGINE_SERVER_VERSION
    VERSION_CHECK(video_engine_server, VIDEO_ENGINE_SERVER_VERSION);
#endif

#ifdef USB_SERVER_VERSION
    VERSION_CHECK(usb_server, USB_SERVER_VERSION);
#endif

#ifdef DEV_ISP_VERSION
    VERSION_CHECK(isp, DEV_ISP_VERSION);
#endif

#ifdef CONFIG_WAV_DEC_ENABLE
    VERSION_CHECK(wav_dec, WAV_DEC_VERSION);
#endif
#ifdef CONFIG_WAV_ENC_ENABLE
    VERSION_CHECK(wav_enc, WAV_ENC_VERSION);
#endif

#ifdef CONFIG_MP3_DEC_ENABLE
    VERSION_CHECK(mp3_dec, MP3_DEC_VERSION);
#endif
#ifdef CONFIG_MP3_ENC_ENABLE
    VERSION_CHECK(mp3_dec, MP3_ENC_VERSION);
#endif

#ifdef CONFIG_WMA_DEC_ENABLE
    VERSION_CHECK(wma_dec, WMA_DEC_VERSION);
#endif
#ifdef CONFIG_WMA_ENC_ENABLE
    VERSION_CHECK(wma_enc, WMA_ENC_VERSION);
#endif

#ifdef CONFIG_M4A_DEC_ENABLE
    VERSION_CHECK(m4a_dec, M4A_DEC_VERSION);
#endif
#ifdef CONFIG_M4A_ENC_ENABLE
    VERSION_CHECK(m4a_enc, M4A_ENC_VERSION);
#endif

#ifdef CONFIG_AAC_ENC_ENABLE
    VERSION_CHECK(aac_enc, AAC_ENC_VERSION);
#endif
#ifdef CONFIG_AAC_DEC_ENABLE
    VERSION_CHECK(aac_dec, AAC_DEC_VERSION);
#endif


#ifdef CONFIG_AMR_DEC_ENABLE
    VERSION_CHECK(amr, AMR_DEC_VERSION);
#endif

    return 0;
}
early_initcall(app_version_check);

