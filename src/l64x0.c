#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(l64x0, LOG_LEVEL_DBG);

#if IS_ENABLED(CONFIG_L6470)
#include "l6470.h"
#elif IS_ENABLED(CONFIG_L6480)
#include "l6480.h"
#else
#error You must choose L6470 or L6480
#endif


#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <stdlib.h>

/* Commands */
#define RX_BYTES_NONE (0)
#define RX_BYTES_GET_STATUS (2)

#define TX_BYTES_NONE (0)
#define TX_BYTES_RUN (3)
#define TX_BYTES_MOVE (3)

#define CMD_NOP          ((0))

#define CMD_SET_PARAM    ((0 << 5))
#define CMD_GET_PARAM    ((1 << 5))

#define CMD_MOVE         ((2 << 5))
#define CMD_RUN          ((2 << 5) | BIT(4))
#define CMD_STEP_CLOCK   ((2 << 5) | BIT(4) | BIT(3))

#define CMD_GOTO         ((3 << 5)
#define CMD_GOTO_DIR     ((3 << 5) |          BIT(3))
#define CMD_GO_HOME      ((3 << 5) | BIT(4))
#define CMD_GO_MARK      ((3 << 5) | BIT(4) | BIT(3))

#define CMD_GO_UNTIL     ((4 << 5) |                   BIT(1))
#define CMD_RELEASE_SW   ((4 << 5) | BIT(4)            BIT(1))

#define CMD_SOFT_HIZ     ((5 << 5))
#define CMD_HARD_HIZ     ((5 << 5) |          BIT(3))
#define CMD_SOFT_STOP    ((5 << 5) | BIT(4))
#define CMD_HARD_STOP    ((5 << 5) | BIT(4) | BIT(3))

#define CMD_RESET_DEVICE ((6 << 5))
#define CMD_GET_STATUS   ((6 << 5) | BIT(4))
#define CMD_RESET_POS    ((6 << 5) | BIT(4) | BIT(3))

#if IS_ENABLED(CONFIG_L6470)
static int bit_len[] = {
        [L64x0_ADDR_ABS_POS]    = 22,
        [L64x0_ADDR_EL_POS]     = 9,
        [L64x0_ADDR_MARK]       = 22,
        [L64x0_ADDR_SPEED]      = 20,
        [L64x0_ADDR_ACC]        = 12,
        [L64x0_ADDR_DEC]        = 12,
        [L64x0_ADDR_MAX_SPEED]  = 10,
        [L64x0_ADDR_MIN_SPEED]  = 13,
        [L64x0_ADDR_KVAL_HOLD]  = 8,
        [L64x0_ADDR_KVAL_RUN]   = 8,
        [L64x0_ADDR_KVAL_ACC]   = 8,
        [L64x0_ADDR_KVAL_DEC]   = 8,
        [L64x0_ADDR_INT_SPEED]  = 14,
        [L64x0_ADDR_ST_SLP]     = 8,
        [L64x0_ADDR_FN_SLP_ACC] = 8,
        [L64x0_ADDR_FN_SLP_DEC] = 8,
        [L64x0_ADDR_K_THERM]    = 4,
        [L64x0_ADDR_ADC_OUT]    = 5,
        [L64x0_ADDR_OCD_TH]     = 4,
        [L64x0_ADDR_STALL_TH]   = 7,
        [L64x0_ADDR_FS_SPD]     = 10,
        [L64x0_ADDR_STEP_MODE]  = 8,
        [L64x0_ADDR_ALARM_EN]   = 8,
        [L64x0_ADDR_CONFIG]     = 16,
        [L64x0_ADDR_STATUS]     = 16,
};
#elif IS_ENABLED(CONFIG_L6480)
static int bit_len[] = {
        [L64x0_ADDR_ABS_POS]    = 22,
        [L64x0_ADDR_EL_POS]     = 9,
        [L64x0_ADDR_MARK]       = 22,
        [L64x0_ADDR_SPEED]      = 20,
        [L64x0_ADDR_ACC]        = 12,
        [L64x0_ADDR_DEC]        = 12,
        [L64x0_ADDR_MAX_SPEED]  = 10,
        [L64x0_ADDR_MIN_SPEED]  = 12,
        [L64x0_ADDR_KVAL_HOLD]  = 8,
        [L64x0_ADDR_KVAL_RUN]   = 8,
        [L64x0_ADDR_KVAL_ACC]   = 8,
        [L64x0_ADDR_KVAL_DEC]   = 8,
        [L64x0_ADDR_INT_SPEED]  = 14,
        [L64x0_ADDR_ST_SLP]     = 8,
        [L64x0_ADDR_FN_SLP_ACC] = 8,
        [L64x0_ADDR_FN_SLP_DEC] = 8,
        [L64x0_ADDR_K_THERM]    = 4,
        [L64x0_ADDR_ADC_OUT]    = 5,
        [L64x0_ADDR_OCD_TH]     = 5,
        [L64x0_ADDR_STALL_TH]   = 5,
        [L64x0_ADDR_FS_SPD]     = 11,
        [L64x0_ADDR_STEP_MODE]  = 4,
        [L64x0_ADDR_ALARM_EN]   = 8,
        [L64x0_ADDR_GATECFG1]   = 11,
        [L64x0_ADDR_GATECFG2]   = 8,
        [L64x0_ADDR_CONFIG]     = 16,
        [L64x0_ADDR_STATUS]     = 16,
};
#endif

