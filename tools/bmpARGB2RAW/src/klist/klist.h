#ifndef __KLIST_H__
#define __KLIST_H__

struct node {
	void *value;
	struct node* next;
};

struct list {
	int count;
	struct node* head;
};

void init(struct list *lptr);
int insert(struct list *lprt, void *value);
void delete(struct list *lptr, int pos);
void *get_data(struct list *lprt, int pos);
void modify(struct list *lptr, void *value, int pos);
void print_list(struct list *lptr);
int delete_all(struct list *lptr);

#endif
