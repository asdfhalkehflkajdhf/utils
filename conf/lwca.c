/*****************************************************************************
Copyright: 2016-2026, liuzc
File name: lwca (�������ý��� Light weight configuration analysis)
Description: ������ϸ˵���˳����ļ���ɵ���Ҫ���ܣ� ������ģ������Ľӿڣ� ���
ֵ��ȡֵ��Χ�����弰������Ŀ��ơ�˳�򡢶����������ȹ�ϵ

Author: liuzc
Version: 1.0
Date: ��
History: 
	2016/11/22 �����ļ� 
*****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


#include "utils.h"
#include "include/list.h"

#define LWCA_TRUE 1
#define LWCA_FALSE 0

#define LWCA_MODE_FILE 1
#define LWCA_MODE_STR 0

#define LWCA_SEARCH_HASH_NUM 1000

typedef char  LWCA_NODE; 
//�����ļ�����ͷ 
typedef struct list_head LWCA_LIST_HEAD;



typedef struct LWCA_RUNING_INFO_NODE_{
	
	LWCA_NODE *lwca_hash[LWCA_SEARCH_HASH_NUM]; //����hash  
	
	int cur_layer;//�� 
	int cur_align; //���� 
	char *cur_k_start;
	char * cur_k_end;

	char *cur_v_start;
	char * cur_v_end;	//ָ���һ��\0 
	char *cur_tmp;//��ǰָ���ͷ 
	
	int buffer_size;
	char *buffer;
	
}LWCA_RUNING_INFO_NODE;


typedef struct LWCA_LIST_NODE_{
	int layer;		//�㼶 
	int indicia;	//��� 
	char *key;
	char *value;
	
	LWCA_LIST_HEAD list;//ͬ��list 
	LWCA_LIST_HEAD next;//�¼�list 
	LWCA_LIST_HEAD hash_list;

	int list_num;	
		
	struct LWCA_LIST_NODE_ **hash;
	int hash_key;

	LWCA_RUNING_INFO_NODE *root;
}LWCA_LIST_NODE;



/*************************************************
Function: 		Lwca_F_Init
Description:	��ʼ�������ļ�������Ϣ 
Called By:		Lwca_Init

Input:			�����ļ�·�� 
Output:			�ļ�������Ϣ�ṹ
Return:			�ļ�������Ϣ�ṹ
Others:
*************************************************/
static LWCA_RUNING_INFO_NODE *Lwca_F_Init(const char const *file_path)
{
	int res;
	int fileSize;
	char *buffer;
	FILE *tfd;
	
	LWCA_RUNING_INFO_NODE *root;
	
	//return 0:yes  -1:no
	res = is_file(file_path);
	if(0 != res){
		printf("[ERROR] [%s] is not file!\n",file_path);
		return NULL;
	}
	
	fileSize = get_file_size(file_path);
	if(0 >= fileSize){
		printf("[ERROR] [%s] is null!\n",file_path);
		return NULL;
	}	

	tfd = fopen(file_path,"r");
	if(tfd < 0){
		printf("[ERROR] [%s] is not open(%d)!\n",file_path, tfd);
		return NULL;
	}
	
	buffer = (char *)malloc(fileSize+1);
	memset(buffer, 0, fileSize+1);
	
	while(res !=  fileSize){
		int i = fread(buffer+res, fileSize-res, 1,tfd);
		res = res+ i;
		if(i<0)break;
		if(i==0){res = fileSize;}
	}
	
	if(res < fileSize){
		fclose(tfd);
		free(buffer);
		printf("[ERROR] [%s] is not read!\n",file_path);
		return NULL;
	}
	
	root = (LWCA_RUNING_INFO_NODE *)malloc(sizeof(LWCA_RUNING_INFO_NODE));
	if(root == NULL){
		fclose(tfd);
		free(buffer);
		printf("[ERROR] malloc false!\n");
		return NULL;
	}
	

	int i=0;
	for(i=0; i<LWCA_SEARCH_HASH_NUM; ++i){
		root->lwca_hash[i]=NULL;
	}
	
	root->cur_layer = 0;
	
	root->cur_align = 0;
	root->cur_k_start = NULL;
	root->cur_k_end = NULL;
	root->cur_v_start = NULL;
	root->cur_v_end = NULL;
	root->buffer_size = fileSize;
	root->buffer = buffer;
	root->cur_tmp = buffer;
	
	return root;
}
/*************************************************
Function: 		Lwca_S_Init
Description:	��ʼ�������ַ���������Ϣ 
Called By:		Lwca_Init

Input:			�ַ��� 
Output:			�ַ���������Ϣ�ṹ
Return:			�ַ���������Ϣ�ṹ
Others:
*************************************************/
static LWCA_RUNING_INFO_NODE *Lwca_S_Init(const char const *string)
{
	int res;
	int fileSize;
	char *buffer;
	int tfd;
	
	LWCA_RUNING_INFO_NODE *root;
	
	fileSize = strlen(string);
	if(0 >= fileSize){
		printf("[ERROR] string is null!\n");
		return NULL;
	}	

	
	buffer = (char *)malloc(fileSize);
	if(buffer == NULL){
		printf("[ERROR] malloc false!\n");
		return NULL;
	}
	
	strcpy(buffer, string);
	
	root = (LWCA_RUNING_INFO_NODE *)malloc(sizeof(LWCA_RUNING_INFO_NODE));
	if(root == NULL){
		close(tfd);
		free(buffer);
		printf("[ERROR] malloc false!\n");
		return NULL;
	}
	
	
	int i=0;
	for(i=0; i<LWCA_SEARCH_HASH_NUM; ++i){
		root->lwca_hash[i]=NULL;
	}
	
	root->cur_layer = 0;
	root->cur_align = 0;
	root->cur_k_start = NULL;
	root->cur_k_end = NULL;
	root->cur_v_start = NULL;
	root->cur_v_end = NULL;
	root->buffer_size = fileSize;
	root->buffer = buffer;
	root->cur_tmp = buffer;
		
	return root;
}
//������һ��ͷָ�� 
char *utils_find_newline_noset(char *string)
{
	int i=0;
	while(*(string+i)!='\0' && *(string+i) != '\n'){
		i++;
	}
	
	if(*(string+i) == '\0'){
		return NULL;
	}


	return (string+i+1);
}
//������һ��ͷָ�� 
char *utils_find_newline(char *string)
{
	int i=0;
	while(*(string+i)!='\0' && *(string+i) != '\n'){
		i++;
	}
	
	if(*(string+i) == '\0'){
		return NULL;
	}
	
	*(string+i)='\0';
	if(*(string+i-1) == '\r'){
		*(string+i-1)='\0';		
	}
	
	return (string+i+1);
}
//ȥ�ո� \r ���ص�һ��\0 
char * utils_move_right_SPACE_R(char *string)
{
	int i=1;
	while(*(string-i)==' ' || *(string-i) == '\r'){
		*(string-i)='\0';
		i++;
	}
	return string-i+1;
}
/*
����ֵ��
NULL ��ʾ����
�ǿգ���ǰ��ʾɨ������һ����ַ 
*/ 

