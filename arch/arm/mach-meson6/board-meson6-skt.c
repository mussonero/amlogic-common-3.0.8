/*
 * arch/arm/mach-meson6/board-meson6-skt.c
 *
 * Copyright (C) 2011-2012 Amlogic, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <asm/mach/map.h>
#include <plat/platform.h>
#include <plat/plat_dev.h>
#include <plat/platform_data.h>

#include <linux/io.h>
#include <plat/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/device.h>
#include <linux/spi/flash.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <asm/mach-types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <plat/platform.h>
#include <plat/plat_dev.h>
#include <plat/platform_data.h>
#include <plat/lm.h>
#include <plat/regops.h>
#include <linux/io.h>
#include <plat/io.h>

#include <mach/map.h>
#include <mach/i2c_aml.h>
#include <mach/nand.h>
#include <mach/usbclock.h>
#include <mach/usbsetting.h>
#include <mach/pm.h>

#include <linux/uart-aml.h>
#include <linux/i2c-aml.h>
#include <mach/gpio_data.h>
#include <mach/pinmux.h>
#include <mach/gpio.h>


#ifdef CONFIG_AM_ETHERNET
#include <mach/am_regs.h>
#include <mach/am_eth_reg.h>
#endif

#include "board-m6skt.h"

#ifdef CONFIG_MMC_AML
#include <mach/mmc.h>
#endif

#ifdef CONFIG_CARDREADER
#include <mach/card_io.h>
#endif // CONFIG_CARDREADER

#include <mach/gpio.h>
#ifdef CONFIG_EFUSE
#include <linux/efuse.h>
#endif

#ifdef CONFIG_AMLOGIC_MIPI
#include <linux/mipi/am_mipi_csi2.h>
#endif

#ifdef CONFIG_VIDEO_AMLOGIC_CAPTURE
#include <media/amlogic/aml_camera.h>
#endif
#ifdef CONFIG_SND_SOC_DUMMY_CODEC
#include <sound/dummy_codec.h>
#endif
/***********************************************************************
 * IO Mapping
 **********************************************************************/
#define PHYS_MEM_START      (0x80000000)
#define PHYS_MEM_SIZE       (512*SZ_1M)
#define PHYS_MEM_END        (PHYS_MEM_START + PHYS_MEM_SIZE -1 )

/******** Reserved memory setting ************************/
#define RESERVED_MEM_START  (0x80000000+64*SZ_1M)   /*start at the second 64M*/

/******** CODEC memory setting ************************/
//  Codec need 16M for 1080p decode
//  4M for sd decode;
#define ALIGN_MSK           ((SZ_1M)-1)
#define U_ALIGN(x)          ((x+ALIGN_MSK)&(~ALIGN_MSK))
#define D_ALIGN(x)          ((x)&(~ALIGN_MSK))

/******** AUDIODSP memory setting ************************/
#define AUDIODSP_ADDR_START U_ALIGN(RESERVED_MEM_START) /*audiodsp memstart*/
#define AUDIODSP_ADDR_END   (AUDIODSP_ADDR_START+SZ_1M-1)   /*audiodsp memend*/

/******** Frame buffer memory configuration ***********/
#define OSD_480_PIX         (640*480)
#define OSD_576_PIX         (768*576)
#define OSD_720_PIX         (1280*720)
#define OSD_1080_PIX        (1920*1080)
#define OSD_PANEL_PIX       (800*480)
#define B16BpP  (2)
#define B32BpP  (4)
#define DOUBLE_BUFFER   (2)

#define OSD1_MAX_MEM        U_ALIGN(OSD_1080_PIX*B32BpP*DOUBLE_BUFFER)
#define OSD2_MAX_MEM        U_ALIGN(32*32*B32BpP)

/******** Reserved memory configuration ***************/
#define OSD1_ADDR_START     U_ALIGN(AUDIODSP_ADDR_END )
#define OSD1_ADDR_END       (OSD1_ADDR_START+OSD1_MAX_MEM - 1)
#define OSD2_ADDR_START     U_ALIGN(OSD1_ADDR_END)
#define OSD2_ADDR_END       (OSD2_ADDR_START +OSD2_MAX_MEM -1)

#if defined(CONFIG_AM_VDEC_H264MVC)
#define CODEC_MEM_SIZE      U_ALIGN(64*SZ_1M)
#elif defined(CONFIG_AM_VDEC_H264)
#define CODEC_MEM_SIZE      U_ALIGN(32*SZ_1M)
#else
#define CODEC_MEM_SIZE      U_ALIGN(16*SZ_1M)
#endif
#define CODEC_ADDR_START    U_ALIGN(OSD2_ADDR_END)
#define CODEC_ADDR_END      (CODEC_ADDR_START+CODEC_MEM_SIZE-1)

/********VDIN memory configuration ***************/
#if defined(CONFIG_TVIN_VDIN)
#define VDIN_MEM_SIZE       (32*SZ_1M)
#else
#define VDIN_MEM_SIZE       (0)
#endif
#define VDIN_ADDR_START     U_ALIGN(CODEC_ADDR_END)
#define VDIN_ADDR_END       (VDIN_ADDR_START + VDIN_MEM_SIZE - 1)

#if defined(CONFIG_AMLOGIC_VIDEOIN_MANAGER)
#define VM_SIZE             (SZ_1M*16)
#else
#define VM_SIZE             (0)
#endif /* CONFIG_AMLOGIC_VIDEOIN_MANAGER  */

#define VM_ADDR_START       U_ALIGN(VDIN_ADDR_END)
#define VM_ADDR_END         (VM_SIZE + VM_ADDR_START - 1)

/********MIPI memory configuration ***************/
#ifdef CONFIG_AMLOGIC_MIPI
#define MIPI_MEM_SIZE       (SZ_1M*32)
#else
#define MIPI_MEM_SIZE       (0)
#endif
#define MIPI_ADDR_START      U_ALIGN(VM_ADDR_END)
#define MIPI_ADDR_END       (MIPI_ADDR_START + MIPI_MEM_SIZE - 1)

#if defined(CONFIG_AM_DEINTERLACE_SD_ONLY)
#define DI_MEM_SIZE         (SZ_1M*3)
#else
#define DI_MEM_SIZE         (SZ_1M*15)
#endif
#define DI_ADDR_START       U_ALIGN(MIPI_ADDR_END)
#define DI_ADDR_END         (DI_ADDR_START+DI_MEM_SIZE-1)

#ifdef CONFIG_POST_PROCESS_MANAGER
#ifdef CONFIG_POST_PROCESS_MANAGER_PPSCALER
#define PPMGR_MEM_SIZE               1920 * 1088 * 21
#else
#define PPMGR_MEM_SIZE               1920 * 1088 * 15
#endif
#else
#define PPMGR_MEM_SIZE		0
#endif /* CONFIG_POST_PROCESS_MANAGER */

#define PPMGR_ADDR_START	U_ALIGN(DI_ADDR_END)
#define PPMGR_ADDR_END		(PPMGR_ADDR_START+PPMGR_MEM_SIZE-1)

#define STREAMBUF_MEM_SIZE          (SZ_1M*7)
#define STREAMBUF_ADDR_START        U_ALIGN(PPMGR_ADDR_END)
#define STREAMBUF_ADDR_END      (STREAMBUF_ADDR_START+STREAMBUF_MEM_SIZE-1)

#define RESERVED_MEM_END    (STREAMBUF_ADDR_END)

static struct resource meson_fb_resource[] = {
    [0] = {
        .start = OSD1_ADDR_START,
        .end   = OSD1_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = OSD2_ADDR_START,
        .end   = OSD2_ADDR_END,
        .flags = IORESOURCE_MEM,
    },

};

