//rasberrypi: master, adc: slave

#include "./pwm.h"
#include "./spi.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#define IN 0
#define OUT 1
#define PWM 2
#define LOW 0
#define HIGH 1
#define PIN 20
#define POUT 21
#define STLIGHT1 300
#define STLIGHT2 600

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n', stderr);
    exit(1);
    }
   
int main(int argc, char **argv){
    
    int ch0sum=0;
    int ch7sum=0;
    int ch0avg=0;
    int ch7avg=0;
    int ch0=0;
    int ch7=0;
    
    int sock;
    struct sockaddr_in serv_addr;
    char msg[2];
    char send[2]="2";
    char bt[2]="0";
    int str_len;
    
    
    if(argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error_handling("socket() error");
        
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    
    if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
            error_handling("connect() error");
            
    while(1){
        
        str_len = read(sock,msg,sizeof(msg));
        if(str_len == -1)
            error_handling("read() error");
        printf("receive message from server: %s\n",msg);
        if (strncmp(bt,msg,1)==0)
            break;
        else {
            continue;
            }
    }
    
    int fd = open(DEVICE, O_RDWR);
    if (fd<=0){
        printf("device %s not found \n", DEVICE);
        return -1;}

    
    if (prepare(fd)==-1){
        return -1;
    }

    PWMExport(0);
    PWMWritePeriod(0,20000000);
    PWMWriteDutyCycle(0, 0);
    PWMEnable(0);
    
    int i = 0;
    
    while(i<5){
        
        ch0 = readadc(fd, 0);
        ch7 = readadc(fd, 7);
        
        ch0sum += ch0;
        ch7sum += ch7;
        
        i++;
        sleep(1);
    }
    
    ch0avg=ch0sum/5;
    ch7avg=ch7sum/5;
    
    printf("ch0avg is %d\n",ch0avg);
    printf("ch1avg is %d\n", ch7avg);
    
    if(ch0avg < STLIGHT1 && ch7avg < STLIGHT1){
        PWMWriteDutyCycle(0,20000000);
    }
            
    else if (ch0avg > STLIGHT2 && ch7avg > STLIGHT2){
        PWMWriteDutyCycle(0,0);
    }
    else if (STLIGHT1 < ch0avg < STLIGHT2 && STLIGHT1 < ch7avg < STLIGHT2)
    {
        PWMWriteDutyCycle(0,1000000);
    }
    else{
        
        int chavg=(ch0avg+ch7avg)/2;
        if(chavg < STLIGHT1){
            PWMWriteDutyCycle(0,20000000);
            }
        else if(chavg > STLIGHT2){
            PWMWriteDutyCycle(0,0);
            }
        else{
            PWMWriteDutyCycle(0,1000000);
            }
        
        }
    
    write(sock,send,sizeof(send));
    
    close(sock);
    
    return 0;
}
