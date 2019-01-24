#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
int player=0;
char map[10][21];
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;
void *game(void *);
void aggiornaclassifica(char name[50]);
void cancellagiocatori(char (*map)[21]);
void print(char (*map)[21]);
int tesoro();
void error(const char *msg) {
    perror(msg);
    exit(1);
}
int Search_in_File(char *fname, char *str);
void crea_mappa (char (* map)[21] );
void mostraclassifica(int sockfd);
void classifica();
int main(int argc, char *argv[]) {
    
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    
    pthread_t tid;

    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
	 fprintf(stderr, "ERROR, no port provided\n");
	 exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
	 error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	error("ERROR on binding");
    listen(sockfd, 1);
    clilen = sizeof(cli_addr);
    

    while(1){
	    remove("CLASSIFICA.txt");
	    srand(time(NULL));
	    crea_mappa(map);
	    while (tesoro()==1) {
		 
		 newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		 if (newsockfd < 0)
			error("ERROR on accept");
		 if( pthread_create( &tid , NULL ,  game , (void*)(intptr_t) newsockfd) < 0)
		 {
		    perror("could not create thread");
		 }
		 listen(sockfd, 1);
		 
	    }
    }
    
   return 0;
}

void *game(void *sockfd){
	int conn, find, t, i, X, Y, ind, count, esc;    int n; char myid;
	char ch;char buf[10];
    	int start, x,y, armed, sel;
   	char buffer[256], job[20], name[50];
	int newsockfd=(int)(intptr_t)sockfd;
	esc=0;	 
 	conn=0;
	int T=1;FILE * fp;
	while( conn == 0) {
	    sel=0;
	    bzero(buffer, 256);
	    bzero(name, 50);
	    n = read(newsockfd, buffer, 255);
	    if (n < 0)
		error("ERROR reading from socket");
	    
	    strncpy(job, buffer, 5);
	    
	    fp = fopen("labirinto.txt", "ab+");
	    if(fp == NULL)
		error("open");
	    
	    
	    if(job[0]=='k'){
		strcpy(name, buffer+5);
	    	find=Search_in_File("labirinto.txt", name);
		if(find==1){
			for ( i=5; i<230; i++) if(buffer[i]==' ') t=i;
			bzero(name, 50);
			strncpy(name, buffer+5, t-5);//printf("\n\nt=%d\n\n%s",t, name);
	    		find=Search_in_File("CLASSIFICA.txt", name);
			if(find!=1){
				conn=1;
			}
			else sel=1;
		}
	    }
	    else if(job[0]=='u'){
		for ( i=5; i<230; i++) if(buffer[i]==' ') t=i;
		strncpy(name, buffer+5, t-5);//printf("\n\nt=%d\n\n%s",t, name);
	    	find=Search_in_File("labirinto.txt", name);

		if(find==0){
			conn=1;
			strcpy(name, buffer+5);
			n = fwrite(name, strlen(name), sizeof(char), fp);
			fclose(fp);
			if (n < 0)
				error("ERROR writing to file");
		}
	    }else;
	    if(conn==1){
		n = write(newsockfd, "y", 1);
		if (n < 0)
			error("ERROR writing to socket");
	    }else if(sel==1){
		n = write(newsockfd, "j", 1);
		if (n < 0)
			error("ERROR writing to socket");
	     }
	     else{
		n = write(newsockfd, "n", 1);
		if (n < 0)
			error("ERROR writing to socket");
	    }
    	}
    	for ( i=5; i<230; i++) if(buffer[i]==' ') t=i;
    	bzero(name, 50);
    	strncpy(name, buffer+5, t-5);
    	strcat(name, " 0\n");
    	classifica(name);
    	bzero(name, 50);
    	strncpy(name, buffer+5, t-5);
    	start=0; 
    	while(start==0) { x=rand()%10; y=rand()%20;
		if((x==0||y==0||x==9||y==19)&&map[x][y]==' '&&(x!=0||y!=0)) {
			sprintf(buf, "%d", x);
			pthread_mutex_lock(&mymutex);
			map[x][y]=player+'0'; start=1;
			myid=player+'0';
			player++;
			pthread_mutex_unlock(&mymutex);
			n=write(newsockfd, buf, sizeof(buf));
			sprintf(buf, "%d", y);

			n=write(newsockfd, buf, sizeof(buf));
		}

    	}
    	
    	armed=0;
    
    	while(esc==0){
	
		ch=' ';
		read(newsockfd, &ch, 1);
		if(map[x][y]!=myid&&map[x][y]!='L') 	{ write(newsockfd, "K", 1); esc=1;}
		else if(map[x][y]=='L') {write(newsockfd, "L", 1); esc=2;}
		else if(ch=='c') {
			mostraclassifica(newsockfd);
			if(map[x][y]==' ') 	{ write(newsockfd, "K", 1); esc=1;}
			else if(map[x][y]=='L') {write(newsockfd, "L", 1); esc=2;}
			else write(newsockfd, "!", 1);
		}
		else if(ch=='e') esc=1; 
		else if(ch=='q') {
			armed=0;
			read(newsockfd, &ch, 1);
			X=x; Y=y;
			count=0;
			do{
				if(ch=='w') X=X-1; else if (ch=='s') X=X+1; else if (ch=='a') Y=Y-1; else if (ch=='d') Y=Y+1;
				count++;
			}while(count<4&&(map[X][Y]<'0'||map[X][Y]>'9'));
			if(map[X][Y]>='0'&&map[X][Y]<='9') {
				pthread_mutex_lock(&mymutex);
				map[X][Y]=' ';
				pthread_mutex_unlock(&mymutex);
				write(newsockfd, "M", 1);
				aggiornaclassifica(name);
			}else write(newsockfd, "!", 1);
			if(map[x][y]==' ') 	{ write(newsockfd, "K", 1); esc=1;}
			else if(map[x][y]=='L') {write(newsockfd, "L", 1); esc=2;}
			else write(newsockfd, "!", 1);
		}
		else{
			X=x; Y=y;
			count=0;
			ind=1;
			if(armed==1) ind=4;
			while(count<ind){
				if(ch=='w') X=X-1; else if (ch=='s') X=X+1; else if (ch=='a') Y=Y-1; else if (ch=='d') Y=Y+1;
				if (X>=0&&X<10&&Y>=0&&Y<20) {
					write(newsockfd, &map[X][Y], 1); 
					if((map[X][Y]==' '||map[X][Y]=='A')&&count==0) {
						
						if(map[X][Y]=='A') armed=1;
						pthread_mutex_lock(&mymutex);
						map[X][Y]=map[x][y];
						map[x][y]=' ';
						pthread_mutex_unlock(&mymutex);
						x=X;
						y=Y;
				
					}
					else if(map[X][Y]=='T'&&count==0) {
						esc=2;
						cancellagiocatori(map); 	
					}
				}
				else write(newsockfd, "!", 1);
				count++;
			}
		}
    }
    close(newsockfd);
}
void crea_mappa (char (*map) [21]) {
	srand(time(NULL));
	int i, j;
	int T = 0;
	for(i=0; i<21; i++) 
	  	for (j=0; j<10; j++)
			map[j][i]=' ';
    	for(i=0; i<21; i++) {
		for (j=0; j<10; j++) {
		
			if(i==20) map[j][i]='\n'; else {
				if (i>0 && j>0 && i<19 && j<9) {
					if(((map[j-1][i-1]!='O'&&map[j-1][i]!='O')&&(map[j][i-1]!='O'))||rand()%20==0){
						if(i>4&&j>2&&T==0&&rand()%15==0) { map[j][i]='T'; T=1;}
						else map[j][i]='O';
					}
					else if (rand()%5==0) map[j][i]='A';
					else map[j][i]=' ';
    				}else map[j][i]=' ';
			}
	}
    }
	if(T==0) map[(rand()%9)+1][(rand()%19)+1]='T';
    	
}
int Search_in_File(char *fname, char *str) {
	FILE *fp;
	size_t len = 0;
	char *temp;
	ssize_t read;

	if((fp = fopen(fname, "r")) == NULL) {
		return(0);
	}

	while ((read = getline(&temp, &len, fp)) != -1) {

			if(strstr(temp, str)){
				
					return(1);
		
		
		}
    	}





	if(fp) {
		fclose(fp);
	}

   	return(0);
}



