#include "./GPIO.h"

#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

#define BTN 16

int serv_sock,clnt_sock=-1,clnt2_sock=-1;
struct sockaddr_in serv_addr, clnt_addr, clnt2_addr;
socklen_t clnt_addr_size, clnt2_addr_size;

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}


int count = 0;
void *clientthd() {
    char *msg = (char*)malloc(sizeof(char));
    char rcv1[1];
    char sensormessage[1];
    char *two = "2";
    snprintf(sensormessage, 1, "%s", two);
    
    //check button
    int state = 1;
    int str_len;
    
    if(clnt_sock<0) {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock =accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
        if(clnt_sock == -1)
            error_handling("accept() error");
    }
        
    while(1) {
        if (GPIOWrite(BTN, OUT) == -1){
            exit(0);
        }
        
        state = GPIORead(BTN);
        int len = 0;
        if(state == 0) {
            sprintf(msg, "%d", state);
            printf("send 0 : %s\n", msg);
            write(clnt_sock, msg, sizeof(msg));
            
            int j=0;
            while(j<10){
                read(clnt_sock, rcv1, sizeof(rcv1));
                if(strncmp(rcv1,"2",1) == 0) {
                    printf("meessage from sever to client1 : %s\n", rcv1);
                    count++;
                    sleep(10-(j+1));
                    j = 10;
                    break;
                }
                j++;
                sleep(1);
            }
        
            if(j == 10) {
                free(msg);
                close(clnt_sock);
                break;
            }
        }
    }
}

void *clientthd2() {
    char *msg = (char*)malloc(sizeof(char));
    char rcv2[1];
    char sensormessage[1];
    char *two = "2";
    snprintf(sensormessage, 1, "%s", two);
    
    //check button
    int state = 1;
    int str_len;
    
    if(clnt2_sock<0) {
        clnt2_addr_size = sizeof(clnt2_addr);
        clnt2_sock =accept(serv_sock, (struct sockaddr*) &clnt2_addr, &clnt2_addr_size);
        if(clnt2_sock == -1)
            error_handling("accept() error");
    }
        
    while(1) {
        if (GPIOWrite(BTN, OUT) == -1){
            exit(0);
        }
        
        state = GPIORead(BTN);
        int len = 0;
        if(state == 0) {
            sprintf(msg, "%d", state);
            printf("send 0 : %s\n", msg);
            len = write(clnt2_sock, msg, sizeof(msg));
            printf("len %d\n", len);
            
            int j=0;
            while(j<10){
                read(clnt2_sock, rcv2, sizeof(rcv2));
                if(strncmp(rcv2,"2",1) == 0) {
                    printf("meessage from sever to client2 : %s\n", rcv2);
                    count++;
                    sleep(10-(j+1));
                    j = 10;
                    break;
                }
                j++;
                sleep(1);
            }
        
            if(j == 10) {
                free(msg);
                close(clnt2_sock);
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    pthread_t p_thread[2];
    int thr_id;
    int status;
    
    char piccmd1[] = "sudo raspistill -w 600 -h 480 -o /home/pi/Pictures/pic1.jpg";
    char piccmd2 [] = "sudo raspistill -w 640 -h 480 -o /home/pi/Pictures/pic2.jpg";
    char piccmd3 [] = "sudo raspistill -w 640 -h 480 -o /home/pi/Pictures/pic3.jpg";
    char preview[] = "sudo raspistill -p -f";
    char print[] = "lp -d Canon_MG2400_series_Printer_via_VNC_from_DESKTOP-LMCA9KU /home/pi/Desktop/final/final.jpg";
    char addimg[] = "python img.py";
    
    if(argc!=2) {
        printf("Usage : %s <port>\n", argv[0]);
    }
    
    //server socket set up
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    
    if(GPIOExport(BTN) == -1){
        return 1;
    }
    
    if(GPIODirection(BTN, OUT) == -1){
        return 2;
    }

    if(listen(serv_sock,5) == -1)
        error_handling("listen() errorr");
    while(1){
        thr_id = pthread_create(&p_thread[0], NULL, clientthd, NULL);
        if (thr_id < 0) {
            perror("thread create error : ");
            exit(0);
        }
        
        thr_id = pthread_create(&p_thread[1], NULL, clientthd2, NULL);
        if (thr_id < 0) {
            perror("thread create error : ");
            exit(0);
        }
        
        pthread_join(p_thread[0], (void**)&status);
        pthread_join(p_thread[1], (void**)&status);
        
        printf("count(count = 2 -> camera on) : %d\n", count);
        if(count == 2){
            system(piccmd1);
            sleep(5);
            system(piccmd2);
            sleep(5);
            system(piccmd3);
            system(addimg);
            system(print);
        }
        count = 0;
        //after setting, client send message to server, then start camera
        
        if(count < 2) {
            break;
        }
    }
     if (GPIOUnexport(BTN) == -1 ){
            return 4;
    }

    close(serv_sock);

    return 0;
}
