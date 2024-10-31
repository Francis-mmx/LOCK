
/*--------------------------------------------------------------------------*/
/**@file     sh60.h
   @brief    芯片资源头文件
   @details
   @author
   @date    2011-3-7
   @note    CD003
*/
/*----------------------------------------------------------------------------*/

#ifndef _AC521x_
#define _AC521x_

//Note:
//there are 256 words(1024 bytes) in the sfr space
//byte(8bit)       SFR offset-address is:   0x03, 0x07, 0x0b, 0x0f, 0x13 ......
//half-word(16bit) SFR offset-address is:   0x02, 0x06, 0x0a, 0x0e, 0x12 ......
//word(24/32bit)   SFR offset-address is:   0x00, 0x04, 0x08, 0x0c, 0x10 ......

#define hs_base     0x0f70000
#define ls_base     0x0f60000

//===============================================================================//
//
//      high speed sfr address define
//
//===============================================================================//
#define ls_io_base    (ls_base + 0x000*4)
#define ls_uart_base  (ls_base + 0x100*4)
#define ls_spi_base   (ls_base + 0x200*4)
#define ls_sd_base    (ls_base + 0x300*4)
#define ls_tmr_base   (ls_base + 0x400*4)
#define ls_fusb_base  (ls_base + 0x500*4)
#define ls_husb_base  (ls_base + 0x600*4)
#define ls_adda_base  (ls_base + 0x700*4)
#define ls_clk_base   (ls_base + 0x800*4)
#define ls_oth_base   (ls_base + 0x900*4)
#define ls_alnk_base  (ls_base + 0xa00*4)

#define hs_cpu_base   (hs_base + 0x000*4)
#define hs_dbg_base   (hs_base + 0x100*4)
#define hs_sdr_base   (hs_base + 0x200*4)
#define hs_eva_base   (hs_base + 0x300*4)
#define hs_sfc_base   (hs_base + 0x400*4)
#define hs_jpg_base   (hs_base + 0x500*4)
#define hs_oth_base   (hs_base + 0x600*4)

#define PORTA_OUT               (*(volatile u16 *)(ls_io_base + 0x00*4))         //
#define PORTA_IN                (*(volatile u16 *)(ls_io_base + 0x01*4))         //Read Only
#define PORTA_DIR               (*(volatile u16 *)(ls_io_base + 0x02*4))         //
#define PORTA_DIE               (*(volatile u16 *)(ls_io_base + 0x03*4))         //
#define PORTA_PU                (*(volatile u16 *)(ls_io_base + 0x04*4))         //
#define PORTA_PD                (*(volatile u16 *)(ls_io_base + 0x05*4))         //
#define PORTA_HD                (*(volatile u16 *)(ls_io_base + 0x06*4))         //

#define PORTB_OUT               (*(volatile u16 *)(ls_io_base + 0x10*4))         //
#define PORTB_IN                (*(volatile u16 *)(ls_io_base + 0x11*4))         //Read Only
#define PORTB_DIR               (*(volatile u16 *)(ls_io_base + 0x12*4))         //
#define PORTB_DIE               (*(volatile u16 *)(ls_io_base + 0x13*4))         //
#define PORTB_PU                (*(volatile u16 *)(ls_io_base + 0x14*4))         //
#define PORTB_PD                (*(volatile u16 *)(ls_io_base + 0x15*4))         //
#define PORTB_HD                (*(volatile u16 *)(ls_io_base + 0x16*4))         //

#define PORTC_OUT               (*(volatile u16 *)(ls_io_base + 0x20*4))         //
#define PORTC_IN                (*(volatile u16 *)(ls_io_base + 0x21*4))         //Read Only
#define PORTC_DIR               (*(volatile u16 *)(ls_io_base + 0x22*4))         //
#define PORTC_DIE               (*(volatile u16 *)(ls_io_base + 0x23*4))         //
#define PORTC_PU                (*(volatile u16 *)(ls_io_base + 0x24*4))         //
#define PORTC_PD                (*(volatile u16 *)(ls_io_base + 0x25*4))         //
#define PORTC_HD                (*(volatile u16 *)(ls_io_base + 0x26*4))         //

#define PORTD_OUT               (*(volatile u16 *)(ls_io_base + 0x30*4))         //
#define PORTD_IN                (*(volatile u16 *)(ls_io_base + 0x31*4))         //Read Only
#define PORTD_DIR               (*(volatile u16 *)(ls_io_base + 0x32*4))         //
#define PORTD_DIE               (*(volatile u16 *)(ls_io_base + 0x33*4))         //
#define PORTD_PU                (*(volatile u16 *)(ls_io_base + 0x34*4))         //
#define PORTD_PD                (*(volatile u16 *)(ls_io_base + 0x35*4))         //
#define PORTD_HD                (*(volatile u16 *)(ls_io_base + 0x36*4))         //

#define PORTE_OUT               (*(volatile u16 *)(ls_io_base + 0x40*4))         //
#define PORTE_IN                (*(volatile u16 *)(ls_io_base + 0x41*4))         //Read Only
#define PORTE_DIR               (*(volatile u16 *)(ls_io_base + 0x42*4))         //
#define PORTE_DIE               (*(volatile u16 *)(ls_io_base + 0x43*4))         //
#define PORTE_PU                (*(volatile u16 *)(ls_io_base + 0x44*4))         //
#define PORTE_PD                (*(volatile u16 *)(ls_io_base + 0x45*4))         //
#define PORTE_HD                (*(volatile u16 *)(ls_io_base + 0x46*4))         //

#define PORTF_OUT               (*(volatile u16 *)(ls_io_base + 0x50*4))         //
#define PORTF_IN                (*(volatile u16 *)(ls_io_base + 0x51*4))         //Read Only
#define PORTF_DIR               (*(volatile u16 *)(ls_io_base + 0x52*4))         //
#define PORTF_DIE               (*(volatile u16 *)(ls_io_base + 0x53*4))         //
#define PORTF_PU                (*(volatile u16 *)(ls_io_base + 0x54*4))         //
#define PORTF_PD                (*(volatile u16 *)(ls_io_base + 0x55*4))         //
#define PORTF_HD                (*(volatile u16 *)(ls_io_base + 0x56*4))         //

#define PORTG_OUT               (*(volatile u16 *)(ls_io_base + 0x60*4))         //
#define PORTG_IN                (*(volatile u16 *)(ls_io_base + 0x61*4))         //Read Only
#define PORTG_DIR               (*(volatile u16 *)(ls_io_base + 0x62*4))         //
#define PORTG_DIE               (*(volatile u16 *)(ls_io_base + 0x63*4))         //
#define PORTG_PU                (*(volatile u16 *)(ls_io_base + 0x64*4))         //
#define PORTG_PD                (*(volatile u16 *)(ls_io_base + 0x65*4))         //
#define PORTG_HD                (*(volatile u16 *)(ls_io_base + 0x66*4))         //

#define PORTH_OUT               (*(volatile u16 *)(ls_io_base + 0x70*4))         //
#define PORTH_IN                (*(volatile u16 *)(ls_io_base + 0x71*4))         //Read Only
#define PORTH_DIR               (*(volatile u16 *)(ls_io_base + 0x72*4))         //
#define PORTH_DIE               (*(volatile u16 *)(ls_io_base + 0x73*4))         //
#define PORTH_PU                (*(volatile u16 *)(ls_io_base + 0x74*4))         //
#define PORTH_PD                (*(volatile u16 *)(ls_io_base + 0x75*4))         //
#define PORTH_HD                (*(volatile u16 *)(ls_io_base + 0x76*4))         //

#define IOMC0                   (*(volatile u32 *)(ls_io_base + 0x80*4))         //
#define IOMC1                   (*(volatile u32 *)(ls_io_base + 0x81*4))         //
#define IOMC2                   (*(volatile u32 *)(ls_io_base + 0x82*4))         //
#define IOMC3                   (*(volatile u32 *)(ls_io_base + 0x83*4))         //
#define IOMC4                   (*(volatile u32 *)(ls_io_base + 0x84*4))         //
#define WKUP_CON0               (*(volatile u32 *)(ls_io_base + 0x85*4))         //
#define WKUP_CON1               (*(volatile u32 *)(ls_io_base + 0x86*4))         //
#define WKUP_CON2               (*(volatile u32 *)(ls_io_base + 0x87*4))         //write only;
#define WKUP_CON3               (*(volatile u16 *)(ls_io_base + 0x88*4))         //


#define UT0_HRXCNT              (*(volatile u32 *)(ls_uart_base + 0x00*4))
#define UT0_OTCNT               (*(volatile u32 *)(ls_uart_base + 0x01*4))
#define UT0_TXADR               (*(volatile u32 *)(ls_uart_base + 0x02*4))       //26bit write only;
#define UT0_TXCNT               (*(volatile u32 *)(ls_uart_base + 0x03*4))
#define UT0_RXEADR              (*(volatile u32 *)(ls_uart_base + 0x04*4))       //26bit write only;
#define UT0_CON                 (*(volatile u16 *)(ls_uart_base + 0x05*4))
#define UT0_BUF                 (*(volatile u8  *)(ls_uart_base + 0x06*4))
#define UT0_BAUD                (*(volatile u16 *)(ls_uart_base + 0x07*4))       //16bit write only;
#define UT0_RXSADR              (*(volatile u32 *)(ls_uart_base + 0x08*4))       //26bit write only;
#define UT0_RXCNT               (*(volatile u32 *)(ls_uart_base + 0x09*4))

#define UT1_HRXCNT              (*(volatile u32 *)(ls_uart_base + 0x10*4))
#define UT1_OTCNT               (*(volatile u32 *)(ls_uart_base + 0x11*4))
#define UT1_TXADR               (*(volatile u32 *)(ls_uart_base + 0x12*4))       //26bit write only;
#define UT1_TXCNT               (*(volatile u32 *)(ls_uart_base + 0x13*4))
#define UT1_RXEADR              (*(volatile u32 *)(ls_uart_base + 0x14*4))       //26bit write only;
#define UT1_CON                 (*(volatile u16 *)(ls_uart_base + 0x15*4))
#define UT1_BUF                 (*(volatile u8  *)(ls_uart_base + 0x16*4))
#define UT1_BAUD                (*(volatile u16 *)(ls_uart_base + 0x17*4))       //16bit write only;
#define UT1_RXSADR              (*(volatile u32 *)(ls_uart_base + 0x18*4))       //26bit write only;
#define UT1_RXCNT               (*(volatile u32 *)(ls_uart_base + 0x19*4))

#define UT2_HRXCNT              (*(volatile u32 *)(ls_uart_base + 0x20*4))
#define UT2_OTCNT               (*(volatile u32 *)(ls_uart_base + 0x21*4))
#define UT2_TXADR               (*(volatile u32 *)(ls_uart_base + 0x22*4))       //26bit write only;
#define UT2_TXCNT               (*(volatile u32 *)(ls_uart_base + 0x23*4))
#define UT2_RXEADR              (*(volatile u32 *)(ls_uart_base + 0x24*4))       //26bit write only;
#define UT2_CON                 (*(volatile u16 *)(ls_uart_base + 0x25*4))
#define UT2_BUF                 (*(volatile u8  *)(ls_uart_base + 0x26*4))
#define UT2_BAUD                (*(volatile u16 *)(ls_uart_base + 0x27*4))       //16bit write only;
#define UT2_RXSADR              (*(volatile u32 *)(ls_uart_base + 0x28*4))       //26bit write only;
#define UT2_RXCNT               (*(volatile u32 *)(ls_uart_base + 0x29*4))

#define UT3_CON                 (*(volatile u16 *)(ls_uart_base + 0x30*4))
#define UT3_BUF                 (*(volatile u8  *)(ls_uart_base + 0x31*4))
#define UT3_BAUD                (*(volatile u16 *)(ls_uart_base + 0x32*4))       //write only;

//SPI
#define SPI0_CON                (*(volatile u16 *)(ls_spi_base + 0x00*4))
#define SPI0_BAUD               (*(volatile u8  *)(ls_spi_base + 0x01*4))
#define SPI0_BUF                (*(volatile u8  *)(ls_spi_base + 0x02*4))
#define SPI0_ADR                (*(volatile u32 *)(ls_spi_base + 0x03*4))       //26bit write only;
#define SPI0_CNT                (*(volatile u16 *)(ls_spi_base + 0x04*4))       //write only;

#define SPI1_CON                (*(volatile u16 *)(ls_spi_base + 0x10*4))
#define SPI1_BAUD               (*(volatile u8  *)(ls_spi_base + 0x11*4))
#define SPI1_BUF                (*(volatile u8  *)(ls_spi_base + 0x12*4))
#define SPI1_ADR                (*(volatile u32 *)(ls_spi_base + 0x13*4))       //26bit write only;
#define SPI1_CNT                (*(volatile u16 *)(ls_spi_base + 0x14*4))       //write only;

//SD
#define SD0_CON0                (*(volatile u16 *)(ls_sd_base + 0x00*4))
#define SD0_CON1                (*(volatile u16 *)(ls_sd_base + 0x01*4))
#define SD0_CON2                (*(volatile u16 *)(ls_sd_base + 0x02*4))
#define SD0_CPTR                (*(volatile u32 *)(ls_sd_base + 0x03*4))        //26bit write only;
#define SD0_DPTR                (*(volatile u32 *)(ls_sd_base + 0x04*4))        //26bit write only;
#define SD0_CTU_CON             (*(volatile u16 *)(ls_sd_base + 0x05*4))
#define SD0_CTU_CNT             (*(volatile u16 *)(ls_sd_base + 0x06*4))

#define SD1_CON0                (*(volatile u16 *)(ls_sd_base + 0x10*4))
#define SD1_CON1                (*(volatile u16 *)(ls_sd_base + 0x11*4))
#define SD1_CON2                (*(volatile u16 *)(ls_sd_base + 0x12*4))
#define SD1_CPTR                (*(volatile u32 *)(ls_sd_base + 0x13*4))        //nbit
#define SD1_DPTR                (*(volatile u32 *)(ls_sd_base + 0x14*4))        //nbit
#define SD1_CTU_CON             (*(volatile u16 *)(ls_sd_base + 0x15*4))
#define SD1_CTU_CNT             (*(volatile u16 *)(ls_sd_base + 0x16*4))

