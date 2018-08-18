#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#include <fcntl.h>
#include <errno.h>



#include <linux/tcp.h>
#include <time.h>


#include "tcp.h"

/*
FD_ZERO(fd_set *fdset) ��ָ�����ļ�����������գ��ڶ��ļ����������Ͻ�������ǰ�����������г�ʼ�����������գ�������ϵͳ�����ڴ�ռ��ͨ����������մ������Խ���ǲ���֪�ġ�
    FD_SET(fd_set *fdset) �������ļ�����������������һ���µ��ļ���������
    FD_CLR(fd_set *fdset) �������ļ�������������ɾ��һ���ļ���������
    FD_ISSET(int fd,fd_set *fdset) ���ڲ���ָ�����ļ��������Ƿ��ڸü�����
*/


int select_fd_init(fd_set *fdset)
{
	FD_ZERO(fdset);
	return 0;
}

int select_fd_del(fd_set *fdset, int fd)
{
	FD_CLR(fd, fdset);
	return 0;
}

int select_fd_add(fd_set *fdset, int fd)
{
	FD_SET(fd, fdset);
	return 0;
}
//�ڼ����з���<0����>0�ɶ�д������0��ʱ
int select_fd_check_r(fd_set *fdset_r, int fd, int sec, int usec)
{
	/*
	����ֵ������״̬�����仯�������������� 
��ֵ��select����

��ֵ��ĳЩ�ļ��ɶ�д�����

0���ȴ���ʱ��û�пɶ�д�������ļ�
	*/
	struct timeval ptv;
	int ret;
	
	ptv.tv_sec = sec;
	ptv.tv_usec = usec;
	ret = select(fd + 1, fdset_r, NULL, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
//�ڼ����з���<0����>0�ɶ�д������0��ʱ
int select_fd_check_w(fd_set *fdset_w, int fd, int sec, int usec)
{
	/*
	����ֵ������״̬�����仯�������������� 
��ֵ��select����

��ֵ��ĳЩ�ļ��ɶ�д�����

0���ȴ���ʱ��û�пɶ�д�������ļ�
	*/
	struct timeval ptv;
	int ret;
	
	ptv.tv_sec = sec;
	ptv.tv_usec = usec;
	ret = select(fd + 1, NULL, fdset_w, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
//�ڼ����з���<0����>0�ɶ�д������0��ʱ
int select_fd_check_rw(fd_set *fdset_r, fd_set *fdset_w, int fd, int sec, int usec)
{
	/*
	����ֵ������״̬�����仯�������������� 
��ֵ��select����

��ֵ��ĳЩ�ļ��ɶ�д�����

0���ȴ���ʱ��û�пɶ�д�������ļ�
	*/
	struct timeval ptv;
	int ret;
	
	ptv.tv_sec = sec;
	ptv.tv_usec = usec;
	ret = select(fd + 1, fdset_r, fdset_w, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
//��������fd����������fdset�з��ط���ֵ�����򣬷����㡣
int select_fd_check2(fd_set *fdset, int fd)
{
	return FD_ISSET(fd, fdset);
}
	/*
	����ֵ������״̬�����仯�������������� 
��ֵ��select����

��ֵ��ĳЩ�ļ��ɶ�д�����

0���ȴ���ʱ��û�пɶ�д�������ļ�
	*/
int select_fd_timeout_r(int fd, int sec, int usec)
{
    struct timeval ptv;
    fd_set fdset;
	int ret;
	
    ptv.tv_sec = sec;
    ptv.tv_usec = usec;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
	ret = select(fd + 1, &fdset, NULL, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
int select_fd_timeout_w(int fd, int sec, int usec)
{
    struct timeval ptv;
    fd_set fdset;
	int ret;
	
    ptv.tv_sec = sec;
    ptv.tv_usec = usec;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
	ret = select(fd + 1, NULL, &fdset, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
int select_fd_timeout_rw(int fd, int sec, int usec)
{
    struct timeval ptv;
    fd_set fdset;
	int ret;
	
    ptv.tv_sec = sec;
    ptv.tv_usec = usec;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
	ret = select(fd + 1, &fdset, &fdset, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
/*
������ģʽ��, send�����Ĺ����ǽ�Ӧ�ó��������͵����ݿ��������ͻ����з��;ͷ���.�����ڷ��ͻ���Ĵ���,
����Ϊ:������ͻ����С�������͵Ĵ�СҪ��,��ôsend������������,ͬʱ�������з�������;
����,send��ȴ����ն˶�֮ǰ�������ݵ�ȷ��,�Ա��ڳ�����ռ������µĴ���������,
�ٷ���(���ն�Э��ջֻҪ�������յ����ջ�����,�ͻ�ȷ��,����һ��Ҫ�ȴ�Ӧ�ó������recv),���һֱû�пռ������ɴ����͵�����,��һֱ����;
�ڷ�����ģʽ��,send�����Ĺ��̽����ǽ����ݿ�����Э��ջ�Ļ���������,������������ÿռ䲻��,�������Ŀ���,�������سɹ������Ĵ�С;�绺�������ÿռ�Ϊ0,�򷵻�-1,ͬʱ����errnoΪEAGAIN.


�������Ǹɲ��겻׼�������������������ȸɣ����ֿ�����������û�У����˸�����һ����
*/


int CloseSocket(int sockfd)
{
    shutdown(sockfd, 2);

    close(sockfd);
    sockfd = INVALID_SOCKET;
    
    return (0);
}



int Writen(int sckid, unsigned char *buf, int len, int sec, int usec)
{
	struct timeval ptv = {0,0}; 
	ptv.tv_sec = sec;
    ptv.tv_usec = usec;
	//���÷��ͳ�ʱ
	setsockopt(sckid, SOL_SOCKET,SO_SNDTIMEO, (char *)&ptv,sizeof(struct timeval));

	if (send(sckid, buf, len, 0) != len)
	{
		return (-1);
	}
    return (len);
}

int Readn(int sckid, char *buf, int len, int sec, int usec)
{
    int rc = 0;
	struct timeval ptv = {0,0}; 
	ptv.tv_sec = sec;
    ptv.tv_usec = usec;
	
	//���ý��ճ�ʱ
	setsockopt(sckid, SOL_SOCKET,SO_RCVTIMEO, (char *)&ptv,sizeof(struct timeval));
	if ((rc = recv(sckid, buf, len, 0)) <= 0)
	{
		return (-1);
	}
    return (rc);
}


int ConnectRemote(char *ip, int port, int sec, int usec)
{
    struct sockaddr_in psckadd;
    struct linger Linger;

	int sckcli;
    int on = 1;
    
	struct timeval ptv;
    
    memset((char *)(&psckadd), '0', sizeof(struct sockaddr_in));
    psckadd.sin_family = AF_INET;
    psckadd.sin_addr.s_addr = inet_addr(ip);
    psckadd.sin_port = htons((u_short) port);
	
    if ((sckcli = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        return (INVALID_SOCKET);
    }
    


	//�������ӳ�ʱʱ��
	ptv.tv_sec = sec;
	ptv.tv_usec = usec;	
	if (setsockopt(sckcli, SOL_SOCKET, SO_SNDTIMEO, &ptv, sizeof(ptv)))
	{
		close(sckcli);
		return (INVALID_SOCKET);
	}	



    if (connect(sckcli, (struct sockaddr *)(&psckadd), sizeof(struct sockaddr_in)) < 0)
    {
        close(sckcli);
        return (INVALID_SOCKET);
    }

	//���ý���tcp�������
    if (setsockopt(sckcli, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on)))
    {
        CloseSocket(sckcli);
        return (INVALID_SOCKET);
    } 
	
    on = 1;
    setsockopt(sckcli, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));
	
	/*
	TCP_NODELAY��TCP_CORK���ǽ���Nagle�㷨��ֻ����NODELAY��ȫ�ر��㷨���������Ͷ�TCP_CORK��ȫ���Լ���������ʱ��
	*/
    setsockopt(sckcli, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

    return (sckcli);
}


int ListenRemote(int port)
{
	int yes=1;
	int server_socket = INVALID_SOCKET;
    struct sockaddr_in server_addr;

    //�����׽���
    if((server_socket=socket(AF_INET,SOCK_STREAM,0))<0){
        return INVALID_SOCKET;
    }
    
    //����Ϊ���ظ�ʹ��
    if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1){
        close(server_socket);
        return INVALID_SOCKET;
    }
    //���÷�������ַ��Ϣ����
    server_addr.sin_family=AF_INET;                    //TCP
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr=INADDR_ANY;            //����IP��ַ
    
    memset(server_addr.sin_zero, 0,sizeof(server_addr.sin_zero));
    
    //���׽������ַ��Ϣ
    if(bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1){
        close(server_socket);
        return INVALID_SOCKET;
    }
    
    //����
    if(listen(server_socket,5)==-1){
        close(server_socket);
        return INVALID_SOCKET;
    }
    
    return server_socket;
}

int AcceptRemote(int sockfd)
{
	int client_sockfd = INVALID_SOCKET;//�ͻ����׽���  
	struct sockaddr_in remote_addr; //�ͻ��������ַ�ṹ��  
	
	int sin_size=sizeof(struct sockaddr_in);  
      
    /*�ȴ��ͻ����������󵽴�*/  
    if((client_sockfd=accept(sockfd,(struct sockaddr *)&remote_addr,&sin_size))<0)  
    {  
        return INVALID_SOCKET;  
    } 
        
    //Ĭ��Ϊ����ģʽ
    
    return client_sockfd;
}


void setOpetinNoBlock(int fd)
{
	//����Ϊ������ģʽ
    int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK); 

}

/*

int socket_recv(int sockfd, char* buffer, int buflen)
{
	int rs=1;
	int ret;
	while(rs)
	{
		ret = recv(activeevents[i].data.fd, buffer, buflen, 0);
		if(ret < 0)
		{
			// �����Ƿ�������ģʽ,���Ե�errnoΪEAGAINʱ,��ʾ��ǰ�������������ݿɶ�
			// ������͵����Ǹô��¼��Ѵ���.
			if(errno == EAGAIN)
				break;

		}
		else if(ret == 0)
		{
			// �����ʾ�Զ˵�socket�������ر�.
			break;
		}
		if(buflen == ret)
			rs = 1;   // ��Ҫ�ٴζ�ȡ
		else
			rs = 0;
	}
	
	return ret;

}



int socket_send(int sockfd, const char* buffer, int buflen)
{
	int tmp;
	int total = buflen;
	const char *p = buffer;

	while(1)
	{
		tmp = send(sockfd, p, total, 0);
		if(tmp < 0)
		{
			// ��send�յ��ź�ʱ,���Լ���д,�����ﷵ��-1.
			if(errno == EINTR)
			return -1;

			// ��socket�Ƿ�����ʱ,�緵�ش˴���,��ʾд�����������,
			// ����������ʱ��������.
			if(errno == EAGAIN)
			{
				usleep(1000);
				continue;
			}

			return -1;
		}

		if((size_t)tmp == total)
			return buflen;

		total -= tmp;
		p += tmp;
	}

	return tmp;
}


*/

