#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <termios.h>

char getch();

void stampakill(int sockfd);
void stampamappa( char (*map)[21], int a);
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void connectusr(char buffer[256], char str[230], int sockfd);
void aggiornamappa(char buffer[256], int disp);
int main(int argc, char *argv[])
{
    int sockfd, portno, n, disp, end=0; int X, Y, tx, ty; int armed=0; int count, ind, aa, bb; int ack=0; int shootx=-1, shooty=-1; 
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char str [230];
    char inp;
    char x,y;
    char buf[10];
    char ch, cc, sh;
    char buffer[256];
    char map[10][21];
    int i, j;
    for(i=0; i<21; i++) {
	for (j=0; j<10; j++) 
		
			if(i==20) map[j][i]='\n'; else map[j][i]='?';
		
    }
    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    write(STDOUT_FILENO, "how to play:\n\n\tw up\na left\t\td right\n\ts down\n\nq shoot (if armed)\ne exit\nc show #kill\n\n", sizeof(char)*86);
    connectusr(buffer, str, sockfd);
    write(STDOUT_FILENO, "user connected.\n", sizeof(char)*17);
    sleep(1);
    system("clear");
    
    
    n = read(sockfd, buf, sizeof(buf));
    X=atoi(buf);
    n = read(sockfd, buf, sizeof(buf));
    Y=atoi(buf);
    map[X][Y]='U';
    armed=0;
    while(end == 0){
	stampamappa(map, ack);
	ack=0;
   	ch=' ';
	while(ch!='w'&&ch!='a'&&ch!='s'&&ch!='d'&&ch!='q'&&ch!='e'&&ch!='c')ch=getch();
	tx=X; ty=Y;
	if(ch=='w') tx=tx-1; else if (ch=='s') tx=tx+1; else if (ch=='a') ty=ty-1; else if (ch=='d') ty=ty+1;
	if(ch!='q'&&ch!='e'&&ch!='c') {
		sh=ch;
		n = write(sockfd, &ch, sizeof(char)); 
		count=0;
		aa=-1; bb=-1;
		ind=1;
		if(armed==1) ind=4;
		shootx=-1; shooty=-1;
	    	while (count<ind)
		{	
			
			n = read(sockfd, &cc, sizeof(char));
			
			if (n < 0) 
				error("ERROR reading from socket");
			if (cc=='K') end=4;
			else if(cc=='L') end=2;
			else if (count==0){
			    	
				  
				if (cc=='T') end=1; 	
				else if (cc==' '||cc=='A') {
				
					map[X][Y]=' ';
					aa=tx; bb=ty;
					
					if(cc=='A') {
						armed=1;
						ack=1;
					}
					
				}
				else if (cc=='O') map[tx][ty]='O';
				else if (cc>='0'&&cc<='9') { 
					map[tx][ty]='N';
					shootx=tx; shooty=ty;
				}
				else if(cc!='!') map[tx][ty]=cc;

			
			}
			else{ 
					if(cc>='0'&&cc<='9') {
						map[tx][ty]='N';
						if(shootx==-1){
							shootx=tx; shooty=ty;
						}
					}
					else if(cc=='A') map[tx][ty]=' ';
					else if(cc!='!') map[tx][ty]=cc;
				
			}
			if(tx<X) tx--;else if (tx>X) tx++;else if(ty<Y) ty--; else if(ty>Y) ty++;
			count++;
			if(aa!=-1) {X=aa; Y=bb;}
			map[X][Y]='U';

		}
	}	
	else if(ch=='q'&&armed==1) 	{
		n = write(sockfd, &ch, sizeof(char)); 
		n = write(sockfd, &sh, sizeof(char));
		armed=0;
		if(shootx!=-1&&map[shootx][shooty]=='N') map[shootx][shooty]=' ';
		n = read(sockfd, &cc, sizeof(char));
		if(cc=='M') ack=2;
		//funzione per eliminare un nemico dalla mappa nelle 4 caselle puntate dal giocatore
		read(sockfd, &cc, sizeof(char));
		if(cc=='L') end=2;
		else if(cc=='K') end=4;
	}
	else if(ch=='e'){
		n = write(sockfd, &ch, sizeof(char)); 
		end=3;
		
	}
	else if(ch=='c'){
		n = write(sockfd, &ch, sizeof(char)); 
		stampakill(sockfd);
		read(sockfd, &cc, sizeof(char));
		if(cc=='L') end=2;
		else if(cc=='K') end=4;		
	}
    }
    if(end==1) printf("congratulazioni! hai vinto!\n");
    else if(end==2) printf("game over! il tesoro è stato trovato!\n");
    else if(end==3) printf("disconnessione riuscita!\n");
    else if(end==4) printf("sei stato ucciso!\n");
    else printf("game over! sei stato ucciso!\n");
    close(sockfd);
    return 0;
}