#define SD2_CON0                (*(volatile u16 *)(ls_sd_base + 0x20*4))
#define SD2_CON1                (*(volatile u16 *)(ls_sd_base + 0x21*4))
#define SD2_CON2                (*(volatile u16 *)(ls_sd_base + 0x22*4))
#define SD2_CPTR                (*(volatile u32 *)(ls_sd_base + 0x23*4))        //nbit
#define SD2_DPTR                (*(volatile u32 *)(ls_sd_base + 0x24*4))        //nbit
#define SD2_CTU_CON             (*(volatile u16 *)(ls_sd_base + 0x25*4))
#define SD2_CTU_CNT             (*(volatile u16 *)(ls_sd_base + 0x26*4))


//TIMER

#define T0_CON                  (*(volatile u16 *)(ls_tmr_base + 0x00*4))
#define T0_CNT                  (*(volatile u32 *)(ls_tmr_base + 0x01*4))
#define T0_PRD                  (*(volatile u32 *)(ls_tmr_base + 0x02*4))
#define T0_PWM                  (*(volatile u32 *)(ls_tmr_base + 0x03*4))

#define T1_CON                  (*(volatile u16 *)(ls_tmr_base + 0x10*4))
#define T1_CNT                  (*(volatile u32 *)(ls_tmr_base + 0x11*4))
#define T1_PRD                  (*(volatile u32 *)(ls_tmr_base + 0x12*4))
#define T1_PWM                  (*(volatile u32 *)(ls_tmr_base + 0x13*4))

#define T2_CON                  (*(volatile u16 *)(ls_tmr_base + 0x20*4))
#define T2_CNT                  (*(volatile u32 *)(ls_tmr_base + 0x21*4))
#define T2_PRD                  (*(volatile u32 *)(ls_tmr_base + 0x22*4))
#define T2_PWM                  (*(volatile u32 *)(ls_tmr_base + 0x23*4))

#define T3_CON                  (*(volatile u16 *)(ls_tmr_base + 0x30*4))
#define T3_CNT                  (*(volatile u32 *)(ls_tmr_base + 0x31*4))
#define T3_PRD                  (*(volatile u32 *)(ls_tmr_base + 0x32*4))
#define T3_PWM                  (*(volatile u32 *)(ls_tmr_base + 0x33*4))

#define PWMTMR0CON              (*(volatile u32 *)(ls_tmr_base + 0x40*4))
#define PWMTMR0CNT              (*(volatile u32 *)(ls_tmr_base + 0x41*4))
#define PWMTMR0PR               (*(volatile u32 *)(ls_tmr_base + 0x42*4))
#define PWMCMP0                 (*(volatile u32 *)(ls_tmr_base + 0x43*4))
#define PWMTMR1CON              (*(volatile u32 *)(ls_tmr_base + 0x44*4))
#define PWMTMR1CNT              (*(volatile u32 *)(ls_tmr_base + 0x45*4))
#define PWMTMR1PR               (*(volatile u32 *)(ls_tmr_base + 0x46*4))
#define PWMCMP1                 (*(volatile u32 *)(ls_tmr_base + 0x47*4))
#define PWMTMR2CON              (*(volatile u32 *)(ls_tmr_base + 0x48*4))
#define PWMTMR2CNT              (*(volatile u32 *)(ls_tmr_base + 0x49*4))
#define PWMTMR2PR               (*(volatile u32 *)(ls_tmr_base + 0x4a*4))
#define PWMCMP2                 (*(volatile u32 *)(ls_tmr_base + 0x4b*4))
#define PWMTMR3CON              (*(volatile u32 *)(ls_tmr_base + 0x4c*4))
#define PWMTMR3CNT              (*(volatile u32 *)(ls_tmr_base + 0x4d*4))
#define PWMTMR3PR               (*(volatile u32 *)(ls_tmr_base + 0x4e*4))
#define PWMCMP3                 (*(volatile u32 *)(ls_tmr_base + 0x4f*4))
#define PWMTMR4CON              (*(volatile u32 *)(ls_tmr_base + 0x50*4))
#define PWMTMR4CNT              (*(volatile u32 *)(ls_tmr_base + 0x51*4))
#define PWMTMR4PR               (*(volatile u32 *)(ls_tmr_base + 0x52*4))
#define PWMCMP4                 (*(volatile u32 *)(ls_tmr_base + 0x53*4))
#define PWMTMR5CON              (*(volatile u32 *)(ls_tmr_base + 0x54*4))
#define PWMTMR5CNT              (*(volatile u32 *)(ls_tmr_base + 0x55*4))
#define PWMTMR5PR               (*(volatile u32 *)(ls_tmr_base + 0x56*4))
#define PWMCMP5                 (*(volatile u32 *)(ls_tmr_base + 0x57*4))
#define PWMCON0                 (*(volatile u32 *)(ls_tmr_base + 0x58*4))
#define PWMCON1                 (*(volatile u32 *)(ls_tmr_base + 0x59*4))


//FUSB
#define FUSB_CON0               (*(volatile u32 *)(ls_fusb_base + 0x00*4))
#define FUSB_CON1               (*(volatile u32 *)(ls_fusb_base + 0x01*4))
#define FUSB_EP0_CNT            (*(volatile u16 *)(ls_fusb_base + 0x02*4))      //write only;
#define FUSB_EP1_CNT            (*(volatile u16 *)(ls_fusb_base + 0x03*4))      //write only;
#define FUSB_EP2_CNT            (*(volatile u16 *)(ls_fusb_base + 0x04*4))      //write only;
#define FUSB_EP3_CNT            (*(volatile u16 *)(ls_fusb_base + 0x05*4))      //write only;
#define FUSB_EP0_ADR            (*(volatile u32 *)(ls_fusb_base + 0x06*4))      //26bit write only;
#define FUSB_EP1_TADR           (*(volatile u32 *)(ls_fusb_base + 0x07*4))      //write only;
#define FUSB_EP1_RADR           (*(volatile u32 *)(ls_fusb_base + 0x08*4))      //write only;
#define FUSB_EP2_TADR           (*(volatile u32 *)(ls_fusb_base + 0x09*4))      //write only;
#define FUSB_EP2_RADR           (*(volatile u32 *)(ls_fusb_base + 0x0a*4))      //write only;
#define FUSB_EP3_TADR           (*(volatile u32 *)(ls_fusb_base + 0x0b*4))      //write only;
#define FUSB_EP3_RADR           (*(volatile u32 *)(ls_fusb_base + 0x0c*4))      //write only;
#define FUSB_IO_CON0            (*(volatile u16 *)(ls_fusb_base + 0x0d*4))      //

#define HUSB_SIE_CON            (*(volatile u32 *)(ls_husb_base + 0x00*4))
#define HUSB_EP0_CNT            (*(volatile u16 *)(ls_husb_base + 0x01*4))      //write only;
#define HUSB_EP1_CNT            (*(volatile u16 *)(ls_husb_base + 0x02*4))      //write only;
#define HUSB_EP2_CNT            (*(volatile u16 *)(ls_husb_base + 0x03*4))      //write only;
#define HUSB_EP3_CNT            (*(volatile u32 *)(ls_husb_base + 0x04*4))      //write only;
#define HUSB_EP4_CNT            (*(volatile u32 *)(ls_husb_base + 0x05*4))      //write only;
#define HUSB_EP5_CNT            (*(volatile u16 *)(ls_husb_base + 0x06*4))      //write only;
#define HUSB_EP6_CNT            (*(volatile u16 *)(ls_husb_base + 0x07*4))      //write only;
#define HUSB_EP1_TADR           (*(volatile u32 *)(ls_husb_base + 0x09*4))      //26bit write only;
#define HUSB_EP1_RADR           (*(volatile u32 *)(ls_husb_base + 0x0a*4))      //26bit write only;
#define HUSB_EP2_TADR           (*(volatile u32 *)(ls_husb_base + 0x0b*4))      //26bit write only;
#define HUSB_EP2_RADR           (*(volatile u32 *)(ls_husb_base + 0x0c*4))      //26bit write only;
#define HUSB_EP3_TADR           (*(volatile u32 *)(ls_husb_base + 0x0d*4))      //26bit write only;
#define HUSB_EP3_RADR           (*(volatile u32 *)(ls_husb_base + 0x0e*4))      //26bit write only;
#define HUSB_EP4_TADR           (*(volatile u32 *)(ls_husb_base + 0x0f*4))      //26bit write only;
#define HUSB_EP4_RADR           (*(volatile u32 *)(ls_husb_base + 0x10*4))      //26bit write only;
#define HUSB_EP5_TADR           (*(volatile u32 *)(ls_husb_base + 0x11*4))      //26bit write only;
#define HUSB_EP5_RADR           (*(volatile u32 *)(ls_husb_base + 0x12*4))      //26bit write only;
#define HUSB_EP6_TADR           (*(volatile u32 *)(ls_husb_base + 0x13*4))      //26bit write only;
#define HUSB_EP6_RADR           (*(volatile u32 *)(ls_husb_base + 0x14*4))      //26bit write only;

#define HUSB_COM_CON0           (*(volatile u32 *)(ls_husb_base + 0x15*4))
#define HUSB_COM_CON1           (*(volatile u32 *)(ls_husb_base + 0x16*4))
#define HUSB_PHY_CON0           (*(volatile u32 *)(ls_husb_base + 0x17*4))
#define HUSB_PHY_CON1           (*(volatile u32 *)(ls_husb_base + 0x18*4))
#define HUSB_PHY_CON2           (*(volatile u32 *)(ls_husb_base + 0x19*4))
#define HUSB_ISO_CON0           (*(volatile u32 *)(ls_husb_base + 0x1a*4))
#define HUSB_ISO_CON1           (*(volatile u32 *)(ls_husb_base + 0x1b*4))

#define DAC_CON                 (*(volatile u16 *)(ls_adda_base + 0x00*4))
#define DAC_CON1                (*(volatile u16 *)(ls_adda_base + 0x01*4))
#define DAC_TRML                (*(volatile u8  *)(ls_adda_base + 0x02*4))      //8bit write only;
#define DAC_TRMR                (*(volatile u8  *)(ls_adda_base + 0x03*4))      //8bit write only;
#define DAC_ADR                 (*(volatile u32 *)(ls_adda_base + 0x04*4))      //26bit write only;
#define DAC_LEN                 (*(volatile u16 *)(ls_adda_base + 0x05*4))      //16bit write only;

#define ADC_CON                 (*(volatile u16 *)(ls_adda_base + 0x08*4))
#define ADC_ADR                 (*(volatile u32 *)(ls_adda_base + 0x09*4))      //26bit write only;
#define ADC_LEN                 (*(volatile u16 *)(ls_adda_base + 0x0a*4))      //16bit write only;

#define DAA_CON0                (*(volatile u32 *)(ls_adda_base + 0x10*4))
#define DAA_CON1                (*(volatile u32 *)(ls_adda_base + 0x11*4))
#define DAA_CON2                (*(volatile u32 *)(ls_adda_base + 0x12*4))
//#define DAA_CON3                (*(volatile u32 *)(ls_adda_base + 0x13*4))
//#define DAA_CON4                (*(volatile u32 *)(ls_adda_base + 0x14*4))
#define DAA_CON5                (*(volatile u32 *)(ls_adda_base + 0x15*4))

#define ADA_CON0                (*(volatile u32 *)(ls_adda_base + 0x18*4))
//#define ADA_CON1                (*(volatile u32 *)(ls_adda_base + 0x19*4))

#define PWR_CON                 (*(volatile u8  *)(ls_clk_base + 0x00*4))
#define CLK_CON0                (*(volatile u32 *)(ls_clk_base + 0x01*4))
#define CLK_CON1                (*(volatile u32 *)(ls_clk_base + 0x02*4))
#define CLK_CON2                (*(volatile u32 *)(ls_clk_base + 0x03*4))
#define LCLK_GAT                (*(volatile u32 *)(ls_clk_base + 0x04*4))
#define HCLK_GAT                (*(volatile u32 *)(ls_clk_base + 0x05*4))
#define ACLK_GAT                (*(volatile u32 *)(ls_clk_base + 0x06*4))
#define PLL0_NF                 (*(volatile u32 *)(ls_clk_base + 0x07*4))
#define PLL0_NR                 (*(volatile u32 *)(ls_clk_base + 0x08*4))
#define PLL1_NF                 (*(volatile u32 *)(ls_clk_base + 0x09*4))
#define PLL1_NR                 (*(volatile u32 *)(ls_clk_base + 0x0a*4))
#define OSA_CON                 (*(volatile u16 *)(ls_clk_base + 0x0b*4))
#define PLL_CON0                (*(volatile u32 *)(ls_clk_base + 0x0c*4))
#define PLL_CON1                (*(volatile u32 *)(ls_clk_base + 0x0d*4))
#define PLL_CON2                (*(volatile u32 *)(ls_clk_base + 0x0e*4))
#define PLL3_NF                 (*(volatile u32 *)(ls_clk_base + 0x0f*4))
#define PLL3_NR                 (*(volatile u32 *)(ls_clk_base + 0x10*4))
#define PLL_CON3                (*(volatile u32 *)(ls_clk_base + 0x11*4))
#define PLL_CON4                (*(volatile u32 *)(ls_clk_base + 0x12*4))
#define PLL_CON5                (*(volatile u32 *)(ls_clk_base + 0x13*4))
#define PLL_CON6                (*(volatile u32 *)(ls_clk_base + 0x14*4))

#define HTC_CON                 (*(volatile u16 *)(ls_oth_base + 0x00*4))
#define LDO_CON                 (*(volatile u16 *)(ls_oth_base + 0x01*4))
#define LVD_CON                 (*(volatile u16 *)(ls_oth_base + 0x02*4))
#define IRTC_CON                (*(volatile u16 *)(ls_oth_base + 0x03*4))
#define IRTC_BUF                (*(volatile u8  *)(ls_oth_base + 0x04*4))
#define MODE_CON                (*(volatile u8  *)(ls_oth_base + 0x05*4))
#define CRC0_FIFO                (*(volatile u8  *)(ls_oth_base + 0x06*4))       //write only;
#define CRC0_REG                 (*(volatile u16 *)(ls_oth_base + 0x07*4))
#define WDT_CON                 (*(volatile u8  *)(ls_oth_base + 0x08*4))
#define CHIP_ID                 (*(volatile u8  *)(ls_oth_base + 0x09*4))       //read only;
#define IRFLT_CON               (*(volatile u8  *)(ls_oth_base + 0x0a*4))
#define IIC_CON                 (*(volatile u16 *)(ls_oth_base + 0x0b*4))
#define IIC_BUF                 (*(volatile u8  *)(ls_oth_base + 0x0c*4))
#define IIC_BAUD                (*(volatile u8  *)(ls_oth_base + 0x0d*4))
#define IIC_DMA_ADR             (*(volatile u32 *)(ls_oth_base + 0x0e*4))       //write only
#define IIC_DMA_CNT             (*(volatile u16 *)(ls_oth_base + 0x0f*4))       //write only
#define IIC_DMA_NRATE           (*(volatile u16 *)(ls_oth_base + 0x10*4))       //write only
#define PWM8_CON                (*(volatile u16 *)(ls_oth_base + 0x11*4))
#define PAP_CON                 (*(volatile u32 *)(ls_oth_base + 0x12*4))
#define PAP_BUF                 (*(volatile u16 *)(ls_oth_base + 0x13*4))
#define PAP_ADR                 (*(volatile u32 *)(ls_oth_base + 0x14*4))       //26bit write only;
#define PAP_CNT                 (*(volatile u16 *)(ls_oth_base + 0x15*4))       //write only;
#define PAP_DAT0                (*(volatile u16 *)(ls_oth_base + 0x16*4))       //write only;
#define PAP_DAT1                (*(volatile u16 *)(ls_oth_base + 0x17*4))       //write only;
#define CRC1_FIFO               (*(volatile u8  *)(ls_oth_base + 0x18*4))       //write only;
#define CRC1_REG                (*(volatile u16 *)(ls_oth_base + 0x19*4))

