#include "server.h"
//#define BUF_SIZE 128
//#define MAX_CLNT 2
//#define NAME_SIZE 20

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock, i;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
	
		if(clnt_cnt >= MAX_CLNT) {
			printf("CONNECT FAIL : %d \n", clnt_sock);
			write(clnt_sock, "too many users. sorry", BUF_SIZE);
			continue;
		}


		pthread_mutex_lock(&mutx);

		clnt_socks[clnt_cnt]=clnt_sock;
		read(clnt_sock, clnt_name, NAME_SIZE);		
		strcpy(clnt_names[clnt_cnt++], clnt_name);
		// ㄴ 클라이언트로부터 받은 접속자 이름입력
		pthread_mutex_unlock(&mutx);

		
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0 ,i;
	int fSize = 0;
	char sig_file[BUF_SIZE] = {"file : cl->sr"};
        char Fmsg_end[BUF_SIZE] = {"FileEnd_cl->sr"};
        char sig_file_all[BUF_SIZE] = {"file : cl->sr_all"};
	char msg[BUF_SIZE];
	char file_msg[BUF_SIZE];

	while((str_len=read(clnt_sock, msg, BUF_SIZE))!=0) 
	{

		if(!strcmp(msg, sig_file))
		{
			int j;
			int noCli = 0;
			int fileGo = NULL;
			char tmpName[NAME_SIZE];

			read(clnt_sock, tmpName, NAME_SIZE);
			

			pthread_mutex_lock(&mutx);
						
			for(j=0; j<clnt_cnt; j++) {

				
				if(!strcmp(tmpName, clnt_names[j]) ) {
					noCli = 0;
					fileGo = j; // 보낼 소켓 번호를 저장
					break;
				}
				else if(j == clnt_cnt - 1) {
					noCli = 1; // 그런 사용자가 없을 때 표시
					break;
				}
			}

			if(noCli == 1) {

				write(clnt_sock, "[NoClient_sorry]", BUF_SIZE);
				pthread_mutex_unlock(&mutx);
				continue;
			}
			else if(noCli == 0) {

				write(clnt_sock, "[continue_ok_nowgo]", BUF_SIZE);
				
			}
			// ㄴ 해당 사용자가 존재하는지 찾기



			write(clnt_socks[fileGo], "file : sr->cl", BUF_SIZE);
			// ㄴ 데이터를 보낸다는 신호를 먼저 보냄

			read(clnt_sock, &fSize, sizeof(int));
			printf("File size %d Byte\n", fSize);
			write(clnt_socks[fileGo], &fSize, sizeof(int));
			// ㄴ 파일 크기 정보를 보냄.(진행중)

			while(1) {
				read(clnt_sock, file_msg, BUF_SIZE);
				if(!strcmp(file_msg, Fmsg_end))
					break;
				write(clnt_socks[fileGo], file_msg, BUF_SIZE);
			}


			write(clnt_socks[fileGo], "FileEnd_sr->cl", BUF_SIZE);
			
			pthread_mutex_unlock(&mutx);
			printf("(!Notice)File data transfered \n");

		} // ㄴ 파일정송
		else if(!strcmp(msg, sig_file_all)) {
			
			pthread_mutex_lock(&mutx);
						
			
			for(i=0; i<clnt_cnt; i++) {
				if(clnt_sock == clnt_socks[i])
					continue;
				write(clnt_socks[i], "file : sr->cl", BUF_SIZE);
			// ㄴ 데이터를 보낸다는 신호를 먼저 보냄
			}

			read(clnt_sock, &fSize, sizeof(int));
			printf("File size %d Byte\n", fSize);
			
			for(i=0; i<clnt_cnt; i++) {
				if(clnt_sock == clnt_socks[i])
					continue;
				write(clnt_socks[i], &fSize, sizeof(int));
			}
			// ㄴ 파일 크기 정보를 보냄.

			while(1) {
				read(clnt_sock, file_msg, BUF_SIZE);
				if(!strcmp(file_msg, Fmsg_end))
					break;

				for(i=0; i<clnt_cnt; i++) {
					if(clnt_sock == clnt_socks[i])
						continue;
					write(clnt_socks[i], file_msg, BUF_SIZE);
				}
			}

			for(i=0; i<clnt_cnt; i++) {
				if(clnt_sock == clnt_socks[i])
					continue;
				write(clnt_socks[i], "FileEnd_sr->cl", BUF_SIZE);
			}
			
			
			pthread_mutex_unlock(&mutx);
			printf("(!Notice)File data transfered \n");
		}
		/*
		else if(!strcmp(msg, sig_whisper)) {
			int j=0;
			int noCli = 0;
			int mGo = 0;
			char tmpName[NAME_SIZE]= {'\0'};
			char msg[NAME_SIZE]= {'\0'};

			read(clnt_sock, tmpName, NAME_SIZE);
			
			pthread_mutex_lock(&mutx);
			for(j=0; j<clnt_cnt; j++) {
				if(!strcmp(tmpName, clnt_names[j]) ) {
					noCli = 0;
					mGo = j; // 보낼 소켓 번호를 저장
					break;
				}
				else if(j == clnt_cnt - 1) {
					noCli = 1; // 그런 사용자가 없을 때 표시
					break;
				}
			}
			pthread_mutex_unlock(&mutx);

			read(clnt_sock, msg, BUF_SIZE);

			if(noCli == 1) {
				write(clnt_sock, "sorry. no client like that", BUF_SIZE);
			} // ㄴ 귓속말할 사용자 없을 때
			else {
				write(clnt_socks[j], msg, BUF_SIZE);
			} // ㄴ 귓속말할 사용자가 존재할 때

		}
		*/
		else
		{
			printf("(!Notice)Chatting message transfered \n");
			send_msg(msg, str_len);
		}// ㄴ 메시지 전송
	}

	
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1) {
				clnt_socks[i]=clnt_socks[i+1];
				strcpy(clnt_names[i], clnt_names[i+1]);
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, BUF_SIZE);
	pthread_mutex_unlock(&mutx);
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
