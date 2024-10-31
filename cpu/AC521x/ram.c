// *INDENT-OFF*

#include "app_config.h"


SDRAM_SIZE = __SDRAM_SIZE__;

#if defined __CPU_AC521x__
FLASH_SIZE = __FLASH_SIZE__;
__cpu_id__ = 52;
#else
#error "undefined CPU"
#endif

MEMORY
{

	rom(rx)    	      : ORIGIN =  0x0000000, LENGTH = FLASH_SIZE
	sdram(rwx)    : ORIGIN =  0x1000000, LENGTH = SDRAM_SIZE
	ram0(rwx)         : ORIGIN =  0x0f00000, LENGTH = 0x0f10000-0x0f00000
}

SECTIONS
{
/********************************************/
#ifdef CONFIG_SFC_ENABLE
    . = ORIGIN(rom);
#else
    . = ORIGIN(sdram);
#endif
    .rom_code ALIGN(4):
    {
        _text_rodata_begin = .;
        PROVIDE(text_rodata_begin = .);

        *startup.o(.text)

        *sfc_boot.o(.text*)
        *sfc_boot.o(.rodata*)
        *boot_main.o(.text*)
        *boot_main.o(.rodata*)

        . = ALIGN(4);
        __VERSION_BEGIN = .;
        KEEP(*(.sys.version))
        __VERSION_END = .;

        *(.text*)
        *(.rodata*)

#ifndef CONFIG_SFC_ENABLE
        *(.opus_enc_lib.text)
        *(.speex_enc_lib.text)
        *(.amr_enc_lib.text)
        *(.amr_dec_lib.text)
        *(.mp3_dec_lib.text)
        *(.wma_dec_lib.text)
        *(.m4a_dec_lib.text)
        *(.ape_dec_lib.text)
        *(.flac_dec_lib.text)
        *(.lib_resample_a.text)
        *(.libBtCalling.text)
        *(.libhtk.text)
        *(.libvad.text)
#endif

        _text_rodata_end = .;
        PROVIDE(text_rodata_end = .);

		. = ALIGN(4); // must at tail, make rom_code size align 4
    }

#ifdef CONFIG_SFC_ENABLE
	>rom
    . = ORIGIN(sdram);
#else
	> sdram

#endif

   	.data ALIGN(4):
	  {
#ifdef CONFIG_SFC_ENABLE
        *(.opus_enc_lib.text)
        *(.speex_enc_lib.text)
        *(.amr_enc_lib.text)
        *(.amr_dec_lib.text)
        *(.mp3_dec_lib.text)
        *(.wma_dec_lib.text)
        *(.m4a_dec_lib.text)
        *(.ape_dec_lib.text)
        *(.flac_dec_lib.text)
        *(.lib_resample_a.text)
        *(.libBtCalling.text)
        *(.libhtk.text)
        *(.libvad.text)
#endif

		. = ALIGN(4);
        *(.data)
        . = ALIGN(32);

		. = ALIGN(4);
		vfs_ops_begin = .;
			*(.vfs_operations)
		vfs_ops_end = .;

		_lcd_device_begin = .;
		PROVIDE(lcd_device_begin = .);
			*(.lcd_device)
		_lcd_device_end = .;
		PROVIDE(lcd_device_end = .);

		_lcd_device_drive_begin = .;
		PROVIDE(lcd_device_drive_begin = .);
			*(.lcd_device_drive)
		_lcd_device_drive_end = .;
		PROVIDE(lcd_device_drive_end = .);

        _gsensor_dev_begin = .;
		PROVIDE(gsensor_dev_begin = .);
			*(.gsensor_dev)
		_gsensor_dev_end = .;
		PROVIDE(gsensor_dev_end = .);

        _touch_panel_dev_begin = .;
        PROVIDE(touch_panel_dev_begin = .);
        *(.touch_panel_dev)
            _touch_panel_dev_end = .;
        PROVIDE(touch_panel_dev_end = .);

		_isp_scr_begin = .;
		PROVIDE(isp_scr_begin = .);
			*(.isp_scr_work)
		_isp_scr_end = .;
		PROVIDE(isp_scr_end = .);

		_power_dev_begin = .;
		PROVIDE(power_dev_begin = .);
			*(.power_dev)
		_power_dev_end = .;
		PROVIDE(power_dev_end = .);

		_eth_phy_device_begin = .;
		PROVIDE(eth_phy_device_begin = .);
			*(.eth_phy_device)
		_eth_phy_device_end = .;
		PROVIDE(eth_phy_device_end = .);

#ifdef CONFIG_PWM_ENABLE
		_pwm_device_begin = .;
		PROVIDE(pwm_device_begin = .);
		 *(.pwm_dev)
		_pwm_device_end = .;
		PROVIDE(pwm_device_end = .);
#endif

		INCLUDE include_lib/ui/ui/ui.ld
		INCLUDE include_lib/system/system.ld

        . = ALIGN(32);
		INCLUDE include_lib/system/cpu/AC521x/device.ld
		. = ALIGN(32);
		INCLUDE include_lib/system/device/device.ld
		INCLUDE include_lib/system/device/device_data.ld
		. = ALIGN(32);
		INCLUDE include_lib/system/system_data.ld

		INCLUDE include_lib/server/server.ld

		. = ALIGN(32);
#ifdef CONFIG_NET_ENABLE
		INCLUDE include_lib/net/device/device_data.ld
		INCLUDE include_lib/net/server/net_server.ld
#endif

		. = (( . + 31) / 32 * 32);
		_layer_buf_begin = .;
		PROVIDE(layer_buf_begin = .);
		*(.layer_buf_t)
		_layer_buf_end = .;
		PROVIDE(layer_buf_end = .);

		. = (( . + 31) / 32 * 32);
		_font_info_begin = .;
		PROVIDE(font_info_begin = .);
		*(.font_info_t)
		_font_info_end = .;
		PROVIDE(font_info_end = .);

		. = (( . + 31) / 32 * 32);
		_screen_tool_info_begin = .;
		PROVIDE(screen_tool_info_begin = .);
		*(.screen_tool_info)
		_screen_tool_info_end = .;
		PROVIDE(screen_tool_info_end = .);

	  } > sdram

    .bss ALIGN(32) :
    {
		*(.stack)
		_system_data_begin = .;
        *(.bss)
        *(COMMON)
		*(.usb_slave_var)
        *(.mem_heap)
		*(.memp_memory_x)
		. = (( . + 31) / 32 * 32);
		_lcd_dev_buf_addr = .;
		PROVIDE(lcd_dev_buf_addr = .);
		*(.lcd_dev_buf)
		_system_data_end = .;
		. = (( . + 31) / 32 * 32);
		*(.sys_malloc)
		. = (( . + 31) / 32 * 32);

    } > sdram
    _HEAP_BEGIN = ( . +31 )/ 32 *32 ;
    PROVIDE(HEAP_BEGIN = (. + 31) / 32 *32);
   _HEAP_END = 0x1000000 + SDRAM_SIZE - 32;
   PROVIDE(HEAP_END = 0x1000000 + SDRAM_SIZE - 32);
   _MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN;
   PROVIDE(MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN);


/********************************************/
    . =ORIGIN(ram0);
    .ram0_data ALIGN(4):
    {
        _VM_CODE_START = . ;
        *(.vm)
        _VM_CODE_END = . ;
        *(.flushinv_icache)

		. = ALIGN(4); // must at tail, make ram0_data size align 4
    } > ram0

    .ram0_bss ALIGN(4):
    {
        /*
         * sd卡全局变量要放sram，否则会读写出错
         */
        *(.sd_var)
		*(.sram)
        *(.usb0_fifo)
    } > ram0


    text_begin = ADDR(.rom_code ) ;
    text_size = SIZEOF(.rom_code ) ;
    bss_begin = ADDR(.bss ) ;
    bss_size  = SIZEOF(.bss);
    bss_size1 = _system_data_end - _system_data_begin;

    bss_begin1 = _system_data_begin;

    data_begin = LOADADDR(.data)  ;
    data_addr  = ADDR(.data) ;
    data_size =  SIZEOF(.data) ;

    data_vma  = ADDR(.data);
    data_lma =  text_begin + SIZEOF(.rom_code);

/********************************************/
    _ram0_bss_vma = ADDR(.ram0_bss);
    _ram0_bss_size  =  SIZEOF(.ram0_bss);
    _ram0_data_vma  = ADDR(.ram0_data);
    _ram0_data_lma =  text_begin + SIZEOF(.rom_code) + SIZEOF(.data);
    _ram0_data_size =  SIZEOF(.ram0_data);

    /* _ram2_bss_vma = ADDR(.ram2_bss); */
    /* _ram2_bss_size  =  SIZEOF(.ram2_bss); */
    /* _ram2_data_vma  = ADDR(.ram2_data); */
    /* _ram2_data_lma =  text_begin + SIZEOF(.rom_code)+SIZEOF(.ram0_data); */
    /* _ram2_data_size =  SIZEOF(.ram2_data); */

/********************************************/
}

EXTERN(
lib_rtos_version
lib_sys_version
lib_fs_version
lib_sd1_version
lib_vm_version
lib_dev_version
lib_audio_dev_version
lib_ui_version
lib_mm_version
lib_cbuf_version
lib_cpu_version
lib_cpu_video_version
lib_cpu_disp_version
lib_usb_server_version
lib_audio_server_version
lib_video_rec_server_version
lib_video_dec_server_version

)

