#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <sys/ioctl.h>

#include <pigpio.h>

#include "CC1101.h"

static int read_status_register(const unsigned int fd, const unsigned char address, unsigned char *value){
        unsigned char rx[2] = {0, 0};
        unsigned char tx[2] = { 0xC0 | address, 0x00};
        struct spi_ioc_transfer tr;

        memset(&tr, 0, sizeof tr);

        tr.tx_buf = (unsigned long)tx;
        tr.rx_buf = (unsigned long)rx;
        tr.len = 2;
        tr.speed_hz = 1000000;
        tr.bits_per_word = 8;

        int status  = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(status > 0){
                *value = rx[1];
                fprintf(stdout, "read_status_register(0x%02X)::status=%d, rx=[0x%02X, 0x%02X]" "\n", address, status, rx[0], rx[1]);
        }else{
                perror("read_status_register::SPI_IOC_MESSAGE");
        }
        return(status);
}
#if 0
static int read_registers(int fd){
        unsigned char rx[0x30];
        unsigned char tx[0x30];
        struct spi_ioc_transfer tr;
        
        memset(&tr, 0, sizeof tr);
        memset(rx, 0, sizeof rx);
        memset(tx, 0, sizeof tx);
        tx[0] = 0xC0 | 0x00; // Burst read starting at register 0

        tr.tx_buf = (unsigned long)tx;
        tr.rx_buf = (unsigned long)rx;
        tr.len = 0x30; // Read command plus 0x2E registers
        tr.speed_hz = 1000000;
        tr.bits_per_word = 8;
        
        int status  = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(status > 0){
                fprintf(stdout, "read_registers()::status=%d, rx[]=[", status);
                int i = 0;
                while(i <= 0x2F){
                        fprintf(stdout, "0x%02X, ", rx[i]);
                        i++;
                }
                fprintf(stdout, "]" "\n");
        }
        return(status);
}
#endif

static int write_register(int fd, const unsigned char address, const unsigned char value){
        unsigned char rx[2] = {0, 0};
        unsigned char tx[2] = { 0x40 | address, value};
        struct spi_ioc_transfer tr;

        memset(&tr, 0, sizeof tr);

        tr.tx_buf = (unsigned long)tx;
        tr.rx_buf = (unsigned long)rx;
        tr.len = 2;
        tr.speed_hz = 1000000;
        tr.bits_per_word = 8;

        int status  = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(status > 0){
                fprintf(stdout, "write_register(0x%02X <- 0x%02X)::status=%d, rx=[0x%02X, 0x%02X]" "\n", address, value, status, rx[0], rx[1]);
        }else{
                perror("write_register::SPI_IOC_MESSAGE");
        }
        return(status);
}

static int read_strobe(const unsigned int fd, const unsigned char address){
        unsigned char rx[1] = {0};
        unsigned char tx[1] = { 0x80 | address};
        struct spi_ioc_transfer tr;

        memset(&tr, 0, sizeof tr);

        tr.tx_buf = (unsigned long)tx;
        tr.rx_buf = (unsigned long)rx;
        tr.len = 1;
        tr.speed_hz = 1000000;
        tr.bits_per_word = 8;

        int status  = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(status > 0){
                fprintf(stdout, "read_strobe(0x%02X)::status=%d, rx=[0x%02X]" "\n", address, status, rx[0]);
        }else{
                perror("read_strobe::SPI_IOC_MESSAGE");
        }
        return(status);
}
#if 0
static int transmit(int fd, const unsigned char *data, int len, int frequency){
        unsigned char *rx_buffer = (unsigned char *)malloc(len);
        unsigned char *tx_buffer = (unsigned char *)malloc(len);
        struct spi_ioc_transfer tr;

        memset(&tr, 0, sizeof tr);
        memset(rx_buffer, 0, len);
        memcpy(tx_buffer, data, len);

        tr.tx_buf = (unsigned long)tx_buffer;
        tr.rx_buf = (unsigned long)rx_buffer;
        tr.len = len;
        tr.speed_hz = frequency;
        tr.bits_per_word = 8;

        int status  = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(status > 0){
                fprintf(stdout, "transmit(len=%d)::status=%d" "\n", len, status);
        }else{
                perror("transmit::SPI_IOC_MESSAGE");
        }
        free(tx_buffer);
        free(rx_buffer);
        return(status);
}