char *utils_find_k_head(char *string, LWCA_RUNING_INFO_NODE *root)
{

	int sum=0;
	int i=0;

	while(1){

		while(*(string+i)!='\0' && (string[i] == ' ' ||  string[i] == '\t')){
			if(*(string+i)=='\0'){
				return NULL;
			}
			if(string[i] == '\t'){
				sum = sum +4;
			}else{
				++sum;
			}
			++i;
		}
		//��һ���� 
		if(*(string+i) == '\n'){
			++i;
			sum=0;
		}else  if(*(string+i)=='\r' && *(string+i+1) == '\n'){
			i=i+2;
			sum=0;
		}else if(*(string+i) == '#'){
			string = utils_find_newline(string+i);
			i=0;
			sum=0; 
		}else{
			root->cur_align=sum;
			root->cur_k_start=string+i;	
			break;
		}
		
	}

	root->cur_align=sum;
	return root->cur_k_start;
}
/*
����ֵ��
NULL ��ʾ����
�ǿգ���ǰ��ʾɨ������һ����ַ 
*/ 
char *utils_find_v_head(char *string, LWCA_RUNING_INFO_NODE *root)
{

	int i=0;

	while(1){

		while(*(string+i)!='\0' && (string[i] == ' ' ||  string[i] == '\t')){
			if(*(string+i)=='\0'){
				root->cur_v_start = NULL;
				return NULL;
			}

			++i;
		}
		//��һ���� 
		if(*(string+i) == '\n'){
			root->cur_v_start = NULL;
			break;
		}else  if(*(string+i)=='\r' && *(string+i+1) == '\n'){
			root->cur_v_start = NULL;
			break;
		}else if(*(string+i) == '#'){
			root->cur_v_start = NULL;
			string = utils_find_newline(string+i);
			break;
		}else{
			root->cur_v_start=string+i;
			break;
		}
		
	}

	return string+i;
}
/*
����ֵ��
NULL ��ʾ����
�ǿգ���ǰ��ʾɨ������һ����ַ 
*/ 
char *utils_find_k_end(char *string, LWCA_RUNING_INFO_NODE *root )
{
	int i=0;
	char *end;
	int j=0;

	while( *(string+i) != ':' ){
		if(*(string+i) == '\n' || *(string+i)=='\0' ){
			//һ����û��key value  �ָ���� 
			return NULL;
		}
		i++;
	}
	*(string+i)='\0'; 

	end = utils_move_right_SPACE_R(string+i);



	root->cur_k_end=end;
		
	return (string+i+1);
}


