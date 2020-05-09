from pyb import I2C
import utime
ACCEL_CONFIG = const(0x02)
GYRO_CONFIG = const(0x03)

ACCEL_XOUT0 = const(0x3B)
ACCEL_YOUT0 = const(0x3D)
ACCEL_ZOUT0 = const(0x3F)
GYRO_XOUT0 = const(0x43)
GYRO_YOUT0 = const(0x45)
GYRO_ZOUT0 = const(0x47)

# Wake up board
addr = 0x68
i2c = I2C(2)
i2c.init(I2C.MASTER)
i2c.mem_write(0x01, addr, 0x6b)

i2c.mem_write(0b00011000, addr, ACCEL_CONFIG)
i2c.mem_write(0b00011000, addr, GYRO_CONFIG)

# helper for converting 2 bytes to int.
btoi = lambda msb, lsb: (msb << 8 | lsb) if not msb & 0x80 else -(((msb ^ 255) << 8) | (lsb ^ 255) + 1)

accel_range = 16.0
accel_rate = 2048.0
gyro_range = 2000.0
gyro_rate = 16.4
accel = [0.0] * 3
gyro = [0.0] * 3

# init buffers
adata = bytearray(6)
gdata = bytearray(6)

# Start loop
while True:
    i2c.mem_read(adata, addr, ACCEL_XOUT0)
    i2c.mem_read(gdata, addr, GYRO_XOUT0)
    accel = [btoi(adata[0], adata[1])/accel_rate, btoi(adata[2], adata[3])/accel_rate, btoi(adata[4], adata[5])/accel_rate]
    gyro = [btoi(gdata[0], gdata[1])/gyro_rate, btoi(gdata[2], gdata[3])/gyro_rate, btoi(gdata[4], gdata[5])/gyro_rate]

    print('accel: ', accel)
    print('gyro: ', gyro)
    utime.sleep_ms(100)