static int write_fifo(int fd, const unsigned char *data, int len){
        unsigned char *rx_buffer = (unsigned char *)malloc(len + 1);
        unsigned char *tx_buffer = (unsigned char *)malloc(len + 1);
        struct spi_ioc_transfer tr;

        memset(&tr, 0, sizeof tr);
        memset(rx_buffer, 0, len + 1);
        memcpy(tx_buffer + 1, data, len);
        tx_buffer[0] = CC1101_FIFO | 0x40; // Write in burst mode

        tr.tx_buf = (unsigned long)tx_buffer;
        tr.rx_buf = (unsigned long)rx_buffer;
        tr.len = len + 1;
        tr.speed_hz = 1000000;
        tr.bits_per_word = 8;

        int status  = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(status > 0){
                fprintf(stdout, "write_fifo(len=%d)::status=%d, rx[]=[", len, status);
                int i = 0;
                while(i <= len){
                        fprintf(stdout, "0x%02X, ", rx_buffer[i]);
                        i++;
                }
                fprintf(stdout, "]" "\n" "tx[]=[");
                int j = 0;
                while(j <= len){
                        fprintf(stdout, "0x%02X, ", tx_buffer[j]);
                        j++;
                }
                fprintf(stdout, "]" "\n");
        }else{
                perror("write_fifo::SPI_IOC_MESSAGE");
        }
        free(tx_buffer);
        free(rx_buffer);
        return(status);
}

#endif

static int write_pa_table(int fd, const unsigned char *data, int len){
        unsigned char *rx_buffer = (unsigned char *)malloc(len + 1);
        unsigned char *tx_buffer = (unsigned char *)malloc(len + 1);
        struct spi_ioc_transfer tr;

        memset(&tr, 0, sizeof tr);
        memset(rx_buffer, 0, len + 1);
        memcpy(tx_buffer + 1, data, len);
        tx_buffer[0] = CC1101_PA_TABLE0 | 0x40; // Write in burst mode

        tr.tx_buf = (unsigned long)tx_buffer;
        tr.rx_buf = (unsigned long)rx_buffer;
        tr.len = len + 1;
        tr.speed_hz = 1000000;
        tr.bits_per_word = 8;

        int status  = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(status > 0){
                fprintf(stdout, "write_pa_table(len=%d)::status=%d, rx[]=[", len, status);
                int i = 0;
                while(i <= len){
                        fprintf(stdout, "0x%02X, ", rx_buffer[i]);
                        i++;
                }
                fprintf(stdout, "]" "\n" "tx[]=[");
                int j = 0;
                while(j <= len){
                        fprintf(stdout, "0x%02X, ", tx_buffer[j]);
                        j++;
                }
                fprintf(stdout, "]" "\n");
        }else{
                perror("write_pa_table::SPI_IOC_MESSAGE");
        }
        free(tx_buffer);
        free(rx_buffer);
        return(status);
}

#define GPIO (25)