#define BITS_TO_BYTES(x) (((x) + 7) >> 3)

static int send_command(const struct device *const dev, struct spi_config *config, uint8_t cmd, int val, int tx_bytes, int rx_bytes)
{
        uint8_t value;
        struct spi_buf bufs;
        struct spi_buf_set bufset;
        bufset.buffers = &bufs;
        bufset.count = 1;
        bufs.buf = &cmd;
        bufs.len = 1;
        int ret = 0;

        spi_write(dev, config, &bufset);

        bufs.buf = &value;

        switch (tx_bytes) {
        case 0:
                break;
        case 1:
                value = val & 0xff;
                spi_write(dev, config, &bufset);

                break;
        case 2:
                value = (val >> 8) & 0xff;
                spi_write(dev, config, &bufset);

                value = val & 0xff;
                spi_write(dev, config, &bufset);

                break;
        case 3:
                value = (val >> 16) & 0xff;
                spi_write(dev, config, &bufset);

                value = (val >> 8) & 0xff;
                spi_write(dev, config, &bufset);

                value = val & 0xff;
                spi_write(dev, config, &bufset);

                break;
        default:
                break;
        }

        switch (rx_bytes) {
        case 0:
                break;
        case 1:
                /* FIXME: this breaks SPI communication */
                //k_busy_wait(10);
                spi_read(dev, config, &bufset);
                ret = value;

                break;
        case 2:
                spi_read(dev, config, &bufset);
                ret |= value << 8;

                spi_read(dev, config, &bufset);
                ret |= value;

                break;
        case 3:
                spi_read(dev, config, &bufset);
                ret |= value << 16;

                spi_read(dev, config, &bufset);
                ret |= value << 8;

                spi_read(dev, config, &bufset);
                ret |= value;

                break;
        default:
                LOG_WRN("rx_bytes is %d for cmd %x", rx_bytes, cmd);
                ret = -1;

                break;
        }

        return ret;
}

static int send_command_simple(const struct device *const dev, struct spi_config *config, uint8_t cmd)
{
        return send_command(dev, config, cmd, 0, TX_BYTES_NONE, RX_BYTES_NONE);
}

int l64x0_nop(const struct device *const dev, struct spi_config *config)
{
        return send_command_simple(dev, config, CMD_NOP);
}

void l64x0_setparam(const struct device *const dev, struct spi_config *config, uint8_t param, uint32_t val)
{
        int tx_bytes;

        if (param == 0 || L64x0_ADDR_LAST <= param) {
                LOG_ERR("param = %x", param);
                return;
        }

        tx_bytes = BITS_TO_BYTES(bit_len[param]);

        send_command(dev, config, param, val, tx_bytes, RX_BYTES_NONE);
}

int l64x0_getparam(const struct device *const dev, struct spi_config *config, uint8_t param)
{
        int rx_bytes;

        if (param == 0 || L64x0_ADDR_LAST <= param) {
                LOG_ERR("param = %x", param);
                return -EINVAL;
        }

        rx_bytes = BITS_TO_BYTES(bit_len[param]);

        return send_command(dev, config, CMD_GET_PARAM | param, 0, TX_BYTES_NONE, rx_bytes);
}

int l64x0_run(const struct device *const dev, struct spi_config *config, int speed)
{
        bool dir = speed >= 0;

        return send_command(dev, config, CMD_RUN | dir, abs(speed), TX_BYTES_RUN, RX_BYTES_NONE);
}

int l64x0_move(const struct device *const dev, struct spi_config *config, int n_step)
{
        bool dir = n_step >= 0;

        send_command(dev, config, CMD_MOVE | dir, abs(n_step), TX_BYTES_MOVE, RX_BYTES_NONE);

        return 0;
}

int l64x0_reset_device(const struct device *const dev, struct spi_config *config)
{
        return send_command_simple(dev, config, CMD_RESET_DEVICE);
}

int l64x0_soft_stop(const struct device *const dev, struct spi_config *config)
{
        return send_command_simple(dev, config, CMD_SOFT_STOP);
}

int l64x0_hard_stop(const struct device *const dev, struct spi_config *config)
{
        return send_command_simple(dev, config, CMD_HARD_STOP);
}

int l64x0_soft_hiz(const struct device *const dev, struct spi_config *config)
{
        return send_command_simple(dev, config, CMD_SOFT_HIZ);
}

int l64x0_hard_hiz(const struct device *const dev, struct spi_config *config)
{
        return send_command_simple(dev, config, CMD_HARD_HIZ);
}

int l64x0_get_status(const struct device *const dev, struct spi_config *config)
{
        return send_command(dev, config, CMD_GET_STATUS, 0, TX_BYTES_NONE, RX_BYTES_GET_STATUS);
}
