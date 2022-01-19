#ifndef USER_IDENTIFIER_H_DEFINED
#define USER_IDENTIFIER_H_DEFINED

#define USER_ID_FILE_NAME "user_id.txt"

// User identifier is expected to be 28 characters long
#define USER_IDENTIFIER_SIZE 29

void set_user_identifier();
void get_copy_of_user_identifier();

#endif
