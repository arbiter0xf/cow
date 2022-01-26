#ifndef COW_USER_IDENTIFIER_H_DEFINED
#define COW_USER_IDENTIFIER_H_DEFINED

#define USER_ID_FILE_NAME "user_id.txt"

// User identifier is expected to be 28 characters long
#define USER_IDENTIFIER_SIZE 29

void get_copy_of_user_identifier(char* user_identifier_copy);

int create_user_identifier_if_none(void);
int load_user_identifier(void);

#endif
