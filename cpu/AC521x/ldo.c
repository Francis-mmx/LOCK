#include "asm/includes.h"
#include "asm/sysinfo.h"
#include "asm/ldo.h"

//如需使用avdd18 avdd28 drcvdd 作为ldo请和硬件同事沟通

void avdd18_ctrl(AVDD18_LEV lev, u8 avdd18en)
{
    avdd18en = (avdd18en ? 1 : 0);
    if (lev > 7) {
        lev = 7;
    }
    SFR(LDO_CON, 9, 1, avdd18en);
    SFR(LDO_CON, 3, 3, lev);
}

void avdd28_ctrl(AVDD28_LEV lev, u8 avdd28en)
{
    avdd28en = (avdd28en ? 1 : 0);
    if (lev > 7) {
        lev = 7;
    }
    SFR(LDO_CON, 10, 1, avdd28en);
    SFR(LDO_CON, 6, 3, lev);
}

void dvdd_ctrl(DVDD_LEV lev)
{
    if (lev > 7) {
        lev = 7;
    }
    SFR(LDO_CON, 0, 3, lev);
}

/*
 *lev: 0--->2.2v
 *lev: 1--->2.3v
 *lev: 2--->2.4v
 *lev: 3--->2.5v
 *lev: 4--->2.6v
 *lev: 5--->2.7v
 *lev: 6--->2.8v
 *lev: 7--->2.9v
 */
void lvd_cfg(u8 lev)
{
#if 1
    LVD_CON = 0;//先关闭,再配置

    LVD_CON |= (0x7 & (lev));//lev
    LVD_CON &= ~BIT(7);//0:force reset at onece   1:force reset delay after 40us
    delay(10);
    LVD_CON &= ~BIT(6);//force reset system
    delay(10);
    LVD_CON &= ~BIT(5);//ldo_in
    delay(10);
    LVD_CON |= BIT(4);//AEN
    delay(100);
    LVD_CON |= BIT(3);//AOE
    delay(10);
#endif
}