#define EFUSE_CON               (*(volatile u16 *)(ls_oth_base + 0x1b*4))
#define MPUCON                  (*(volatile u32 *)(ls_oth_base + 0x1c*4))
#define MPUSTART                (*(volatile u32 *)(ls_oth_base + 0x1d*4))
#define MPUEND                  (*(volatile u32 *)(ls_oth_base + 0x1e*4))
#define MPUCATCH0               (*(volatile u32 *)(ls_oth_base + 0x1f*4))
#define MPUCATCH1               (*(volatile u32 *)(ls_oth_base + 0x20*4))
#define PLCNTCON                (*(volatile u8  *)(ls_oth_base + 0x21*4))
#define PLCNTVL                 (*(volatile u16 *)(ls_oth_base + 0x22*4))       //read only;
#define CS_CON                  (*(volatile u16 *)(ls_oth_base + 0x23*4))
#define CS_REG                  (*(volatile u32 *)(ls_oth_base + 0x24*4))
#define CS_FIFO                 (*(volatile u32 *)(ls_oth_base + 0x25*4))       //write only
#define CS_RADR                 (*(volatile u32 *)(ls_oth_base + 0x26*4))       //write only
#define CS_RCNT                 (*(volatile u32 *)(ls_oth_base + 0x27*4))       //write only
#define RAND64L                 (*(volatile u32 *)(ls_oth_base + 0x28*4))       //read only
#define RAND64H                 (*(volatile u32 *)(ls_oth_base + 0x29*4))       //read only
#define GPADC_CON               (*(volatile u16 *)(ls_oth_base + 0x2a*4))
#define GPADC_RES               (*(volatile u16 *)(ls_oth_base + 0x2b*4))      //10bit read only;

#define ALNK_CON0               (*(volatile u32 *)(ls_alnk_base + 0x00*4))
#define ALNK_CON1               (*(volatile u32 *)(ls_alnk_base + 0x01*4))
#define ALNK_CON2               (*(volatile u32 *)(ls_alnk_base + 0x02*4))
#define ALNK_CON3               (*(volatile u32 *)(ls_alnk_base + 0x03*4))
#define ALNK_ADR0               (*(volatile u32 *)(ls_alnk_base + 0x04*4))
#define ALNK_ADR1               (*(volatile u32 *)(ls_alnk_base + 0x05*4))
#define ALNK_ADR2               (*(volatile u32 *)(ls_alnk_base + 0x06*4))
#define ALNK_ADR3               (*(volatile u32 *)(ls_alnk_base + 0x07*4))
#define ALNK_LEN                (*(volatile u32 *)(ls_alnk_base + 0x08*4))

//===============================================================================//
//
//      high speed sfr address define
//
//===============================================================================//

//DSP
#define DREG00                  (*(volatile u32 *)(hs_cpu_base + 0x00*4))       //Read Only
#define DREG01                  (*(volatile u32 *)(hs_cpu_base + 0x01*4))       //Read Only
#define DREG02                  (*(volatile u32 *)(hs_cpu_base + 0x02*4))       //Read Only
#define DREG03                  (*(volatile u32 *)(hs_cpu_base + 0x03*4))       //Read Only
#define DREG04                  (*(volatile u32 *)(hs_cpu_base + 0x04*4))       //Read Only
#define DREG05                  (*(volatile u32 *)(hs_cpu_base + 0x05*4))       //Read Only
#define DREG06                  (*(volatile u32 *)(hs_cpu_base + 0x06*4))       //Read Only
#define DREG07                  (*(volatile u32 *)(hs_cpu_base + 0x07*4))       //Read Only
#define DREG08                  (*(volatile u32 *)(hs_cpu_base + 0x08*4))       //Read Only
#define DREG09                  (*(volatile u32 *)(hs_cpu_base + 0x09*4))       //Read Only
#define DREG10                  (*(volatile u32 *)(hs_cpu_base + 0x0a*4))       //Read Only
#define DREG11                  (*(volatile u32 *)(hs_cpu_base + 0x0b*4))       //Read Only
#define DREG12                  (*(volatile u32 *)(hs_cpu_base + 0x0c*4))       //Read Only
#define DREG13                  (*(volatile u32 *)(hs_cpu_base + 0x0d*4))       //Read Only
#define DREG14                  (*(volatile u32 *)(hs_cpu_base + 0x0e*4))       //Read Only
#define DREG15                  (*(volatile u32 *)(hs_cpu_base + 0x0f*4))       //Read Only

#define SREG00                  (*(volatile u32 *)(hs_cpu_base + 0x10*4))       //Read Only
#define SREG01                  (*(volatile u32 *)(hs_cpu_base + 0x11*4))       //Read Only
#define SREG02                  (*(volatile u32 *)(hs_cpu_base + 0x12*4))       //Read Only
#define SREG03                  (*(volatile u32 *)(hs_cpu_base + 0x13*4))       //Read Only
#define SREG04                  (*(volatile u32 *)(hs_cpu_base + 0x14*4))       //Read Only
#define SREG05                  (*(volatile u32 *)(hs_cpu_base + 0x15*4))       //Read Only
#define SREG06                  (*(volatile u32 *)(hs_cpu_base + 0x16*4))       //Read Only
#define SREG07                  (*(volatile u32 *)(hs_cpu_base + 0x17*4))       //Read Only
#define SREG08                  (*(volatile u32 *)(hs_cpu_base + 0x18*4))       //Read Only
#define SREG09                  (*(volatile u32 *)(hs_cpu_base + 0x19*4))       //Read Only
#define SREG10                  (*(volatile u32 *)(hs_cpu_base + 0x1a*4))       //Read Only
#define SREG11                  (*(volatile u32 *)(hs_cpu_base + 0x1b*4))       //Read Only
#define SREG12                  (*(volatile u32 *)(hs_cpu_base + 0x1c*4))       //Read Only
#define SREG13                  (*(volatile u32 *)(hs_cpu_base + 0x1d*4))       //Read Only
#define SREG14                  (*(volatile u32 *)(hs_cpu_base + 0x1e*4))       //Read Only
#define SREG15                  (*(volatile u32 *)(hs_cpu_base + 0x1f*4))       //Read Only

#define ICFG0                   (*(volatile u32 *)(hs_cpu_base + 0x20*4))
#define ICFG1                   (*(volatile u32 *)(hs_cpu_base + 0x21*4))
#define ICFG2                   (*(volatile u32 *)(hs_cpu_base + 0x22*4))
#define ICFG3                   (*(volatile u32 *)(hs_cpu_base + 0x23*4))
#define ICFG4                   (*(volatile u32 *)(hs_cpu_base + 0x24*4))
#define ICFG5                   (*(volatile u32 *)(hs_cpu_base + 0x25*4))
#define ICFG6                   (*(volatile u32 *)(hs_cpu_base + 0x26*4))
#define ICFG7                   (*(volatile u32 *)(hs_cpu_base + 0x27*4))
#define ICFG8                   (*(volatile u32 *)(hs_cpu_base + 0x28*4))
#define ICFG9                   (*(volatile u32 *)(hs_cpu_base + 0x29*4))
#define ICFG10                  (*(volatile u32 *)(hs_cpu_base + 0x2a*4))
#define ICFG11                  (*(volatile u32 *)(hs_cpu_base + 0x2b*4))
#define ICFG12                  (*(volatile u32 *)(hs_cpu_base + 0x2c*4))
#define ICFG13                  (*(volatile u32 *)(hs_cpu_base + 0x2d*4))
#define ICFG14                  (*(volatile u32 *)(hs_cpu_base + 0x2e*4))
#define ICFG15                  (*(volatile u32 *)(hs_cpu_base + 0x2f*4))
#define ICFG16                  (*(volatile u32 *)(hs_cpu_base + 0x30*4))
#define ICFG17                  (*(volatile u32 *)(hs_cpu_base + 0x31*4))
#define ICFG18                  (*(volatile u32 *)(hs_cpu_base + 0x32*4))
#define ICFG19                  (*(volatile u32 *)(hs_cpu_base + 0x33*4))
#define ICFG20                  (*(volatile u32 *)(hs_cpu_base + 0x34*4))
#define ICFG21                  (*(volatile u32 *)(hs_cpu_base + 0x35*4))
#define ICFG22                  (*(volatile u32 *)(hs_cpu_base + 0x36*4))
#define ICFG23                  (*(volatile u32 *)(hs_cpu_base + 0x37*4))
#define ICFG24                  (*(volatile u32 *)(hs_cpu_base + 0x38*4))
#define ICFG25                  (*(volatile u32 *)(hs_cpu_base + 0x39*4))
#define ICFG26                  (*(volatile u32 *)(hs_cpu_base + 0x3a*4))
#define ICFG27                  (*(volatile u32 *)(hs_cpu_base + 0x3b*4))
#define ICFG28                  (*(volatile u32 *)(hs_cpu_base + 0x3c*4))
#define ICFG29                  (*(volatile u32 *)(hs_cpu_base + 0x3d*4))
#define ICFG30                  (*(volatile u32 *)(hs_cpu_base + 0x3e*4))
#define ICFG31                  (*(volatile u32 *)(hs_cpu_base + 0x3f*4))

#define IPND0                   (*(volatile u32 *)(hs_cpu_base + 0x40*4))       //Read Only
#define IPND1                   (*(volatile u32 *)(hs_cpu_base + 0x41*4))       //Read Only
#define IPND2                   (*(volatile u32 *)(hs_cpu_base + 0x42*4))       //Read Only
#define IPND3                   (*(volatile u32 *)(hs_cpu_base + 0x43*4))       //Read Only
#define IPND4                   (*(volatile u32 *)(hs_cpu_base + 0x44*4))       //Read Only
#define IPND5                   (*(volatile u32 *)(hs_cpu_base + 0x45*4))       //Read Only
#define IPND6                   (*(volatile u32 *)(hs_cpu_base + 0x46*4))       //Read Only
#define IPND7                   (*(volatile u32 *)(hs_cpu_base + 0x47*4))       //Read Only
#define ILAT_SET                (*(volatile u32 *)(hs_cpu_base + 0x48*4))       //Write Only
#define ILAT_CLR                (*(volatile u32 *)(hs_cpu_base + 0x49*4))       //Write Only
#define TTMR_CON                (*(volatile u32 *)(hs_cpu_base + 0x4a*4))
#define TTMR_CNT                (*(volatile u32 *)(hs_cpu_base + 0x4b*4))
#define TTMR_PRD                (*(volatile u32 *)(hs_cpu_base + 0x4c*4))

#define BPCON                   (*(volatile u32 *)(hs_cpu_base + 0x50*4))
#define BSP                     (*(volatile u32 *)(hs_cpu_base + 0x51*4))
#define BP0                     (*(volatile u32 *)(hs_cpu_base + 0x52*4))
#define BP1                     (*(volatile u32 *)(hs_cpu_base + 0x53*4))
#define BP2                     (*(volatile u32 *)(hs_cpu_base + 0x54*4))
#define BP3                     (*(volatile u32 *)(hs_cpu_base + 0x55*4))

//FPGA DEBUG
#define DSPCON                  (*(volatile u32 *)(hs_dbg_base + 0x00*4))

#define CPASS_CON               (*(volatile u32 *)(hs_dbg_base + 0x10*4))
#define CPASS_ADRH              (*(volatile u32 *)(hs_dbg_base + 0x11*4))
#define CPASS_ADRL              (*(volatile u32 *)(hs_dbg_base + 0x12*4))
#define CPASS_BUF_LAST          (*(volatile u32 *)(hs_dbg_base + 0x13*4))
#define CPASS_CPF_ADRH          (*(volatile u32 *)(hs_dbg_base + 0x14*4))
#define CPASS_CPF_ADRL          (*(volatile u32 *)(hs_dbg_base + 0x15*4))

#define DSP_BF_CON              (*(volatile u32 *)(hs_dbg_base + 0x20*4))       // for debug only
#define DEBUG_WR_EN             (*(volatile u32 *)(hs_dbg_base + 0x21*4))       // for debug only
#define DEBUG_MSG               (*(volatile u32 *)(hs_dbg_base + 0x22*4))       // for debug only
#define DEBUG_MSG_CLR           (*(volatile u32 *)(hs_dbg_base + 0x23*4))       // for debug only
#define DEBUG_WRNUM             (*(volatile u32 *)(hs_dbg_base + 0x24*4))       // for debug only
#define PRP_ALLOW_NUM0          (*(volatile u32 *)(hs_dbg_base + 0x28*4))       // for debug only
#define PRP_ALLOW_NUM1          (*(volatile u32 *)(hs_dbg_base + 0x29*4))       // for debug only
#define DSP_PC_LIMH             (*(volatile u32 *)(hs_dbg_base + 0x2e*4))       // for debug only
#define DSP_PC_LIML             (*(volatile u32 *)(hs_dbg_base + 0x2f*4))       // for debug only

