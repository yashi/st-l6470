/*
 * Copyright (c) 2023 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <stdlib.h>

#include "l64x0.h"

#define MOTOR_DRIVER DT_NODELABEL(motor_driver)

#if IS_ENABLED(CONFIG_L6470)
static void debug_print(const struct device *const dev, int count)
{
	uint16_t adc_out = l64x0_getparam_adc_out(dev);
	uint32_t speed = l64x0_getparam_speed(dev);
	uint32_t status = l64x0_get_status(dev);
	printk("%d: Speed: %u (0x%x), Status: 0x%x (%s%s%s%s%s%s%s%s%s%s%s%s%s), ADC_OUT %x\n",
	       count,
	       speed, speed, status,
	       (status & L6470_STATUS_SCK_MOD) ? "SCK_MOD " : "",
	       (status & L6470_STATUS_STEP_LOSS_B) ? "" : "STEP_LOSS_B ",
	       (status & L6470_STATUS_STEP_LOSS_A) ? "" : "STEP_LOSS_A ",
	       (status & L6470_STATUS_OCD) ? "" : "OCD ",
	       (status & L6470_STATUS_TH_SD) ? "" : "TH_SD ",
	       (status & L6470_STATUS_TH_WRN) ? "" : "TH_WRN ",
	       (status & L6470_STATUS_UVLO) ? "" : "UVLO ",
	       (status & L6470_STATUS_WRONG_CMD) ? "WRONG_CMD " : "",
	       (status & L6470_STATUS_NOTPERF_CMD) ? "NOTPERF_CMD " : "",
	       (status & L6470_STATUS_DIR) ? "DIR_f " : "DIR_r ",
	       (status & L6470_STATUS_SW_EVN) ? "SW_EVN " : "",
	       (status & L6470_STATUS_BUSY) ? "" : "BUSY ",
	       (status & L6470_STATUS_HiZ) ? "HiZ" : "",
	       adc_out);
}
#elif IS_ENABLED(CONFIG_L6480)
static void debug_print(const struct device *const dev, int count)
{
	uint16_t adc_out = l64x0_getparam_adc_out(dev);
	uint32_t speed = l64x0_getparam_speed(dev);
	uint32_t status = l64x0_get_status(dev);
	printk("%d: Speed: %u (0x%x), Status: 0x%x (%s%s%s%s%s%s%s%s%s%s%s%s), ADC_OUT %x\n",
	       count,
	       speed, speed, status,
	       (status & L6480_STATUS_STEP_LOSS_B) ? "" : "STEP_LOSS_B ",
	       (status & L6480_STATUS_STEP_LOSS_A) ? "" : "STEP_LOSS_A ",
	       (status & L6480_STATUS_OCD) ? "" : "OCD ",
	       (status & L6480_STATUS_UVLO_ADC) ? "" : "UVLO_ADC ",
	       (status & L6480_STATUS_UVLO) ? "" : "UVLO ",
	       (status & L6480_STATUS_STCK_MOD) ? "STCK_MOD " : "",
	       (status & L6480_STATUS_CMD_ERROR) ? "CMD_ERROR " : "",
	       (status & L6480_STATUS_DIR) ? "DIR_f " : "DIR_r ",
	       (status & L6480_STATUS_SW_EVN) ? "SW_EVN " : "",
	       (status & L6480_STATUS_SW_F) ? "SW_F " : "",
	       (status & L6480_STATUS_BUSY) ? "" : "BUSY ",
	       (status & L6480_STATUS_HiZ) ? "HiZ" : "",
	       adc_out);
}
#endif

int main(void)
{
	const struct device *const dev = DEVICE_DT_GET(MOTOR_DRIVER);

	if (!device_is_ready(dev)) {
		printk("%s: device not ready.\n", dev->name);
		return EXIT_FAILURE;
	}

	const struct gpio_dt_spec n_stby =
		GPIO_DT_SPEC_GET(DT_NODELABEL(motor_driver), stby_gpios);
	if (!device_is_ready(n_stby.port)) {
		printk("%s: device not ready.\n", dev->name);
		return EXIT_FAILURE;
	}
	gpio_pin_configure_dt(&n_stby, GPIO_OUTPUT_ACTIVE);
	k_usleep(10);
	gpio_pin_set_dt(&n_stby, 0);

	k_sleep(K_MSEC(1));


	l64x0_nop(dev);
	l64x0_nop(dev);
	l64x0_nop(dev);
	l64x0_nop(dev);
	l64x0_reset_device(dev);
	k_usleep(500);
	debug_print(dev, -1);

	/* K_VAL = (K_VAL_X + BEMF_COMP) * VSCOMP * K_THERM) * microstep */
	/* No compensation is active */
	/** The K_VALs */
	l64x0_setparam_kval_hold(dev, L64X0_KVAL_X_256TH(22));
	l64x0_setparam_kval_acc(dev, L64X0_KVAL_X_256TH(22));
	l64x0_setparam_kval_dec(dev, L64X0_KVAL_X_256TH(22));
	l64x0_setparam_kval_run(dev, L64X0_KVAL_X_256TH(22));

	/** No BEMF for now */
	/* l64x0_setparam_st_slp(dev, 0); */
	/* l64x0_setparam_fn_slp_acc(dev, 0); */
	/* l64x0_setparam_fn_slp_dec(dev, 0); */

	/** VSCOMP is at CONFIG register */
	/** K_THERM is default 0 */

	/** 128 micro-steps */
	l64x0_setparam_step_mode(dev, L64X0_STEP_MODE_128TH);

	/* Accel and speed */
	l64x0_setparam_acc(dev, 0x19);
	l64x0_setparam_dec(dev, 0x10);
	l64x0_setparam_max_speed(dev, 30);
	l64x0_setparam_min_speed(dev, 1, false);

	l64x0_setparam_fs_spd(dev, 0x3ff);

	if (IS_ENABLED(CONFIG_L6470)) {
		/* OCD and Stall */
		l64x0_setparam_ocd_th(dev, L6470_OCD_TH_4125_mV);
		l64x0_setparam_stall_th(dev, L6470_STALL_TH_4000_mA);
	}
	else if (IS_ENABLED(CONFIG_L6480)) {
		/* OCD and Stall */
		l64x0_setparam_ocd_th(dev, L6480_OCD_TH_125_mV);
		l64x0_setparam_stall_th(dev, L6480_STALL_TH_125_mV);

		/* Gate Drive */
		l64x0_setparam_gatecfg1(dev, L6480_IGATE_8_mA | L6480_TCC_2875_ns);

		/* Configs */
		uint32_t reg;
		reg = l64x0_getparam_config(dev);
		l64x0_setparam_config(dev, reg | L6480_CONFIG_VCCVAL | L6480_CONFIG_UVLOVAL);
	}

	/* Now setttings are all done, wait for the charge pump to be above the threshold */
	WAIT_FOR(l64x0_get_status(dev) & L64X0_STATUS_UVLO, 2000, printk("."));
	printk("\n");

	l64x0_run(dev, 0x8000);
	//l64x0_move(dev, 0x128000);
	//l64x0_soft_stop(dev);

	for (int i=0; i<20; i++) {
		debug_print(dev, i);
		k_sleep(K_SECONDS(1));
	}
	printk("loop end\n");
	l64x0_soft_hiz(dev);
	printk("done\n");

	return EXIT_SUCCESS;
}