/*
����ֵ��
NULL ��ʾ����
�ǿգ���ǰ��ʾɨ������һ����ַ 
*/ 
char *utils_find_v_end(char *string, LWCA_RUNING_INFO_NODE *root)
{
	int i=0;
	char *end;
	int j=0;
	

	switch(*string){
		case '"':{
			i=1;
			while(*(string + i) != '"'){
				++i;
			}
			if(*(string + i+1) == '\n' || *(string + i+1) == '\r')break;
			*(string + i+1)='\0';
			break;
		}
		case '{':{
			int k=1;
			i=1;
			while(k !=0 ){
				if( *(string + i) == '{' ){
					++k;
				}
				if( *(string + i) == '}' ){
					--k;
				}
				++i;
			}
			if(*(string + i) == '\n' || *(string + i) == '\r')break;
			*(string + i)='\0';
			break;
		}
		
		
		case '[':{
			break;
		}
		default:{
			i=0;
			while(1){
				
				if(*(string+i)=='#' ){
					break;
				}
				if( string[i] == '\n' ){
					break;
				}
				if(*(string+i)=='\0'){
					return NULL;
				}
				++i;
			}
			
			break;
		}
		
	}
	
	
	//ȥ��ո� 
	end = utils_move_right_SPACE_R(string+i);
	root->cur_v_end=end;

	//������ 
	return utils_find_newline(string+i);;
}

int utils_find_headspace_num(char *string)
{
	int sum=0;
	int i=0;

	while(1){

		while(*(string+i)!='\0' && (string[i] == ' ' ||  string[i] == '\t')){
			if(*(string+i)=='\0'){
				return sum;
			}
			if(string[i] == '\t'){
				sum = sum +4;
			}else{
				++sum;
			}
			++i;
		}
		//��һ���� 
		if(*(string+i) == '\n'){
			++i;
			sum=0;
		}else  if(*(string+i)=='\r' && *(string+i+1) == '\n'){
			i=i+2;
			sum=0;
		}else if(*(string+i) == '#'){
			string = utils_find_newline_noset(string+i);
			i=0;
			sum=0; 
		}else{
			break;
		}
		
	}

	return sum;
} 


