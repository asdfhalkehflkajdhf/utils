#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mem_pool.h"

typedef struct mem_pool_list
{
	struct list_head list;//池连
	struct list_head order_head;//回收连头
}mem_pool_l;
typedef struct mem_node_head
{
	mem_pool_o *mem_pool_h;//池头
	mem_pool_l *mem_pool_l;//池buff头
	struct list_head order_list;//回收连
	struct list_head size_list;//可用分配连
	int size;//可用大小
	int used;//使用状态
}mem_node_h;

typedef struct mem_unused_hash_
{
	int key;
	struct rb_node rb;
	struct list_head unused_head;
	int count;
}mem_unused_hash;

#define MEM_BLC_HEAD_OFFSET (sizeof(mem_pool_l))
#define MEM_OBJ_HEAD_OFFSET (sizeof(mem_node_h))

#define MEM_OBJ_HEAD_GET(ptr) ((mem_node_h *)((void *)ptr-MEM_OBJ_HEAD_OFFSET))

enum 
{
	USED,
	UNUSED,
};

static void insert(mem_unused_hash *node, struct rb_root *root)
{
	struct rb_node **new = &root->rb_node, *parent = NULL;
	int key = node->key;

	while (*new) {
		parent = *new;
		if (key < rb_entry(parent, mem_unused_hash, rb)->key)
			new = &parent->rb_left;
		else
			new = &parent->rb_right;
	}

	rb_link_node(&node->rb, parent, new);
	rb_insert_color(&node->rb, root);
}

static inline void erase(mem_unused_hash *node, struct rb_root *root)
{
	rb_erase(&node->rb, root);
}
mem_unused_hash *search(int key, struct rb_root *root)
{
	struct rb_node *node = root->rb_node;
	struct rb_node *p_node=node;
	mem_unused_hash *data;

	while (node) {
		data = container_of(node, mem_unused_hash, rb);

		if (key < data->key){
			p_node=node;
			node = node->rb_left;
		}else if (key > data->key){
			p_node=node;
			node = node->rb_right;
		}else{
			return data;
		}
	}

	if(p_node != NULL){
		data = container_of(p_node, mem_unused_hash, rb);
		return data;
	}
	return NULL;
}

inline void unused_hash_inc(mem_unused_hash *hash_head,mem_pool_o *mem_pool)
{
	hash_head->count++;
	if(hash_head->count == 1){
		insert(hash_head, &mem_pool->rb_root);
	}
}
inline void unused_hash_dec(mem_unused_hash *hash_head,mem_pool_o *mem_pool)
{
	hash_head->count--;
	if(hash_head->count == 0){
		erase(hash_head, &mem_pool->rb_root);
	}
}


inline void MEM_OBJ_USED_HASH_LIST_INIT(mem_pool_l *pool_buff_h, mem_pool_o *mem_pool_h)
{
	mem_node_h *ptr=NULL;
	
	int objcount= mem_pool_h->objcount;
	int objsize = mem_pool_h->objsize;
	ptr = (mem_node_h *)((void *)pool_buff_h + MEM_BLC_HEAD_OFFSET);

	ptr->mem_pool_h = mem_pool_h;
	ptr->mem_pool_l = pool_buff_h;

	ptr->size = objcount*(objsize + MEM_OBJ_HEAD_OFFSET) - MEM_OBJ_HEAD_OFFSET;
	ptr->used = UNUSED;
	mem_unused_hash *unused_hash=(mem_unused_hash *)mem_pool_h->unused_hash;

	list_add_tail(&ptr->size_list, &unused_hash[objcount-1].unused_head);
	unused_hash_inc(&unused_hash[objcount-1], mem_pool_h);

	list_add_tail(&ptr->order_list, &pool_buff_h->order_head);
	return ;
}



