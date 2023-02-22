#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <stdlib.h>

#include "l6470.h"

#define MOTOR_SPI DT_NODELABEL(spi4)

static void debug_print(const struct device *const dev, struct spi_config *config, int count)
{
	uint16_t adc_out = l64x0_getparam_adc_out(dev, config);
	uint32_t speed = l64x0_getparam_speed(dev, config);
	uint32_t status = l64x0_get_status(dev, config);
	printk("%d: Speed: %u (0x%x), Status: 0x%x (%s%s%s%s%s%s%s%s%s%s%s%s%s), ADC_OUT %x\n",
	       count,
	       speed, speed, status,
	       (status & L64X0_STATUS_SCK_MOD) ? "SCK_MOD " : "",
	       (status & L64X0_STATUS_STEP_LOSS_B) ? "" : "STEP_LOSS_B ",
	       (status & L64X0_STATUS_STEP_LOSS_A) ? "" : "STEP_LOSS_A ",
	       (status & L64X0_STATUS_OCD) ? "" : "OCD ",
	       (status & L64X0_STATUS_TH_SD) ? "" : "TH_SD ",
	       (status & L64X0_STATUS_TH_WRN) ? "" : "TH_WRN ",
	       (status & L64X0_STATUS_UVLO) ? "" : "UVLO ",
	       (status & L64X0_STATUS_WRONG_CMD) ? "WRONG_CMD " : "",
	       (status & L64X0_STATUS_NOTPERF_CMD) ? "NOTPERF_CMD " : "",
	       (status & L64X0_STATUS_DIR) ? "DIR_f " : "DIR_r ",
	       (status & L64X0_STATUS_SW_EVN) ? "SW_EVN " : "",
	       (status & L64X0_STATUS_BUSY) ? "" : "BUSY ",
	       (status & L64X0_STATUS_HiZ) ? "HiZ" : "",
	       adc_out);
}

void main(void)
{
	const struct device *const dev = DEVICE_DT_GET(MOTOR_SPI);

	if (!device_is_ready(dev)) {
		printk("%s: device not ready.\n", dev->name);
		return;
	}

	const struct gpio_dt_spec n_stby =
		GPIO_DT_SPEC_GET(DT_NODELABEL(motor_driver), stby_gpios);
	if (!device_is_ready(n_stby.port)) {
		printk("%s: device not ready.\n", dev->name);
		return;
	}
	gpio_pin_configure_dt(&n_stby, GPIO_OUTPUT_ACTIVE);
	k_usleep(10);
	gpio_pin_set_dt(&n_stby, 0);

	k_sleep(K_MSEC(1));


	struct spi_config config;
	config.frequency = KHZ(500);
	config.operation = SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LOCK_ON;
	config.slave = 0;
	config.cs = SPI_CS_CONTROL_PTR_DT(DT_NODELABEL(motor_driver), 0);

	l64x0_nop(dev, &config);
	l64x0_nop(dev, &config);
	l64x0_nop(dev, &config);
	l64x0_nop(dev, &config);
	l64x0_reset_device(dev, &config);
	k_usleep(500);
	debug_print(dev, &config, -1);

	/* K_VAL = (K_VAL_X + BEMF_COMP) * VSCOMP * K_THERM) * microstep */
	/* No compensation is active */
	/** The K_VALs */
	l64x0_setparam_kval_hold(dev, &config, L64X0_KVAL_X_256TH(22));
	l64x0_setparam_kval_acc(dev, &config, L64X0_KVAL_X_256TH(22));
	l64x0_setparam_kval_dec(dev, &config, L64X0_KVAL_X_256TH(22));
	l64x0_setparam_kval_run(dev, &config, L64X0_KVAL_X_256TH(22));

	/** No BEMF for now */
	/* l64x0_setparam_st_slp(dev, &config, 0); */
	/* l64x0_setparam_fn_slp_acc(dev, &config, 0); */
	/* l64x0_setparam_fn_slp_dec(dev, &config, 0); */

	/** VSCOMP is at CONFIG register */
	/** K_THERM is default 0 */

	/** 128 micro-steps */
	l64x0_setparam_step_mode(dev, &config, L64X0_STEP_MODE_128TH);

	/* Accel and speed */
	l64x0_setparam_acc(dev, &config, 0x19);
	l64x0_setparam_dec(dev, &config, 0x10);
	l64x0_setparam_max_speed(dev, &config, 30);
	l64x0_setparam_min_speed(dev, &config, 1, false);

	l64x0_setparam_fs_spd(dev, &config, 0x3ff);

	/* OCD and Stall */
	l64x0_setparam_ocd_th(dev, &config, L64X0_OCD_TH_4125_mV);
	l64x0_setparam_stall_th(dev, &config, L64X0_STALL_TH_4000_mA);

	/* Configs */
	//l64x0_setparam_config(dev, &config, CONFIG_F_PWM_INT(0) | CONFIG_F_PWM_DEC(7) | CONFIG_POW_SR(2) | CONFIG_OC_SD | CONFIG_EXT_CLK);

	/* Now setttings are all done, wait for the charge pump to be above the threshold */
	WAIT_FOR(l64x0_get_status(dev, &config) & L64X0_STATUS_UVLO, 2000, printk("."));
	printk("\n");

	l64x0_run(dev, &config, 0x8000);
	//l64x0_move(dev, &config, 0x128000);
	//l64x0_soft_stop(dev, &config);

	for (int i=0; i<20; i++) {
		debug_print(dev, &config, i);
		k_sleep(K_SECONDS(1));
	}
	printk("loop end\n");
	l64x0_soft_hiz(dev, &config);
	printk("done\n");

	return;
}
