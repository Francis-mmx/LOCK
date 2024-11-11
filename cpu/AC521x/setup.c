#include "asm/includes.h"
#include "asm/sysinfo.h"
#include "asm/ldo.h"
#include "asm/cache.h"
#include "system/task.h"

#include "app_config.h"
#include "power_manage.h"

extern void dv15_dac_early_init(u8 ldo_sel, u8 pwr_sel, u32 dly_msecs);


#ifdef CONFIG_DEBUG_ENABLE
    extern void debug_uart_init();
#endif

static char *debug_msg[32] =
{
    /*0---7*/
    "reserved",
    "reserved",
    "prp_ex_limit_err",
    "sdr_wr_err",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    /*8---15*/
    "c1_div_zero",
    "c0_div_zero",
    "c1_pc_limit_err_r",
    "c1_wr_limit_err_r",
    "c0_pc_limit_err_r",
    "c0_wr_limit_err_r",
    "c1_misaligned",
    "c0_misaligned",
    /*16---23*/
    "c1_if_bus_inv",
    "c1_rd_bus_inv",
    "c1_wr_bus_inv",
    "c0_if_bus_inv",
    "c0_rd_bus_inv",
    "c0_wr_bus_inv",
    "prp_bus_inv",
    "reserved",
    /*24---31*/
    "c1_mmu_wr_excpt",
    "c1_mmu_rd_excpt",
    "c0_mmu_wr_excpt",
    "c0_mmu_rd_excpt",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
};


___interrupt
void exception_irq_handler(void)
{
    u32 rets_addr, reti_addr;
    u32 i;
    u32 tmp_sp, tmp_usp, tmp_ssp;
    u32 *tmp_sp_ptr;


    __asm__ volatile("[--sp] = {r15-r0}");

    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));
    __asm__ volatile("%0 = reti ;" : "=r"(reti_addr));

    __asm__ volatile("%0 = sp ;" : "=r"(tmp_sp));
    __asm__ volatile("%0 = usp ;" : "=r"(tmp_usp));
    __asm__ volatile("%0 = ssp ;" : "=r"(tmp_ssp));


    FPGA_TRI = 0;

    /*puts("\nput sp\n");
    tmp_sp_ptr = (u32 *)tmp_sp;
    for (i = 0; i < 20; i++) {
        put_u32hex(*tmp_sp_ptr);
        tmp_sp_ptr++;
    }*/

#if 1
    if(current_cpu_id() == 0)
    {
        printf("!!!!! cpu 0 %s: rets_addr = 0x%x, reti_addr = 0x%x\n DEBUG_MSG = 0x%x, DEBUG_PRP_NUM = 0x%x DSPCON=%x\n",
               __func__, rets_addr, reti_addr, DEBUG_MSG, DEBUG_WRNUM, DSPCON);
    }
    else
    {
        /*printf("\n\n\n!!!!! cpu 1 %s: rets_addr = 0x%x, reti_addr = 0x%x\n DEBUG_MSG = 0x%x, DEBUG_PRP_NUM = 0x%x DSPCON=%x\n",*/
        /*__func__, rets_addr, reti_addr, DEBUG_MSG, DEBUG_PRP_NUM, C1_CON);*/
    }

    printf("\nsp : 0x%x, usp : 0x%x, ssp : 0x%x\n", tmp_sp, tmp_usp, tmp_ssp);

    printf("\nWR_LIM4H : 0x%x, WR_LIM4L : 0x%x, SDRDBG : 0x%x\n", WR_LIM4H, WR_LIM4L, SDRDBG);

    for(i = 0; i < 32; i++)
    {
        if(BIT(i)&DEBUG_MSG)
        {
            puts(debug_msg[i]);
        }
    }
#endif

    log_flush();
    while(1);
}

static void cpu_xbus_init()
{
#if 1
    u8 level;

    level = 0;
    xbus_ch01_lvl = level; //isp0 stc wr
    xbus_ch19_lvl = level; //imc ch0 osd
    xbus_ch20_lvl = level; //imc ch1 osd
    xbus_ch06_lvl = level; //imc ch3 wr display
    xbus_ch07_lvl = level; //imr0 wr
    xbus_ch23_lvl = level; //imr0 rd

    level = 1;
    xbus_ch18_lvl = level; //imc replay
    xbus_ch08_lvl = level; //imr1 wr
    xbus_ch24_lvl = level; //imr1 rd
    xbus_ch09_lvl = level; //imb wr
    xbus_ch25_lvl = level; //imb rd
    xbus_ch10_lvl = level; //imc ch4 wr

    level = 3;
    xbus_ch03_lvl = level; //imc ch0 wr encode
    xbus_ch04_lvl = level; //imc ch1 wr encode
    xbus_ch05_lvl = level; //imc ch2 wr display
    xbus_ch26_lvl = level; //imd rd

    xbus_lv1_prd = 2;
    xbus_lv2_prd = 8;

#endif
}

extern u32 text_rodata_begin, text_rodata_end;

void cpu1_main()
{
    local_irq_disable();

    interrupt_init();

    request_irq(1, 7, exception_irq_handler, 1);

    /*debug_init();*/

    os_start();

    local_irq_enable();

    while(1)
    {
        __asm__ volatile("idle");
    }
}

static void wdt_init()
{
    /*
     * 超时: 0-15 对应 {1ms, 2ms, 4ms, 8ms, ...512ms, 1s, 2s, 4s, 8s, 16s, 32s}
     */
    CLK_CON0 |= BIT(9);
    CRC1_REG = 0x6EA5;
    WDT_CON = BIT(6) | BIT(4) | 0x0c;
    CRC1_REG = 0;
}

__attribute__((noinline))
void clr_wdt()
{
    WDT_CON |= BIT(6);
    //do st here

}

void close_wdt()
{
    CRC1_REG = 0x6EA5;
    WDT_CON &= ~BIT(4);
    CRC1_REG = 0;
}




u32 sdfile_init(u32 cmd_zone_addr, u32 *head_addr, int num);

/*
 * 此函数在cpu0上电后首先被调用,负责初始化cpu内部模块
 *
 * 此函数返回后，操作系统才开始初始化并运行
 *
 */

#if 0
static void early_putchar(char a)
{
    if(a == '\n')
    {
        UT2_BUF = '\r';
        __asm_csync();
        while((UT2_CON & BIT(15)) == 0);
    }
    UT2_BUF = a;
    __asm_csync();
    while((UT2_CON & BIT(15)) == 0);
}

void early_puts(char *s)
{
    do
    {
        early_putchar(*s);
    }
    while(*(++s));
}
#endif

void setup_arch()
{
    wdt_init();

    clk_early_init();

    interrupt_init();

#ifdef CONFIG_DEBUG_ENABLE
    debug_uart_init();
#endif
    log_early_init(8 * 1024);

    puts("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    printf("\n         setup_arch %s %s", __DATE__, __TIME__);
    puts("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

#if 0
    dcache_way_use_select(1, 1); /* CPU和PRP各使用2个独立的WAY */
#endif

    dv15_dac_early_init(1, 1, 1000);

    rtc_early_init();

    sys_power_early_init();

    cpu_xbus_init();
#ifndef CONFIG_SFC_ENABLE
    debug_init();
    /*sdr_write_range_limit((void *)NO_CACHE_ADDR(&text_rodata_begin),
                          (u32)&text_rodata_end - (u32)&text_rodata_begin, false, 0);*/
#endif

    /*sdfile_init(boot.cmd_zone_addr, boot.sdfile_head_addr, 2);*/


    request_irq(1, 7, exception_irq_handler, 0);

}