/*
return: pool buff ptr
*/
void *mem_p_show_list(mem_pool_o *mem_pool)
{
	mem_node_h *node=NULL;	
	mem_node_h *tem;

	mem_unused_hash *hash_index=(mem_unused_hash *)mem_pool->unused_hash;

	int i;
	for(i=0; i<mem_pool->objcount; i++)
	{
		if(hash_index[i].count == 0){
			continue;
		}
		printf("id:%d c:%d | ",i, hash_index[i].count);
		list_for_each_entry_safe(node, tem, &hash_index[i].unused_head, size_list){
			printf("[%p %d %d] ", node , node->size, node->used);
		}
		printf("\n");
	}

	mem_pool_l *p_node=NULL;
	mem_pool_l *tem_node=NULL;

	
	list_for_each_entry_safe(p_node, tem_node, &mem_pool->pool_list, list){
		printf("order %p | ", p_node);
		list_for_each_entry_safe(node, tem, &p_node->order_head, order_list){
			printf("[%p %d %d] ", node , node->size, node->used);
		}
		printf("\n");
	}
	return NULL;
	p_node = node->mem_pool_l;
	list_for_each_entry_safe(node, tem, &p_node->order_head, order_list){
		printf("[%p %d %d] ", node , node->size, node->used);
	}
	printf("\n");
	return NULL;
}

/*
return: pool buff ptr
*/
mem_pool_l *mem_p_blc_calloc_get_buff(mem_pool_o *mem_pool, unsigned int objcount)
{
	void *buff=NULL;
	mem_pool_l *newpool=NULL;
	int block_size;
	block_size = objcount*(mem_pool->objsize + MEM_OBJ_HEAD_OFFSET) + MEM_BLC_HEAD_OFFSET;

	buff = calloc(1, block_size);
	if(buff == NULL){
		return NULL;
	}
	newpool = (mem_pool_l *)buff;
	
	INIT_LIST_HEAD(&newpool->order_head);
	list_add_tail(&newpool->list, &mem_pool->pool_list);
	return newpool;
}

/*
	mem_p_obj*  是基于对象结构的api
*/
mem_pool_o *mem_p_obj_create(int objsize, int objcount, int inc, char *name)
{
	int i;
	mem_pool_o *mem_pool=NULL;
	if(objsize<=0 || objcount<=0 || name == NULL){
		return NULL;
	}
	mem_pool = (mem_pool_o *)calloc(1, sizeof(mem_pool_o));
	if(mem_pool == NULL){
		return NULL;
	}
	mem_pool->inc_count = inc;
	mem_pool->objsize = objsize;
	//mem_pool->unused_count = objcount;
	//mem_pool->used_count = 0;
	mem_pool->objcount = objcount;
	INIT_LIST_HEAD(&mem_pool->pool_list);

	mem_pool->unused_hash=(void *)malloc(sizeof(mem_unused_hash)*(objcount+1));
	mem_unused_hash *unused_hash=(mem_unused_hash *)mem_pool->unused_hash;
	for(i=0; i<objcount; i++){
		INIT_LIST_HEAD(&unused_hash[i].unused_head);
		unused_hash[i].count=0;
		unused_hash[i].key = i;
	}
	pthread_spin_init(&mem_pool->lock, PTHREAD_PROCESS_PRIVATE);
	mem_pool->name = strdup(name);
	if(mem_pool->name == NULL){
		printf("mem pool name error\n");
		mem_p_obj_destroy(mem_pool);
		return NULL;
	}
	mem_pool_l *pool_buff = mem_p_blc_calloc_get_buff(mem_pool, mem_pool->objcount);
	if(pool_buff == NULL){
		printf("mem pool name error\n");
		mem_p_obj_destroy(mem_pool);
		return NULL;
	}

	MEM_OBJ_USED_HASH_LIST_INIT(pool_buff, mem_pool);

	return mem_pool;
}
/*
	最好是在所以malloc free 之后再执行
*/
void mem_p_obj_destroy(mem_pool_o *mem_pool)
{
	mem_pool_l *node;
	mem_pool_l *tem_node;
	if(mem_pool == NULL){
		return ;
	}
	if(mem_pool->name!=NULL){
		free(mem_pool->name);
	}
	if(mem_pool->unused_hash!=NULL){
		free(mem_pool->unused_hash);
	}

	list_for_each_entry_safe(node, tem_node, &mem_pool->pool_list, list){
		list_del(&node->list);
		free(node);
	}
	
    pthread_spin_destroy(&mem_pool->lock);
	free(mem_pool);
	return ;
}

