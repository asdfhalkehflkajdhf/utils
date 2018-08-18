#ifndef __LIBFTP_H__
#define __LIBFTP_H__

/*
ÿ������ֻ��ʹ��һ�� global_ftp_client_init

*/
enum {
	GLOBAL_FTP_INIT_DEFAULT,
	GLOBAL_FTP_INIT_DEBUG,
};

extern int global_ftp_client_init(int option);
extern int global_ftp_client_exit(void);

extern int ftp_client_exit(void);

/*
           -p: enable passive mode
           -i: turn off prompting during mget
           -n: inhibit auto-login
           -g: disable filename globbing
           -m: don't force data channel interface to the same as control channel
           -v: verbose mode ,need on -d
           -t: enable packet tracing [nonfunctional]
           -d: enable debugging, ֻ����GLOBAL_FTP_INIT_DEBUG ���òſ�������

	ÿ���߳�ֻ��ִ��һ��
	����ֵ:-1δ֪����; >400����ftp_get_E_errors ������Ϣ
*/
extern int ftp_client_init(char *argv);

/*
!               debug           mdir            sendport        site
$               dir             mget            put             size
account         disconnect      mkdir           pwd             status
append          exit            mls             quit            struct
ascii           form            mode            quote           system
bell            get             modtime         recv            sunique
binary          glob            mput            reget           tenex
bye             hash            newer           rstatus         tick
case            help            nmap            rhelp           trace
cd              idle            nlist           rename          type
cdup            image           ntrans          reset           user
chmod           lcd             open            restart         umask
close           ls              prompt          rmdir           verbose
cr              macdef          passive         runique         ?
delete          mdelete         proxy           send
*/
extern int ftp_client_cmd(char *cmds);
/*ͨ��code���ȡӢ��˵��*/
extern char *ftp_get_E_errors(int code);
/*ͨ��code���ȡ����˵��*/
extern char *ftp_get_C_errors(int code);


#endif
