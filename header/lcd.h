#ifndef lcd_h
#define lcd_h

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_ADDR 0x27

#define LCD_WIDTH 16
#define LCD_CHR 1 // sending data
#define LCD_CMD 0 // sending command

#define LCD_BACKLIGHT 0x08 // ON
//#define LCD_BACKLIGHT  0x00 //OFF

#define ENABLE 0b00000100

#define E_PULSE 0.0005
#define E_DELAY 0.0005

int file_i2c;
int length;
unsigned char buffer[60] = {0};
unsigned char text_buffer[15] = {0};

void bus_open() {
    char *filename = (char *)"/dev/i2c-1";
    if ((file_i2c = open(filename, O_RDWR)) < 0) {
        printf("Failed to open i2c bus");
        return;
    }

    if (ioctl(file_i2c, I2C_SLAVE, I2C_ADDR) < 0) {
        printf("Failed to acquire bus access and / or talk to slave. \n");
        return;
    }
}

void bus_read() {
    length = 4;
    if (read(file_i2c, buffer, length) != length) {
        printf("Failed to read from the i2c bus.\n");
    }
    else {
        printf("Data read: %s\n", buffer);
    }
}

void bus_write_bit(int write_bit) {
    buffer[0] = write_bit;
    length = 1;
    if (write(file_i2c, buffer, length) != length) {
        printf("Failed to write to the i2c bus.\n");
    }
}

void bus_write_text(char character) {
    text_buffer[0] = character;
    length = 1;
    if (write(file_i2c, text_buffer, length) != length) {
        printf("Failed to write to the i2c bus.\n");
    }
}
void lcd_toggle_enable(int bits) {
    sleep(E_DELAY);
    bus_write_bit(bits | ENABLE);

    sleep(E_PULSE);
    bus_write_bit(bits & ~ENABLE);

    sleep(E_DELAY);
}

void lcd_byte(char bits, int mode) {
    char bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    char bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    bus_write_text(bits_high);
    lcd_toggle_enable(bits_high);

    bus_write_text(bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_init()
{
    lcd_byte(0x33, LCD_CMD);
    lcd_byte(0x32, LCD_CMD);
    lcd_byte(0x06, LCD_CMD);
    lcd_byte(0x0C, LCD_CMD);
    lcd_byte(0x28, LCD_CMD);
    lcd_byte(0x01, LCD_CMD);
    sleep(E_DELAY);
}

void lcd_string(char *message, char line) {
    lcd_byte(line, LCD_CMD);

    for (int i = 0; i < strlen(message); i++)
    {
        lcd_byte(message[i], LCD_CHR);
    }
    for (int i = strlen(message); i < LCD_WIDTH; i++)
    {
        lcd_byte(' ', LCD_CHR);
    }
}

#endif /* lcd_h */
