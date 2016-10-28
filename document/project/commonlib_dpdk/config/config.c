#include "config.h"

static struct lib_config *config = NULL;

/* Return a pointer to the configuration structure */
struct lib_config *
lib_get_configuration(void)
{
	if(config == NULL)
		perror("config is not create");
	return config;
}


void
lib_config_create(void)
{
	void *lib_cf_addr;
	int fd;
	int retval;


	fd = open(RUNTIME_CONFIG_PATH, O_RDWR | O_CREAT | O_TRUNC, 0660);
	if (fd < 0) {
		perror("Cannot open  RUNTIME_CONFIG_PATH for lib_config\n");
	}
	retval = ftruncate(fd, sizeof(struct lib_config));
	if (retval < 0){
		close(fd);
		perror("Cannot resize ' RUNTIME_CONFIG_PATH' for lib_config\n");
	}
	lib_cf_addr = mmap(NULL, sizeof(struct lib_config),
			   PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (lib_cf_addr == NULL){
		perror("Cannot mmap memory for lib_config\n");
	}
	config = (struct lib_config *) lib_cf_addr;
	memset(config, 0, sizeof(struct lib_config));
	config->version = 1;
	config->magic = 100;
}

void
lib_config_destroy(void)
{
	void *lib_cf_addr;
	int retval;
	
	lib_cf_addr = (void*)config;
	if (lib_cf_addr == NULL)
	{
		return ;
	}
	retval = munmap(lib_cf_addr, sizeof(struct lib_config));
	if(retval < 0)
	{
		perror("munmap config fail");
	}		
	
	config = NULL;
}


