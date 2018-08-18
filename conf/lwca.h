/*****************************************************************************
Copyright: 2016-2026, liuzc
File name: lwca.h (�������ý��� Light weight configuration analysis)
Description: ����ʹ�ýӿ� 

Author: liuzc
Version: 1.0
Date: ��
History: 
	2016/11/22 �����ļ� 
*****************************************************************************/


#ifndef _LWCA_H__
#define _LWCA_H__


#ifdef __cplusplus
extern "C" {
#endif 

#define LWCA_MODE_FILE 1
#define LWCA_MODE_STR 0

#define LWCA_SEARCH_HASH_NUM 1000

typedef char  LWCA_NODE; 

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
LWCA_NODE *Lwca_Init(int mode, char *arg);
/*************************************************
Function: 		Lwca_Destroy
Description:	�û�ʹ������root����� 
Called By:		

Input:			LWCA_NODE ����� ָ��
				
Output:			
	
Return:

Others:			�� 
*************************************************/
void Lwca_Destroy(LWCA_NODE *root);

/*************************************************
Function: 		Lwca_Traverse_Node
Description:	�����Ե�ǰ���Ϊ���������ӽ����Ϣ 
Called By:		

Input:			LWCA_NODE ���ָ�� 
Output:			��
Return:			��
Others:			�� 
*************************************************/
void Lwca_Traverse_Node(LWCA_NODE *root);


/*************************************************
Function: 		Lwca_Get_ListIndex_By_Node
Description:	�û�ʹ��root����㷵�ص�ǰ����index ֵ 
Called By:		

Input:			LWCA_NODE ����� ָ��
				
Output:			
	
Return:			index 

Others:			�� 
*************************************************/
int Lwca_Get_ListIndex_By_Node(LWCA_NODE *node);
/*************************************************
Function: 		Lwca_Get_Key_By_Node
Description:	�û�ʹ��root����㷵�ص�ǰ����key ֵ 
Called By:		

Input:			LWCA_NODE ����� ָ��
				
Output:			
	
Return:			ָ��key ֵ��ָ�� 

Others:			�� 
*************************************************/
char *Lwca_Get_Key_By_Node(LWCA_NODE *node);
/*************************************************
Function: 		Lwca_Get_Value_By_Node
Description:	�û�ʹ��root����㷵�ص�ǰ����value ֵ 
Called By:		

Input:			LWCA_NODE ����� ָ��
				
Output:			
	
Return:			ָ��value ֵ��ָ�� 

Others:			�� 
*************************************************/
char *Lwca_Get_Value_By_Node(LWCA_NODE *node);
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
char *Lwca_Get_Value_By_Key(LWCA_NODE *root, char *key);
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
LWCA_NODE *Lwca_Get_Node_By_Key(LWCA_NODE *root, char *key);
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
char *Lwca_Get_Value_By_ListIndex(LWCA_NODE *root, int index);
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
LWCA_NODE *Lwca_Get_Node_By_ListIndex(LWCA_NODE *root, int index);
/*************************************************
Function: 		Lwca_Get_Node_ListNum_By_Node
Description:	//���ص�ǰnode�ڵ��list����  
Called By:		

Input:			@root; LWCA_NODE ����� ָ��
				
Output:			
	
Return:			//���ص�ǰnode�ڵ��list���� 

Others:			�� 
*************************************************/
int Lwca_Get_Node_ListNum_By_Node(LWCA_NODE *root);
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
char *Lwca_Get_Value_By_LayerKey(LWCA_NODE *root, int layer, char *key);



// ****************************************************************************

#ifdef __cplusplus
}
#endif  // cplusplus 

#endif  //  

// ****************************************************************************