int main(int argc, char *argv[]){
        int status = 0;
        if(argc < 2){
                fprintf(stdout, "Usage %s <control_spi>" "\n", argv[0]);
                return(1);
        }else{
                int cr = gpioInitialise();
                fprintf(stderr, "gpioInitialise() => %d" "\n", cr);
                fprintf(stderr, "Using device [%s] as control and GPIO%d as transmit" "\n", argv[1], GPIO);
                int control_fd = open(argv[1], O_RDWR);
                if(control_fd < 0){
                        perror(argv[1]);
                        return(2);
                }else{
                        unsigned char partnum = 0;
                        unsigned char version = 0;

                        status = read_strobe(control_fd, CC1101_SRES);

                        status = read_status_register(control_fd, 0x30, &partnum);
                        status = read_status_register(control_fd, 0x31, &version);

                        gpioSetMode(GPIO, PI_INPUT);
                        gpioSetPullUpDown(GPIO, PI_PUD_DOWN);

                        /* Address Config = No address check */
                        /* Base Frequency = 433.454468 */
                        /* CRC Autoflush = false */
                        /* CRC Enable = false */
                        /* Carrier Frequency = 433.454468 */
                        /* Channel Number = 0 */
                        /* Channel Spacing = 199.951172 */
                        /* Data Format = Asynchronous serial mode */
                        /* Data Rate = 3.12424 */
                        /* Deviation = 47.607422 */
                        /* Device Address = 0 */
                        /* Manchester Enable = false */
                        /* Modulated = false */
                        /* Modulation Format = ASK/OOK */
                        /* PA Ramping = false */
                        /* Packet Length = 255 */
                        /* Packet Length Mode = Infinite packet length mode */
                        /* Preamble Count = 4 */
                        /* RX Filter BW = 203.125000 */
                        /* Sync Word Qualifier Mode = No preamble/sync */
                        /* TX Power = 0 */
                        /* Whitening = false */

                        /* PA table */
#ifdef __PA_RAMPING__
#define PA_TABLE {0x00,0x12,0x0e,0x34,0x60,0xc5,0xc1,0xC0} // for PA Ramping
#else
#define PA_TABLE {0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00} // NO PA Ramping (-30dBm:0x12, -20dBm:0x0E, -15dBm:0x1D, -10dBm:0x34, 0dBm:0x60, 5dBm:0x84, 7dBm:0xC8, 10dBm:0xC0
#endif
                        status = write_register(control_fd, CC1101_IOCFG2, 0x0B);  //GDO2 Output Pin Configuration
                        status = write_register(control_fd, CC1101_IOCFG0, 0x0D);  //GDO0 Output Pin Configuration
                        status = write_register(control_fd, CC1101_FIFOTHR, 0x47); //RX FIFO and TX FIFO Thresholds
                        status = write_register(control_fd, CC1101_PKTCTRL0, 0x32);//Packet Automation Control
                        status = write_register(control_fd, CC1101_FREQ2, 0x10);   //Frequency Control Word, High Byte
                        status = write_register(control_fd, CC1101_FREQ1, 0xAB);   //Frequency Control Word, Middle Byte
                        status = write_register(control_fd, CC1101_FREQ0, 0xDC);   //Frequency Control Word, Low Byte
                        status = write_register(control_fd, CC1101_MDMCFG4, 0x86); //Modem Configuration
                        status = write_register(control_fd, CC1101_MDMCFG3, 0xF8); //Modem Configuration
                        status = write_register(control_fd, CC1101_MDMCFG2, 0x30); //Modem Configuration

#ifdef __PA_RAMPING__
                        status = write_register(control_fd, CC1101_FREND0, 0x17);  //Front End TX Configuration
#else
                        status = write_register(control_fd, CC1101_FREND0, 0x11);  //Front End TX Configuration
#endif

                        status = write_register(control_fd, CC1101_FSCAL3, 0xE9);  //Frequency Synthesizer Calibration
                        status = write_register(control_fd, CC1101_FSCAL2, 0x2A);  //Frequency Synthesizer Calibration
                        status = write_register(control_fd, CC1101_FSCAL1, 0x00);  //Frequency Synthesizer Calibration
                        status = write_register(control_fd, CC1101_FSCAL0, 0x1F);  //Frequency Synthesizer Calibration
                        status = write_register(control_fd, CC1101_TEST2, 0x81);   //Various Test Settings
                        status = write_register(control_fd, CC1101_TEST1, 0x35);   //Various Test Settings
                        status = write_register(control_fd, CC1101_TEST0, 0x09);   //Various Test Settings
                        
                        const unsigned char pa_table[]  = PA_TABLE;

#define WRITE_PA_TABLE(TABLE) status = write_pa_table(control_fd, TABLE, sizeof(TABLE))

                        WRITE_PA_TABLE(pa_table);

                        status = read_strobe(control_fd, CC1101_SCAL);
                        status = read_strobe(control_fd, CC1101_STX);

                        // printf("Ready to toggle GDO0, press Enter" "\n");
                        // getchar();

                        // fprintf(stderr, "Using pigpio waveform" "\n");
                        // Use pigpio waveform capability
                        // First example: create a square waveform with 2*320µs period
                        gpioWrite(GPIO, 0);
                        gpioSetMode(GPIO, PI_OUTPUT);
                        gpioWaveClear();
                        int max_pulses = 6000; // hard limit
                        int actual_pulses = 0;
                        // Read pulses from stdin
                        // lines start with a space else they are ignored
                        // the second character is either '0' or '1'
                        // it is followed by a colon and the µs duration of the pulse
                        // Here is a snippet of the input from smoove2wave
                        //  1:10000us
                        //  0:88000us
                        //  1:2560us
                        //  0:2560us
                        //  1:2560us
                        //  0:2560us
                        //  1:4800us
                        //  0:640us
                        //  0:640us
                        //  1:640us
                        //
                        gpioPulse_t *pulses = (gpioPulse_t*)malloc(max_pulses * sizeof(gpioPulse_t));
                        gpioPulse_t *p = pulses;

                        char line[256];
                        while(fgets(line, sizeof(line), stdin)){
                                if((actual_pulses < max_pulses) && (' ' == line[0]) && (':' == line[2])){
                                        int gpio_on = 0;
                                        int gpio_off = 0;
                                        if('0' == line[1]){
                                                gpio_off = (1 << GPIO);
                                        }else{
                                                gpio_on = (1 << GPIO);
                                        }
                                        int duration = strtol(line + 3, NULL, 10);
                                        *p = (gpioPulse_t){ .gpioOn = gpio_on, .gpioOff = gpio_off, .usDelay = duration };
                                        // fprintf(stderr, "Add pulse gpioOn=0x%08X, gpioOff=0x%08X, usDelay=%d" "\n", p->gpioOn, p->gpioOff, p->usDelay);
                                        actual_pulses++;
                                        p++;
                                }
                        }
                        gpioWaveAddNew();
                        gpioWaveAddGeneric(actual_pulses, pulses);
                        int wave_id = gpioWaveCreate();
                        if(wave_id < 0){
                                fprintf(stderr, "gpioWaveCreate() failed: %d" "\n", wave_id);
                        }else{
                                gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);
                                sleep(1);
                                gpioWaveTxStop();
                        }
                        free(pulses);

                        // printf("Press Enter to reset CC1101" "\n");
                        // getchar();

                        // Ensure GDO0 is not driven before resetting CC1101
                        gpioSetMode(GPIO, PI_INPUT);
                        gpioSetPullUpDown(GPIO, PI_PUD_OFF);
                        status = read_strobe(control_fd, CC1101_SRES);


                        close(control_fd);
                }
                gpioTerminate();
        }
        return((status < 0)?1:0);
}