void utils_traverse(LWCA_LIST_HEAD *head)
{
	
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*show all list*/
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		int i=0;
		for(i=0; i<node->layer; ++i){
			printf("\t");
		}
		printf("%d|%d|%d [%s]:[%s]\n", node->layer, node->indicia, node->list_num, node->key, node->value);
		
		utils_traverse(&node->next);
	}
	
}
/*************************************************
Function: 		Lwca_Traverse_Node
Description:	�����Ե�ǰ���Ϊ���������ӽ����Ϣ 
Called By:		

Input:			LWCA_NODE ���ָ�� 
Output:			��
Return:			��
Others:			�� 
*************************************************/
void Lwca_Traverse_Node(LWCA_NODE *root)
{
	if(root == NULL){
		printf("Lwca_Traverse_Node parameter is empty\n");
		return; 
	}
	LWCA_LIST_NODE *root2 = (LWCA_LIST_NODE *)root;
	utils_traverse(&root2->next);
}

void utils_serach_hash_set(int layer, char *key, LWCA_RUNING_INFO_NODE *root, LWCA_LIST_NODE *new_node)
{
	char k[100]={0};
	sprintf(k, "%05d%s", layer-1, key);
	
	unsigned t = str_hash(k)%1000;

	
	if(root->lwca_hash[t]){
		LWCA_LIST_NODE *node=NULL;
		node = (LWCA_LIST_NODE *)root->lwca_hash[t];
		LWCA_LIST_HEAD *head = 	&node->hash_list;
		//printf("%d|%s %d\n", new_node->layer, new_node->key, t);
		list_add_tail(&new_node->hash_list, head);
	}else{
		//printf("%d|%s %d\n", new_node->layer, new_node->key, t);
		root->lwca_hash[t] = (LWCA_NODE *)new_node;
	}

}