static struct resource meson_codec_resource[] = {
    [0] = {
        .start = CODEC_ADDR_START,
        .end   = CODEC_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = STREAMBUF_ADDR_START,
        .end   = STREAMBUF_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
};

#ifdef CONFIG_POST_PROCESS_MANAGER
static struct resource ppmgr_resources[] = {
    [0] = {
        .start = PPMGR_ADDR_START,
        .end   = PPMGR_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device ppmgr_device = {
    .name       = "ppmgr",
    .id         = 0,
    .num_resources = ARRAY_SIZE(ppmgr_resources),
    .resource      = ppmgr_resources,
};
#endif

static  int __init setup_devices_resource(void)
{
    setup_fb_resource(meson_fb_resource, ARRAY_SIZE(meson_fb_resource));
#ifdef CONFIG_AM_STREAMING
    setup_codec_resource(meson_codec_resource, ARRAY_SIZE(meson_codec_resource));
#endif
    return 0;
}

#ifdef CONFIG_AM_LCD_OUTPUT
#include "board-m6skt.h"
#endif

#if defined(CONFIG_VIDEO_AMLOGIC_CAPTURE_GC0339)
static int gc0339_have_inited = 0;
static pinmux_item_t gc0339_pins[] = {
    {
        .reg = PINMUX_REG(9),
        .setmask = 1 << 12
    },
    PINMUX_END_ITEM
};

static pinmux_set_t gc0339_pinmux_set = {
    .chip_select = NULL,
    .pinmux = &gc0339_pins[0]
};

static void gc0339_init(void)
{
    pinmux_set(&gc0339_pinmux_set);
    aml_set_reg32_bits(P_HHI_GEN_CLK_CNTL, 1, 8, 5); //select XTAL as camera clock

    msleep(20);
    // set camera power enable
    //gpio_out(PAD_GPIOE_10, 1);    // set camera power enable
    //msleep(20);

    gpio_out(PAD_GPIOZ_0, 0);    // reset IO
    msleep(20);

    gpio_out(PAD_GPIOZ_0, 1);    // reset IO
    msleep(20);

    // set camera power enable
    //gpio_out(PAD_GPIOE_10, 0);    // set camera power enable
    //msleep(20);

    printk("gc0339_init OK!!!!\n");
    return;
}

static void gc0339_v4l2_init(void)
{
    gc0339_have_inited=1;
    gc0339_init();
    return;
}
static void gc0339_v4l2_uninit(void)
{
    gc0339_have_inited=0;
    printk( "amlogic camera driver: gc0339_v4l2_uninit. \n");
    gpio_out(PAD_GPIOE_10, 1);    // set camera power disable
    msleep(5);
    aml_set_reg32_bits(P_HHI_GEN_CLK_CNTL, 0, 8, 1); //close clock
    return;    
}
static void gc0339_v4l2_early_suspend(void)
{
    return;
}

static void gc0339_v4l2_late_resume(void)
{
    pinmux_set(&gc0339_pinmux_set);

    aml_set_reg32_bits(P_HHI_GEN_CLK_CNTL, 1, 8, 5); //select XTAL as camera clock
    msleep(20);

    gpio_out(PAD_GPIOZ_0, 1);    // reset IO
    msleep(20);

    // set camera power enable
    //gpio_out(PAD_GPIOE_10, 0);   // set camera power enable
    //msleep(20);
    return;
}

static aml_plat_cam_data_t mipi_gc0339_data = {
    .name = "mipi-gc0339",
    .video_nr = -1,//1,
    .device_init = gc0339_v4l2_init,
    .device_uninit = gc0339_v4l2_uninit,
    .early_suspend = gc0339_v4l2_early_suspend,
    .late_resume = gc0339_v4l2_late_resume,
};
#endif //CONFIG_VIDEO_AMLOGIC_CAPTURE_GC0339

#if defined(CONFIG_VIDEO_AMLOGIC_CAPTURE_HI2056)
static int hi2056_have_inited = 0;
static pinmux_item_t hi2056_pins[] = {
    {
        .reg = PINMUX_REG(9),
        .setmask = 1 << 12
    },
    PINMUX_END_ITEM
};

static pinmux_set_t hi2056_pinmux_set = {
    .chip_select = NULL,
    .pinmux = &hi2056_pins[0]
};

static void hi2056_init(void)
{
    pinmux_set(&hi2056_pinmux_set);

    msleep(20);

    // set camera power enable
    gpio_out(PAD_GPIOC_6, 1);    // set camera power enable
    msleep(1);

    gpio_out(PAD_GPIOZ_0, 0);    // reset IO
    msleep(1);

    gpio_out(PAD_GPIOZ_0, 1);    // reset IO
    msleep(1);

    // set camera power enable
    gpio_out(PAD_GPIOC_6, 0);     // set camera power enable
    msleep(1);

    aml_set_reg32_bits(P_HHI_GEN_CLK_CNTL, 1, 8, 5); //select XTAL as camera clock
    msleep(10);

    printk("hi2056_init OK!!!!\n");
    return;
}

static void hi2056_v4l2_init(void)
{
    hi2056_have_inited=1;
    hi2056_init();
    return;
}
static void hi2056_v4l2_uninit(void)
{
    hi2056_have_inited=0;
    printk( "amlogic camera driver: hi2056_v4l2_uninit. \n");
    gpio_out(PAD_GPIOC_6, 1);    // set camera power disable
    msleep(5);
    aml_set_reg32_bits(P_HHI_GEN_CLK_CNTL, 0, 8, 1); //close clock
    return;    
}
static void hi2056_v4l2_early_suspend(void)
{
    return;
}

static void hi2056_v4l2_late_resume(void)
{
    pinmux_set(&hi2056_pinmux_set);

    msleep(20);

    gpio_out(PAD_GPIOZ_0, 1);    // reset IO
    msleep(1);

    // set camera power enable
    gpio_out(PAD_GPIOC_6, 0);   // set camera power enable
    
    msleep(1);

    aml_set_reg32_bits(P_HHI_GEN_CLK_CNTL, 1, 8, 5); //select XTAL as camera clock

    msleep(10);
    return;
}

static aml_plat_cam_data_t mipi_hi2056_data = {
    .name = "mipi-hi2056",
    .video_nr = -1,//1,
    .device_init = hi2056_v4l2_init,
    .device_uninit = hi2056_v4l2_uninit,
    .early_suspend = hi2056_v4l2_early_suspend,
    .late_resume = hi2056_v4l2_late_resume,
};
#endif //CONFIG_VIDEO_AMLOGIC_CAPTURE_HI2056


/***********************************************************
*Remote Section
************************************************************/
#ifdef CONFIG_AM_REMOTE
#include <plat/remote.h>

static pinmux_item_t aml_remote_pins[] = {
	{
		.reg = PINMUX_REG(AO),
		.clrmask = 0,
		.setmask = 1 << 0,
	},
	PINMUX_END_ITEM
};

static struct aml_remote_platdata aml_remote_pdata __initdata = {
	.pinmux_items  = aml_remote_pins,
	.ao_baseaddr = P_AO_IR_DEC_LDR_ACTIVE, 
};

static void __init setup_remote_device(void)
{
	meson_remote_set_platdata(&aml_remote_pdata);
}
#endif
/***********************************************************************
 * I2C Section
 **********************************************************************/
#if defined(CONFIG_I2C_AML) || defined(CONFIG_I2C_HW_AML)
static bool pinmux_dummy_share(bool select)
{
    return select;
}

static pinmux_item_t aml_i2c_a_pinmux_item[] = {
    {
        .reg = 5,
        //.clrmask = (3<<24)|(3<<30),
        .setmask = 3<<26
    },
    PINMUX_END_ITEM
};

static struct aml_i2c_platform aml_i2c_plat_a = {
    .wait_count             = 50000,
    .wait_ack_interval   = 5,
    .wait_read_interval  = 5,
    .wait_xfer_interval   = 5,
    .master_no          = AML_I2C_MASTER_A,
    .use_pio            = 0,
    .master_i2c_speed   = AML_I2C_SPPED_300K,

    .master_pinmux      = {
        .chip_select    = pinmux_dummy_share,
        .pinmux         = &aml_i2c_a_pinmux_item[0]
    }
};

static pinmux_item_t aml_i2c_b_pinmux_item[]={
    {
        .reg = 5,
        //.clrmask = (3<<28)|(3<<26),
        .setmask = 3<<30
    },
    PINMUX_END_ITEM
};

static struct aml_i2c_platform aml_i2c_plat_b = {
    .wait_count         = 50000,
    .wait_ack_interval = 5,
    .wait_read_interval = 5,
    .wait_xfer_interval = 5,
    .master_no          = AML_I2C_MASTER_B,
    .use_pio            = 0,
    .master_i2c_speed   = AML_I2C_SPPED_300K,

    .master_pinmux      = {
        .chip_select    = pinmux_dummy_share,
        .pinmux         = &aml_i2c_b_pinmux_item[0]
    }
};

static pinmux_item_t aml_i2c_ao_pinmux_item[] = {
    {
        .reg = AO,
        .clrmask  = 3<<1,
        .setmask = 3<<5
    },
    PINMUX_END_ITEM
};

static struct aml_i2c_platform aml_i2c_plat_ao = {
    .wait_count         = 50000,
    .wait_ack_interval  = 5,
    .wait_read_interval = 5,
    .wait_xfer_interval = 5,
    .master_no          = AML_I2C_MASTER_AO,
    .use_pio            = 0,
    .master_i2c_speed   = AML_I2C_SPPED_100K,

    .master_pinmux      = {
        .pinmux         = &aml_i2c_ao_pinmux_item[0]
    }
};

static struct resource aml_i2c_resource_a[] = {
    [0] = {
        .start = MESON_I2C_MASTER_A_START,
        .end   = MESON_I2C_MASTER_A_END,
        .flags = IORESOURCE_MEM,
    }
};

static struct resource aml_i2c_resource_b[] = {
    [0] = {
        .start = MESON_I2C_MASTER_B_START,
        .end   = MESON_I2C_MASTER_B_END,
        .flags = IORESOURCE_MEM,
    }
};

static struct resource aml_i2c_resource_ao[] = {
    [0]= {
        .start =    MESON_I2C_MASTER_AO_START,
        .end   =    MESON_I2C_MASTER_AO_END,
        .flags =    IORESOURCE_MEM,
    }
};

static struct platform_device aml_i2c_device_a = {
    .name         = "aml-i2c",
    .id       = 0,
    .num_resources    = ARRAY_SIZE(aml_i2c_resource_a),
    .resource     = aml_i2c_resource_a,
    .dev = {
        .platform_data = &aml_i2c_plat_a,
    },
};

static struct platform_device aml_i2c_device_b = {
    .name         = "aml-i2c",
    .id       = 1,
    .num_resources    = ARRAY_SIZE(aml_i2c_resource_b),
    .resource     = aml_i2c_resource_b,
    .dev = {
        .platform_data = &aml_i2c_plat_b,
    },
};

static struct platform_device aml_i2c_device_ao = {
    .name         = "aml-i2c",
    .id       = 2,
    .num_resources    = ARRAY_SIZE(aml_i2c_resource_ao),
    .resource     = aml_i2c_resource_ao,
    .dev = {
        .platform_data = &aml_i2c_plat_ao,
    },
};

static struct i2c_board_info __initdata aml_i2c_bus_info_a[] = {
#ifdef CONFIG_VIDEO_AMLOGIC_CAPTURE_GC0339
    {
        /*gc0339 i2c address is 0x42/0x43*/
        I2C_BOARD_INFO("gc0339_i2c",  0x42 >> 1),
        .platform_data = (void *)&mipi_gc0339_data,
    },
#endif
#ifdef CONFIG_VIDEO_AMLOGIC_CAPTURE_HI2056
    {
        /*hi2056 i2c address is 0x48/0x49*/
        I2C_BOARD_INFO("hi2056_i2c",  0x48 >> 1),
        .platform_data = (void *)&mipi_hi2056_data,
    },
#endif
};

static struct i2c_board_info __initdata aml_i2c_bus_info_ao[] = {

};


static struct i2c_board_info __initdata aml_i2c_bus_info_b[] = {

};

static int __init aml_i2c_init(void)
{
    i2c_register_board_info(0, aml_i2c_bus_info_a,
        ARRAY_SIZE(aml_i2c_bus_info_a));
    i2c_register_board_info(1, aml_i2c_bus_info_b,
        ARRAY_SIZE(aml_i2c_bus_info_b));
    i2c_register_board_info(2, aml_i2c_bus_info_ao,
        ARRAY_SIZE(aml_i2c_bus_info_ao));
    return 0;
}
#endif

#if defined(CONFIG_I2C_SW_AML)
#include "gpio_data.c"
static struct aml_sw_i2c_platform aml_sw_i2c_plat_1 = {
    .sw_pins = {
        .scl_reg_out        = P_GPIO_OUT_4,
        .scl_reg_in     = P_GPIO_IN_4,
        .scl_bit            = 28,
        .scl_oe         = P_GPIO_OEN_4,
        .sda_reg_out        = P_GPIO_OUT_4,
        .sda_reg_in     = P_GPIO_IN_4,
        .sda_bit            = 27,
        .sda_oe         = P_GPIO_OEN_4,
    },  
    .udelay         = 2,
    .timeout            = 100,
};

static struct platform_device aml_sw_i2c_device_1 = {
    .name         = "aml-sw-i2c",
    .id       = -1,
    .dev = {
        .platform_data = &aml_sw_i2c_plat_1,
    },
};
#endif


/***********************************************************************
 * UART Section
 **********************************************************************/
static pinmux_item_t uart_pins[] = {
    {
        .reg = PINMUX_REG(AO),
        .setmask = 3 << 11
    },
    PINMUX_END_ITEM
};

static pinmux_set_t aml_uart_ao = {
    .chip_select = NULL,
    .pinmux = &uart_pins[0]
};

static struct aml_uart_platform  __initdata aml_uart_plat = {
    .uart_line[0]   = UART_AO,
    .uart_line[1]   = UART_A,
    .uart_line[2]   = UART_B,
    .uart_line[3]   = UART_C,
    .uart_line[4]   = UART_D,

    .pinmux_uart[0] = (void*)&aml_uart_ao,
    .pinmux_uart[1] = NULL,
    .pinmux_uart[2] = NULL,
    .pinmux_uart[3] = NULL,
    .pinmux_uart[4] = NULL
};

static struct platform_device aml_uart_device = {
    .name       = "mesonuart",
    .id     = -1,
    .num_resources  = 0,
    .resource   = NULL,
    .dev = {
        .platform_data = &aml_uart_plat,
    },
};

#ifdef CONFIG_AM_ETHERNET
//#define ETH_MODE_RGMII
//#define ETH_MODE_RMII_INTERNAL
#define ETH_MODE_RMII_EXTERNAL
static void aml_eth_reset(void)
{
	unsigned int val = 0;

	printk(KERN_INFO "****** aml_eth_reset() ******\n");
#ifdef ETH_MODE_RGMII
	val = 0x211;
#else
	val = 0x241;
#endif
	/* setup ethernet mode */
	aml_set_reg32_mask(P_PREG_ETHERNET_ADDR0, val);

	/* setup ethernet interrupt */
	aml_set_reg32_mask(P_SYS_CPU_0_IRQ_IN0_INTR_MASK, 1 << 8);
	aml_set_reg32_mask(P_SYS_CPU_0_IRQ_IN1_INTR_STAT, 1 << 8);

	/* hardware reset ethernet phy */
	gpio_out(PAD_GPIOY_15, 0);
	msleep(20);
	gpio_out(PAD_GPIOY_15, 1);
}

static void aml_eth_clock_enable(void)
{
	unsigned int val = 0;

	printk(KERN_INFO "****** aml_eth_clock_enable() ******\n");
#ifdef ETH_MODE_RGMII
	val = 0x309;
#elif defined(ETH_MODE_RMII_EXTERNAL)
	val = 0x130;
#else
	val = 0x702;
#endif
	/* setup ethernet clk */
	aml_set_reg32_mask(P_HHI_ETH_CLK_CNTL, val);
}

static void aml_eth_clock_disable(void)
{
	printk(KERN_INFO "****** aml_eth_clock_disable() ******\n");
	/* disable ethernet clk */
	aml_clr_reg32_mask(P_HHI_ETH_CLK_CNTL, 1 << 8);
}

static pinmux_item_t aml_eth_pins[] = {
	/* RMII pin-mux */
	{
		.reg = PINMUX_REG(6),
		.clrmask = 0,
#ifdef ETH_MODE_RMII_EXTERNAL
		.setmask = 0x8007ffe0,
#else
		.setmask = 0x4007ffe0,
#endif
	},
	PINMUX_END_ITEM
};

static pinmux_set_t aml_eth_pinmux = {
	.chip_select = NULL,
	.pinmux = aml_eth_pins,
};

static void aml_eth_pinmux_setup(void)
{
	printk(KERN_INFO "****** aml_eth_pinmux_setup() ******\n");

	pinmux_set(&aml_eth_pinmux);
}

static void aml_eth_pinmux_cleanup(void)
{
	printk(KERN_INFO "****** aml_eth_pinmux_cleanup() ******\n");

	pinmux_clr(&aml_eth_pinmux);
}

static void aml_eth_init(void)
{
	aml_eth_pinmux_setup();
	aml_eth_clock_enable();
	aml_eth_reset();
}

static struct aml_eth_platdata aml_eth_pdata __initdata = {
	.pinmux_items   = aml_eth_pins,
	.pinmux_setup   = aml_eth_pinmux_setup,
	.pinmux_cleanup = aml_eth_pinmux_cleanup,
	.clock_enable   = aml_eth_clock_enable,
	.clock_disable  = aml_eth_clock_disable,
	.reset          = aml_eth_reset,
};

static void __init setup_eth_device(void)
{
	meson_eth_set_platdata(&aml_eth_pdata);
	aml_eth_init();
}
#endif

/***********************************************************************
 * Nand Section
 **********************************************************************/

#ifdef CONFIG_AM_NAND
static struct mtd_partition normal_partition_info[] = {
    {
        .name = "logo",
        .offset = 32*SZ_1M+40*SZ_1M,
        .size = 8*SZ_1M,
    },
    {
        .name = "aml_logo",
        .offset = 48*SZ_1M+40*SZ_1M,
        .size = 8*SZ_1M,
    },
    {
        .name = "recovery",
        .offset = 64*SZ_1M+40*SZ_1M,
        .size = 8*SZ_1M,
    },
    {
        .name = "boot",
        .offset = 96*SZ_1M+40*SZ_1M,
        .size = 8*SZ_1M,
    },
    {
        .name = "system",
        .offset = 128*SZ_1M+40*SZ_1M,
        .size = 512*SZ_1M,
    },
    {
        .name = "cache",
        .offset = 640*SZ_1M+40*SZ_1M,
        .size = 128*SZ_1M,
    },
    {
        .name = "userdata",
        .offset = 768*SZ_1M+40*SZ_1M,
        .size = 512*SZ_1M,
    },
    {
        .name = "NFTL_Part",
        .offset = MTDPART_OFS_APPEND,
        .size = MTDPART_SIZ_FULL,
    },
};


static struct aml_nand_platform aml_nand_mid_platform[] = {
#ifndef CONFIG_AMLOGIC_SPI_NOR
    {
        .name = NAND_BOOT_NAME,
        .chip_enable_pad = AML_NAND_CE0,
        .ready_busy_pad = AML_NAND_CE0,
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_BCH30_1K_MODE),
            },
        },
        .T_REA = 20,
        .T_RHOH = 15,
    },
#endif
    {
        .name = NAND_NORMAL_NAME,
        .chip_enable_pad = (AML_NAND_CE0/* | (AML_NAND_CE1 << 4) | (AML_NAND_CE2 << 8) | (AML_NAND_CE3 << 12)*/),
        .ready_busy_pad = (AML_NAND_CE0 /*| (AML_NAND_CE0 << 4) | (AML_NAND_CE1 << 8) | (AML_NAND_CE1 << 12)*/),
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
                .nr_partitions = ARRAY_SIZE(normal_partition_info),
                .partitions = normal_partition_info,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_BCH30_1K_MODE | NAND_TWO_PLANE_MODE),
            },
        },
        .T_REA = 20,
        .T_RHOH = 15,
    }
};

