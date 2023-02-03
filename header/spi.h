
#ifndef spi_h
#define spi_h

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])


static const char *DEVICE ="/dev/spidev0.0";
static uint8_t MODE = SPI_MODE_0;
static uint8_t BITS =8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY=5;

//SPI_MODE_0 (0,0)
//CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge//ensure all settings are correct for the ADC

static int prepare(int fd){
    if (ioctl(fd, SPI_IOC_WR_MODE,&MODE)==-1){
        perror("can't set MODE");
        return -1;
    }
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS)==-1){
        perror("can't set number of bits");
        return -1;
    }
    //return (RD) or assign (WR) the maximum SPI transferspeed, in Hz
    if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK)==-1){
        perror("can't set write clock");
        return -1;
    }
    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK)==-1){
        perror("can't set read clock");
        return -1;
    }
    return 0;
}

//sgl/dif = 0, d2=d1=d0=0
uint8_t control_bits_differential(uint8_t channel){
    return (channel & 7)<<4;
}

//sgl/dif =1, d2=d1=d0=0
uint8_t control_bits(uint8_t channel){
    return 0x8 | control_bits_differential(channel);
}

//given a prep'd descriptor, and an adc channel, fetch raw adc value for given channel
int readadc(int fd, uint8_t channel){
    uint8_t tx[] = {1, control_bits(channel),0};
    uint8_t rx[3];
    
    struct spi_ioc_transfer tr={
        .tx_buf = (unsigned long)tx, //transmit from datta
        .rx_buf = (unsigned long)rx, //receive into data
        .len = ARRAY_SIZE(tx),
        .delay_usecs = DELAY,
        .speed_hz = CLOCK,
        .bits_per_word=BITS,
    };
    if(ioctl(fd, SPI_IOC_MESSAGE(1), &tr)==1){
        perror("IO ERROR");
        abort();
    }
    return ((rx[1]<<8) & 0x300 | rx[2] & 0xFF);
}

#endif /* spi_h */