void *utils_serach_hash_get(int layer, char *key, LWCA_RUNING_INFO_NODE *root)
{
	char k[100]={0};
	sprintf(k, "%05d%s", layer-1, key);

	unsigned t = str_hash(k)%1000;
	
	printf("%x\n", root->lwca_hash[t]);
	if( root->lwca_hash[t] ){
		LWCA_LIST_NODE *node=NULL;
		node = (LWCA_LIST_NODE *)root->lwca_hash[t];
		printf("%d | %s | %d\n", node->layer, node->key, t);
		if(layer == node->layer && strcmp(key, node->key) == 0){
			return (void *)node->value;
		}
		
		LWCA_LIST_HEAD *head = 	&node->hash_list;
		LWCA_LIST_NODE *temp_node;
		/*show all list*/
		list_for_each_entry_safe(node, temp_node, head, hash_list)
		{	
			printf("%d|%s %d\n", node->layer, node->key, t);
			if(layer == node->layer && strcmp(key, node->key) == 0){
				return (void *)node->value;
			}
		}
	}
	return NULL;	
	
}
LWCA_LIST_NODE *utils_serach_node_get(char *key, LWCA_LIST_HEAD *head)
{
	if(head == NULL || key == NULL)return NULL;
	
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*show all list*/
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		if(strcmp(key, node->key) == 0){
			return node;
		}
	}
	return NULL;
}
void utils_destroy(LWCA_LIST_HEAD *head)
{
	
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*del all list*/
again:
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		if(node->next.next == &node->next){
			list_del(&node->list);
			free(node);
		}else if(node->next.next != &node->next){
			utils_destroy(&node->next);		
		}
	}
	if( node->list.next != &node->list ){
 		goto again;
	}
	
}
/*************************************************
Function: 		Lwca_Destroy
Description:	�û�ʹ������root����� 
Called By:		

Input:			LWCA_NODE ����� ָ��
				
Output:			
	
Return:

Others:			�� 
*************************************************/
void Lwca_Destroy(LWCA_NODE *root)
{
	if(root == NULL){
		printf("Lwca_Destroy parameter is empty\n");
		return; 
	}

	LWCA_RUNING_INFO_NODE *root2 = ((LWCA_LIST_NODE *)root)->root;

	if(root2){

		if(root2->buffer){
			free(root2->buffer);
		}
		free(root2);	
	}
	
	utils_destroy(&((LWCA_LIST_NODE *)root)->next);
	free(root);
	return; 
}
/*************************************************
Function: 		utils_analysis
Description:	�������ݽ������������ͽṹ 
Called By:		Lwca_Init

Input:			@layer:��ǰ���㼶
				@root; �����ļ�������Ϣ�ṹ
				@head; ��ǰ���ĸ��������ͽṹ����ͷ 
				
Output:			@head;  ��ǰ�����ӵ�����
				@listNum; ͳ��list �����ӽ����� 
				
Return:			LWCA_TRUE; �����ɹ�
				LWCA_FALSE; ����ʧ�� 
Others:			�� 
*************************************************/
int utils_analysis(int layer,  LWCA_RUNING_INFO_NODE *root, LWCA_LIST_HEAD *head, int *listNum)
{
	int k=0;
	int cur_align;
	int frist_align;
	int i;
	int indicia=0;
	LWCA_LIST_NODE *new_node;

	while(root->cur_tmp !=NULL){
		
		if(k==0){
			frist_align = utils_find_headspace_num(root->cur_tmp);
			cur_align = frist_align;
			k=1;	
		}else{
			cur_align = utils_find_headspace_num(root->cur_tmp);			
		}

		if(cur_align > frist_align){
		//�ӽ�� 
				int res;
				res = utils_analysis(layer+1, root, &new_node->next, &new_node->list_num);
				if(res == LWCA_FALSE)return LWCA_FALSE;
				if(root->cur_tmp == NULL)return LWCA_TRUE;

		}else if(cur_align < frist_align){
		//���ظ���㣻 
			//��������Ϊ0ʱ��ִ�� 
			if(layer != 0){
				return  LWCA_TRUE;
			}else{
			//������Ϊ0,���������������Ա� 
				frist_align = cur_align;
			}
		}			


		//����keyͷ�� 
		root->cur_tmp = utils_find_k_head(root->cur_tmp, root);
		if(root->cur_tmp == NULL){
			//�������� 
			return LWCA_TRUE;
		}

		root->cur_tmp = utils_find_k_end(root->cur_tmp, root);
		if(root->cur_tmp == NULL){
			//�������� ,��keyûֵ 
			return LWCA_TRUE;
		}

		//����valueͷ 
		root->cur_tmp = utils_find_v_head(root->cur_tmp, root);
		if(root->cur_tmp == NULL){
			//�������� 	��keyûֵ 
			return LWCA_TRUE;
		}
		root->cur_tmp = utils_find_v_end(root->cur_tmp, root);
		//���key value

		//������ӽ�� 
		new_node = (LWCA_LIST_NODE *)malloc(sizeof(LWCA_LIST_NODE));
		if(new_node != NULL)
		{
			
			INIT_LIST_HEAD(&new_node->next);
			INIT_LIST_HEAD(&new_node->hash_list);
			
			new_node->next.prev=&new_node->next;
			new_node->next.next=&new_node->next;
			
			new_node->layer=layer;
			new_node->indicia=indicia++;
			new_node->key = root->cur_k_start;
			new_node->value = root->cur_v_start;
			new_node->root = root;
			utils_serach_hash_set(layer, new_node->key, root, new_node);

			new_node->list_num=0;
			list_add_tail(&new_node->list,head);

		}
		
		*listNum = indicia;

		int tlen=0;
		switch(*(new_node->value)){
			case '"'://�Ǵ���ȥ���� 	
				tlen = strlen(new_node->value);
				*(new_node->value+tlen-1) = '\0';				
				new_node->value = new_node->value+1;

				break;
			case '{'://�����ٴη��� 
				tlen = strlen(new_node->value);
				*(new_node->value+tlen-1) = '\0';				
				new_node->value = new_node->value+1;

				break;
			default:
				break;
		}

	}	

	return  LWCA_TRUE;

	
}
/*************************************************
Function: 		Lwca_Init
Description:	�û�ʹ�÷��ؽ���root����� 
Called By:		

Input:			@mode:�ļ������ַ��� 
				@arg; ����
				
Output:			
			
				
Return:			LWCA_NODE ����� ָ�룬 ����ʧ�ܷ���NULL 

Others:			�� 
*************************************************/
LWCA_NODE *Lwca_Init(int mode, char *arg)
{
	LWCA_LIST_NODE *node_root;
	LWCA_RUNING_INFO_NODE *root;
	int res;
	
	node_root = (LWCA_LIST_NODE *)malloc(sizeof(LWCA_LIST_NODE));
	memset(node_root, 0, sizeof(LWCA_LIST_NODE));
	INIT_LIST_HEAD(&node_root->next);
	INIT_LIST_HEAD(&node_root->list);

	if(mode == LWCA_MODE_FILE){
		root = Lwca_F_Init(arg);
		node_root->root = root;
		if(root == NULL){
			printf("lwca init false\n");
			return NULL;
		}
		res = utils_analysis(0, root, &node_root->next, &node_root->list_num);
		if(res == LWCA_FALSE){
			Lwca_Destroy((LWCA_NODE *)node_root);
			return NULL;
		}
	}else if(mode == LWCA_MODE_STR){
		root = Lwca_S_Init(arg);
		node_root->root = root;
		if(root == NULL){
			printf("lwca init false\n");
			return NULL;
		}
		res = utils_analysis(0, root, &node_root->next, &node_root->list_num);
		if(res == LWCA_FALSE){
			Lwca_Destroy((LWCA_NODE *)node_root);
			return NULL;
		}
	}else{
		printf("mode para error\n");
		return NULL;
	}
	
	return (LWCA_NODE *)node_root;
}