static struct aml_nand_device aml_nand_mid_device = {
    .aml_nand_platform = aml_nand_mid_platform,
    .dev_num = ARRAY_SIZE(aml_nand_mid_platform),
};

static struct resource aml_nand_resources[] = {
    {
        .start = 0xc1108600,
        .end = 0xc1108624,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device aml_nand_device = {
    .name = "aml_nand",
    .id = 0,
    .num_resources = ARRAY_SIZE(aml_nand_resources),
    .resource = aml_nand_resources,
    .dev = {
        .platform_data = &aml_nand_mid_device,
    },
};
#endif

/***********************************************************************
 * Card Reader Section
 **********************************************************************/
/* WIFI ON Flag */
static int WIFI_ON;
/* BT ON Flag */
static int BT_ON;
/* WL_BT_REG_ON control function */
static void reg_on_control(int is_on)
{
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_1,(1<<11)); //WIFI_RST GPIO mode
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_0,(1<<18)); //WIFI_EN GPIO mode
    CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO2_EN_N, (1<<8));//GPIOC_8 ==WIFI_EN

    if(is_on){
        SET_CBUS_REG_MASK(PREG_PAD_GPIO2_O, (1<<8));
    }
    else{
        /* only pull donw reg_on pin when wifi and bt off */
        if((!WIFI_ON) && (!BT_ON)){
            CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO2_O, (1<<8));
            printk("WIFI BT Power down\n");
        }
    }
}
#ifdef CONFIG_CARDREADER
static struct resource meson_card_resource[] = {
    [0] = {
        .start = 0x1200230,   //physical address
        .end   = 0x120024c,
        .flags = 0x200,
    }
};
#if 0
static void extern_wifi_power(int is_power)
{
    WIFI_ON = is_power;
    reg_on_control(is_power);
    /*
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_1,(1<<11));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_0,(1<<18));
    CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO2_EN_N, (1<<8));
    if(is_power)
        SET_CBUS_REG_MASK(PREG_PAD_GPIO2_O, (1<<8));
    else
        CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO2_O, (1<<8));
        */
}

