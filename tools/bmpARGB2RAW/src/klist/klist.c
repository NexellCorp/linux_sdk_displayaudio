#include <stdio.h>
#include <stdlib.h>

#include "klist.h"

void init(struct list *lptr)
{
	lptr->count = 0;
	lptr->head = NULL;
}

int insert(struct list *lptr, void *value)
{
	struct node *new_node = (struct node *)(malloc(sizeof(struct node)));

	new_node->value = value;

	if (!lptr->head)
		lptr->head = new_node;
	else {
		struct node *tail = lptr->head;

		while (!(tail->next == NULL))
			tail = tail->next;

		tail->next = new_node;
	}

	lptr->count++;

	return lptr->count;
}

void delete(struct list *lptr, int pos)
{
	int i = 0;

	struct node *head = lptr->head;
	struct node *tmp = NULL;
	struct node *del_node = head;

	if (head == NULL) {
		printf("list data is NULL!\n");
		return;
	}

	if (pos <= 0 || lptr->count < pos) {
		printf("position value index error!\n");
		return;
	}

	for (i = 0; i < pos - 1; i++) {
		tmp = del_node;
		del_node = del_node->next;
	}

	if (!tmp)
		lptr->head = del_node->next;
	else
		tmp->next = del_node->next;

	if (del_node->value)
		free(del_node->value);

	free(del_node);

	lptr->count--;
}

void *get_data(struct list *lptr, int pos)
{
	int i = 1;
	struct node *head = lptr->head;

	if (head == NULL) {
		printf("list data is NULL!\n");
		return NULL;
	}

	if (pos <= 0 || lptr->count < pos) {
		printf("position value index error!\n");
		return NULL;
	}

	while (head != NULL) {
		if (pos == i)
			break;
		i++;
		head = head->next;
	}

	return head->value;
}

void modify(struct list *lptr, void *value, int pos)
{
	if (pos < 1 || pos > lptr->count) {
		printf("Position Out of Bound!!\n");
		return;
	}

	struct node *tmp = lptr->head;
	int i;

	for (i = 1; i < pos; i++)
		tmp = tmp->next;

	tmp->value = value;
}

int delete_all(struct list *lptr)
{
	struct node *tmp = lptr->head;

	if (lptr->count <= 0) {
		printf("Position Out of Bound!!\n");
		return lptr->count;
	}

	while (tmp != NULL) {
		struct node *tmp2 = tmp->next;

		if (tmp->value)
			free(tmp->value);

		free(tmp);
		lptr->count--;

		lptr->head = tmp2;
		tmp = lptr->head;
	}

	return lptr->count;
}

void print_list(struct list *lptr)
{
	struct node *tmp = lptr->head;
	printf("[List value]\n");

	while (tmp != NULL) {
		printf("%p\n", tmp);
		tmp = tmp->next;
	}

	printf("Total: %d value(s)\n", lptr->count);
}