#define WR_LIM0H                (*(volatile u32 *)(hs_dbg_base + 0x30*4))       // for debug only
#define WR_LIM0L                (*(volatile u32 *)(hs_dbg_base + 0x31*4))       // for debug only
#define WR_LIM1H                (*(volatile u32 *)(hs_dbg_base + 0x32*4))       // for debug only
#define WR_LIM1L                (*(volatile u32 *)(hs_dbg_base + 0x33*4))       // for debug only
#define WR_LIM2H                (*(volatile u32 *)(hs_dbg_base + 0x34*4))       // for debug only
#define WR_LIM2L                (*(volatile u32 *)(hs_dbg_base + 0x35*4))       // for debug only
#define WR_LIM3H                (*(volatile u32 *)(hs_dbg_base + 0x36*4))       // for debug only
#define WR_LIM3L                (*(volatile u32 *)(hs_dbg_base + 0x37*4))       // for debug only
#define WR_LIM4H                (*(volatile u32 *)(hs_dbg_base + 0x38*4))       // for debug only
#define WR_LIM4L                (*(volatile u32 *)(hs_dbg_base + 0x39*4))       // for debug only
#define WR_LIM5H                (*(volatile u32 *)(hs_dbg_base + 0x3a*4))       // for debug only
#define WR_LIM5L                (*(volatile u32 *)(hs_dbg_base + 0x3b*4))       // for debug only
#define WR_LIM6H                (*(volatile u32 *)(hs_dbg_base + 0x3c*4))       // for debug only
#define WR_LIM6L                (*(volatile u32 *)(hs_dbg_base + 0x3d*4))       // for debug only
#define WR_LIM7H                (*(volatile u32 *)(hs_dbg_base + 0x3e*4))       // for debug only
#define WR_LIM7L                (*(volatile u32 *)(hs_dbg_base + 0x3f*4))       // for debug only

#define SDTAP_CON               (*(volatile u16 *)(hs_dbg_base + 0x80*4))
#define FPGA_TRI                (*(volatile u16 *)(hs_dbg_base + 0xff*4))


//SDRAM
#define SDRCON0                 (*(volatile u32 *)(hs_sdr_base + 0x00*4))
//#define SDRSPTR                 (*(volatile u32 *)(hs_sdr_base + 0x01*4))
//#define SDRQPTR                 (*(volatile u32 *)(hs_sdr_base + 0x02*4))
#define SDRREFREG               (*(volatile u32 *)(hs_sdr_base + 0x03*4))       //write only
//#define SDRDMACNT               (*(volatile u32 *)(hs_sdr_base + 0x04*4))
#define SDRCON1                 (*(volatile u32 *)(hs_sdr_base + 0x05*4))       //write only
#define SDRREFSUM               (*(volatile u32 *)(hs_sdr_base + 0x06*4))       //13bit
#define SDRDBG                  (*(volatile u32 *)(hs_sdr_base + 0x07*4))       //32bit
#define SDRCON2                 (*(volatile u32 *)(hs_sdr_base + 0x08*4))       //write only 32bit
#define SDRCON3                 (*(volatile u32 *)(hs_sdr_base + 0x09*4))       //32bit
#define SDRCON4                 (*(volatile u32 *)(hs_sdr_base + 0x0a*4))       //32bit
#define SDRCON5                 (*(volatile u32 *)(hs_sdr_base + 0x0b*4))       //32bit
#define SDRCON6                 (*(volatile u32 *)(hs_sdr_base + 0x0c*4))       //32bit
#define SDRCON7                 (*(volatile u32 *)(hs_sdr_base + 0x0d*4))       //32bit
#define SDRCON8                 (*(volatile u32 *)(hs_sdr_base + 0x0e*4))       //32bit
#define SDRCON9                 (*(volatile u32 *)(hs_sdr_base + 0x0f*4))       //32bit

//graph
#define EVA_CON                 (*(volatile u32 *)(hs_eva_base + 0x00*4))

//SFC
#define SFC_CON                 (*(volatile u32 *)(hs_sfc_base + 0x00*4))
#define SFC_BAUD                (*(volatile u16 *)(hs_sfc_base + 0x01*4))

#define SFC_BASE_ADR            (*(volatile u32 *)(hs_sfc_base + 0x03*4))

#define SFC_ECON                (*(volatile u32 *)(hs_sfc_base + 0x05*4))


#define   jpg0_base 0x0f74000
#define   jpg1_base 0x0f75000

#define JPG0_CON0                (*(volatile u32 *)(jpg0_base + 0x00*4))
#define JPG0_CON1                (*(volatile u32 *)(jpg0_base + 0x01*4))
#define JPG0_CON2                (*(volatile u32 *)(jpg0_base + 0x02*4))
#define JPG0_YDCVAL              (*(volatile u32 *)(jpg0_base + 0x03*4))
#define JPG0_UDCVAL              (*(volatile u32 *)(jpg0_base + 0x04*4))
#define JPG0_VDCVAL              (*(volatile u32 *)(jpg0_base + 0x05*4))
#define JPG0_YPTR0               (*(volatile u32 *)(jpg0_base + 0x06*4))
#define JPG0_UPTR0               (*(volatile u32 *)(jpg0_base + 0x07*4))
#define JPG0_VPTR0               (*(volatile u32 *)(jpg0_base + 0x08*4))
#define JPG0_YPTR1               (*(volatile u32 *)(jpg0_base + 0x09*4))
#define JPG0_UPTR1               (*(volatile u32 *)(jpg0_base + 0x0a*4))
#define JPG0_VPTR1               (*(volatile u32 *)(jpg0_base + 0x0b*4))
#define JPG0_BADDR               (*(volatile u32 *)(jpg0_base + 0x0c*4))
#define JPG0_BCNT                (*(volatile u32 *)(jpg0_base + 0x0d*4))
#define JPG0_MCUCNT              (*(volatile u32 *)(jpg0_base + 0x0e*4))
#define JPG0_PRECNT              (*(volatile u32 *)(jpg0_base + 0x0f*4))
#define JPG0_YUVLINE             (*(volatile u32 *)(jpg0_base + 0x10*4))
#define JPG0_CFGRAMADDR          (*(volatile u32 *)(jpg0_base + 0x11*4))
#define JPG0_CFGRAMVAL           (*(volatile u32 *)(jpg0_base + 0x12*4))
//#define JPG0_CFGRAMVAL         (*(volatile u32 *)(jpg0_base + 0x1c00*4))
#define JPG0_PTR_NUM             (*(volatile u32 *)(jpg0_base + 0x13*4))

#define JPG1_CON0                (*(volatile u32 *)(jpg1_base + 0x00*4))
#define JPG1_CON1                (*(volatile u32 *)(jpg1_base + 0x01*4))
#define JPG1_CON2                (*(volatile u32 *)(jpg1_base + 0x02*4))
#define JPG1_YDCVAL              (*(volatile u32 *)(jpg1_base + 0x03*4))
#define JPG1_UDCVAL              (*(volatile u32 *)(jpg1_base + 0x04*4))
#define JPG1_VDCVAL              (*(volatile u32 *)(jpg1_base + 0x05*4))
#define JPG1_YPTR0               (*(volatile u32 *)(jpg1_base + 0x06*4))
#define JPG1_UPTR0               (*(volatile u32 *)(jpg1_base + 0x07*4))
#define JPG1_VPTR0               (*(volatile u32 *)(jpg1_base + 0x08*4))
#define JPG1_YPTR1               (*(volatile u32 *)(jpg1_base + 0x09*4))
#define JPG1_UPTR1               (*(volatile u32 *)(jpg1_base + 0x0a*4))
#define JPG1_VPTR1               (*(volatile u32 *)(jpg1_base + 0x0b*4))
#define JPG1_BADDR               (*(volatile u32 *)(jpg1_base + 0x0c*4))
#define JPG1_BCNT                (*(volatile u32 *)(jpg1_base + 0x0d*4))
#define JPG1_MCUCNT              (*(volatile u32 *)(jpg1_base + 0x0e*4))
#define JPG1_PRECNT              (*(volatile u32 *)(jpg1_base + 0x0f*4))
#define JPG1_YUVLINE             (*(volatile u32 *)(jpg1_base + 0x10*4))
#define JPG1_CFGRAMADDR          (*(volatile u32 *)(jpg1_base + 0x11*4))
#define JPG1_CFGRAMVAL           (*(volatile u32 *)(jpg1_base + 0x12*4))
//#define JPG1_CFGRAMVAL         (*(volatile u32 *)(jpg1_base + 0x1c00*4))
#define JPG1_PTR_NUM             (*(volatile u32 *)(jpg1_base + 0x13*4))


#define ENC_CON                 (*(volatile u32 *)(hs_oth_base + 0x00*4))
#define ENC_KEY                 (*(volatile u32 *)(hs_oth_base + 0x01*4))
#define ENC_ADR                 (*(volatile u32 *)(hs_oth_base + 0x02*4))
#define SFC_UNENC_ADRH          (*(volatile u32 *)(hs_oth_base + 0x03*4))
#define SFC_UNENC_ADRL          (*(volatile u32 *)(hs_oth_base + 0x04*4))
#define DMA_CON                 (*(volatile u32 *)(hs_oth_base + 0x05*4))
#define DMA_CNT                 (*(volatile u32 *)(hs_oth_base + 0x06*4))   //write only
#define DMA_RADR                (*(volatile u32 *)(hs_oth_base + 0x07*4))   //write only
#define DMA_WADR                (*(volatile u32 *)(hs_oth_base + 0x08*4))   //write only

#define AES_CON                 (*(volatile u32 *)(hs_oth_base + 0x10*4))
#define AES_DAT0                (*(volatile u32 *)(hs_oth_base + 0x11*4))
#define AES_DAT1                (*(volatile u32 *)(hs_oth_base + 0x12*4))
#define AES_DAT2                (*(volatile u32 *)(hs_oth_base + 0x13*4))
#define AES_DAT3                (*(volatile u32 *)(hs_oth_base + 0x14*4))
#define AES_KEY                 (*(volatile u32 *)(hs_oth_base + 0x15*4))

#define GPDMA_RD_CON            (*(volatile u32 *)(hs_oth_base + 0x20*4))
#define GPDMA_RD_SPTR           (*(volatile u32 *)(hs_oth_base + 0x21*4))
#define GPDMA_RD_CNT            (*(volatile u32 *)(hs_oth_base + 0x22*4))

#define GPDMA_WR_CON            (*(volatile u32 *)(hs_oth_base + 0x30*4))
#define GPDMA_WR_SPTR           (*(volatile u32 *)(hs_oth_base + 0x31*4))
#define GPDMA_WR_CNT            (*(volatile u32 *)(hs_oth_base + 0x32*4))

//DMA BYTE COPY
#define   hs_dmacopy_base  0x0f77000
#define DMA_COPY_TASK_ADR       (*(volatile u32 *)(hs_dmacopy_base + 0x00*4))   //write only
#define DMA_COPY_CON            (*(volatile u32 *)(hs_dmacopy_base + 0x01*4))


//...........  Full Speed USB .....................
#define FADDR       0x00
#define POWER       0x01
#define INTRTX1     0x02
#define INTRTX2     0x03
#define INTRRX1     0x04
#define INTRRX2     0x05
#define INTRUSB     0x06
#define INTRTX1E    0x07
#define INTRTX2E    0x08
#define INTRRX1E    0x09
#define INTRRX2E    0x0a
#define INTRUSBE    0x0b
#define FRAME1      0x0c
#define FRAME2      0x0d
#define INDEX       0x0e
#define DEVCTL      0x0f
#define TXMAXP      0x10
#define CSR0        0x11
#define TXCSR1      0x11
#define TXCSR2      0x12
#define RXMAXP      0x13
#define RXCSR1      0x14
#define RXCSR2      0x15
#define COUNT0      0x16
#define RXCOUNT1    0x16
#define RXCOUNT2    0x17
#define TXTYPE      0x18
#define TXINTERVAL  0x19
#define RXTYPE      0x1a
#define RXINTERVAL  0x1b

//...........  High Speed USB .....................
#define husb_base /*0x7e000*/ ls_base + 0x8000
#define H_FADDR         (*(volatile u8  *)(husb_base + 0x000))
#define H_POWER         (*(volatile u8  *)(husb_base + 0x001))
#define H_INTRTX        (*(volatile u16 *)(husb_base + 0x002))
#define H_INTRRX        (*(volatile u16 *)(husb_base + 0x004))
#define H_INTRTXE       (*(volatile u16 *)(husb_base + 0x006))
#define H_INTRRXE       (*(volatile u16 *)(husb_base + 0x008))
#define H_INTRUSB       (*(volatile u8  *)(husb_base + 0x00a))
#define H_INTRUSBE      (*(volatile u8  *)(husb_base + 0x00b))
#define H_FRAME         (*(volatile u16 *)(husb_base + 0x00c))
#define H_INDEX         (*(volatile u8  *)(husb_base + 0x00e))
#define H_TESTMODE      (*(volatile u8  *)(husb_base + 0x00f))

#define H_FIFO0         (*(volatile u8  *)(husb_base + 0x020))
#define H_FIFO1         (*(volatile u8  *)(husb_base + 0x024))
#define H_FIFO2         (*(volatile u8  *)(husb_base + 0x028))
#define H_FIFO3         (*(volatile u8  *)(husb_base + 0x02c))
#define H_FIFO4         (*(volatile u8  *)(husb_base + 0x030))
#define H_DEVCTL        (*(volatile u8  *)(husb_base + 0x060))

#define H_CSR0          (*(volatile u16 *)(husb_base + 0x102))
#define H_COUNT0        (*(volatile u16 *)(husb_base + 0x108))
#define H_NAKLIMIT0     (*(volatile u8  *)(husb_base + 0x10b))
#define H_CFGDATA       (*(volatile u8  *)(husb_base + 0x10f))

#define H_EP1TXMAXP     (*(volatile u16 *)(husb_base + 0x110))
#define H_EP1TXCSR      (*(volatile u16 *)(husb_base + 0x112))
#define H_EP1RXMAXP     (*(volatile u16 *)(husb_base + 0x114))
#define H_EP1RXCSR      (*(volatile u16 *)(husb_base + 0x116))
#define H_EP1RXCOUNT    (*(volatile u16 *)(husb_base + 0x118))
#define H_EP1TXTYPE     (*(volatile u8  *)(husb_base + 0x11a))
#define H_EP1TXINTERVAL (*(volatile u8  *)(husb_base + 0x11b))
#define H_EP1RXTYPE     (*(volatile u8  *)(husb_base + 0x11c))
#define H_EP1RXINTERVAL (*(volatile u8  *)(husb_base + 0x11d))
#define H_EP1FIFOSIZE   (*(volatile u8  *)(husb_base + 0x11f))

