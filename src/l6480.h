#include <zephyr/sys/util.h>
#include <zephyr/drivers/spi.h>

enum L64x0Addr {
	L64x0_ADDR_ABS_POS    = 0x01,
	L64x0_ADDR_EL_POS     = 0x02,
	L64x0_ADDR_MARK	      = 0x03,
	L64x0_ADDR_SPEED      = 0x04,
	L64x0_ADDR_ACC	      = 0x05,
	L64x0_ADDR_DEC	      = 0x06,
	L64x0_ADDR_MAX_SPEED  = 0x07,
	L64x0_ADDR_MIN_SPEED  = 0x08,
	L64x0_ADDR_KVAL_HOLD  = 0x09,
	L64x0_ADDR_KVAL_RUN   = 0x0a,
	L64x0_ADDR_KVAL_ACC   = 0x0b,
	L64x0_ADDR_KVAL_DEC   = 0x0c,
	L64x0_ADDR_INT_SPEED  = 0x0d,
	L64x0_ADDR_ST_SLP     = 0x0e,
	L64x0_ADDR_FN_SLP_ACC = 0x0f,
	L64x0_ADDR_FN_SLP_DEC = 0x10,
	L64x0_ADDR_K_THERM    = 0x11,
	L64x0_ADDR_ADC_OUT    = 0x12,
	L64x0_ADDR_OCD_TH     = 0x13,
	L64x0_ADDR_STALL_TH   = 0x14,
	L64x0_ADDR_FS_SPD     = 0x15,
	L64x0_ADDR_STEP_MODE  = 0x16,
	L64x0_ADDR_ALARM_EN   = 0x17,
	L64x0_ADDR_GATECFG1   = 0x18,
	L64x0_ADDR_GATECFG2   = 0x19,
	L64x0_ADDR_CONFIG     = 0x1a,
	L64x0_ADDR_STATUS     = 0x1b,
	L64x0_ADDR_LAST,
};

/* Minimum speed */
#define L64X0_MIN_SPEED_LSPD_OPT (0x800)

/* KVAL_X */
#define L64X0_KVAL_X_256TH(x) ((x) & GENMASK(7, 0))

/* Overcurrent Threshold */
#define L64X0_OCD_TH_31_25_mV         (0)
#define L64X0_OCD_TH_62_50_mV         (1)
#define L64X0_OCD_TH_93_75_mV         (2)
#define L64X0_OCD_TH_125_mV           (3)
#define L64X0_OCD_TH_156_25_mV        (4)
#define L64X0_OCD_TH_187_50_mV        (5)
#define L64X0_OCD_TH_218_75_mV        (6)
#define L64X0_OCD_TH_250_mV           (7)
#define L64X0_OCD_TH_281_25_mV        (8)
#define L64X0_OCD_TH_312_50_mV        (9)
#define L64X0_OCD_TH_343_75_mV        (10)
#define L64X0_OCD_TH_375_mV           (11)
#define L64X0_OCD_TH_406_25_mV        (12)
#define L64X0_OCD_TH_437_50_mV        (13)
#define L64X0_OCD_TH_468_75_mV        (14)
#define L64X0_OCD_TH_500_mV           (15)
#define L64X0_OCD_TH_531_25_mV        (16)
#define L64X0_OCD_TH_562_50_mV        (17)
#define L64X0_OCD_TH_593_75_mV        (18)
#define L64X0_OCD_TH_625_mV           (19)
#define L64X0_OCD_TH_656_25_mV        (20)
#define L64X0_OCD_TH_687_50_mV        (21)
#define L64X0_OCD_TH_718_75_mV        (22)
#define L64X0_OCD_TH_750_mV           (23)
#define L64X0_OCD_TH_781_25_mV        (24)
#define L64X0_OCD_TH_812_50_mV        (25)
#define L64X0_OCD_TH_843_75_mV        (26)
#define L64X0_OCD_TH_875_mV           (27)
#define L64X0_OCD_TH_906_25_mV        (28)
#define L64X0_OCD_TH_937_50_mV        (29)
#define L64X0_OCD_TH_968_75_mV        (30)
#define L64X0_OCD_TH_1000_mV          (31)