EXPORT_SYMBOL(extern_wifi_power);
#endif
static void sdio_extern_init(void)
{
#if defined(CONFIG_BCM4329_HW_OOB) || defined(CONFIG_BCM4329_OOB_INTR_ONLY)/* Jone add */
    gpio_set_status(PAD_GPIOX_11,gpio_status_in);
    gpio_irq_set(PAD_GPIOX_11,GPIO_IRQ(4,GPIO_IRQ_RISING));

    extern_wifi_power(1);
#endif
}

static struct aml_card_info meson_card_info[] = {
    [0] = {
        .name           = "sd_card",
        .work_mode      = CARD_HW_MODE,
        .io_pad_type        = SDHC_CARD_0_5,
        .card_ins_en_reg    = CARD_GPIO_ENABLE,
        .card_ins_en_mask   = PREG_IO_29_MASK,
        .card_ins_input_reg = CARD_GPIO_INPUT,
        .card_ins_input_mask    = PREG_IO_29_MASK,
        .card_power_en_reg  = CARD_GPIO_ENABLE,
        .card_power_en_mask = PREG_IO_31_MASK,
        .card_power_output_reg  = CARD_GPIO_OUTPUT,
        .card_power_output_mask = PREG_IO_31_MASK,
        .card_power_en_lev  = 0,
        .card_wp_en_reg     = 0,
        .card_wp_en_mask    = 0,
        .card_wp_input_reg  = 0,
        .card_wp_input_mask = 0,
        .card_extern_init   = 0,
    },
    [1] = {
        .name           = "sdio_card",
        .work_mode      = CARD_HW_MODE,
        .io_pad_type        = SDHC_GPIOX_0_9,
        .card_ins_en_reg    = 0,
        .card_ins_en_mask   = 0,
        .card_ins_input_reg = 0,
        .card_ins_input_mask    = 0,
        .card_power_en_reg  = EGPIO_GPIOC_ENABLE,
        .card_power_en_mask = PREG_IO_7_MASK,
        .card_power_output_reg  = EGPIO_GPIOC_OUTPUT,
        .card_power_output_mask = PREG_IO_7_MASK,
        .card_power_en_lev  = 1,
        .card_wp_en_reg     = 0,
        .card_wp_en_mask    = 0,
        .card_wp_input_reg  = 0,
        .card_wp_input_mask = 0,
        .card_extern_init   = sdio_extern_init,
    },
};

static struct aml_card_platform meson_card_platform = {
    .card_num   = ARRAY_SIZE(meson_card_info),
    .card_info  = meson_card_info,
};

static struct platform_device meson_card_device = {
    .name       = "AMLOGIC_CARD",
    .id     = -1,
    .num_resources  = ARRAY_SIZE(meson_card_resource),
    .resource   = meson_card_resource,
    .dev = {
        .platform_data = &meson_card_platform,
    },
};

/**
 *  Some Meson6 socket board has card detect issue.
 *  Force card detect success for socket board.
 */
static int meson_mmc_detect(void)
{
    return 0;
}
#endif // CONFIG_CARDREADER

/***********************************************************************
 * MMC SD Card  Section
 **********************************************************************/
#ifdef CONFIG_MMC_AML
struct platform_device;
struct mmc_host;
struct mmc_card;
struct mmc_ios;

//return 1: no inserted  0: inserted
static int aml_sdio_detect(struct aml_sd_host * host)
{
    aml_set_reg32_mask(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6 input mode
    if((aml_read_reg32(P_PREG_PAD_GPIO5_I)&(1<<29)) == 0)
        return 0;
    else{ //for socket card box
                return 0;
    }
    return 1; //no insert.
}

static void  cpu_sdio_pwr_prepare(unsigned port)
{
    switch(port)
    {
        case MESON_SDIO_PORT_A:
            aml_clr_reg32_mask(P_PREG_PAD_GPIO4_EN_N,0x30f);
            aml_clr_reg32_mask(P_PREG_PAD_GPIO4_O   ,0x30f);
            aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_8,0x3f);
            break;
        case MESON_SDIO_PORT_B:
            aml_clr_reg32_mask(P_PREG_PAD_GPIO5_EN_N,0x3f<<23);
            aml_clr_reg32_mask(P_PREG_PAD_GPIO5_O   ,0x3f<<23);
            aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_2,0x3f<<10);
            break;
        case MESON_SDIO_PORT_C:
            aml_clr_reg32_mask(P_PREG_PAD_GPIO3_EN_N,0xc0f);
            aml_clr_reg32_mask(P_PREG_PAD_GPIO3_O   ,0xc0f);
            aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_6,(0x3f<<24));
            break;
        case MESON_SDIO_PORT_XC_A:
            break;
        case MESON_SDIO_PORT_XC_B:
            break;
        case MESON_SDIO_PORT_XC_C:
            break;
    }
}