#define H_EP2TXMAXP     (*(volatile u16 *)(husb_base + 0x120))
#define H_EP2TXCSR      (*(volatile u16 *)(husb_base + 0x122))
#define H_EP2RXMAXP     (*(volatile u16 *)(husb_base + 0x124))
#define H_EP2RXCSR      (*(volatile u16 *)(husb_base + 0x126))
#define H_EP2RXCOUNT    (*(volatile u16 *)(husb_base + 0x128))
#define H_EP2TXTYPE     (*(volatile u8  *)(husb_base + 0x12a))
#define H_EP2TXINTERVAL (*(volatile u8  *)(husb_base + 0x12b))
#define H_EP2RXTYPE     (*(volatile u8  *)(husb_base + 0x12c))
#define H_EP2RXINTERVAL (*(volatile u8  *)(husb_base + 0x12d))
#define H_EP2FIFOSIZE   (*(volatile u8  *)(husb_base + 0x12f))

#define H_EP3TXMAXP     (*(volatile u16 *)(husb_base + 0x130))
#define H_EP3TXCSR      (*(volatile u16 *)(husb_base + 0x132))
#define H_EP3RXMAXP     (*(volatile u16 *)(husb_base + 0x134))
#define H_EP3RXCSR      (*(volatile u16 *)(husb_base + 0x136))
#define H_EP3RXCOUNT    (*(volatile u16 *)(husb_base + 0x138))
#define H_EP3TXTYPE     (*(volatile u8  *)(husb_base + 0x13a))
#define H_EP3TXINTERVAL (*(volatile u8  *)(husb_base + 0x13b))
#define H_EP3RXTYPE     (*(volatile u8  *)(husb_base + 0x13c))
#define H_EP3RXINTERVAL (*(volatile u8  *)(husb_base + 0x13d))
#define H_EP3FIFOSIZE   (*(volatile u8  *)(husb_base + 0x13f))

#define H_EP4TXMAXP     (*(volatile u16 *)(husb_base + 0x140))
#define H_EP4TXCSR      (*(volatile u16 *)(husb_base + 0x142))
#define H_EP4RXMAXP     (*(volatile u16 *)(husb_base + 0x144))
#define H_EP4RXCSR      (*(volatile u16 *)(husb_base + 0x146))
#define H_EP4RXCOUNT    (*(volatile u16 *)(husb_base + 0x148))
#define H_EP4TXTYPE     (*(volatile u8  *)(husb_base + 0x14a))
#define H_EP4TXINTERVAL (*(volatile u8  *)(husb_base + 0x14b))
#define H_EP4RXTYPE     (*(volatile u8  *)(husb_base + 0x14c))
#define H_EP4RXINTERVAL (*(volatile u8  *)(husb_base + 0x14d))
#define H_EP4FIFOSIZE   (*(volatile u8  *)(husb_base + 0x14f))

#define H_TX_DPBUFDIS   (*(volatile u16 *)(husb_base + 0x342))

//==============================================================//
//  xbus
//==============================================================//
#define eva_base    0x0f78000
#define xbus_base   eva_base + 0x0000
#define xbus_ch00_lvl           (*(volatile u32 *)(xbus_base + 0x00*4))
#define xbus_ch01_lvl           (*(volatile u32 *)(xbus_base + 0x01*4))
#define xbus_ch02_lvl           (*(volatile u32 *)(xbus_base + 0x02*4))
#define xbus_ch03_lvl           (*(volatile u32 *)(xbus_base + 0x03*4))
#define xbus_ch04_lvl           (*(volatile u32 *)(xbus_base + 0x04*4))
#define xbus_ch05_lvl           (*(volatile u32 *)(xbus_base + 0x05*4))
#define xbus_ch06_lvl           (*(volatile u32 *)(xbus_base + 0x06*4))
#define xbus_ch07_lvl           (*(volatile u32 *)(xbus_base + 0x07*4))
#define xbus_ch08_lvl           (*(volatile u32 *)(xbus_base + 0x08*4))
#define xbus_ch09_lvl           (*(volatile u32 *)(xbus_base + 0x09*4))
#define xbus_ch10_lvl           (*(volatile u32 *)(xbus_base + 0x0a*4))
#define xbus_ch11_lvl           (*(volatile u32 *)(xbus_base + 0x0b*4))
#define xbus_ch12_lvl           (*(volatile u32 *)(xbus_base + 0x0c*4))
#define xbus_ch13_lvl           (*(volatile u32 *)(xbus_base + 0x0d*4))
#define xbus_ch14_lvl           (*(volatile u32 *)(xbus_base + 0x0e*4))
#define xbus_ch15_lvl           (*(volatile u32 *)(xbus_base + 0x0f*4))

#define xbus_ch16_lvl           (*(volatile u32 *)(xbus_base + 0x10*4))
#define xbus_ch17_lvl           (*(volatile u32 *)(xbus_base + 0x11*4))
#define xbus_ch18_lvl           (*(volatile u32 *)(xbus_base + 0x12*4))
#define xbus_ch19_lvl           (*(volatile u32 *)(xbus_base + 0x13*4))
#define xbus_ch20_lvl           (*(volatile u32 *)(xbus_base + 0x14*4))
#define xbus_ch21_lvl           (*(volatile u32 *)(xbus_base + 0x15*4))
#define xbus_ch22_lvl           (*(volatile u32 *)(xbus_base + 0x16*4))
#define xbus_ch23_lvl           (*(volatile u32 *)(xbus_base + 0x17*4))
#define xbus_ch24_lvl           (*(volatile u32 *)(xbus_base + 0x18*4))
#define xbus_ch25_lvl           (*(volatile u32 *)(xbus_base + 0x19*4))
#define xbus_ch26_lvl           (*(volatile u32 *)(xbus_base + 0x1a*4))
#define xbus_ch27_lvl           (*(volatile u32 *)(xbus_base + 0x1b*4))
#define xbus_ch28_lvl           (*(volatile u32 *)(xbus_base + 0x1c*4))
#define xbus_ch29_lvl           (*(volatile u32 *)(xbus_base + 0x1d*4))
#define xbus_ch30_lvl           (*(volatile u32 *)(xbus_base + 0x1e*4))
#define xbus_ch31_lvl           (*(volatile u32 *)(xbus_base + 0x1f*4))

#define xbus_lv1_prd            (*(volatile u32 *)(xbus_base + 0x20*4))
#define xbus_lv2_prd            (*(volatile u32 *)(xbus_base + 0x21*4))
#define xbus_dist0_con          (*(volatile u32 *)(xbus_base + 0x22*4))
#define xbus_dist1_con          (*(volatile u32 *)(xbus_base + 0x23*4))

//==============================================================//
//  isc
//==============================================================//
#define isc_base    eva_base + 0x0800
#define isc_pnd_con             (*(volatile u32 *)(isc_base + 0x00*4))
#define isc_dmx_con0            (*(volatile u32 *)(isc_base + 0x01*4))
#define isc_dmx_con1            (*(volatile u32 *)(isc_base + 0x02*4))

#define isc_sen0_con            (*(volatile u32 *)(isc_base + 0x08*4))
#define isc_sen0_vblk           (*(volatile u32 *)(isc_base + 0x09*4))
#define isc_sen0_vact           (*(volatile u32 *)(isc_base + 0x0a*4))
#define isc_sen0_hblk           (*(volatile u32 *)(isc_base + 0x0b*4))
#define isc_sen0_hact           (*(volatile u32 *)(isc_base + 0x0c*4))

#define isc_sen1_con            (*(volatile u32 *)(isc_base + 0x10*4))
#define isc_sen1_vblk           (*(volatile u32 *)(isc_base + 0x11*4))
#define isc_sen1_vact           (*(volatile u32 *)(isc_base + 0x12*4))
#define isc_sen1_hblk           (*(volatile u32 *)(isc_base + 0x13*4))
#define isc_sen1_hact           (*(volatile u32 *)(isc_base + 0x14*4))

#define isc_lcds_con            (*(volatile u32 *)(isc_base + 0x18*4))
#define isc_lcds_vblk           (*(volatile u32 *)(isc_base + 0x19*4))
#define isc_lcds_vact           (*(volatile u32 *)(isc_base + 0x1a*4))
#define isc_lcds_hblk           (*(volatile u32 *)(isc_base + 0x1b*4))
#define isc_lcds_hact           (*(volatile u32 *)(isc_base + 0x1c*4))

//==============================================================//
//  isp0
//==============================================================//
#define isp0_base   eva_base + 0x1000
#define isp0_pnd_con            (*(volatile u32 *)(isp0_base + 0x000*4))
#define isp0_scn_con            (*(volatile u32 *)(isp0_base + 0x001*4))

#define isp0_src_con            (*(volatile u32 *)(isp0_base + 0x004*4))
#define isp0_src_haw            (*(volatile u32 *)(isp0_base + 0x005*4))
#define isp0_src_vaw            (*(volatile u32 *)(isp0_base + 0x006*4))

#define isp0_blc_off_r          (*(volatile u32 *)(isp0_base + 0x008*4))
#define isp0_blc_off_gr         (*(volatile u32 *)(isp0_base + 0x009*4))
#define isp0_blc_off_gb         (*(volatile u32 *)(isp0_base + 0x00a*4))
#define isp0_blc_off_b          (*(volatile u32 *)(isp0_base + 0x00b*4))

#define isp0_dpc_th0            (*(volatile u32 *)(isp0_base + 0x00c*4))
#define isp0_dpc_th1            (*(volatile u32 *)(isp0_base + 0x00d*4))
#define isp0_dpc_th2            (*(volatile u32 *)(isp0_base + 0x00e*4))

#define isp0_lsc_cen_x          (*(volatile u32 *)(isp0_base + 0x010*4))
#define isp0_lsc_cen_y          (*(volatile u32 *)(isp0_base + 0x011*4))
#define isp0_lsc_dth_th         (*(volatile u32 *)(isp0_base + 0x012*4))
#define isp0_lsc_dth_prm0       (*(volatile u32 *)(isp0_base + 0x013*4))
#define isp0_lsc_dth_prm1       (*(volatile u32 *)(isp0_base + 0x014*4))
#define isp0_lsc_lut_r          (*(volatile u32 *)(isp0_base + 0x015*4))
#define isp0_lsc_lut_g          (*(volatile u32 *)(isp0_base + 0x016*4))
#define isp0_lsc_lut_b          (*(volatile u32 *)(isp0_base + 0x017*4))

#define isp0_awb_gain_r         (*(volatile u32 *)(isp0_base + 0x018*4))
#define isp0_awb_gain_g         (*(volatile u32 *)(isp0_base + 0x019*4))
#define isp0_awb_gain_b         (*(volatile u32 *)(isp0_base + 0x01a*4))

#define isp0_drc_lut            (*(volatile u32 *)(isp0_base + 0x01b*4))

#define isp0_tnr_con            (*(volatile u32 *)(isp0_base + 0x01c*4))
#define isp0_tnr_base           (*(volatile u32 *)(isp0_base + 0x01d*4))
#define isp0_tnr_size           (*(volatile u32 *)(isp0_base + 0x01e*4))
#define isp0_tnr_2d_str         (*(volatile u32 *)(isp0_base + 0x01f*4))
#define isp0_tnr_3d_th0         (*(volatile u32 *)(isp0_base + 0x020*4))
#define isp0_tnr_3d_th1         (*(volatile u32 *)(isp0_base + 0x021*4))
#define isp0_tnr_mt_th          (*(volatile u32 *)(isp0_base + 0x022*4))
#define isp0_tnr_wmax           (*(volatile u32 *)(isp0_base + 0x023*4))
#define isp0_tnr_wmin           (*(volatile u32 *)(isp0_base + 0x024*4))
#define isp0_tnr_wslope         (*(volatile u32 *)(isp0_base + 0x025*4))
#define isp0_tnr_break          (*(volatile u32 *)(isp0_base + 0x026*4))
#define isp0_tnr_scale0         (*(volatile u32 *)(isp0_base + 0x027*4))
#define isp0_tnr_scale1         (*(volatile u32 *)(isp0_base + 0x028*4))
#define isp0_tnr_scale2         (*(volatile u32 *)(isp0_base + 0x029*4))
#define isp0_tnr_scale3         (*(volatile u32 *)(isp0_base + 0x02a*4))
#define isp0_tnr_scale4         (*(volatile u32 *)(isp0_base + 0x02b*4))
#define isp0_tnr_scale5         (*(volatile u32 *)(isp0_base + 0x02c*4))
#define isp0_tnr_scale6         (*(volatile u32 *)(isp0_base + 0x02d*4))
#define isp0_tnr_scale7         (*(volatile u32 *)(isp0_base + 0x02e*4))

#define isp0_ccm_r_coe0         (*(volatile u32 *)(isp0_base + 0x030*4))
#define isp0_ccm_r_coe1         (*(volatile u32 *)(isp0_base + 0x031*4))
#define isp0_ccm_r_coe2         (*(volatile u32 *)(isp0_base + 0x032*4))
#define isp0_ccm_r_off          (*(volatile u32 *)(isp0_base + 0x033*4))
#define isp0_ccm_g_coe0         (*(volatile u32 *)(isp0_base + 0x034*4))
#define isp0_ccm_g_coe1         (*(volatile u32 *)(isp0_base + 0x035*4))
#define isp0_ccm_g_coe2         (*(volatile u32 *)(isp0_base + 0x036*4))
#define isp0_ccm_g_off          (*(volatile u32 *)(isp0_base + 0x037*4))
#define isp0_ccm_b_coe0         (*(volatile u32 *)(isp0_base + 0x038*4))
#define isp0_ccm_b_coe1         (*(volatile u32 *)(isp0_base + 0x039*4))
#define isp0_ccm_b_coe2         (*(volatile u32 *)(isp0_base + 0x03a*4))
#define isp0_ccm_b_off          (*(volatile u32 *)(isp0_base + 0x03b*4))

#define isp0_gma_r_lut          (*(volatile u32 *)(isp0_base + 0x03c*4))
#define isp0_gma_g_lut          (*(volatile u32 *)(isp0_base + 0x03d*4))
#define isp0_gma_b_lut          (*(volatile u32 *)(isp0_base + 0x03e*4))
#define isp0_csc_y_lut          (*(volatile u32 *)(isp0_base + 0x03f*4))

