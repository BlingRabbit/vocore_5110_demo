#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

//quick demo for nokia 5110
//for further questions please check spidev_test.c from linux kernel
//GreenWire-Elektronik, 2015

#define SPI_WRITE 1
#define SPI_READ 2


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *gpio_dc_dir = "/sys/class/gpio/gpio0/direction";
static const char *gpio_dc_val = "/sys/class/gpio/gpio0/value";
static const char *gpio_rst_dir = "/sys/class/gpio/gpio22/direction";
static const char *gpio_rst_val = "/sys/class/gpio/gpio22/value";
static const char *device = "/dev/spidev32766.1";
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 1000000;
static uint16_t delay = 1;



static int transfer(int direction, int fd, uint8_t value)
{
	int ret;

	uint8_t tx[1] = {};
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };

	tx[0] = value;
		
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.len = 1,
		.delay_usecs = delay,
		.speed_hz = speed,
		.cs_change = 1,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
	{
		printf("can't send spi message %i %i\n", ret, tr.len);
		return -1;
	}
	return 0;

}

int main() {

	int fd;
	int fd_dc_dir;
	int fd_dc_val;
	int fd_rst_dir;
	int fd_rst_val;
	int ret;
	int i;
		
	
struct timespec tim, tim2;
tim.tv_sec = 0;
tim.tv_nsec = 10000000;


	fd = open(device, O_RDWR);
	fd_dc_dir = open(gpio_dc_dir, O_WRONLY);
	fd_dc_val = open(gpio_dc_val, O_WRONLY);
	
	fd_rst_dir = open(gpio_rst_dir, O_WRONLY);
	fd_rst_val = open(gpio_rst_val, O_WRONLY);

	if (fd < 0)
	{
		printf("can't open device");
		return -1;
	}
	
	if (fd_dc_dir < 0)
	{
		printf("can't open D/C# gpio direction file");
		return -1;
	}
	
	if (fd_dc_val < 0)
	{
		printf("can't open D/C# gpio value file");
		return -1;
	}
	
	if (fd_rst_dir < 0)
	{
		printf("can't open reset gpio direction file");
		return -1;
	}
	
	if (fd_rst_val < 0)
	{
		printf("can't open reset gpio value file");
		return -1;
	}
	
	write(fd_dc_dir, "out", 3);
	write(fd_dc_val, "0", 1);
	
	write(fd_rst_dir, "out", 3);
	write(fd_rst_val, "1", 1);
		
	if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1)
	{
		return -1;
	}
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1)
	{
		return -1;
	}
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1)
	{
		return -1;
	}
	
	//RESET
	write(fd_rst_val, "0", 1);
	nanosleep(&tim, &tim2);
	write(fd_rst_val, "1", 1);
	
	transfer(SPI_WRITE, fd, 0x20); //chip is active, H=0
	transfer(SPI_WRITE, fd, 0x0C); //Display normal mode
	transfer(SPI_WRITE, fd, 0x41); //V-Addr 1
	transfer(SPI_WRITE, fd, 0x21); //H=1
	transfer(SPI_WRITE, fd, 0x9F); //Vop=0x1Fh
	transfer(SPI_WRITE, fd, 0x13); //BIAS 3
	transfer(SPI_WRITE, fd, 0x01); //TC 01
	
	
	write(fd_dc_val, "1", 1);
	for (i=0; i<504; i++)
		transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x0E); //G
	transfer(SPI_WRITE, fd, 0x15);
	transfer(SPI_WRITE, fd, 0x1D);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //R
	transfer(SPI_WRITE, fd, 0x0D);
	transfer(SPI_WRITE, fd, 0x17);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //E
	transfer(SPI_WRITE, fd, 0x15);
	transfer(SPI_WRITE, fd, 0x11);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //E
	transfer(SPI_WRITE, fd, 0x15);
	transfer(SPI_WRITE, fd, 0x11);	
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //N
	transfer(SPI_WRITE, fd, 0x0C);
	transfer(SPI_WRITE, fd, 0x1F);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x0F); //W
	transfer(SPI_WRITE, fd, 0x18);
	transfer(SPI_WRITE, fd, 0x0E);
	transfer(SPI_WRITE, fd, 0x18);
	transfer(SPI_WRITE, fd, 0x0F);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //I
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //R
	transfer(SPI_WRITE, fd, 0x0D);
	transfer(SPI_WRITE, fd, 0x17);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //E
	transfer(SPI_WRITE, fd, 0x15);
	transfer(SPI_WRITE, fd, 0x11);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x04);
	transfer(SPI_WRITE, fd, 0x04);
	transfer(SPI_WRITE, fd, 0x04);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //E
	transfer(SPI_WRITE, fd, 0x15);
	transfer(SPI_WRITE, fd, 0x11);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //L
	transfer(SPI_WRITE, fd, 0x10);
	transfer(SPI_WRITE, fd, 0x10);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //E
	transfer(SPI_WRITE, fd, 0x15);
	transfer(SPI_WRITE, fd, 0x11);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //K
	transfer(SPI_WRITE, fd, 0x0A);
	transfer(SPI_WRITE, fd, 0x11);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x01); //T
	transfer(SPI_WRITE, fd, 0x1F);
	transfer(SPI_WRITE, fd, 0x01);

	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //R
	transfer(SPI_WRITE, fd, 0x0D);
	transfer(SPI_WRITE, fd, 0x17);	
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //O
	transfer(SPI_WRITE, fd, 0x11);
	transfer(SPI_WRITE, fd, 0x1F);
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //N
	transfer(SPI_WRITE, fd, 0x0C);
	transfer(SPI_WRITE, fd, 0x1F);	
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //I
	
	transfer(SPI_WRITE, fd, 0x00);
	
	transfer(SPI_WRITE, fd, 0x1F); //K
	transfer(SPI_WRITE, fd, 0x0A);
	transfer(SPI_WRITE, fd, 0x11);
	


	close(fd);
	close(fd_dc_val);
	close(fd_dc_dir);
	close(fd_rst_val);
	close(fd_rst_dir);
	return 0;
}