static int cpu_sdio_init(unsigned port)
{
    switch(port)
    {
        case MESON_SDIO_PORT_A:
                aml_set_reg32_mask(P_PERIPHS_PIN_MUX_8,0x3d<<0);
                aml_set_reg32_mask(P_PERIPHS_PIN_MUX_8,0x1<<1);
                break;
        case MESON_SDIO_PORT_B:
                aml_set_reg32_mask(P_PERIPHS_PIN_MUX_2,0x3d<<10);
                aml_set_reg32_mask(P_PERIPHS_PIN_MUX_2,0x1<<11);
                break;
        case MESON_SDIO_PORT_C://SDIOC GPIOB_2~GPIOB_7
            aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_2,(0x1f<<22));
            aml_set_reg32_mask(P_PERIPHS_PIN_MUX_6,(0x1f<<25));
            aml_set_reg32_mask(P_PERIPHS_PIN_MUX_6,(0x1<<24));
            break;
        case MESON_SDIO_PORT_XC_A:
            #if 0
            //sdxc controller can't work
            aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_8,(0x3f<<0));
            aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_3,(0x0f<<27));
            aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_7,((0x3f<<18)|(0x7<<25)));
            //aml_set_reg32_mask(P_PERIPHS_PIN_MUX_5,(0x1f<<10));//data 8 bit
            aml_set_reg32_mask(P_PERIPHS_PIN_MUX_5,(0x1b<<10));//data 4 bit
            #endif
            break;
        case MESON_SDIO_PORT_XC_B:
            //sdxc controller can't work
            //aml_set_reg32_mask(P_PERIPHS_PIN_MUX_2,(0xf<<4));
            break;
        case MESON_SDIO_PORT_XC_C:
            #if 0
            //sdxc controller can't work
            aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_6,(0x3f<<24));
            aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_2,((0x13<<22)|(0x3<<16)));
            aml_set_reg32_mask(P_PERIPHS_PIN_MUX_4,(0x1f<<26));
            printk(KERN_INFO "inand sdio xc-c init\n");
            #endif
            break;
        default:
            return -1;
    }
    return 0;
}

static void aml_sdio_pwr_prepare(unsigned port)
{
    /// @todo NOT FINISH
    ///do nothing here
    cpu_sdio_pwr_prepare(port);
}