#define isp0_dnr_sim_th         (*(volatile u32 *)(isp0_base + 0x040*4))
#define isp0_dnr_rng_sgm        (*(volatile u32 *)(isp0_base + 0x041*4))
#define isp0_dnr_gaus_c00       (*(volatile u32 *)(isp0_base + 0x042*4))
#define isp0_dnr_gaus_c01       (*(volatile u32 *)(isp0_base + 0x043*4))
#define isp0_dnr_gaus_c02       (*(volatile u32 *)(isp0_base + 0x044*4))
#define isp0_dnr_gaus_c03       (*(volatile u32 *)(isp0_base + 0x045*4))
#define isp0_dnr_gaus_c11       (*(volatile u32 *)(isp0_base + 0x046*4))
#define isp0_dnr_gaus_c12       (*(volatile u32 *)(isp0_base + 0x047*4))
#define isp0_dnr_gaus_c13       (*(volatile u32 *)(isp0_base + 0x048*4))
#define isp0_dnr_gaus_c22       (*(volatile u32 *)(isp0_base + 0x049*4))
#define isp0_dnr_gaus_c23       (*(volatile u32 *)(isp0_base + 0x04a*4))
#define isp0_dnr_gaus_c33       (*(volatile u32 *)(isp0_base + 0x04b*4))
#define isp0_dnr_cmid_en        (*(volatile u32 *)(isp0_base + 0x04c*4))

#define isp0_shp_lone_th        (*(volatile u32 *)(isp0_base + 0x04d*4))
#define isp0_shp_ech_min        (*(volatile u32 *)(isp0_base + 0x04e*4))
#define isp0_shp_ech_max        (*(volatile u32 *)(isp0_base + 0x04f*4))

#define isp0_shp_hf_th0         (*(volatile u32 *)(isp0_base + 0x050*4))
#define isp0_shp_hf_th1         (*(volatile u32 *)(isp0_base + 0x051*4))
#define isp0_shp_hf_th2         (*(volatile u32 *)(isp0_base + 0x052*4))
#define isp0_shp_hf_amt         (*(volatile u32 *)(isp0_base + 0x053*4))
#define isp0_shp_hf_gain        (*(volatile u32 *)(isp0_base + 0x054*4))
#define isp0_shp_hf_c00         (*(volatile u32 *)(isp0_base + 0x055*4))
#define isp0_shp_hf_c01         (*(volatile u32 *)(isp0_base + 0x056*4))
#define isp0_shp_hf_c02         (*(volatile u32 *)(isp0_base + 0x057*4))
#define isp0_shp_hf_c10         (*(volatile u32 *)(isp0_base + 0x058*4))
#define isp0_shp_hf_c11         (*(volatile u32 *)(isp0_base + 0x059*4))
#define isp0_shp_hf_c12         (*(volatile u32 *)(isp0_base + 0x05a*4))
#define isp0_shp_hf_c20         (*(volatile u32 *)(isp0_base + 0x05b*4))
#define isp0_shp_hf_c21         (*(volatile u32 *)(isp0_base + 0x05c*4))
#define isp0_shp_hf_c22         (*(volatile u32 *)(isp0_base + 0x05d*4))
#define isp0_shp_hf_sft         (*(volatile u32 *)(isp0_base + 0x05e*4))

#define isp0_shp_mf_th0         (*(volatile u32 *)(isp0_base + 0x060*4))
#define isp0_shp_mf_th1         (*(volatile u32 *)(isp0_base + 0x061*4))
#define isp0_shp_mf_amt         (*(volatile u32 *)(isp0_base + 0x062*4))
#define isp0_shp_mf_gain        (*(volatile u32 *)(isp0_base + 0x063*4))
#define isp0_shp_mf_c00         (*(volatile u32 *)(isp0_base + 0x064*4))
#define isp0_shp_mf_c01         (*(volatile u32 *)(isp0_base + 0x065*4))
#define isp0_shp_mf_c02         (*(volatile u32 *)(isp0_base + 0x066*4))
#define isp0_shp_mf_c10         (*(volatile u32 *)(isp0_base + 0x067*4))
#define isp0_shp_mf_c11         (*(volatile u32 *)(isp0_base + 0x068*4))
#define isp0_shp_mf_c12         (*(volatile u32 *)(isp0_base + 0x069*4))
#define isp0_shp_mf_c20         (*(volatile u32 *)(isp0_base + 0x06a*4))
#define isp0_shp_mf_c21         (*(volatile u32 *)(isp0_base + 0x06b*4))
#define isp0_shp_mf_c22         (*(volatile u32 *)(isp0_base + 0x06c*4))
#define isp0_shp_mf_sft         (*(volatile u32 *)(isp0_base + 0x06d*4))

#define isp0_shp_cr_smt_th      (*(volatile u32 *)(isp0_base + 0x070*4))
#define isp0_shp_cr_c00         (*(volatile u32 *)(isp0_base + 0x071*4))
#define isp0_shp_cr_c01         (*(volatile u32 *)(isp0_base + 0x072*4))
#define isp0_shp_cr_c02         (*(volatile u32 *)(isp0_base + 0x073*4))
#define isp0_shp_cr_c10         (*(volatile u32 *)(isp0_base + 0x074*4))
#define isp0_shp_cr_c11         (*(volatile u32 *)(isp0_base + 0x075*4))
#define isp0_shp_cr_c12         (*(volatile u32 *)(isp0_base + 0x076*4))
#define isp0_shp_cr_c20         (*(volatile u32 *)(isp0_base + 0x077*4))
#define isp0_shp_cr_c21         (*(volatile u32 *)(isp0_base + 0x078*4))
#define isp0_shp_cr_c22         (*(volatile u32 *)(isp0_base + 0x079*4))
#define isp0_shp_cr_sft         (*(volatile u32 *)(isp0_base + 0x07a*4))

#define isp0_cbs_y_gain         (*(volatile u32 *)(isp0_base + 0x080*4))
#define isp0_cbs_u_gain         (*(volatile u32 *)(isp0_base + 0x081*4))
#define isp0_cbs_v_gain         (*(volatile u32 *)(isp0_base + 0x082*4))
#define isp0_cbs_y_offs         (*(volatile u32 *)(isp0_base + 0x083*4))
#define isp0_cbs_u_offs         (*(volatile u32 *)(isp0_base + 0x084*4))
#define isp0_cbs_v_offs         (*(volatile u32 *)(isp0_base + 0x085*4))

#define isp0_out_hst            (*(volatile u32 *)(isp0_base + 0x088*4))
#define isp0_out_hed            (*(volatile u32 *)(isp0_base + 0x089*4))
#define isp0_out_vst            (*(volatile u32 *)(isp0_base + 0x08a*4))
#define isp0_out_ved            (*(volatile u32 *)(isp0_base + 0x08b*4))

#define isp0_stc_ae_base0       (*(volatile u32 *)(isp0_base + 0x090*4))
#define isp0_stc_ae_base1       (*(volatile u32 *)(isp0_base + 0x091*4))
#define isp0_stc_ae_base2       (*(volatile u32 *)(isp0_base + 0x092*4))
#define isp0_stc_ae_base3       (*(volatile u32 *)(isp0_base + 0x093*4))
#define isp0_stc_ae_basex       (*(volatile u32 *)(isp0_base + 0x094*4))
#define isp0_stc_ae_en          (*(volatile u32 *)(isp0_base + 0x095*4))
#define isp0_stc_ae_lv1         (*(volatile u32 *)(isp0_base + 0x096*4))
#define isp0_stc_ae_lv2         (*(volatile u32 *)(isp0_base + 0x097*4))
#define isp0_stc_ae_lv3         (*(volatile u32 *)(isp0_base + 0x098*4))
#define isp0_stc_ae_lv4         (*(volatile u32 *)(isp0_base + 0x099*4))
#define isp0_stc_ae_lv5         (*(volatile u32 *)(isp0_base + 0x09a*4))
#define isp0_stc_ae_lv6         (*(volatile u32 *)(isp0_base + 0x09b*4))
#define isp0_stc_ae_lv7         (*(volatile u32 *)(isp0_base + 0x09c*4))

#define isp0_stc_wb_base0       (*(volatile u32 *)(isp0_base + 0x0a0*4))
#define isp0_stc_wb_base1       (*(volatile u32 *)(isp0_base + 0x0a1*4))
#define isp0_stc_wb_base2       (*(volatile u32 *)(isp0_base + 0x0a2*4))
#define isp0_stc_wb_base3       (*(volatile u32 *)(isp0_base + 0x0a3*4))
#define isp0_stc_wb_basex       (*(volatile u32 *)(isp0_base + 0x0a4*4))
#define isp0_stc_wb_en          (*(volatile u32 *)(isp0_base + 0x0a5*4))
#define isp0_stc_wb_r_th        (*(volatile u32 *)(isp0_base + 0x0a6*4))
#define isp0_stc_wb_g_th        (*(volatile u32 *)(isp0_base + 0x0a7*4))
#define isp0_stc_wb_b_th        (*(volatile u32 *)(isp0_base + 0x0a8*4))
#define isp0_stc_wb_w_th        (*(volatile u32 *)(isp0_base + 0x0a9*4))
#define isp0_stc_wb_y_min       (*(volatile u32 *)(isp0_base + 0x0aa*4))
#define isp0_stc_wb_y_max       (*(volatile u32 *)(isp0_base + 0x0ab*4))
#define isp0_stc_wb_rg_min      (*(volatile u32 *)(isp0_base + 0x0ac*4))
#define isp0_stc_wb_rg_max      (*(volatile u32 *)(isp0_base + 0x0ad*4))
#define isp0_stc_wb_bg_min      (*(volatile u32 *)(isp0_base + 0x0ae*4))
#define isp0_stc_wb_bg_max      (*(volatile u32 *)(isp0_base + 0x0af*4))

//==============================================================//
//  isp1
//==============================================================//
#define isp1_base   eva_base + 0x1800
#define isp1_pnd_con            (*(volatile u32 *)(isp1_base + 0x000*4))
#define isp1_src_con            (*(volatile u32 *)(isp1_base + 0x001*4))
#define isp1_src_haw            (*(volatile u32 *)(isp1_base + 0x002*4))
#define isp1_src_vaw            (*(volatile u32 *)(isp1_base + 0x003*4))

//==============================================================//
//  imc
//==============================================================//
#define imc_base    eva_base + 0x2000
#define imc_sfr_ptr(num)        (*(volatile u32 *)(imc_base + num*4))

#define imc_pnd_con             imc_sfr_ptr(0x000)
#define imc_rep_con             imc_sfr_ptr(0x001)
#define imc_rep_h_cfg           imc_sfr_ptr(0x002)
#define imc_rep_v_cfg           imc_sfr_ptr(0x003)
#define imc_rep_y_base          imc_sfr_ptr(0x004)
#define imc_rep_u_base          imc_sfr_ptr(0x005)
#define imc_rep_v_base          imc_sfr_ptr(0x006)

#define imc_tpz_con             imc_sfr_ptr(0x008)
#define imc_tpz_cfg0            imc_sfr_ptr(0x009)
#define imc_tpz_cfg1            imc_sfr_ptr(0x00a)
#define imc_tpz_cfg2            imc_sfr_ptr(0x00b)

#define imc_ch0_com_con         imc_sfr_ptr(0x020)
#define imc_ch0_src_con         imc_sfr_ptr(0x021)
#define imc_ch0_crop_h          imc_sfr_ptr(0x022)
#define imc_ch0_crop_v          imc_sfr_ptr(0x023)
#define imc_ch0_h_stp           imc_sfr_ptr(0x024)
#define imc_ch0_h_wth           imc_sfr_ptr(0x025)
#define imc_ch0_v_stp           imc_sfr_ptr(0x026)
#define imc_ch0_v_wth           imc_sfr_ptr(0x027)
#define imc_ch0_osd_con         imc_sfr_ptr(0x028)
#define imc_ch0_osd_color0      imc_sfr_ptr(0x029)
#define imc_ch0_osd_color1      imc_sfr_ptr(0x02a)
#define imc_ch0_osd_color2      imc_sfr_ptr(0x02b)
#define imc_ch0_osd_color3      imc_sfr_ptr(0x02c)
#define imc_ch0_osd0_h_cfg      imc_sfr_ptr(0x02d)
#define imc_ch0_osd0_v_cfg      imc_sfr_ptr(0x02e)
#define imc_ch0_osd0_base       imc_sfr_ptr(0x02f)
#define imc_ch0_osd1_h_cfg      imc_sfr_ptr(0x030)
#define imc_ch0_osd1_v_cfg      imc_sfr_ptr(0x031)
#define imc_ch0_osd1_base       imc_sfr_ptr(0x032)
#define imc_ch0_dma_con         imc_sfr_ptr(0x033)
#define imc_ch0_dma_cnt         imc_sfr_ptr(0x034)
#define imc_ch0_dma_y_bs0       imc_sfr_ptr(0x035)
#define imc_ch0_dma_u_bs0       imc_sfr_ptr(0x036)
#define imc_ch0_dma_v_bs0       imc_sfr_ptr(0x037)
#define imc_ch0_dma_y_bs1       imc_sfr_ptr(0x038)
#define imc_ch0_dma_u_bs1       imc_sfr_ptr(0x039)
#define imc_ch0_dma_v_bs1       imc_sfr_ptr(0x03a)

#define imc_ch1_com_con         imc_sfr_ptr(0x040)
#define imc_ch1_src_con         imc_sfr_ptr(0x041)
#define imc_ch1_crop_h          imc_sfr_ptr(0x042)
#define imc_ch1_crop_v          imc_sfr_ptr(0x043)
#define imc_ch1_h_stp           imc_sfr_ptr(0x044)
#define imc_ch1_h_wth           imc_sfr_ptr(0x045)
#define imc_ch1_v_stp           imc_sfr_ptr(0x046)
#define imc_ch1_v_wth           imc_sfr_ptr(0x047)
#define imc_ch1_osd_con         imc_sfr_ptr(0x048)
#define imc_ch1_osd_color0      imc_sfr_ptr(0x049)
#define imc_ch1_osd_color1      imc_sfr_ptr(0x04a)
#define imc_ch1_osd_color2      imc_sfr_ptr(0x04b)
#define imc_ch1_osd_color3      imc_sfr_ptr(0x04c)
#define imc_ch1_osd0_h_cfg      imc_sfr_ptr(0x04d)
#define imc_ch1_osd0_v_cfg      imc_sfr_ptr(0x04e)
#define imc_ch1_osd0_base       imc_sfr_ptr(0x04f)
#define imc_ch1_osd1_h_cfg      imc_sfr_ptr(0x050)
#define imc_ch1_osd1_v_cfg      imc_sfr_ptr(0x051)
#define imc_ch1_osd1_base       imc_sfr_ptr(0x052)
#define imc_ch1_dma_con         imc_sfr_ptr(0x053)
#define imc_ch1_dma_cnt         imc_sfr_ptr(0x054)
#define imc_ch1_dma_y_bs0       imc_sfr_ptr(0x055)
#define imc_ch1_dma_u_bs0       imc_sfr_ptr(0x056)
#define imc_ch1_dma_v_bs0       imc_sfr_ptr(0x057)
#define imc_ch1_dma_y_bs1       imc_sfr_ptr(0x058)
#define imc_ch1_dma_u_bs1       imc_sfr_ptr(0x059)
#define imc_ch1_dma_v_bs1       imc_sfr_ptr(0x05a)