static int mem_p_obj_realloc(mem_pool_o *mem_pool)
{
	if(mem_pool->inc_count == 0){
		return -1;
	}
	mem_pool_l *pool_buff = mem_p_blc_calloc_get_buff(mem_pool, mem_pool->inc_count);
	if(pool_buff == NULL){
		return -1;
	}

	MEM_OBJ_USED_HASH_LIST_INIT(pool_buff, mem_pool);
	return 0;
}
static inline void *mem_p_obj_malloc_from_list(mem_unused_hash *hash_head, int index, int size)
{
	mem_node_h *node=NULL;
	mem_node_h *tem_node=NULL;
	int objcount;
	int objsize;
	int node_size;
	if(hash_head->count == 0 || ((hash_head->key+1)*index) >size){
		return NULL;
	}
	node = list_entry(hash_head->unused_head.next, typeof(*node), size_list);
	list_del(&node->size_list);
	unused_hash_dec(hash_head, node->mem_pool_h);

	
	objcount= node->mem_pool_h->objcount;
	objsize = node->mem_pool_h->objsize;
	node_size=node->size;

	if(node == NULL) return NULL;
	node->used = USED;
	if(node_size-size >= objsize + MEM_OBJ_HEAD_OFFSET){
		tem_node = (mem_node_h *)((void *)node + MEM_OBJ_HEAD_OFFSET + size);
		tem_node->mem_pool_h = node->mem_pool_h;
		tem_node->mem_pool_l = node->mem_pool_l;
		tem_node->used = UNUSED;
		tem_node->size = node_size-size - MEM_OBJ_HEAD_OFFSET;
		
		list_add_head(&tem_node->order_list,&node->order_list);
		
		int hash_index = (node_size-size - MEM_OBJ_HEAD_OFFSET)/objsize;
		//防止下标超界
		if(hash_index >= objcount)hash_index=objcount-1;
		mem_unused_hash *unused_hash=(mem_unused_hash *)node->mem_pool_h->unused_hash;
		list_add_tail(&tem_node->size_list, &unused_hash[hash_index].unused_head);
		unused_hash_inc(&unused_hash[hash_index], tem_node->mem_pool_h);
	}
	node->size = size;

	return (void *)node+MEM_OBJ_HEAD_OFFSET;
}
void *mem_p_obj_malloc(mem_pool_o *mem_pool, int size)
{
	void *node=NULL;
	int hash_index = size/mem_pool->objsize;
	int objcount = mem_pool->objcount;
	mem_unused_hash *unused_hash=NULL;
	
	//防止下标超界
	if(hash_index >= objcount)hash_index=objcount-1;

realloc:
	unused_hash = search(hash_index, &mem_pool->rb_root);
	if(unused_hash == NULL){
		if(mem_p_obj_realloc(mem_pool)){
			return NULL;
		}
		hash_index = objcount-1;
		goto realloc;
	}else{
		node = mem_p_obj_malloc_from_list(&unused_hash[hash_index], hash_index, size);
	}
	return node;
}

static inline void mem_p_obj_del_from_order_list(mem_node_h *ptr_h)
{
	list_del(&ptr_h->size_list);
	return ;
}

static inline void mem_p_obj_del_from_size_list(mem_node_h *ptr_h)
{
	mem_pool_o *mem_pool=ptr_h->mem_pool_h;
	mem_unused_hash *unused_hash=(mem_unused_hash *)mem_pool->unused_hash;
	list_del(&ptr_h->size_list);
	unused_hash_dec(&unused_hash[ptr_h->size/mem_pool->objsize], mem_pool);
	return ;
}
static inline void mem_p_obj_del_from_size_and_order_list(mem_node_h *ptr_h)
{
	mem_p_obj_del_from_size_list(ptr_h);
	list_del(&ptr_h->order_list);
	return ;
}
static inline void mem_p_obj_add_to_size_list(mem_node_h *ptr_h)
{
	mem_pool_o *mem_pool=ptr_h->mem_pool_h;
	int hash_index;
	int objcount= mem_pool->objcount;
	int objsize = mem_pool->objsize;
	mem_unused_hash *unused_hash=(mem_unused_hash *)mem_pool->unused_hash;
	ptr_h->used = UNUSED;
	hash_index = ptr_h->size/objsize;
	//防止下标超界
	if(hash_index >= objcount)hash_index=objcount-1;
	list_add_tail(&ptr_h->size_list, &unused_hash[hash_index].unused_head);
	unused_hash_inc(&unused_hash[hash_index], mem_pool);
	return ;
}