static void aml_sdio_pwr_on(unsigned port)
{
    if((aml_read_reg32(P_PREG_PAD_GPIO5_O) & (1<<31)) != 0){
    aml_clr_reg32_mask(P_PREG_PAD_GPIO5_O,(1<<31));
    aml_clr_reg32_mask(P_PREG_PAD_GPIO5_EN_N,(1<<31));
    udelay(1000);
    }
    /// @todo NOT FINISH
}
static void aml_sdio_pwr_off(unsigned port)
{
    if((aml_read_reg32(P_PREG_PAD_GPIO5_O) & (1<<31)) == 0){
        aml_set_reg32_mask(P_PREG_PAD_GPIO5_O,(1<<31));
        aml_clr_reg32_mask(P_PREG_PAD_GPIO5_EN_N,(1<<31));//GPIOD13
        udelay(1000);
    }
    /// @todo NOT FINISH
}
static int aml_sdio_init(struct aml_sd_host * host)
{ //set pinumx ..
    aml_set_reg32_mask(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6
    cpu_sdio_init(host->sdio_port);
        host->clk = clk_get_sys("clk81",NULL);
        if(!IS_ERR(host->clk))
                host->clk_rate = clk_get_rate(host->clk);
        else
                host->clk_rate = 0;
    return 0;
}

static struct resource aml_mmc_resource[] = {
   [0] = {
        .start = 0x1200230,   //physical address
        .end   = 0x1200248,
        .flags = IORESOURCE_MEM, //0x200
    },
};

static u64 aml_mmc_device_dmamask = 0xffffffffUL;
static struct aml_mmc_platform_data aml_mmc_def_platdata = {
    .no_wprotect = 1,
    .no_detect = 0,
    .wprotect_invert = 0,
    .detect_invert = 0,
    .use_dma = 0,
    .gpio_detect=1,
    .gpio_wprotect=0,
    .ocr_avail = MMC_VDD_33_34,

    .sdio_port = MESON_SDIO_PORT_B,
    .max_width  = 4,
    .host_caps  = (MMC_CAP_4_BIT_DATA |
                            MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED | MMC_CAP_NEEDS_POLL),

    .f_min = 200000,
    .f_max = 40000000,
    .clock = 300000,

    .sdio_init = aml_sdio_init,
    .sdio_detect = aml_sdio_detect,
    .sdio_pwr_prepare = aml_sdio_pwr_prepare,
    .sdio_pwr_on = aml_sdio_pwr_on,
    .sdio_pwr_off = aml_sdio_pwr_off,
};

static struct platform_device aml_mmc_device = {
    .name       = "aml_sd_mmc",
    .id     = 0,
    .num_resources  = ARRAY_SIZE(aml_mmc_resource),
    .resource   = aml_mmc_resource,
    .dev        = {
        .dma_mask       =       &aml_mmc_device_dmamask,
        .coherent_dma_mask  = 0xffffffffUL,
        .platform_data      = &aml_mmc_def_platdata,
    },
};
#endif //CONFIG_MMC_AML
/***********************************************************************
 * IO Mapping
 **********************************************************************/
/*
#define IO_CBUS_BASE        0xf1100000  ///2M
#define IO_AXI_BUS_BASE     0xf1300000  ///1M
#define IO_PL310_BASE       0xf2200000  ///4k
#define IO_PERIPH_BASE      0xf2300000  ///4k
#define IO_APB_BUS_BASE     0xf3000000  ///8k
#define IO_DOS_BUS_BASE     0xf3010000  ///64k
#define IO_AOBUS_BASE       0xf3100000  ///1M
#define IO_USB_A_BASE       0xf3240000  ///256k
#define IO_USB_B_BASE       0xf32C0000  ///256k
#define IO_WIFI_BASE        0xf3300000  ///1M
#define IO_SATA_BASE        0xf3400000  ///64k
#define IO_ETH_BASE         0xf3410000  ///64k

#define IO_SPIMEM_BASE      0xf4000000  ///64M
#define IO_A9_APB_BASE      0xf8000000  ///256k
#define IO_DEMOD_APB_BASE   0xf8044000  ///112k
#define IO_MALI_APB_BASE    0xf8060000  ///128k
#define IO_APB2_BUS_BASE    0xf8000000
#define IO_AHB_BASE         0xf9000000  ///128k
#define IO_BOOTROM_BASE     0xf9040000  ///64k
#define IO_SECBUS_BASE      0xfa000000
#define IO_EFUSE_BASE       0xfa000000  ///4k
*/
static __initdata struct map_desc meson_io_desc[] = {
    {
        .virtual    = IO_CBUS_BASE,
        .pfn        = __phys_to_pfn(IO_CBUS_PHY_BASE),
        .length     = SZ_2M,
        .type       = MT_DEVICE,
    } , {
        .virtual    = IO_AXI_BUS_BASE,
        .pfn        = __phys_to_pfn(IO_AXI_BUS_PHY_BASE),
        .length     = SZ_1M,
        .type       = MT_DEVICE,
    } , {
        .virtual    = IO_PL310_BASE,
        .pfn        = __phys_to_pfn(IO_PL310_PHY_BASE),
        .length     = SZ_4K,
        .type       = MT_DEVICE,
    } , {
        .virtual    = IO_PERIPH_BASE,
        .pfn        = __phys_to_pfn(IO_PERIPH_PHY_BASE),
        .length     = SZ_1M,
        .type       = MT_DEVICE,
    } , {
           .virtual    = IO_APB_BUS_BASE,
           .pfn        = __phys_to_pfn(IO_APB_BUS_PHY_BASE),
           .length     = SZ_1M,
           .type       = MT_DEVICE,
       } , /*{

           .virtual    = IO_DOS_BUS_BASE,
           .pfn        = __phys_to_pfn(IO_DOS_BUS_PHY_BASE),
           .length     = SZ_64K,
           .type       = MT_DEVICE,
       } , */{
           .virtual    = IO_AOBUS_BASE,
        .pfn        = __phys_to_pfn(IO_AOBUS_PHY_BASE),
        .length     = SZ_1M,
        .type       = MT_DEVICE,
    } , {
        .virtual    = IO_AHB_BUS_BASE,
        .pfn        = __phys_to_pfn(IO_AHB_BUS_PHY_BASE),
        .length     = SZ_8M,
        .type       = MT_DEVICE,
    } , {
        .virtual    = IO_SPIMEM_BASE,
        .pfn        = __phys_to_pfn(IO_SPIMEM_PHY_BASE),
        .length     = SZ_64M,
        .type       = MT_ROM,
    } , {
        .virtual    = IO_APB2_BUS_BASE,
        .pfn        = __phys_to_pfn(IO_APB2_BUS_PHY_BASE),
        .length     = SZ_512K,
        .type       = MT_DEVICE,
    } , {
        .virtual    = IO_AHB_BASE,
        .pfn        = __phys_to_pfn(IO_AHB_PHY_BASE),
        .length     = SZ_128K,
        .type       = MT_DEVICE,
    } , {
        .virtual    = IO_BOOTROM_BASE,
        .pfn        = __phys_to_pfn(IO_BOOTROM_PHY_BASE),
        .length     = SZ_64K,
        .type       = MT_DEVICE,
    } , {
        .virtual    = IO_SECBUS_BASE,
        .pfn        = __phys_to_pfn(IO_SECBUS_PHY_BASE),
        .length     = SZ_4K,
        .type       = MT_DEVICE,
    }, {
        .virtual    = IO_SECURE_BASE,
        .pfn        = __phys_to_pfn(IO_SECURE_PHY_BASE),
        .length     = SZ_16K,
        .type       = MT_DEVICE,
    }, {
        .virtual    = PAGE_ALIGN(__phys_to_virt(RESERVED_MEM_START)),
        .pfn        = __phys_to_pfn(RESERVED_MEM_START),
        .length     = RESERVED_MEM_END - RESERVED_MEM_START + 1,
        .type       = MT_MEMORY_NONCACHED,
    },
#ifdef CONFIG_MESON_SUSPEND
        {
        .virtual    = PAGE_ALIGN(__phys_to_virt(0x9ff00000)),
        .pfn        = __phys_to_pfn(0x9ff00000),
        .length     = SZ_1M,
        .type       = MT_MEMORY_NONCACHED,
        },
#endif

};

static void __init meson_map_io(void)
{
    iotable_init(meson_io_desc, ARRAY_SIZE(meson_io_desc));
}

static void __init meson_fixup(struct machine_desc *mach, struct tag *tag, char **cmdline, struct meminfo *m)
{
    struct membank *pbank;
    mach->video_start    = RESERVED_MEM_START;
    mach->video_end      = RESERVED_MEM_END;

    m->nr_banks = 0;
    pbank = &m->bank[m->nr_banks];
    pbank->start = PAGE_ALIGN(PHYS_MEM_START);
    pbank->size  = SZ_64M & PAGE_MASK;
    m->nr_banks++;
    pbank = &m->bank[m->nr_banks];
    pbank->start = PAGE_ALIGN(RESERVED_MEM_END + 1);
#ifdef CONFIG_MESON_SUSPEND
    pbank->size  = (PHYS_MEM_END-RESERVED_MEM_END-SZ_1M) & PAGE_MASK;
#else
    pbank->size  = (PHYS_MEM_END-RESERVED_MEM_END) & PAGE_MASK;
#endif
    m->nr_banks++;
}

/***********************************************************************
 *USB Setting section
 **********************************************************************/
static void set_usb_a_vbus_power(char is_power_on)
{
//M6 SKT: GPIOD_9,  OEN: 0x2012, OUT:0x2013 , Bit25

#define USB_A_POW_GPIO  PREG_EGPIO
#define USB_A_POW_GPIO_BIT  25
#define USB_A_POW_GPIO_BIT_ON   1
#define USB_A_POW_GPIO_BIT_OFF  0
    if(is_power_on){
        printk( "set usb port power on (board gpio %d)!\n",USB_A_POW_GPIO_BIT);

        aml_set_reg32_bits(CBUS_REG_ADDR(0x2012),0,USB_A_POW_GPIO_BIT,1);//mode
        aml_set_reg32_bits(CBUS_REG_ADDR(0x2013),1,USB_A_POW_GPIO_BIT,1);//out
        //set_gpio_mode(USB_A_POW_GPIO,USB_A_POW_GPIO_BIT,GPIO_OUTPUT_MODE);
        //set_gpio_val(USB_A_POW_GPIO,USB_A_POW_GPIO_BIT,USB_A_POW_GPIO_BIT_ON);
    }
    else    {
        printk("set usb port power off (board gpio %d)!\n",USB_A_POW_GPIO_BIT);
        aml_set_reg32_bits(CBUS_REG_ADDR(0x2012),0,USB_A_POW_GPIO_BIT,1);//mode
        aml_set_reg32_bits(CBUS_REG_ADDR(0x2013),0,USB_A_POW_GPIO_BIT,1);//out
    }
}

static  int __init setup_usb_devices(void)
{
    struct lm_device * usb_ld_a, *usb_ld_b;
    usb_ld_a = alloc_usb_lm_device(USB_PORT_IDX_A);
    usb_ld_b = alloc_usb_lm_device (USB_PORT_IDX_B);
    usb_ld_a->param.usb.set_vbus_power = set_usb_a_vbus_power;
#ifdef CONFIG_SMP    
	usb_ld_a->param.usb.port_type = USB_PORT_TYPE_HOST;
#endif    
    lm_device_register(usb_ld_a);
    lm_device_register(usb_ld_b);
    return 0;
}
/***********************************************************************/
#ifdef CONFIG_EFUSE
static bool efuse_data_verify(unsigned char *usid)
{  int len;

    len = strlen(usid);    
    if((len > 0)&&(len<58) )
        return true;
    else
        return false;
}

static struct efuse_platform_data aml_efuse_plat = {
    .pos = 454,
    .count = 58,
    .data_verify = efuse_data_verify,
};

static struct platform_device aml_efuse_device = {
    .name   = "efuse",
    .id = -1,
    .dev = {
                .platform_data = &aml_efuse_plat,
           },
};

// BSP EFUSE layout setting
static  efuseinfo_item_t aml_efuse_setting[] = {
	// usid layout can be defined by customer
	{
		.title = "usid",   
		.id = EFUSE_USID_ID,
		.offset = 454,      // customer can modify the offset which must >= 454
		.enc_len = 58,     // customer can modify the encode length by self must <=58
		.data_len = 58,	// customer can modify the data length by self must <=58
		.bch_en = 0,		// customer can modify do bch or not
		.bch_reverse = 0,
	},	
	// customer also can add new EFUSE item to expand, but must be correct and bo conflict
};

static int aml_efuse_getinfoex_byID(unsigned param, efuseinfo_item_t *info)
{
	unsigned num = sizeof(aml_efuse_setting)/sizeof(efuseinfo_item_t);
	int i=0;
	int ret = -1;
	for(i=0; i<num; i++){
		if(aml_efuse_setting[i].id == param){
				strcpy(info->title, aml_efuse_setting[i].title);				
				info->offset = aml_efuse_setting[i].offset;
				info->id = aml_efuse_setting[i].id;				
				info->data_len = aml_efuse_setting[i].data_len;			
				info->enc_len = aml_efuse_setting[i].enc_len;
				info->bch_en = aml_efuse_setting[i].bch_en;
				info->bch_reverse = aml_efuse_setting[i].bch_reverse;					
				ret = 0;
				break;
		}
	}
	return ret;
}

static int aml_efuse_getinfoex_byPos(unsigned param, efuseinfo_item_t *info)
{
	unsigned num = sizeof(aml_efuse_setting)/sizeof(efuseinfo_item_t);
	int i=0;
	int ret = -1;
	for(i=0; i<num; i++){
		if(aml_efuse_setting[i].offset == param){
				strcpy(info->title, aml_efuse_setting[i].title);				
				info->offset = aml_efuse_setting[i].offset;
				info->id = aml_efuse_setting[i].id;				
				info->data_len = aml_efuse_setting[i].data_len;			
				info->enc_len = aml_efuse_setting[i].enc_len;
				info->bch_en = aml_efuse_setting[i].bch_en;
				info->bch_reverse = aml_efuse_setting[i].bch_reverse;					
				ret = 0;
				break;
		}
	}
	return ret;	
}

extern pfn efuse_getinfoex;
extern pfn efuse_getinfoex_byPos;
static void setup_aml_efuse(void)
{
    efuse_getinfoex = aml_efuse_getinfoex_byID;
    efuse_getinfoex_byPos = aml_efuse_getinfoex_byPos;
}	
#endif

#if defined(CONFIG_AML_RTC)
static  struct platform_device aml_rtc_device = {
            .name            = "aml_rtc",
            .id               = -1,
    };
#endif
#if defined(CONFIG_SUSPEND)
static void m6ref_set_vccx2(int power_on)
{
    if (power_on) {
        //restore_pinmux();
        printk(KERN_INFO "%s() Power ON\n", __FUNCTION__);
        aml_clr_reg32_mask(P_PREG_PAD_GPIO0_EN_N,(1<<26));
        aml_clr_reg32_mask(P_PREG_PAD_GPIO0_O,(1<<26));
    }
    else {
        printk(KERN_INFO "%s() Power OFF\n", __FUNCTION__);
        aml_clr_reg32_mask(P_PREG_PAD_GPIO0_EN_N,(1<<26));
        aml_set_reg32_mask(P_PREG_PAD_GPIO0_O,(1<<26));
        //save_pinmux();
    }
}

static struct meson_pm_config aml_pm_pdata = {
    .pctl_reg_base = (void *)IO_APB_BUS_BASE,
    .mmc_reg_base = (void *)APB_REG_ADDR(0x1000),
    .hiu_reg_base = (void *)CBUS_REG_ADDR(0x1000),
    .power_key = (1<<8),
    .ddr_clk = 0x00110820,
    .sleepcount = 128,
    .set_vccx2 = m6ref_set_vccx2,
    .core_voltage_adjust = 7,  //5,8
};

static struct platform_device aml_pm_device = {
    .name           = "pm-meson",
    .dev = {
        .platform_data  = &aml_pm_pdata,
    },
    .id             = -1,
};
#endif

#if defined(CONFIG_TVIN_VDIN)
static struct resource vdin_resources[] = {
    [0] = {
        .start =  VDIN_ADDR_START,  //pbufAddr
        .end   = VDIN_ADDR_END,     //pbufAddr + size
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = VDIN_ADDR_START,
        .end   = VDIN_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
    [2] = {
        .start = INT_VDIN_VSYNC,
        .end   = INT_VDIN_VSYNC,
        .flags = IORESOURCE_IRQ,
    },
    [3] = {
        .start = INT_VDIN_VSYNC,
        .end   = INT_VDIN_VSYNC,
        .flags = IORESOURCE_IRQ,
    },
};

static struct platform_device vdin_device = {
    .name       = "vdin",
    .id         = -1,
    .num_resources = ARRAY_SIZE(vdin_resources),
    .resource      = vdin_resources,
};
#endif

#if defined (CONFIG_AMLOGIC_VIDEOIN_MANAGER)
static struct resource vm_resources[] = {    
    [0] = {        
        .start = VM_ADDR_START,        
        .end   = VM_ADDR_END,        
        .flags = IORESOURCE_MEM,    
    },
};
static struct platform_device vm_device ={    
    .name = "vm",    
    .id = 0,    
    .num_resources = ARRAY_SIZE(vm_resources),    
    .resource      = vm_resources,
};
#endif /* AMLOGIC_VIDEOIN_MANAGER */

#ifdef CONFIG_AMLOGIC_MIPI
static struct am_csi2_client_config am_mipi_csi2_clients[] = {
#if defined(CONFIG_VIDEO_AMLOGIC_CAPTURE_GC0339)
    {
        .mode = AM_CSI2_ALL_MEM,
        .lanes = 1,
        .channel = 0x1,
        .pdev = (void *)&mipi_gc0339_data,

    },
#endif
#if defined(CONFIG_VIDEO_AMLOGIC_CAPTURE_HI2056)
    {
        .mode = AM_CSI2_VDIN,//AM_CSI2_ALL_MEM,
        .lanes = 1,
        .channel = 0x1,
        .vdin_num = 0,
        .pdev = (void *)&mipi_hi2056_data,

    },
#endif
};

static struct am_csi2_pdata am_mipi_csi2_info = {
    .clients = am_mipi_csi2_clients,
    .num_clients = ARRAY_SIZE(am_mipi_csi2_clients),
};

static struct resource am_mipi_csi2_resources[] = {
    [0] = {
        .start = MIPI_ADDR_START,
        .end = MIPI_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = INT_CSI2_ADAPTER,
        .flags = IORESOURCE_IRQ,
    },
};

static struct platform_device am_mipi_csi2_device = {
    .name   = "am-mipi-csi2",
    .id     = 0,
    .num_resources	= ARRAY_SIZE(am_mipi_csi2_resources),
    .resource	= am_mipi_csi2_resources,
    .dev = {
        .platform_data = &am_mipi_csi2_info,
    },
};
#endif

#ifdef CONFIG_SARADC_AM
#include <linux/saradc.h>
static struct platform_device saradc_device = {
    .name = "saradc",
    .id = 0,
    .dev = {
        .platform_data = NULL,
    },
};
#endif

#if defined(CONFIG_ADC_KEYPADS_AM)||defined(CONFIG_ADC_KEYPADS_AM_MODULE)
#include <linux/input.h>
#include <linux/adc_keypad.h>

static struct adc_key adc_kp_key[] = {
    {KEY_VOLUMEDOWN,    "vol-", CHAN_4, 150, 40},
    {KEY_VOLUMEUP,      "vol+", CHAN_4, 275, 40},
};

static struct adc_kp_platform_data adc_kp_pdata = {
    .key = &adc_kp_key[0],
    .key_num = ARRAY_SIZE(adc_kp_key),
};

static struct platform_device adc_kp_device = {
    .name = "m1-adckp",
    .id = 0,
    .num_resources = 0,
    .resource = NULL,
    .dev = {
        .platform_data = &adc_kp_pdata,
    }
};
#endif

/***********************************************************************
 * Power Key Section
 **********************************************************************/


#if defined(CONFIG_KEY_INPUT_CUSTOM_AM) || defined(CONFIG_KEY_INPUT_CUSTOM_AM_MODULE)
#include <linux/input.h>
#include <linux/input/key_input.h>

static int _key_code_list[] = {KEY_POWER};

static inline int key_input_init_func(void)
{
    WRITE_AOBUS_REG(AO_RTC_ADDR0, (READ_AOBUS_REG(AO_RTC_ADDR0) &~(1<<11)));
    WRITE_AOBUS_REG(AO_RTC_ADDR1, (READ_AOBUS_REG(AO_RTC_ADDR1) &~(1<<3)));
    return 0;
}
static inline int key_scan(void* data)
{
    int *key_state_list = (int*)data;
    int ret = 0;
    key_state_list[0] = ((READ_AOBUS_REG(AO_RTC_ADDR1) >> 2) & 1) ? 0 : 1;
    return ret;
}

static  struct key_input_platform_data  key_input_pdata = {
    .scan_period = 20,
    .fuzz_time = 60,
    .key_code_list = &_key_code_list[0],
    .key_num = ARRAY_SIZE(_key_code_list),
    .scan_func = key_scan,
    .init_func = key_input_init_func,
    .config = 0,
};

static struct platform_device input_device_key = {
    .name = "meson-keyinput",
    .id = 0,
    .num_resources = 0,
    .resource = NULL,
    .dev = {
        .platform_data = &key_input_pdata,
    }
};
#endif



/***********************************************************************
 * Audio section
 **********************************************************************/
static struct resource aml_m6_audio_resource[] = {
    [0] =   {
        .start      =   0,
        .end        =   0,
        .flags      =   IORESOURCE_MEM,
    },
};

static struct platform_device aml_audio = {
    .name           = "aml-audio",
    .id             = 0,
};

static struct platform_device aml_audio_dai = {
    .name           = "aml-dai",
    .id             = 0,
};

#if defined(CONFIG_SND_SOC_DUMMY_CODEC)
static pinmux_item_t dummy_codec_pinmux[] = {
    /* I2S_MCLK I2S_BCLK I2S_LRCLK I2S_DOUT */
    {
        .reg = PINMUX_REG(9),
        .setmask = (1 << 7) | (1 << 5) | (1 << 9) | (1 << 4),
        .clrmask = (7 << 19) | (7 << 1) | (3 << 10) | (1 << 6),
    },
    {
        .reg = PINMUX_REG(8),
        .clrmask = (0x7f << 24),
    },
    PINMUX_END_ITEM
};

static pinmux_set_t dummy_codec_pinmux_set = {
    .chip_select = NULL,
    .pinmux = &dummy_codec_pinmux[0],
};

static void dummy_codec_device_init(void)
{
    /* audio pinmux */
    pinmux_set(&dummy_codec_pinmux_set);
}

static void dummy_codec_device_deinit(void)
{
    pinmux_clr(&dummy_codec_pinmux_set);
}


static struct dummy_codec_platform_data dummy_codec_pdata = {
    .device_init    = dummy_codec_device_init,
    .device_uninit  = dummy_codec_device_deinit,
};

static struct platform_device aml_dummy_codec_audio = {
    .name           = "aml_dummy_codec_audio",
    .id             = 0,
    .resource       = aml_m6_audio_resource,
    .num_resources  = ARRAY_SIZE(aml_m6_audio_resource),
    .dev = {
        .platform_data = &dummy_codec_pdata,
    },
};

static struct platform_device aml_dummy_codec = {
    .name           = "dummy_codec",
    .id             = 0,
};
#endif

#if defined(CONFIG_AM_DVB)
static struct resource amlogic_dvb_resource[]  = {
	[0] = {
		.start = INT_DEMUX,                   //demux 0 irq
		.end   = INT_DEMUX,
		.flags = IORESOURCE_IRQ,
		.name  = "demux0_irq"
	},
	[1] = {
		.start = INT_DEMUX_1,                    //demux 1 irq
		.end   = INT_DEMUX_1,
		.flags = IORESOURCE_IRQ,
		.name  = "demux1_irq"
	},
	[2] = {
		.start = INT_DEMUX_2,                    //demux 2 irq
		.end   = INT_DEMUX_2,
		.flags = IORESOURCE_IRQ,
		.name  = "demux2_irq"
	},	
	[3] = {
		.start = INT_ASYNC_FIFO_FLUSH,                   //dvr 0 irq
		.end   = INT_ASYNC_FIFO_FLUSH,
		.flags = IORESOURCE_IRQ,
		.name  = "dvr0_irq"
	},
	[4] = {
		.start = INT_ASYNC_FIFO2_FLUSH,          //dvr 1 irq
		.end   = INT_ASYNC_FIFO2_FLUSH,
		.flags = IORESOURCE_IRQ,
		.name  = "dvr1_irq"
	},	
};

static  struct platform_device amlogic_dvb_device = {
	.name             = "amlogic-dvb",
	.id               = -1,
	.num_resources    = ARRAY_SIZE(amlogic_dvb_resource),
	.resource         = amlogic_dvb_resource,
};

static int m6skt_dvb_init(void)
{
	{
//#define FEC_B
		static pinmux_item_t fec_pins[] = {
		    {/*fec mode*/
		        .reg = PINMUX_REG(0),
		        .clrmask = 1 << 6
		    },
#if FEC_B	/*FEC_B*/
		    {
		        .reg = PINMUX_REG(6),
		        .setmask = 0x1f << 19
		    },
		    {
		        .reg = PINMUX_REG(3),
		        .clrmask = 1 << 5
		    },
#else		/*FEC_A*/
		    {
		        .reg = PINMUX_REG(3),
		        .setmask = 0x3f
		    },
		    {
		        .reg = PINMUX_REG(6),
		        .clrmask = 0x1f << 19
		    },		    
#endif
		    PINMUX_END_ITEM
		};
		static pinmux_set_t fec_pinmux_set = {
		    .chip_select = NULL,
		    .pinmux = &fec_pins[0]
		};
		pinmux_set(&fec_pinmux_set);
	}

	{
		static pinmux_item_t i2c_pins[] = {
		    {/*sw i2c*/
		        .reg = PINMUX_REG(5),
		        .clrmask = 0xf << 28
		    },
		    PINMUX_END_ITEM
		};

		static pinmux_set_t i2c_pinmux_set = {
		    .chip_select = NULL,
		    .pinmux = &i2c_pins[0]
		};
		pinmux_set(&i2c_pinmux_set);
	}
	return 0;
}

#ifdef CONFIG_AM_DIB7090P
static struct resource dib7090p_resource[]  = {

	[0] = {
		.start = 0,                                    //frontend  i2c adapter id
		.end   = 0,
		.flags = IORESOURCE_MEM,
		.name  = "frontend0_i2c"
	},
	[1] = {
		.start = 0x10,                                 //frontend 0 demod address
		.end   = 0x10,
		.flags = IORESOURCE_MEM,
		.name  = "frontend0_demod_addr"
	},
};

static  struct platform_device dib7090p_device = {
	.name             = "DiB7090P",
	.id               = -1,
	.num_resources    = ARRAY_SIZE(dib7090p_resource),
	.resource         = dib7090p_resource,
};
#endif
#endif

#ifdef CONFIG_AM_SMARTCARD
static int m6skt_smc_init(void)
{
	{
		static pinmux_item_t smc_pins[] = {
		    {/*disable i2s_in*/
		        .reg = PINMUX_REG(8),
		        .clrmask = 0x7f << 24
		    },
		    {/*disable uart_b*/
		        .reg = PINMUX_REG(4),
		        .clrmask = 0xf << 6
		    },
		    {/*enable 7816*/
		        .reg = PINMUX_REG(4),
		        .setmask = 0xf << 18
		    },
		    {/*disable pcm*/
		        .reg = PINMUX_REG(4),
		        .clrmask = 0xf << 22
		    },
		    PINMUX_END_ITEM
		};

		static pinmux_set_t smc_pinmux_set = {
		    .chip_select = NULL,
		    .pinmux = &smc_pins[0]
		};
		pinmux_set(&smc_pinmux_set);
	}
	return 0;
}

static struct resource smartcard_resource[] =
{
	[0] = {
	       .start = PAD_GPIOX_18,
		.end   = PAD_GPIOX_18,
		.flags = IORESOURCE_MEM,
		.name  = "smc0_reset"
	},
	[1] = {
		.start = INT_SMART_CARD,
		.end   = INT_SMART_CARD,
		.flags = IORESOURCE_IRQ,
		.name  = "smc0_irq"
	},
};

static struct platform_device smartcard_device =
	{

		.name 			= "amlogic-smc",
		.id 				= -1,
		.num_resources	= ARRAY_SIZE(smartcard_resource),
		.resource 		= smartcard_resource,
};

#endif //def CONFIG_AM_SMARTCARD

/***********************************************************************
 * Device Register Section
 **********************************************************************/
static struct platform_device  *platform_devs[] = {
#if defined(CONFIG_I2C_AML) || defined(CONFIG_I2C_HW_AML)
    &aml_i2c_device_a,
    &aml_i2c_device_b,
    &aml_i2c_device_ao,
#endif
#if defined(CONFIG_I2C_SW_AML)
    &aml_sw_i2c_device_1,
#endif
    &aml_uart_device,
#ifdef CONFIG_AM_ETHERNET
	&meson_device_eth,
#endif
    &meson_device_fb,
    &meson_device_vout,
#ifdef CONFIG_AM_STREAMING
    &meson_device_codec,
#endif
#if defined(CONFIG_AM_NAND)
    &aml_nand_device,
#endif
#ifdef CONFIG_SARADC_AM
    &saradc_device,
#endif
#if defined(CONFIG_ADC_KEYPADS_AM)||defined(CONFIG_ADC_KEYPADS_AM_MODULE)
    &adc_kp_device,
#endif
#if defined(CONFIG_KEY_INPUT_CUSTOM_AM) || defined(CONFIG_KEY_INPUT_CUSTOM_AM_MODULE)
    &input_device_key,
#endif
#if defined(CONFIG_CARDREADER)
    &meson_card_device,
#endif // CONFIG_CARDREADER
#if defined(CONFIG_MMC_AML)
    &aml_mmc_device,
#endif
#if defined(CONFIG_SUSPEND)
    &aml_pm_device,
#endif
#ifdef CONFIG_EFUSE
    &aml_efuse_device,
#endif
#if defined(CONFIG_AML_RTC)
    &aml_rtc_device,
#endif
    &aml_audio,
    &aml_audio_dai,
#if defined(CONFIG_SND_SOC_DUMMY_CODEC)
    &aml_dummy_codec_audio,
    &aml_dummy_codec,
#endif
#ifdef CONFIG_AM_REMOTE
    &meson_device_remote,
#endif
#ifdef CONFIG_TVIN_VDIN
    &vdin_device,
#endif
#ifdef CONFIG_AMLOGIC_VIDEOIN_MANAGER    
    &vm_device,
#endif
#ifdef CONFIG_AMLOGIC_MIPI
    &am_mipi_csi2_device,
#endif
#ifdef CONFIG_POST_PROCESS_MANAGER
    &ppmgr_device,
#endif
#ifdef CONFIG_AM_DVB
	&amlogic_dvb_device,
#ifdef CONFIG_AM_DIB7090P
	&dib7090p_device,
#endif
#endif
#ifdef CONFIG_AM_SMARTCARD
	&smartcard_device,
#endif
};

static __init void meson_init_machine(void)
{
//    meson_cache_init();

    /**
     *  Meson6 socket board ONLY
     *  Do *NOT* merge for other BSP
     */
#if defined(CONFIG_CARDREADER)
    {
        extern int (*sd_mmc_ins_register) (void);
        sd_mmc_ins_register = meson_mmc_detect;
    }
#endif // CONFIG_CARDREADER
#ifdef CONFIG_AM_ETHERNET
	setup_eth_device();
#endif
#ifdef CONFIG_AM_REMOTE
	setup_remote_device();
#endif
#ifdef CONFIG_EFUSE
	setup_aml_efuse();	
#endif
    setup_usb_devices();
    setup_devices_resource();
    platform_add_devices(platform_devs, ARRAY_SIZE(platform_devs));

#if defined(CONFIG_SUSPEND)
    {//todo: remove it after verified. need set it in uboot environment variable.
    extern  int console_suspend_enabled;
    console_suspend_enabled = 0;
    }
#endif
#if defined(CONFIG_I2C_AML) || defined(CONFIG_I2C_HW_AML)
    aml_i2c_init();
#endif
#ifdef CONFIG_AM_LCD_OUTPUT
    m6skt_lcd_init();
#endif
#ifdef CONFIG_AM_DVB
   m6skt_dvb_init();
#endif
#ifdef CONFIG_AM_SMARTCARD
   m6skt_smc_init();
#endif
}
static __init void meson_init_early(void)
{///boot seq 1

}

MACHINE_START(MESON6_SKT, "Amlogic Meson6 socket board")
    .boot_params    = BOOT_PARAMS_OFFSET,
    .map_io         = meson_map_io,///2
    .init_early     = meson_init_early,///3
    .init_irq       = meson_init_irq,///0
    .timer          = &meson_sys_timer,
    .init_machine   = meson_init_machine,
    .fixup          = meson_fixup,///1
MACHINE_END
