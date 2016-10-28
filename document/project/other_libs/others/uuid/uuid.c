#include <uuid/uuid.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
/*libgcrypt11-dev*/
/*uuid-dev*/
/*gcc -o uuid uuid.c -luuid*/
#define UUID_STR_LEN 40
/* generate uuid string */


char * get_uuid(char * in, int in_len)
{
    if (in && in_len < UUID_STR_LEN)
	{
        return NULL;
	}

    if (in == NULL) 
	{
        in = malloc(UUID_STR_LEN);
        if (in == NULL)
		{
            return NULL;
		}
        bzero(in, UUID_STR_LEN);
    }

    uuid_t u;
    uuid_generate(u);
    uuid_unparse(u, in);
    return in;
}
int main()
{
	char * uuid = get_uuid(NULL, 0);
	printf("uuid:%s\n", uuid);
	free(uuid);
	
	return 0;
}
