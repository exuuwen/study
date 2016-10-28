#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/types.h>

#pragma message("compile with -lcurl option")
#pragma message("./main ftp://user:path@192.168.60.133/test.c test")
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	curl_off_t nwrite;
	int written = fwrite(ptr, size,nmemb, (FILE*)stream);
	
	nwrite = (curl_off_t)written;

	fprintf(stderr, "*** We write %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nwrite);
	return written;
}
int CurlDownload(const char* url, const char* filename)
{
	FILE *fp;
	int ret = -1;
	CURLcode code;
	
	if((fp = fopen(filename,"w")) == NULL)
	{
		printf("%s: failed to open file(%s) to write", __FUNCTION__, filename);
		return ret;
	}

	printf("Download \"%s\" ==> \"%s\"\n", url, filename);

	CURL *curl;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl)
	{
		code = curl_easy_setopt(curl, CURLOPT_URL, url);
		if(code != CURLE_OK)
		{
			printf("%s: fail to set curl URL option", __FUNCTION__);
			goto out;
		}
		code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		if(code != CURLE_OK)
		{
			printf("%s: fail to set curl WRITEDATA option", __FUNCTION__);
			goto out;
		}
		
		code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		if(code != CURLE_OK)
		{
			printf("%s: fail to set curl CURLOPT_WRITEFUNCTION option", __FUNCTION__);
			goto out;
		}
		code = curl_easy_perform(curl);
		if(code != CURLE_OK)
		{
			printf("%s: fail to download %s to %s\n", __FUNCTION__, url, filename);
			goto out;
		}

		ret = 0;
	}
out:
	curl_global_cleanup();
	fclose(fp);

	return ret;
}



int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("usage: ./main  ftp_uri  savefile\n");
		return -1;
	}


	int ret = CurlDownload(argv[1], argv[2]);
	if(ret != 0)
	{
		printf("download error %d\n", ret);
		return -1;
	}

	printf("%s: success to download %s to %s\n", __FUNCTION__, argv[1], argv[2]);

	return 0;

}

