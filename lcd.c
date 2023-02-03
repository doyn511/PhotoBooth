
#include <arpa/inet.h>
#include <sys/socket.h>
#include "./GPIO.h"
#include "./lcd.h"

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define POUT 23
#define PIN 24
#define LCD_LINE_1  0X80
#define LCD_LINE_2  0xC0
// #define LCD_LINE_3  0x94
// #define LCD_LINE_4  0xD4

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n',stderr);
    exit(1);
}

double distance = 0;



int main (int argc, char *argv[]) {
    clock_t start_t, end_t;
    double time;
    double target_distance = 95.0;
    
    int sock;
    struct sockaddr_in serv_addr;
    char send_msg[1] = "2";
    char on[1] = "0";
    int str_len;
    
    if (-1 == GPIOExport(PIN) || -1 == GPIOExport(POUT))
        return(1);
        
    if (-1 == GPIODirection(PIN,IN) || -1 == GPIODirection(POUT,OUT))
        return(2);
        
    if (-1 == GPIOWrite(POUT,0))
        return(3);
    
    bus_open();
    lcd_init();
    
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
      
    while(1) {
      char *msg = (char*)malloc(sizeof(char));
    
      read(sock, msg, sizeof(msg));
      
      int sec_count = 0;
      
      if(strcmp(msg,on) == 0) {
          printf("message from server : %d\n", msg);
          while (sec_count < 10) {
              int send_count = 0;
          
              if(-1 == GPIOWrite(POUT,1)) {
                  printf("gpio write trigger err\n");
                  exit(0);
              }
        
              GPIOWrite(POUT,0);
        
              while(GPIORead(PIN) == 0) {
                  start_t = clock();
              }
              
              while(GPIORead(PIN) == 1) {
                  end_t = clock();
              }
                
              time = (double)(end_t-start_t)/CLOCKS_PER_SEC;
              distance = time/2*34000;
    
              if(distance > 900)
                  distance = 900;
          
              printf("distance: %2.lf cm\n", distance);
        
              if (distance > target_distance) {
                  lcd_string("!!!WARNING!!!", LCD_LINE_1);
                  lcd_string("TOO FAR",LCD_LINE_2);
                  sleep(1);
                  printf("far\n");
                  sec_count++;
              }
              
              if (distance <= target_distance) {
                  lcd_string("HELLO", LCD_LINE_1);
                  lcd_string("SETTING....",LCD_LINE_2);
        
                  send_count++;
                  sec_count++;
                  printf("setting (1/3)\n");
                  sleep(1);
                  
                  if(-1 == GPIOWrite(POUT,1)) {
                      printf("gpio write trigger err\n");
                      exit(0);
                  }
            
                  GPIOWrite(POUT,0);
            
                  while(GPIORead(PIN) == 0) {
                      start_t = clock();
                  }
                  
                  while(GPIORead(PIN) == 1) {
                      end_t = clock();
                  }
                    
                  time = (double)(end_t-start_t)/CLOCKS_PER_SEC;
                  distance = time/2*34000;
        
                  if(distance > 900)
                      distance = 900;
                  
                  if (distance <= target_distance) {
                      send_count++;
                      sec_count++;
                      printf("setting (2/3)\n");
                      sleep(1);
                      
                      if(-1 == GPIOWrite(POUT,1)) {
                          printf("gpio write trigger err\n");
                          exit(0);
                      }
                
                      GPIOWrite(POUT,0);
                
                      while(GPIORead(PIN) == 0) {
                          start_t = clock();
                      }
                      
                      while(GPIORead(PIN) == 1) {
                          end_t = clock();
                      }
                        
                      time = (double)(end_t-start_t)/CLOCKS_PER_SEC;
                      distance = time/2*34000;
            
                      if(distance > 900)
                          distance = 900;
      
                      if (distance <= target_distance) {
                          send_count++;
                          sec_count++;
                          printf("setting (3/3)\n");
                          write(sock, send_msg, sizeof(send_msg));
                          printf("send message to sever : %s\n", send_msg);
                          sec_count = 10-(sec_count+1);
                          sleep(sec_count);
                          sec_count = 10;
                      }
                  }
              }
          }
          free(msg);
          lcd_string("BYE", LCD_LINE_1);
          lcd_string("BYE!!!!!",LCD_LINE_2);
          close(sock);
          sleep(3);
          lcd_init();
          if(-1 == GPIOUnexport(PIN) || -1 == GPIOUnexport(POUT))
            return(4);
    
          return 0;
      }
    }
}

