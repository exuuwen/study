#include <stdio.h>
#include <string.h>

#define HEXSTR_LEN 16
/**/

int main(void)
{
       
  	char hexstr[HEXSTR_LEN+1] = "1234567890ABCDEF";
        unsigned char hex[HEXSTR_LEN/2] = {0x00};
        int tmp = 0;
        int i = 0;
        unsigned char hex_ch[2] = {0x00};
       
        printf("hex str: %s\n", hexstr);
        printf("hex val: ");
        for(i = 0; i < strlen(hexstr)/2; i++)
        {
                memset(hex_ch, 0x00, 2);
                memcpy(hex_ch, hexstr + 2 * i, 2);
                sscanf(hex_ch, "%2x", &tmp);   //must be %2x
		printf("tmp:%x ", tmp);
                hex[i] = (unsigned char)tmp;
                printf("%.2x ", hex[i]);
                tmp = 0;
        } 
        printf("\n");
        return 0;
}