#define imc_ch2_com_con         imc_sfr_ptr(0x060)
#define imc_ch2_src_con         imc_sfr_ptr(0x061)
#define imc_ch2_crop_h          imc_sfr_ptr(0x062)
#define imc_ch2_crop_v          imc_sfr_ptr(0x063)
#define imc_ch2_h_stp           imc_sfr_ptr(0x064)
#define imc_ch2_h_wth           imc_sfr_ptr(0x065)
#define imc_ch2_v_stp           imc_sfr_ptr(0x066)
#define imc_ch2_v_wth           imc_sfr_ptr(0x067)
#define imc_ch2_dma_con         imc_sfr_ptr(0x073)
#define imc_ch2_dma_cnt         imc_sfr_ptr(0x074)
#define imc_ch2_dma_y_bs0       imc_sfr_ptr(0x075)
#define imc_ch2_dma_u_bs0       imc_sfr_ptr(0x076)
#define imc_ch2_dma_v_bs0       imc_sfr_ptr(0x077)
#define imc_ch2_dma_y_bs1       imc_sfr_ptr(0x078)
#define imc_ch2_dma_u_bs1       imc_sfr_ptr(0x079)
#define imc_ch2_dma_v_bs1       imc_sfr_ptr(0x07a)

#define imc_ch3_com_con         imc_sfr_ptr(0x080)
#define imc_ch3_src_con         imc_sfr_ptr(0x081)
#define imc_ch3_crop_h          imc_sfr_ptr(0x082)
#define imc_ch3_crop_v          imc_sfr_ptr(0x083)
#define imc_ch3_h_stp           imc_sfr_ptr(0x084)
#define imc_ch3_h_wth           imc_sfr_ptr(0x085)
#define imc_ch3_v_stp           imc_sfr_ptr(0x086)
#define imc_ch3_v_wth           imc_sfr_ptr(0x087)
#define imc_ch3_dma_con         imc_sfr_ptr(0x093)
#define imc_ch3_dma_cnt         imc_sfr_ptr(0x094)
#define imc_ch3_dma_y_bs0       imc_sfr_ptr(0x095)
#define imc_ch3_dma_u_bs0       imc_sfr_ptr(0x096)
#define imc_ch3_dma_v_bs0       imc_sfr_ptr(0x097)
#define imc_ch3_dma_y_bs1       imc_sfr_ptr(0x098)
#define imc_ch3_dma_u_bs1       imc_sfr_ptr(0x099)
#define imc_ch3_dma_v_bs1       imc_sfr_ptr(0x09a)

#define imc_ch4_com_con         imc_sfr_ptr(0x0a0)
#define imc_ch4_tpz_cfg0        imc_sfr_ptr(0x0a1)
#define imc_ch4_tpz_cfg1        imc_sfr_ptr(0x0a2)
#define imc_ch4_tpz_cfg2        imc_sfr_ptr(0x0a3)
#define imc_ch4_sca_stp         imc_sfr_ptr(0x0a4)
#define imc_ch4_sca_pha         imc_sfr_ptr(0x0a5)
#define imc_ch4_sca_wth         imc_sfr_ptr(0x0a6)
#define imc_ch4_dma_con         imc_sfr_ptr(0x0a7)
#define imc_ch4_dma_cnt         imc_sfr_ptr(0x0a8)
#define imc_ch4_dma_r_bs0       imc_sfr_ptr(0x0a9)
#define imc_ch4_dma_g_bs0       imc_sfr_ptr(0x0aa)
#define imc_ch4_dma_b_bs0       imc_sfr_ptr(0x0ab)
#define imc_ch4_dma_r_bs1       imc_sfr_ptr(0x0ac)
#define imc_ch4_dma_g_bs1       imc_sfr_ptr(0x0ad)
#define imc_ch4_dma_b_bs1       imc_sfr_ptr(0x0ae)

//==============================================================//
//  imb
//==============================================================//
#define imb_base    eva_base + 0x2800
#define imb_sfr_ptr(num)        (*(volatile u32 *)(imb_base + num*4))

#define imb_con                 imb_sfr_ptr(0x0)
#define imb_h_cfg               imb_sfr_ptr(0x1)
#define imb_v_cfg               imb_sfr_ptr(0x2)
#define imb_y_bs0               imb_sfr_ptr(0x3)
#define imb_u_bs0               imb_sfr_ptr(0x4)
#define imb_v_bs0               imb_sfr_ptr(0x5)
#define imb_y_bs1               imb_sfr_ptr(0x6)
#define imb_u_bs1               imb_sfr_ptr(0x7)
#define imb_v_bs1               imb_sfr_ptr(0x8)
#define imb_y_bs2               imb_sfr_ptr(0x9)
#define imb_u_bs2               imb_sfr_ptr(0xa)
#define imb_v_bs2               imb_sfr_ptr(0xb)

//==============================================================//
//  imd
//==============================================================//
#define imd_dmm_base    eva_base + 0x3000

#define imd_l0_con              (*(volatile u32 *)(imd_dmm_base + 0x00*4))
#define imd_l0_hs               (*(volatile u32 *)(imd_dmm_base + 0x01*4))
#define imd_l0_he               (*(volatile u32 *)(imd_dmm_base + 0x02*4))
#define imd_l0_vs               (*(volatile u32 *)(imd_dmm_base + 0x03*4))
#define imd_l0_ve               (*(volatile u32 *)(imd_dmm_base + 0x04*4))
#define imd_l0_haw              (*(volatile u32 *)(imd_dmm_base + 0x05*4))
#define imd_l0_htw              (*(volatile u32 *)(imd_dmm_base + 0x06*4))
#define imd_l0_aph              (*(volatile u32 *)(imd_dmm_base + 0x07*4))
#define imd_l0_badr0a           (*(volatile u32 *)(imd_dmm_base + 0x08*4))
#define imd_l0_badr1a           (*(volatile u32 *)(imd_dmm_base + 0x09*4))
#define imd_l0_badr2a           (*(volatile u32 *)(imd_dmm_base + 0x0a*4))
#define imd_l0_badr0b           (*(volatile u32 *)(imd_dmm_base + 0x0b*4))
#define imd_l0_badr1b           (*(volatile u32 *)(imd_dmm_base + 0x0c*4))
#define imd_l0_badr2b           (*(volatile u32 *)(imd_dmm_base + 0x0d*4))

#define imd_l1_con              (*(volatile u32 *)(imd_dmm_base + 0x10*4))
#define imd_l1_hs               (*(volatile u32 *)(imd_dmm_base + 0x11*4))
#define imd_l1_he               (*(volatile u32 *)(imd_dmm_base + 0x12*4))
#define imd_l1_vs               (*(volatile u32 *)(imd_dmm_base + 0x13*4))
#define imd_l1_ve               (*(volatile u32 *)(imd_dmm_base + 0x14*4))
#define imd_l1_haw              (*(volatile u32 *)(imd_dmm_base + 0x15*4))
#define imd_l1_htw              (*(volatile u32 *)(imd_dmm_base + 0x16*4))
#define imd_l1_aph              (*(volatile u32 *)(imd_dmm_base + 0x17*4))
#define imd_l1_badr0a           (*(volatile u32 *)(imd_dmm_base + 0x18*4))
#define imd_l1_badr1a           (*(volatile u32 *)(imd_dmm_base + 0x19*4))
#define imd_l1_badr2a           (*(volatile u32 *)(imd_dmm_base + 0x1a*4))
#define imd_l1_badr0b           (*(volatile u32 *)(imd_dmm_base + 0x1b*4))
#define imd_l1_badr1b           (*(volatile u32 *)(imd_dmm_base + 0x1c*4))
#define imd_l1_badr2b           (*(volatile u32 *)(imd_dmm_base + 0x1d*4))

#define imd_l2_con              (*(volatile u32 *)(imd_dmm_base + 0x20*4))
#define imd_l2_hs               (*(volatile u32 *)(imd_dmm_base + 0x21*4))
#define imd_l2_he               (*(volatile u32 *)(imd_dmm_base + 0x22*4))
#define imd_l2_vs               (*(volatile u32 *)(imd_dmm_base + 0x23*4))
#define imd_l2_ve               (*(volatile u32 *)(imd_dmm_base + 0x24*4))
#define imd_l2_haw              (*(volatile u32 *)(imd_dmm_base + 0x25*4))
#define imd_l2_htw              (*(volatile u32 *)(imd_dmm_base + 0x26*4))
#define imd_l2_aph              (*(volatile u32 *)(imd_dmm_base + 0x27*4))
#define imd_l2_badr0a           (*(volatile u32 *)(imd_dmm_base + 0x28*4))
#define imd_l2_badr1a           (*(volatile u32 *)(imd_dmm_base + 0x29*4))
#define imd_l2_badr2a           (*(volatile u32 *)(imd_dmm_base + 0x2a*4))
#define imd_l2_badr0b           (*(volatile u32 *)(imd_dmm_base + 0x2b*4))
#define imd_l2_badr1b           (*(volatile u32 *)(imd_dmm_base + 0x2c*4))
#define imd_l2_badr2b           (*(volatile u32 *)(imd_dmm_base + 0x2d*4))

#define imd_l3_con              (*(volatile u32 *)(imd_dmm_base + 0x30*4))
#define imd_l3_hs               (*(volatile u32 *)(imd_dmm_base + 0x31*4))
#define imd_l3_he               (*(volatile u32 *)(imd_dmm_base + 0x32*4))
#define imd_l3_vs               (*(volatile u32 *)(imd_dmm_base + 0x33*4))
#define imd_l3_ve               (*(volatile u32 *)(imd_dmm_base + 0x34*4))
#define imd_l3_haw              (*(volatile u32 *)(imd_dmm_base + 0x35*4))
#define imd_l3_htw              (*(volatile u32 *)(imd_dmm_base + 0x36*4))
#define imd_l3_aph              (*(volatile u32 *)(imd_dmm_base + 0x37*4))
#define imd_l3_badr0a           (*(volatile u32 *)(imd_dmm_base + 0x38*4))
#define imd_l3_badr1a           (*(volatile u32 *)(imd_dmm_base + 0x39*4))
#define imd_l3_badr2a           (*(volatile u32 *)(imd_dmm_base + 0x3a*4))
#define imd_l3_badr0b           (*(volatile u32 *)(imd_dmm_base + 0x3b*4))
#define imd_l3_badr1b           (*(volatile u32 *)(imd_dmm_base + 0x3c*4))
#define imd_l3_badr2b           (*(volatile u32 *)(imd_dmm_base + 0x3d*4))

#define imd_r_gain              (*(volatile u32 *)(imd_dmm_base + 0x40*4))
#define imd_g_gain              (*(volatile u32 *)(imd_dmm_base + 0x41*4))
#define imd_b_gain              (*(volatile u32 *)(imd_dmm_base + 0x42*4))
#define imd_r_offs              (*(volatile u32 *)(imd_dmm_base + 0x43*4))
#define imd_g_offs              (*(volatile u32 *)(imd_dmm_base + 0x44*4))
#define imd_b_offs              (*(volatile u32 *)(imd_dmm_base + 0x45*4))
#define imd_r_coe0              (*(volatile u32 *)(imd_dmm_base + 0x46*4))
#define imd_r_coe1              (*(volatile u32 *)(imd_dmm_base + 0x47*4))
#define imd_r_coe2              (*(volatile u32 *)(imd_dmm_base + 0x48*4))
#define imd_g_coe0              (*(volatile u32 *)(imd_dmm_base + 0x49*4))
#define imd_g_coe1              (*(volatile u32 *)(imd_dmm_base + 0x4a*4))
#define imd_g_coe2              (*(volatile u32 *)(imd_dmm_base + 0x4b*4))
#define imd_b_coe0              (*(volatile u32 *)(imd_dmm_base + 0x4c*4))
#define imd_b_coe1              (*(volatile u32 *)(imd_dmm_base + 0x4d*4))
#define imd_b_coe2              (*(volatile u32 *)(imd_dmm_base + 0x4e*4))

#define imd_l2_ot0              (*(volatile u32 *)(imd_dmm_base + 0x50*4))
#define imd_l2_ot1              (*(volatile u32 *)(imd_dmm_base + 0x51*4))
#define imd_l3_ot0              (*(volatile u32 *)(imd_dmm_base + 0x52*4))
#define imd_l3_ot1              (*(volatile u32 *)(imd_dmm_base + 0x53*4))
#define imd_gmm_r0              (*(volatile u32 *)(imd_dmm_base + 0x54*4))
#define imd_gmm_g0              (*(volatile u32 *)(imd_dmm_base + 0x55*4))
#define imd_gmm_b0              (*(volatile u32 *)(imd_dmm_base + 0x56*4))
#define imd_gmm_r1              (*(volatile u32 *)(imd_dmm_base + 0x57*4))
#define imd_gmm_g1              (*(volatile u32 *)(imd_dmm_base + 0x58*4))
#define imd_gmm_b1              (*(volatile u32 *)(imd_dmm_base + 0x59*4))
#define imd_y_gain              (*(volatile u32 *)(imd_dmm_base + 0x5a*4))
#define imd_u_gain              (*(volatile u32 *)(imd_dmm_base + 0x5b*4))
#define imd_v_gain              (*(volatile u32 *)(imd_dmm_base + 0x5c*4))
#define imd_y_offs              (*(volatile u32 *)(imd_dmm_base + 0x5d*4))
#define imd_u_offs              (*(volatile u32 *)(imd_dmm_base + 0x5e*4))
#define imd_v_offs              (*(volatile u32 *)(imd_dmm_base + 0x5f*4))

#define imd_scn_con             (*(volatile u32 *)(imd_dmm_base + 0x60*4))
#define imd_lc_con              (*(volatile u32 *)(imd_dmm_base + 0x61*4))
#define imd_img_hw              (*(volatile u32 *)(imd_dmm_base + 0x62*4))
#define imd_img_vw              (*(volatile u32 *)(imd_dmm_base + 0x63*4))
#define imd_mc_con              (*(volatile u32 *)(imd_dmm_base + 0x64*4))
#define imd_sc_dat              (*(volatile u32 *)(imd_dmm_base + 0x65*4))