void cancellagiocatori(char (*map)[21]){
	for (int i=0; i<10; i++){
		for (int j=0; j<21; j++){
			if(map[i][j]>='0'&&map[i][j]<='9') {
				pthread_mutex_lock(&mymutex);
				map[i][j]='L';
				pthread_mutex_unlock(&mymutex);
				
			}
			else if(map[i][j]='T') {
				pthread_mutex_lock(&mymutex);
				map[i][j]=' ';
				pthread_mutex_unlock(&mymutex);
			}
		}
	}
}


void print(char (*map)[21]){
	for (int i=0; i<10; i++)
		for (int j=0; j<21; j++)
			write(STDOUT_FILENO, &map[i][j], 1);
}

int tesoro(){
	int ret;
	ret=0;
	
	for (int i=0; i<10; i++){
		for (int j=0; j<21; j++){
			if(map[i][j]=='T') ret=1;
		}
	}
	return ret;
}

void classifica(char name[50]){
	FILE * fd;

   	fd = fopen("CLASSIFICA.txt", "ab+");
   	fprintf(fd, "%s", name);
   	fclose(fd);
}

void mostraclassifica(int sockfd){
	int fd;
	char c; int n;
	fd=open("CLASSIFICA.txt", O_RDONLY , 0);
	while((n=read(fd, &c, 1))>0)  { write(sockfd, &c, 1);}
	c='#';write(sockfd, &c, 1);
	close(fd);
}

void aggiornaclassifica(char name[50]){
	FILE *fp;
	int i,j, tmp, no,z;
	size_t len = 0;
	char temp[50][50];
	char * a;
	int count=0;
	int fd, readx;
	char x;
	fp = fopen("CLASSIFICA.txt", "r");
	i=0;
	while (getline(&a, &len, fp) != -1) {
			j=0;
			if(strstr(a, name)){

				tmp=a[strlen(a)-2]-'0';
				tmp++;
				a[strlen(a)-2]=tmp+'0';
			}
			strcpy(temp[i], a);
			i++;
	}
				
	fclose(fp);
	remove("CLASSIFICA.txt");
	fp=fopen("CLASSIFICA.txt", "w");
	while(i>=0){
		fprintf(fp, "%s", temp[i]);
		
		
		
		i--;
	}
	fclose(fp);
}


