#include <stdio.h>
#include <stdlib.h>

#include "klist.h"

struct data {
	int index;
};

int main(int argc, char *argv[])
{
	struct list list;
	struct data *data;
	int i;
	struct data *out;

	init(&list);

	for (i = 0; i<10; i++) {
		data = (struct data *)malloc(sizeof(struct data));

		data->index = i+1;

		insert(&list, (void *)data);
	}

	for (i = 0; i<10; i++) {
		out = (struct data *)get_data(&list, i + 1);
		printf("data->index : %d\n", out->index);
	}

	//delete(&list, 1);
	//delete(&list, 2);
	//delete(&list, 3);
	//delete(&list, 10);

	print_list(&list);

	delete_all(&list);

	return 0;
}