#define imd_sca_con             (*(volatile u32 *)(imd_dmm_base + 0x70*4))
#define imd_sca_stp_h           (*(volatile u32 *)(imd_dmm_base + 0x71*4))
#define imd_sca_wth_h           (*(volatile u32 *)(imd_dmm_base + 0x72*4))
#define imd_sca_stp_v           (*(volatile u32 *)(imd_dmm_base + 0x73*4))
#define imd_sca_wth_v           (*(volatile u32 *)(imd_dmm_base + 0x74*4))

#define imd_dpi_base    eva_base + 0x3200
#define imd_dpi_clk_con         (*(volatile u32 *)(imd_dpi_base + 0x00*4))
#define imd_dpi_io_con          (*(volatile u32 *)(imd_dpi_base + 0x01*4))
#define imd_dpi_con             (*(volatile u32 *)(imd_dpi_base + 0x02*4))
#define imd_dpi_fmt             (*(volatile u32 *)(imd_dpi_base + 0x03*4))
#define imd_dpi_emi             (*(volatile u32 *)(imd_dpi_base + 0x04*4))
#define imd_dpi_htt_cfg         (*(volatile u32 *)(imd_dpi_base + 0x05*4))
#define imd_dpi_hsw_cfg         (*(volatile u32 *)(imd_dpi_base + 0x06*4))
#define imd_dpi_hst_cfg         (*(volatile u32 *)(imd_dpi_base + 0x07*4))
#define imd_dpi_haw_cfg         (*(volatile u32 *)(imd_dpi_base + 0x08*4))
#define imd_dpi_vtt_cfg         (*(volatile u32 *)(imd_dpi_base + 0x09*4))
#define imd_dpi_vsw_cfg         (*(volatile u32 *)(imd_dpi_base + 0x0a*4))
#define imd_dpi_vst_o_cfg       (*(volatile u32 *)(imd_dpi_base + 0x0b*4))
#define imd_dpi_vst_e_cfg       (*(volatile u32 *)(imd_dpi_base + 0x0c*4))
#define imd_dpi_vaw_cfg         (*(volatile u32 *)(imd_dpi_base + 0x0d*4))

//==============================================================//
//  cvbs
//==============================================================//
#define cvbs_base   eva_base + 0x3800
#define cvbs_sfr_ptr(num)       (*(volatile u32 *)(cvbs_base + num*4))
#define cvbs_clk_con            cvbs_sfr_ptr(0x0)
#define cvbs_dac_con0           cvbs_sfr_ptr(0x1)
#define cvbs_dac_con1           cvbs_sfr_ptr(0x2)
#define cvbs_dac_con2           cvbs_sfr_ptr(0x3)
#define cvbs_dac_con3           cvbs_sfr_ptr(0x4)
#define cvbs_dac_con4           cvbs_sfr_ptr(0x5)

#define tve_base    eva_base + 0x3c00
#define tve_sfr_ptr(num)        (*(volatile u32 *)(tve_base + num*4))
#define tve_con0                tve_sfr_ptr(0)
#define tve_con1                tve_sfr_ptr(1)
#define tve_con2                tve_sfr_ptr(2)
#define tve_con3                tve_sfr_ptr(3)
#define tve_con4                tve_sfr_ptr(4)
#define tve_con5                tve_sfr_ptr(5)
#define tve_con6                tve_sfr_ptr(6)
#define tve_con7                tve_sfr_ptr(7)
#define tve_con8                tve_sfr_ptr(8)
#define tve_con9                tve_sfr_ptr(9)
#define tve_con10               tve_sfr_ptr(10)
#define tve_con11               tve_sfr_ptr(11)
#define tve_con12               tve_sfr_ptr(12)
#define tve_con13               tve_sfr_ptr(13)
#define tve_con14               tve_sfr_ptr(14)
#define tve_con15               tve_sfr_ptr(15)
#define tve_con16               tve_sfr_ptr(16)
#define tve_con17               tve_sfr_ptr(17)
#define tve_con18               tve_sfr_ptr(18)
#define tve_con19               tve_sfr_ptr(19)
#define tve_con20               tve_sfr_ptr(20)
#define tve_con21               tve_sfr_ptr(21)
#define tve_con22               tve_sfr_ptr(22)
#define tve_con23               tve_sfr_ptr(23)
#define tve_con24               tve_sfr_ptr(24)
#define tve_con25               tve_sfr_ptr(25)
#define tve_con26               tve_sfr_ptr(26)
#define tve_con27               tve_sfr_ptr(27)
#define tve_con28               tve_sfr_ptr(28)
#define tve_con29               tve_sfr_ptr(29)
#define tve_con30               tve_sfr_ptr(30)
#define tve_con31               tve_sfr_ptr(31)

#define cve_base    eva_base + 0x3e00
#define cve_sfr_ptr(num)        (*(volatile u32 *)(cve_base + num*4))
#define cve_soft_reset          cve_sfr_ptr(62)
#define cve_system_con          cve_sfr_ptr(52)
#define cve_slave_mode          cve_sfr_ptr(13)
#define cve_clrbar_mode         cve_sfr_ptr(5)
#define cve_chroma_freq0        cve_sfr_ptr(0)
#define cve_chroma_freq1        cve_sfr_ptr(1)
#define cve_chroma_freq2        cve_sfr_ptr(2)
#define cve_chroma_freq3        cve_sfr_ptr(3)
#define cve_chroma_phase        cve_sfr_ptr(4)
#define cve_black_level_h       cve_sfr_ptr(14)
#define cve_black_level_l       cve_sfr_ptr(15)
#define cve_blank_level_h       cve_sfr_ptr(16)
#define cve_blank_level_l       cve_sfr_ptr(17)
#define cve_white_level_h       cve_sfr_ptr(30)
#define cve_white_level_l       cve_sfr_ptr(31)
#define cve_hsync_width         cve_sfr_ptr(8)
#define cve_burst_width         cve_sfr_ptr(9)
#define cve_back_porch          cve_sfr_ptr(10)
#define cve_breeze_way          cve_sfr_ptr(41)
#define cve_front_porch         cve_sfr_ptr(44)
#define cve_start_line          cve_sfr_ptr(51)
#define cve_num_lines_h         cve_sfr_ptr(23)
#define cve_num_lines_l         cve_sfr_ptr(24)
#define cve_line_period_h       cve_sfr_ptr(49)
#define cve_line_period_l       cve_sfr_ptr(50)
#define cve_cb_burst_amp        cve_sfr_ptr(11)
#define cve_cr_burst_amp        cve_sfr_ptr(12)
#define cve_cb_gain             cve_sfr_ptr(32)
#define cve_cr_gain             cve_sfr_ptr(34)

//==============================================================//
//  mipi
//==============================================================//
#define mipi_base   eva_base + 0x4000
#define csi_base    (mipi_base + 0x0000)
#define dsi_s_base  (mipi_base + 0x0200)
#define dsi_d_base  (mipi_base + 0x0300)
#define mp_phy_base (mipi_base + 0x0400)

#define csi_sys_con                 (*(volatile u32 *)(csi_base + 0x00*4))
#define csi_rmap_con                (*(volatile u32 *)(csi_base + 0x01*4))
#define csi_lane_con                (*(volatile u32 *)(csi_base + 0x02*4))
#define csi_tval_con                (*(volatile u32 *)(csi_base + 0x03*4))
#define csi_task_con                (*(volatile u32 *)(csi_base + 0x04*4))
#define csi_task_haw                (*(volatile u32 *)(csi_base + 0x05*4))

#define dsi_sys_con                 (*(volatile u32 *)(dsi_s_base + 0x00*4))
#define dsi_vdo_fmt                 (*(volatile u32 *)(dsi_s_base + 0x10*4))
#define dsi_vdo_vsa                 (*(volatile u32 *)(dsi_s_base + 0x11*4))
#define dsi_vdo_vbp                 (*(volatile u32 *)(dsi_s_base + 0x12*4))
#define dsi_vdo_vact                (*(volatile u32 *)(dsi_s_base + 0x13*4))
#define dsi_vdo_vfp                 (*(volatile u32 *)(dsi_s_base + 0x14*4))
#define dsi_vdo_hsa                 (*(volatile u32 *)(dsi_s_base + 0x15*4))
#define dsi_vdo_hbp                 (*(volatile u32 *)(dsi_s_base + 0x16*4))
#define dsi_vdo_hact                (*(volatile u32 *)(dsi_s_base + 0x17*4))
#define dsi_vdo_hfp                 (*(volatile u32 *)(dsi_s_base + 0x18*4))
#define dsi_vdo_bllp0               (*(volatile u32 *)(dsi_s_base + 0x19*4))
#define dsi_vdo_bllp1               (*(volatile u32 *)(dsi_s_base + 0x1a*4))
#define dsi_frm_tval                (*(volatile u32 *)(dsi_s_base + 0x1b*4))
#define dsi_tval_con0               (*(volatile u32 *)(dsi_s_base + 0x20*4))
#define dsi_tval_con1               (*(volatile u32 *)(dsi_s_base + 0x21*4))
#define dsi_tval_con2               (*(volatile u32 *)(dsi_s_base + 0x22*4))
#define dsi_tval_con3               (*(volatile u32 *)(dsi_s_base + 0x23*4))
#define dsi_tval_con4               (*(volatile u32 *)(dsi_s_base + 0x24*4))
#define dsi_tval_con5               (*(volatile u32 *)(dsi_s_base + 0x25*4))
#define dsi_rmap_con                (*(volatile u32 *)(dsi_s_base + 0x26*4))

#define dsi_task_con                (*(volatile u32 *)(dsi_d_base + 0x00*4))
#define dsi_bus_con                 (*(volatile u32 *)(dsi_d_base + 0x01*4))
#define dsi_cmd_con0                (*(volatile u32 *)(dsi_d_base + 0x02*4))
#define dsi_cmd_con1                (*(volatile u32 *)(dsi_d_base + 0x03*4))
#define dsi_cmd_con2                (*(volatile u32 *)(dsi_d_base + 0x04*4))
#define dsi_cmd_fifo                (*(volatile u32 *)(dsi_d_base + 0x05*4))
#define dsi_lane_con                (*(volatile u32 *)(dsi_d_base + 0x06*4))

#define mipi_pll_con0               (*(volatile u32 *)(mp_phy_base + 0x00*4))
#define mipi_pll_con1               (*(volatile u32 *)(mp_phy_base + 0x01*4))
#define mipi_phy_con0               (*(volatile u32 *)(mp_phy_base + 0x10*4))
#define mipi_phy_con1               (*(volatile u32 *)(mp_phy_base + 0x11*4))
#define mipi_phy_con2               (*(volatile u32 *)(mp_phy_base + 0x12*4))
#define mipi_phy_con3               (*(volatile u32 *)(mp_phy_base + 0x13*4))
#define mipi_phy_con4               (*(volatile u32 *)(mp_phy_base + 0x14*4))
#define mipi_phy_con5               (*(volatile u32 *)(mp_phy_base + 0x15*4))
#define mipi_phy_con6               (*(volatile u32 *)(mp_phy_base + 0x16*4))
#define mipi_phy_con7               (*(volatile u32 *)(mp_phy_base + 0x17*4))
#define mipi_phy_con8               (*(volatile u32 *)(mp_phy_base + 0x18*4))
#define mipi_phy_con9               (*(volatile u32 *)(mp_phy_base + 0x19*4))
#define mipi_phy_con10              (*(volatile u32 *)(mp_phy_base + 0x1a*4))
#define mipi_phy_con11              (*(volatile u32 *)(mp_phy_base + 0x1b*4))
#define mipi_phy_con12              (*(volatile u32 *)(mp_phy_base + 0x1c*4))
#define mipi_phy_con13              (*(volatile u32 *)(mp_phy_base + 0x1d*4))
#define mipi_phy_con14              (*(volatile u32 *)(mp_phy_base + 0x1e*4))
#define mipi_phy_con15              (*(volatile u32 *)(mp_phy_base + 0x1f*4))

//==============================================================//
//  imr
//==============================================================//
#define imr_base    eva_base + 0x4800

#define imr_scn_con                 (*(volatile u32 *)(imr_base + 0x00*4))

#define imr0_con                    (*(volatile u32 *)(imr_base + 0x01*4))
#define imr0_haw                    (*(volatile u32 *)(imr_base + 0x02*4))
#define imr0_vaw                    (*(volatile u32 *)(imr_base + 0x03*4))
#define imr0_b0_adr0                (*(volatile u32 *)(imr_base + 0x04*4))
#define imr0_b0_adr1                (*(volatile u32 *)(imr_base + 0x05*4))
#define imr0_b0_adr2                (*(volatile u32 *)(imr_base + 0x06*4))
#define imr0_b1_adr0                (*(volatile u32 *)(imr_base + 0x07*4))
#define imr0_b1_adr1                (*(volatile u32 *)(imr_base + 0x08*4))
#define imr0_b1_adr2                (*(volatile u32 *)(imr_base + 0x09*4))
#define imr0_wr_adr0                (*(volatile u32 *)(imr_base + 0x0a*4))
#define imr0_wr_adr1                (*(volatile u32 *)(imr_base + 0x0b*4))
#define imr0_wr_adr2                (*(volatile u32 *)(imr_base + 0x0c*4))

#define imr1_con                    (*(volatile u32 *)(imr_base + 0x11*4))
#define imr1_haw                    (*(volatile u32 *)(imr_base + 0x12*4))
#define imr1_vaw                    (*(volatile u32 *)(imr_base + 0x13*4))
#define imr1_b0_adr0                (*(volatile u32 *)(imr_base + 0x14*4))
#define imr1_b0_adr1                (*(volatile u32 *)(imr_base + 0x15*4))
#define imr1_b0_adr2                (*(volatile u32 *)(imr_base + 0x16*4))
#define imr1_b1_adr0                (*(volatile u32 *)(imr_base + 0x17*4))
#define imr1_b1_adr1                (*(volatile u32 *)(imr_base + 0x18*4))
#define imr1_b1_adr2                (*(volatile u32 *)(imr_base + 0x19*4))
#define imr1_wr_adr0                (*(volatile u32 *)(imr_base + 0x1a*4))
#define imr1_wr_adr1                (*(volatile u32 *)(imr_base + 0x1b*4))
#define imr1_wr_adr2                (*(volatile u32 *)(imr_base + 0x1c*4))

#endif



