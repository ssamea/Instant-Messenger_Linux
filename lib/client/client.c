#include "client.h"

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	
	//pthread_mutex_init(&mutx, NULL);
	// pthread_mutex_lock(&mutx);
	// pthread_mutex_unlock(&mutx); 
	// 뮤텍스 이제 이용가능

	sprintf(name, "%s", argv[3]);
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

	write(sock, name, NAME_SIZE);
	// ㄴ 이름을 서버쪽으로 보냄.

	printf("\n\n");
	printf("CONNECTING..... \n [TIP] If you want \"MENU\" -> /menu \n\n");

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);  
	return 0;
}
	
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg);
	int Flength = 0;
	int i=0;
	int fSize = 0;
	int fEnd = 0;
	char name_msg[NAME_SIZE+BUF_SIZE];
	char t_msg[BUF_SIZE];
	char last_msg[BUF_SIZE];
	char t_name_msg[BUF_SIZE];
	char noUse[BUF_SIZE];
	const char enter[BUF_SIZE] = {"\n"};


	// ㄴ (진행중)


	while(1) 
	{
		if(wOk == 0) {
			sleep(1);
		}

		fgets(msg, BUF_SIZE, stdin);
		
		

		if(!strcmp(msg,"/exit\n")) 
		{
			close(sock);
			exit(0);
		}

		else if(!strcmp(msg, "/sendfile all\n")) {

			char location[BUF_SIZE];
			FILE *fp;
			FILE *size;

			printf("(!Record)File location : ");
			scanf("%s", location);

			size = fopen(location, "rb");
			if(size == NULL) {
				printf("(!Notice)No file like that \n");
				continue;
			}
			// ㄴ 보낼 파일이 유효한가 확인

			write(sock, "file : cl->sr_all", BUF_SIZE);
			// ㄴ 먼저 파일을 보낸다는 신호를 서버쪽에 보냄.

			while(1) {	
				fEnd = fread(noUse, 1 , BUF_SIZE, size);
				fSize += fEnd;

				if(fEnd != BUF_SIZE)
					break;
			}
			fclose(size);

			printf("(!Notice)File transfer start \n(File Size : %d Byte)\n", fSize); 
			write(sock, &fSize, sizeof(int)); // 파일 크기정보 먼저 보냄.
			fSize = 0;
			
			fp = fopen(location, "rb");
			

			while(1) {
				
				Flength = fread(t_msg, 1 , BUF_SIZE, fp);

				if(Flength != BUF_SIZE) {
					for(i=0; i<Flength; i++) {
						last_msg[i] = t_msg[i];
					} 
					//ㄴ fread 는 파일끝에 닿았을 때 이전의 데이터와 함쳐짐이 있을 수 있으므로 방지하였음.

					write(sock, last_msg, BUF_SIZE);

					write(sock, "FileEnd_cl->sr", BUF_SIZE);
					break;
				}
				write(sock, t_msg, BUF_SIZE); 

			}
			// ㄴ 서버에 파일의 내용을 보냅니다.		

			fclose(fp);
			printf("(!Notice)File transfer finish \n");		
		} // ㄴ 전체에게 파일 보낼 

		else if(!strcmp(msg, "/menu\n")) {
			
			printf("\n");
			printf("[MENU]\n\n");
			printf("1. /menu -> some orders \n");
			printf("2. /sendfile all -> 1:N file transfer \n");
			printf("3. /exit -> chatting program exit \n");
			printf("\n[END MENU] \n\n");

		} // ㄴ 메뉴를 보여주는 경우 


		else if(setFName == 1) {
			if(strcmp(msg, enter)) {
				setFName = 0;
			}
		} // ㄴ 파일 수신시 파일 이름을 설정하는 경우
		
		else 
		{
			strcpy(t_msg, "\n");
			sprintf(t_name_msg,"[%s] %s", name, t_msg);
			sprintf(name_msg,"[%s] %s", name, msg);

			if(strcmp(name_msg, t_name_msg) != 0) 
				write(sock, name_msg, BUF_SIZE);
			// ㄴ 아무것도 입력받지 않았을때는 출력하지 않음
			// ㄴ 메시지 보내기
			
		}
		
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_msg[BUF_SIZE];
	char file_msg[BUF_SIZE];
	const char signal[BUF_SIZE] = {"file : sr->cl"};
	const char end_msg[BUF_SIZE] = {"FileEnd_sr->cl"};
	const char nocl_msg[BUF_SIZE] = {"[NoClient_sorry]"};
	const char yescl_msg[BUF_SIZE] = {"[continue_ok_nowgo]"};
	const char noConnect[BUF_SIZE] = {"too many users. sorry"};
	int str_len = 0;
	int fSize = 0;

	while(1)
	{
		str_len=read(sock, name_msg, BUF_SIZE);

		
		if(!strcmp(name_msg, signal)) {
			
			setFName = 1;
			wOk = 0;

			printf("(!Notice)receive request. ");

			read(sock, &fSize, sizeof(int));
			printf("(File size : %d Byte)\n [press Enter key to continue]", fSize);
			//파일 사이즈 받아 출력하기.

			printf("(!Notice)set file name : ");
			
			wOk = 1;
			while(setFName == 1) {
				sleep(1);
			}

			msg[strlen(msg)-1] = '\0';
			
			FILE *fp;
			fp = fopen(msg, "wb"); 
		
			while(1)
			{		
				read(sock, file_msg, BUF_SIZE);
				
				if(!strcmp(file_msg, end_msg)) 
					break;
				fwrite(file_msg, 1, BUF_SIZE, fp);
			}

			fclose(fp);
			
			printf("(!Notice)File receive finished \n");
			// ㄴ send_msg 쓰레드의 활동 재개


		}
		else if(strcmp(name_msg, yescl_msg) == 0) {

			cli_exist = EXIST;

		}
		else if(strcmp(name_msg, nocl_msg) == 0) {

			cli_exist = NOTEXIST; 
		}
		else if(!strcmp(name_msg, noConnect)) {
			printf("too many users. sorry \n");
			exit(0);
		}
		else {
			fputs(name_msg, stdout);
		}
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

