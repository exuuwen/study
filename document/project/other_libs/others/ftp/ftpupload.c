#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/types.h>
#include <sys/stat.h>
#include <errno.h>

#pragma message("compile with -lcurl option")
#pragma message("./main ftp://user:path@192.168.60.133/aa test")
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	curl_off_t nread;
	/* in real-world cases, this would probably get this data differently
	 as this fread() stuff is exactly what the library already would do
	 by default internally */ 
	size_t retcode = fread(ptr, size, nmemb, stream);

	nread = (curl_off_t)retcode;

	fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);
	return retcode;
}
 
static int CurlUpload(const char* url, const char* filename)
{
	CURL *curl;
	CURLcode res;
	FILE *hd_src;
	struct stat file_info;
	curl_off_t fsize;
	
	int ret = -1;
	/* get the file size of the local file */ 
	if (stat(filename, &file_info)) 
	{
		perror("can not open:");
		return -1;
	}
	fsize = (curl_off_t)file_info.st_size;

	printf("Local file size: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", fsize);

	/* get a FILE * of the same file */ 
	hd_src = fopen(filename, "rb");

	/* In windows, this will init the winsock stuff */ 
	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */ 
	curl = curl_easy_init();
	if(curl) 
	{
		ret = 0;
		/* we want to use our own read function */ 
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

		/* enable uploading */ 
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* specify target */ 
		curl_easy_setopt(curl,CURLOPT_URL, url);

		/* now specify which file to upload */ 
		curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

		/* Set the size of the file to upload (optional).  If you give a *_LARGE
		   option you MUST make sure that the type of the passed-in argument is a
		   curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
		   make sure that to pass in a type 'long' argument. */ 
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);

		/* Now run off and do what you've been told! */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));		
			ret = -1;	
		}

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}

	fclose(hd_src); /* close the local file */ 
	curl_global_cleanup();

	return ret;
}


int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("usage: ./main  ftp_uri  uploadfile\n");
		return -1;
	}


	int ret = CurlUpload(argv[1], argv[2]);
	if(ret != 0)
	{
		printf("upload error %d\n", ret);
		return -1;
	}

	printf("%s: success to upload %s to %s\n", __FUNCTION__, argv[2], argv[1]);

	return 0;

}