void mem_p_obj_free(void *ptr)
{
	if(ptr == NULL){
		return ;
	}
	mem_node_h *ptr_h = MEM_OBJ_HEAD_GET(ptr);
	mem_node_h *pnode=NULL;
	mem_node_h *nnode=NULL;

	if(ptr_h->order_list.prev != &ptr_h->mem_pool_l->order_head){
		pnode = list_entry(ptr_h->order_list.prev, typeof(*pnode), order_list);
		/*
		list_for_each_exit_safe(pnode, tem_node, &ptr_h->order_list, order_list){
			break;
		}
	*/

	}
	if(ptr_h->order_list.next != &ptr_h->mem_pool_l->order_head){
		nnode = list_entry(ptr_h->order_list.next, typeof(*nnode), order_list);
		/*
		list_for_each_entry_safe(nnode, tem_node, &ptr_h->order_list, order_list){
			break;
		}
	*/
	}
	if(pnode == NULL && nnode->used == USED){
		//printf("1\n");
		mem_p_obj_add_to_size_list(ptr_h);
	}else if(pnode == NULL && nnode->used == UNUSED){
		//printf("2\n");
		ptr_h->size += nnode->size+MEM_OBJ_HEAD_OFFSET;
		mem_p_obj_add_to_size_list(ptr_h);
		mem_p_obj_del_from_size_and_order_list(nnode);
		
	}else if(nnode == NULL && pnode->used == USED){
		//printf("3\n");
		mem_p_obj_add_to_size_list(ptr_h);
	}else if(nnode == NULL && pnode->used == UNUSED){
		//printf("4\n");
		ptr_h->size += pnode->size+MEM_OBJ_HEAD_OFFSET;
		mem_p_obj_add_to_size_list(ptr_h);
		mem_p_obj_del_from_size_and_order_list(pnode);
	}else if(pnode->used == USED && nnode->used == USED){
		//printf("5\n");
		mem_p_obj_add_to_size_list(ptr_h);
	}else if(pnode->used == USED && nnode->used == UNUSED){
		//printf("6\n");
		ptr_h->size += nnode->size+MEM_OBJ_HEAD_OFFSET;
		mem_p_obj_add_to_size_list(ptr_h);
		mem_p_obj_del_from_size_and_order_list(nnode);
	}else if(pnode->used == UNUSED && nnode->used == USED){
		//printf("7\n");
		mem_p_obj_del_from_size_list(pnode);
		pnode->size += ptr_h->size+MEM_OBJ_HEAD_OFFSET;
		mem_p_obj_add_to_size_list(pnode);
		mem_p_obj_del_from_order_list(ptr_h);
	}else if(pnode->used == UNUSED && nnode->used == UNUSED){
		//printf("8\n");
		mem_p_obj_del_from_size_and_order_list(nnode);
		mem_p_obj_add_to_size_list(pnode);
		pnode->size += ptr_h->size+MEM_OBJ_HEAD_OFFSET+nnode->size+MEM_OBJ_HEAD_OFFSET;
		mem_p_obj_add_to_size_list(pnode);
		mem_p_obj_del_from_order_list(ptr_h);
	}

	return ;

}


void *mem_p_obj_malloc_safe(mem_pool_o *mem_pool, int size)
{
	void *res=NULL;
	pthread_spin_lock(&mem_pool->lock);
	res = mem_p_obj_malloc(mem_pool, size);
	pthread_spin_unlock(&mem_pool->lock);
	return res;
}
void mem_p_obj_free_safe(void *ptr)
{
	mem_node_h *ptr_h = MEM_OBJ_HEAD_GET(ptr);
	mem_pool_o *mem_pool=ptr_h->mem_pool_h;
	pthread_spin_lock(&mem_pool->lock);
	mem_p_obj_free(ptr);
	pthread_spin_unlock(&mem_pool->lock);
	return ;
}