/* Stall detection Threshold */
#define L64X0_STALL_TH_31_25_mV         (0)
#define L64X0_STALL_TH_62_50_mV         (1)
#define L64X0_STALL_TH_93_75_mV         (2)
#define L64X0_STALL_TH_125_mV           (3)
#define L64X0_STALL_TH_156_25_mV        (4)
#define L64X0_STALL_TH_187_50_mV        (5)
#define L64X0_STALL_TH_218_75_mV        (6)
#define L64X0_STALL_TH_250_mV           (7)
#define L64X0_STALL_TH_281_25_mV        (8)
#define L64X0_STALL_TH_312_50_mV        (9)
#define L64X0_STALL_TH_343_75_mV        (10)
#define L64X0_STALL_TH_375_mV           (11)
#define L64X0_STALL_TH_406_25_mV        (12)
#define L64X0_STALL_TH_437_50_mV        (13)
#define L64X0_STALL_TH_468_75_mV        (14)
#define L64X0_STALL_TH_500_mV           (15)
#define L64X0_STALL_TH_531_25_mV        (16)
#define L64X0_STALL_TH_562_50_mV        (17)
#define L64X0_STALL_TH_593_75_mV        (18)
#define L64X0_STALL_TH_625_mV           (19)
#define L64X0_STALL_TH_656_25_mV        (20)
#define L64X0_STALL_TH_687_50_mV        (21)
#define L64X0_STALL_TH_718_75_mV        (22)
#define L64X0_STALL_TH_750_mV           (23)
#define L64X0_STALL_TH_781_25_mV        (24)
#define L64X0_STALL_TH_812_50_mV        (25)
#define L64X0_STALL_TH_843_75_mV        (26)
#define L64X0_STALL_TH_875_mV           (27)
#define L64X0_STALL_TH_906_25_mV        (28)
#define L64X0_STALL_TH_937_50_mV        (29)
#define L64X0_STALL_TH_968_75_mV        (30)
#define L64X0_STALL_TH_1000_mV          (31)

/* Step mode */
#define L64X0_STEP_MODE_FULL          (0)
#define L64X0_STEP_MODE_HALF          (1)
#define L64X0_STEP_MODE_QUARTER       (2)
#define L64X0_STEP_MODE_8TH           (3)
#define L64X0_STEP_MODE_16TH          (4)
#define L64X0_STEP_MODE_32TH          (5)
#define L64X0_STEP_MODE_64TH          (6)
#define L64X0_STEP_MODE_128TH         (7)

/* Gate driver configuration */
#define L64X0_WD_EN	BIT(11)

#define L64X0_TBOOST_0_ns		(0)
#define L64X0_TBOOST_625_833_125_ns	(1 << 8)
#define L64X0_TBOOST_125_ns		(2 << 8)
#define L64X0_TBOOST_250_ns		(3 << 8)
#define L64X0_TBOOST_375_ns		(4 << 8)
#define L64X0_TBOOST_500_ns		(5 << 8)
#define L64X0_TBOOST_750_ns		(6 << 8)
#define L64X0_TBOOST_1000_ns		(7 << 8)

#define L64X0_IGATE_4_mA (0)
/* #define L64X0_IGATE_4_mA (1 << 5) */
#define L64X0_IGATE_8_mA (2 << 5)
#define L64X0_IGATE_16_mA (3 << 5)
#define L64X0_IGATE_24_mA (4 << 5)
#define L64X0_IGATE_32_mA (5 << 5)
#define L64X0_IGATE_64_mA (6 << 5)
#define L64X0_IGATE_96_mA (7 << 5)

