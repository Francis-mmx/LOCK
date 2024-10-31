#ifndef LBUF_H
#define LBUF_H


#include "typedef.h"
#include "list.h"
#include "system/spinlock.h"


struct lbuff_head {
    int magic_a;
    struct list_head head;
    struct list_head free;
    spinlock_t lock;
    u16 align;
    u16 priv_len;
    u32 total_size;
    u32 last_addr;
    void *priv;
    int magic_b;
};

struct lbuff_state {
    u32 avaliable;
    u32 fragment;
    u32 max_continue_len;
    int num;
};



struct lbuff_head *lbuf_init(void *buf, u32 len, int align, int priv_head_len);

void *lbuf_alloc(struct lbuff_head *head, u32 len);

void *lbuf_realloc(struct lbuff_head *head, void *lbuf, int size);

int lbuf_empty(struct lbuff_head *head);

void lbuf_clear(struct lbuff_head *head);

void lbuf_push(struct lbuff_head *head, void *lbuf, u8 channel_map);

void *lbuf_pop(struct lbuff_head *head, u8 channel);

void lbuf_free(struct lbuff_head *head, void *lbuf);

u32 lbuf_free_space(struct lbuff_head *head);

void lbuf_state(struct lbuff_head *head, struct lbuff_state *state);

void lbuf_dump(struct lbuff_head *head);

int lbuf_traversal(struct lbuff_head *head);

/**********************************
 *  设备读写对应的lbuf管理
 *
 *
 * *******************************/
struct dev_lbuf_map {
    void *head_buf; /*管理的head buffer*/
    u32  head_len;/*头的大小*/
    u32  addr; /*设备地址*/
    u32  len; /*设备长度*/
    int  align; /*对齐长度*/
    int  dev_align;
    int  priv_len;
};

u32 dlbuf_mapping(void *lbuf);

struct lbuff_head *dlbuf_init(struct dev_lbuf_map *map);

void *dlbuf_alloc(struct lbuff_head *head, u32 len);

void *dlbuf_realloc(struct lbuff_head *head, void *lbuf, int size);

int dlbuf_empty(struct lbuff_head *head);

void dlbuf_clear(struct lbuff_head *head);

void dlbuf_push(struct lbuff_head *head, void *lbuf, u8 channel_map);

void dlbuf_repush(struct lbuff_head *head, void *lbuf, u8 channel_map);

void *dlbuf_pop(struct lbuff_head *head, u8 channel);

void dlbuf_free(struct lbuff_head *head, void *lbuf);

u32 dlbuf_free_space(struct lbuff_head *head);

void dlbuf_state(struct lbuff_head *head, struct lbuff_state *state);

void dlbuf_dump(struct lbuff_head *head);

int dlbuf_traversal(struct lbuff_head *head);
#endif


