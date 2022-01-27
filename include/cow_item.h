#ifndef COW_FORMULATE_ITEM_H_DEFINED
#define COW_FORMULATE_ITEM_H_DEFINED

#define ITEM_SIZE 1024

// 8 + '\0'
#define HEADER_TAG_SIZE 9

struct item {
	char data[ITEM_SIZE];
	int unused_bytes;
};

int item_append(struct item* item, const char* new_data);
void item_initialize(struct item* item);
void item_count_unused_bytes(struct item* item);

#endif