#define L64X0_TCC_125_ns	(0)
#define L64X0_TCC_250_ns	(1)
#define L64X0_TCC_375_ns	(2)
#define L64X0_TCC_500_ns	(3)
#define L64X0_TCC_625_ns	(4)
#define L64X0_TCC_750_ns	(5)
#define L64X0_TCC_875_ns	(6)
#define L64X0_TCC_1000_ns	(7)
#define L64X0_TCC_1125_ns	(8)
#define L64X0_TCC_1250_ns	(9)
#define L64X0_TCC_1375_ns	(10)
#define L64X0_TCC_1500_ns	(11)
#define L64X0_TCC_1625_ns	(12)
#define L64X0_TCC_1750_ns	(13)
#define L64X0_TCC_1875_ns	(14)
#define L64X0_TCC_2000_ns	(15)
#define L64X0_TCC_2125_ns	(16)
#define L64X0_TCC_2250_ns	(17)
#define L64X0_TCC_2375_ns	(18)
#define L64X0_TCC_2500_ns	(19)
#define L64X0_TCC_2625_ns	(20)
#define L64X0_TCC_2750_ns	(21)
#define L64X0_TCC_2875_ns	(22)
#define L64X0_TCC_3000_ns	(23)
#define L64X0_TCC_3125_ns	(24)
#define L64X0_TCC_3250_ns	(25)
#define L64X0_TCC_3375_ns	(26)
#define L64X0_TCC_3500_ns	(27)
#define L64X0_TCC_3625_ns	(28)
/* #define L64X0_TCC_3750_ns	(29) */
/* #define L64X0_TCC_3750_ns	(30) */
#define L64X0_TCC_3750_ns	(31)

/* IC configuration Register */
#define L64X0_CONFIG_F_PWM_INT(x)	(((x) << 13) & GENMASK(15, 13))
#define L64X0_CONFIG_F_PWM_DEC(x)	(((x) << 10) & GENMASK(12, 10))
#define L64X0_CONFIG_VCCVAL		BIT(9)
#define L64X0_CONFIG_UVLOVAL		BIT(8)
#define L64X0_CONFIG_OC_SD		BIT(7)
#define L64X0_CONFIG_EN_VSCOMP		BIT(5)
#define L64X0_CONFIG_SW_MODE		BIT(4)
#define L64X0_CONFIG_EXT_CLK		BIT(3)
#define L64X0_CONFIG_OSC_SEL(x)		((x) & GENMASK(2, 0))

/* Status */
#define L64X0_STATUS_STEP_LOSS_B	BIT(15)
#define L64X0_STATUS_STEP_LOSS_A	BIT(14)
#define L64X0_STATUS_OCD		BIT(13)
#define L64X0_STATUS_TH_STATUS(x)	(((x) << 11) & GENMASK(12, 11))
#define L64X0_STATUS_UVLO_ADC		BIT(10)
#define L64X0_STATUS_UVLO		BIT(9)
#define L64X0_STATUS_STCK_MOD		BIT(8)
#define L64X0_STATUS_CMD_ERROR		BIT(7)
#define L64X0_STATUS_MOT_STATUS(x)	(((x) << 5) & GENMASK(6, 5))
#define L64X0_STATUS_DIR		BIT(4)
#define L64X0_STATUS_SW_EVN		BIT(3)
#define L64X0_STATUS_SW_F		BIT(2)
#define L64X0_STATUS_BUSY		BIT(1)
#define L64X0_STATUS_HiZ		BIT(0)
#define L64X0_STATUS_HIZ		STATUS_HiZ

int l64x0_nop(const struct device *const dev, struct spi_config *config);
void l64x0_setparam(const struct device *const dev, struct spi_config *config, uint8_t param, uint32_t val);
int l64x0_getparam(const struct device *const dev, struct spi_config *config, uint8_t param);
int l64x0_run(const struct device *const dev, struct spi_config *config, int speed);
int l64x0_move(const struct device *const dev, struct spi_config *config, int n_step);
int l64x0_reset_device(const struct device *const dev, struct spi_config *config);
int l64x0_soft_stop(const struct device *const dev, struct spi_config *config);
int l64x0_hard_stop(const struct device *const dev, struct spi_config *config);
int l64x0_soft_hiz(const struct device *const dev, struct spi_config *config);
int l64x0_hard_hiz(const struct device *const dev, struct spi_config *config);
int l64x0_get_status(const struct device *const dev, struct spi_config *config);