/*************************************************
Function: 		Lwca_Get_ListIndex_By_Node
Description:	�û�ʹ��root����㷵�ص�ǰ����index ֵ 
Called By:		

Input:			LWCA_NODE ����� ָ��
				
Output:			
	
Return:			index 

Others:			�� 
*************************************************/
int Lwca_Get_ListIndex_By_Node(LWCA_NODE *node)
{
	if(node == NULL )return -1;
	
	LWCA_LIST_NODE *node2=(LWCA_LIST_NODE *)node;
	return node2->indicia;
}
/*************************************************
Function: 		Lwca_Get_Key_By_Node
Description:	�û�ʹ��root����㷵�ص�ǰ����key ֵ 
Called By:		

Input:			LWCA_NODE ����� ָ��
				
Output:			
	
Return:			ָ��key ֵ��ָ�� 

Others:			�� 
*************************************************/
char *Lwca_Get_Key_By_Node(LWCA_NODE *node)
{
	if(node == NULL )return NULL;
	
	LWCA_LIST_NODE *node2=(LWCA_LIST_NODE *)node;
	
	return (char *)node2->key;
}
/*************************************************
Function: 		Lwca_Get_Value_By_Node
Description:	�û�ʹ��root����㷵�ص�ǰ����value ֵ 
Called By:		

Input:			LWCA_NODE ����� ָ��
				
Output:			
	
Return:			ָ��value ֵ��ָ�� 

Others:			�� 
*************************************************/
char *Lwca_Get_Value_By_Node(LWCA_NODE *node)
{
	if(node == NULL )return NULL;
	
	LWCA_LIST_NODE *node2=(LWCA_LIST_NODE *)node;
	
	return (char *)node2->value;
}
/*************************************************
Function: 		Lwca_Get_Value_By_Node
Description:	�� ���ڵ� ����key���� value ֵ��key����Ϊ������ 
Called By:		

Input:			@root; LWCA_NODE ����� ָ��
				@key����Ϊ����				
Output:			
	
Return:			ָ��value ֵ��ָ�� 

Others:			�� 
*************************************************/
char *Lwca_Get_Value_By_Key(LWCA_NODE *root, char *key)
{
	struct str_split split;
	int i;

	LWCA_LIST_NODE *node=NULL;
	LWCA_LIST_NODE *root2 = ((LWCA_LIST_NODE *)root);
	if(root == NULL){
		printf("Lwca_Get_Value_By_Key %s parameter is empty\n", split.str_array[i]);
		return; 
	}	
	str_split_func(&split, key, ':');	
	LWCA_LIST_HEAD *head=&root2->next;
	for(i=0; i<split.count; ++i)
	{
		node = utils_serach_node_get(split.str_array[i], head);
		if(node){
			head = &node->next;
		}else{
			break;
		}
	}
	
	//�򵽷���value 
	if( i == split.count ){
		str_split_free(&split);
		return Lwca_Get_Value_By_Node((LWCA_NODE *)node);
	}
	str_split_free(&split);	
	return NULL;
	
}
/*************************************************
Function: 		Lwca_Get_Node_By_Key
Description:	�� ���ڵ� ����key���� node��� ָ�루key����Ϊ������ 
Called By:		

Input:			@root; LWCA_NODE ����� ָ��
				@key����Ϊ����				
Output:			
	
Return:			ָ��node ֵ��ָ�� 

Others:			�� 
*************************************************/
LWCA_NODE *Lwca_Get_Node_By_Key(LWCA_NODE *root, char *key)
{
	struct str_split split;
	int i;
	if(root == NULL || key == NULL){
		printf("Lwca_Get_Node_By_Key %s parameter is empty\n", split.str_array[i]);
		return NULL; 
	}	
	
	LWCA_LIST_NODE *node=NULL;
	LWCA_LIST_NODE *root2 = ((LWCA_LIST_NODE *)root);

	str_split_func(&split, key, ':');	
	LWCA_LIST_HEAD *head=&root2->next;
	for(i=0; i<split.count; ++i)
	{
		node = utils_serach_node_get(split.str_array[i], head);
		if(node){
			head = &node->next;
		}else{
			break;
		}
	}
	
	//�򵽷���value 
	if( i == split.count ){
		str_split_free(&split);
		return (LWCA_NODE *)node;
	}
	str_split_free(&split);	
	return NULL;

}
/*************************************************
Function: 		Lwca_Get_Value_By_ListIndex
Description:	�� ���ڵ� ����key���� valueֵ���� 
Called By:		

Input:			@root; LWCA_NODE ����� ָ��
				@index Ϊ�ڵ�ǰ��������				
Output:			
	
Return:			ָ��vlaue ֵ��ָ�� 

Others:			�� 
*************************************************/
char *Lwca_Get_Value_By_ListIndex(LWCA_NODE *root, int index)
{
	if(root == NULL )return NULL;
	LWCA_LIST_HEAD *head=&(((LWCA_LIST_NODE *)root)->next);
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*show all list*/
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		if(node->indicia == index){
			return node->value;
		}
	}
	return NULL;
	
}
/*************************************************
Function: 		Lwca_Get_Node_By_ListIndex
Description:	�� ���ڵ� ����key���� node ָ�루�� 
Called By:		

Input:			@root; LWCA_NODE ����� ָ��
				@index Ϊ�ڵ�ǰ��������				
Output:			
	
Return:			ָ��node ֵ��ָ�� 

Others:			�� 
*************************************************/
LWCA_NODE *Lwca_Get_Node_By_ListIndex(LWCA_NODE *root, int index)
{
	if(root == NULL )return NULL;
	LWCA_LIST_HEAD *head=&(((LWCA_LIST_NODE *)root)->next);
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*show all list*/
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		if(node->indicia == index){
			return (LWCA_NODE *)node;
		}
	}
	return NULL;
	
}
/*************************************************
Function: 		Lwca_Get_Node_ListNum_By_Node
Description:	//���ص�ǰnode�ڵ��list����  
Called By:		

Input:			@root; LWCA_NODE ����� ָ��
				
Output:			
	
Return:			//���ص�ǰnode�ڵ��list���� 

Others:			�� 
*************************************************/
int Lwca_Get_Node_ListNum_By_Node(LWCA_NODE *root)
{
	if(root == NULL )return 0;
	return ((LWCA_LIST_NODE *)root)->list_num;
}
/*************************************************
Function: 		Lwca_Get_Value_By_LayerKey
Description:	//hash���� ,ֻ��������keyֵ����ʱ������ 
Called By:		

Input:			@root; LWCA_NODE ����� ָ��
				@layer
				@key ֻ���ǵ��� 
Output:			
	
Return:			//���ص�ǰkey�ڵ��value 

Others:			�� 
*************************************************/
char *Lwca_Get_Value_By_LayerKey(LWCA_NODE *root, int layer, char *key)
{
	return (char *)utils_serach_hash_get(layer, key, ((LWCA_LIST_NODE *)root)->root);
}
