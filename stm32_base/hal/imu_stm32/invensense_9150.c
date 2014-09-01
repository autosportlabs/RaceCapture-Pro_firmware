#include <i2c_device_stm32.h>
#include <invensense_9150.h>
#include <stdint.h>
#include <stddef.h>

static struct  {
	struct i2c_dev *i2c;
	uint8_t addr;
} is9150_dev;

static int is9150_readreg(uint8_t reg_addr, uint8_t *reg_val)
{
	int res = 0;
	res = i2c_read_reg8(is9150_dev.i2c, is9150_dev.addr,
			    reg_addr, reg_val);
	return res;
}

static int is9150_writereg(uint8_t reg_addr, uint8_t reg_val)
{
	int res = 0;
	res = i2c_write_reg8(is9150_dev.i2c, is9150_dev.addr,
			     reg_addr, reg_val);
	return res;
}

static int is9150_write_reg_bits(uint8_t reg_addr, size_t bit_pos,
			   size_t num_bits, uint8_t bit_val)
{
	int res = 0;
	res = i2c_write_reg_bits(is9150_dev.i2c, is9150_dev.addr,
				 reg_addr, bit_pos, num_bits, bit_val);
	return res;
}

static int is9150_read_reg_bits(uint8_t reg_addr, size_t bit_pos,
				size_t num_bits, uint8_t *bit_val)
{
	int res = 0;
	res = i2c_read_reg_bits(is9150_dev.i2c, is9150_dev.addr,
				reg_addr, bit_pos, num_bits, bit_val);
	return res;
}

static int is9150_read_reg_block(uint8_t start_addr, size_t len,
				 uint8_t *rx_buf)
{
	int res = 0;
	res = i2c_read_mem_block(is9150_dev.i2c, is9150_dev.addr,
				 start_addr, rx_buf,
				 len);
	return res;
}

int is9150_init(struct i2c_dev *dev, uint8_t addr)
{
	int res;
	uint8_t reg = 0x00;

	is9150_dev.i2c = dev;
	is9150_dev.addr = addr;

	/* Read the 'who am I' register to make sure that the device
	 * is out there and alive */
	res = is9150_readreg(IS_REG_WHOAMI, &reg);
	if (res)
		return IS_9150_ERR_INIT;
	else if (reg != IS_WHOAMI_DEFAULT)
		return IS_9150_ERR_INIT;
	
	/* Set the device clock source to internal PLL + GYRO_X sync */
	/* NOTE: This has shown to be more accurate than the PLL
	 * alone according to datasheet */
	res = is9150_write_reg_bits(IS_REG_PWR_MGMT_1, IS_CLOCK_POS,
				    IS_CLOCK_NUM_BITS, IS_CLOCK_GYRO_X);
	if (res)
		return IS_9150_ERR_INIT;

	/* Set the Gyro range to 1000* per second */
	res = is9150_write_reg_bits(IS_REG_GYRO_CONFIG, IS_GYRO_SCALE_POS,
				    IS_GYRO_NUM_BITS, IS_GYRO_SCALE_1000);
	if (res)
		return IS_9150_ERR_INIT;

	/* Set the accelerometer range to 4G per second */
	res = is9150_write_reg_bits(IS_REG_ACCEL_CONFIG, IS_ACCEL_SCALE_POS,
				    IS_ACCEL_NUM_BITS, IS_ACCEL_SCALE_4G);
	if (res)
		return IS_9150_ERR_INIT;

	/* Clear the sleep bit in the power management register, this
	 * beings sampling of the internal devices */
	res = is9150_write_reg_bits(IS_REG_PWR_MGMT_1, IS_POWER_SLEEP_POS,
				    1, 0);
	if (res)
		return IS_9150_ERR_INIT;

	return 0;
}

int is9150_read_gyro(struct is9150_gyro_data *data)
{
	int res = 0;
	uint8_t reg_res[6] = {0};
	res = is9150_read_reg_block(IS_GYRO_MEAS_START, IS_GYRO_MEAS_COUNT,
				    reg_res);

	data->gyro_x = reg_res[0] << 8 | reg_res[1];
	data->gyro_y = reg_res[2] << 8 | reg_res[3];
	data->gyro_z = reg_res[4] << 8 | reg_res[5];
	return res;
}

int is9150_read_accel(struct is9150_accel_data *data)
{
	int res = 0;
	uint8_t reg_res[6] = {0};
	res = is9150_read_reg_block(IS_ACCEL_MEAS_START, IS_ACCEL_MEAS_COUNT,
				    reg_res);

	data->accel_x = reg_res[0] << 8 | reg_res[1];
	data->accel_y = reg_res[2] << 8 | reg_res[3];
	data->accel_z = reg_res[4] << 8 | reg_res[5];
	return res;
}

int is9150_read_temp(uint16_t *temp)
{
	int res = 0;
	uint8_t reg_res[2] = {0};
	res = is9150_read_reg_block(IS_TEMP_MEAS_START, IS_TEMP_MEAS_COUNT,
				    reg_res);

	*temp = reg_res[0] << 8 | reg_res[1];
	return res;	
}

int is9150_read_all_sensors(struct is9150_all_sensor_data *data)
{
	int res = 0;
	uint8_t reg_res[14] = {0};
	res = is9150_read_reg_block(IS_ACCEL_MEAS_START,
				    IS_ACCEL_MEAS_COUNT + IS_GYRO_MEAS_COUNT + IS_TEMP_MEAS_COUNT,
				    reg_res);

	data->accel.accel_x = reg_res[0] << 8 | reg_res[1];
	data->accel.accel_y = reg_res[2] << 8 | reg_res[3];
	data->accel.accel_z = reg_res[4] << 8 | reg_res[5];

	data->temp = reg_res[6] << 8 | reg_res[7];

	data->gyro.gyro_x = reg_res[8] << 8 | reg_res[9];
	data->gyro.gyro_y = reg_res[10] << 8 | reg_res[11];
	data->gyro.gyro_z = reg_res[12] << 8 | reg_res[13];
	
	return res;
}