#define GEN_SETPARAM(fname, pname)					\
	static inline void l64x0_setparam_ ##fname(const struct device *const dev, struct spi_config *config, uint32_t val) \
	{								\
		l64x0_setparam(dev, config, L64x0_ADDR_ ##pname, val);	\
	}

GEN_SETPARAM(acc, ACC)
GEN_SETPARAM(dec, DEC)
GEN_SETPARAM(max_speed, MAX_SPEED)
static inline void l64x0_setparam_min_speed(const struct device *const dev, struct spi_config *config, uint32_t speed, bool lspd_opt)
{
	uint32_t val = speed & GENMASK(11, 0);

	if (lspd_opt)
		val |= L64X0_MIN_SPEED_LSPD_OPT;

	l64x0_setparam(dev, config, L64x0_ADDR_MIN_SPEED, val);
}
GEN_SETPARAM(kval_hold, KVAL_HOLD)
GEN_SETPARAM(kval_run, KVAL_RUN)
GEN_SETPARAM(kval_acc, KVAL_ACC)
GEN_SETPARAM(kval_dec, KVAL_DEC)
GEN_SETPARAM(int_speed, INT_SPEED)
GEN_SETPARAM(st_slp, ST_SLP)
GEN_SETPARAM(fn_slp_acc, FN_SLP_ACC)
GEN_SETPARAM(fn_slp_dec, FN_SLP_DEC)
GEN_SETPARAM(k_therm, K_THERM)
GEN_SETPARAM(adc_out, ADC_OUT)
GEN_SETPARAM(ocd_th, OCD_TH)
GEN_SETPARAM(stall_th, STALL_TH)
GEN_SETPARAM(fs_spd, FS_SPD)
GEN_SETPARAM(step_mode, STEP_MODE)
GEN_SETPARAM(alarm_en, ALARM_EN)
GEN_SETPARAM(gatecfg1, GATECFG1)
GEN_SETPARAM(gatecfg2, GATECFG2)
GEN_SETPARAM(config, CONFIG)
GEN_SETPARAM(status, STATUS)

#define GEN_GETPARAM(fname, pname)					\
	static inline int l64x0_getparam_ ##fname(const struct device *const dev, struct spi_config *config) \
	{								\
		return l64x0_getparam(dev, config, L64x0_ADDR_ ##pname);	\
	}

GEN_GETPARAM(abs_pos, ABS_POS)
GEN_GETPARAM(el_pos, EL_POS)
GEN_GETPARAM(mark, MARK)
GEN_GETPARAM(speed, SPEED)
GEN_GETPARAM(acc, ACC)
GEN_GETPARAM(dec, DEC)
GEN_GETPARAM(max_speed, MAX_SPEED)
GEN_GETPARAM(min_speed, MIN_SPEED)
GEN_GETPARAM(kval_hold, KVAL_HOLD)
GEN_GETPARAM(kval_run, KVAL_RUN)
GEN_GETPARAM(kval_acc, KVAL_ACC)
GEN_GETPARAM(kval_dec, KVAL_DEC)
GEN_GETPARAM(int_speed, INT_SPEED)
GEN_GETPARAM(st_slp, ST_SLP)
GEN_GETPARAM(fn_slp_acc, FN_SLP_ACC)
GEN_GETPARAM(fn_slp_dec, FN_SLP_DEC)
GEN_GETPARAM(k_therm, K_THERM)
GEN_GETPARAM(adc_out, ADC_OUT)
GEN_GETPARAM(ocd_th, OCD_TH)
GEN_GETPARAM(stall_th, STALL_TH)
GEN_GETPARAM(fs_spd, FS_SPD)
GEN_GETPARAM(step_mode, STEP_MODE)
GEN_GETPARAM(alarm_en, ALARM_EN)
GEN_GETPARAM(gatecfg1, GATECFG1)
GEN_GETPARAM(gatecfg2, GATECFG2)
GEN_GETPARAM(config, CONFIG)
GEN_GETPARAM(status, STATUS)