void connectusr(char buffer[256], char str[230], int sockfd){
	int conn=0, ch=0, n=0, space; int i;
	char c;
	while (conn==0){
		printf("Enter 1 to connect as a known user\nEnter 2 to create a new user\n");
		scanf("%d", &ch);
		printf("Please enter your username and password: ");
		bzero(buffer,256);
		bzero(str,256);
		scanf("%c", &c);
		fgets(str,230,stdin);
		space=0;
		for(i=0; i<230; i++){
			if(str[i]==' ') space++;
		}
		if (space!=1) ch=0;
		if(ch==1){
		    strcpy(buffer, "known");
		    
		    strcat(buffer, str);
		    n = write(sockfd,buffer,strlen(buffer));
		    if (n < 0) 
			 error("ERROR writing to socket");
		    bzero(buffer,256);
		    n = read(sockfd,buffer,1);
		    if (n < 0) 
			 error("ERROR reading from socket");
		    if(buffer[0]=='y')  conn=1;
		    else if (buffer[0]=='n')printf("not found\n");
		    else printf("already playing\n");
		}
		else if(ch == 2){
		    strcpy(buffer, "unkno");
		    
		    strcat(buffer, str);
		    n = write(sockfd,buffer,strlen(buffer));
		    if (n < 0) 
			 error("ERROR writing to socket");
		    bzero(buffer,256);
		    n = read(sockfd,buffer, 1);
		    if (n < 0) 
			 error("ERROR reading from socket");
		    if(buffer[0]=='y') {printf ("user created and connected\n"); conn=1;}
		    else if (buffer[0]=='n')printf("username already taken\n");
		}
		else printf("wrong input\n");
	}
}





void stampamappa( char (*map)[21], int a){
	int i, j;
	system("clear");
	for (i=0; i<10; i++){
		for (j=0; j<21; j++){
			write(STDOUT_FILENO, &map[i][j], sizeof(char));
			
		}
	}
	if(a==1) 	write(STDOUT_FILENO, "\narma acquisita\n" , sizeof(char)*18);
	if(a==2)	write(STDOUT_FILENO, "\nnemico eliminato\n" , sizeof(char)*20);
}


char getch()
{
	  int ch;
	  struct termios oldt;
	  struct termios newt;
	  tcgetattr(STDIN_FILENO, &oldt); 
	  newt = oldt; 
	  newt.c_lflag &= ~(ICANON | ECHO); 
	  tcsetattr(STDIN_FILENO, TCSANOW, &newt); 
	  ch = getchar(); 
	  tcsetattr(STDIN_FILENO, TCSANOW, &oldt); 
	  return ch; 
}

void stampakill(int sockfd){
	system("clear");
	char c;
	read(sockfd, &c, sizeof(char));
	while(c!='#'){
		write(STDOUT_FILENO, &c, sizeof(char));
		read(sockfd, &c, sizeof(char));
		
	}  sleep(1);
}


