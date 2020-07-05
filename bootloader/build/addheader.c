/* 
** 在BL0阶段，iROM内固化的代码读取nandflash或SD卡前面最大16K的内容（即BL1）到iRAM， 
** 并比对前16字节中的校验和是否正确，正确则继续，错误则尝试启动下一个设备。 
** BL1的头信息规定如下 
** 0x0：BL1的大小（最大16K，包括BL1头信息的大小） 
** 0x4: 0（规定） 
0** 0x8：校验和 
** 0xC：0（规定） 
*/  
#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
  
#define IMG_SIZE                (16*1024)  
#define HEADER_SIZE             16  
#define BLKSIZE                 512  
 
int main (int argc, char *argv[])  
{  
    FILE            *fp_s, *fp_d;  
    unsigned        char *Buf;  
    int             BufLen;  
    int             nbytes, fileLen; 
	unsigned int    checksum, count;  
	int             i;  
  
    if (argc != 3)  
    {  
        printf("Usage: %s <source file> <destination file>\n", argv[0]);  
        return -1;  
    }  
  
    /* 分配16K的buffer */  
    BufLen = IMG_SIZE;  
    Buf = (unsigned char *)malloc(BufLen);  
    if (!Buf)  
    {  
        perror("Alloc buffer failed!");  
        return -1;  
    }  
    memset(Buf, 0x00, BufLen);  
  
    /* 读源bin到buffer */  
    fp_s = fopen(argv[1], "rb");  
    if( fp_s == NULL)  
    {  
       perror("source file open error");  
       free(Buf);  
        return -1;  
    }  
    /* 获取源bin长度 */  
    fseek(fp_s, 0L, SEEK_END);  
    fileLen = ftell(fp_s);  
    fseek(fp_s, 0L, SEEK_SET);  
  
    /* 源bin长度不得超过16K-16byte */  
    fileLen = (fileLen < (IMG_SIZE - HEADER_SIZE)) ? fileLen : (IMG_SIZE - HEADER_SIZE);  
  
    /* 读源bin到buffer[16] */  
    nbytes = fread(Buf + HEADER_SIZE, 1, fileLen, fp_s);  
    if (nbytes != fileLen)  
    {  
        perror("source file read error\n");  
        free(Buf);  
        fclose(fp_s);  
        return -1;  
    }  
      
    /* 计算校验和 */  
    for(i = 0, checksum = 0; i < fileLen; i++)  
        checksum += Buf[HEADER_SIZE + i];  
  
    /* 计算BL1的大小: 
    ** BL1的大小包括BL1的头信息 
    ** 另外iROM从SD卡拷贝是按块拷贝的，因此这里需要调整大小为512字节的整数倍 
    */  
    fileLen += HEADER_SIZE;  
    count = fileLen / BLKSIZE * BLKSIZE;  
    if (count < fileLen)  
        count += BLKSIZE;

    memcpy(Buf, &count, 4);     // 保存BL1的大小到Buf[0-3]  
  
    // 将校验和保存在buffer[8~15]  
    memcpy(Buf + 8, &checksum, 4);  
  
    fp_d = fopen(argv[2], "wb");  
    
    if (fp_d == NULL)  
    {  
        perror("destination file open error");  
        free(Buf);  
        return -1;  
    }  
    // 将count + HEADER_SIZE字节的buffer拷贝到目的bin中 
    nbytes  = fwrite(Buf, 1, count, fp_d);

    if (nbytes != count)  
    {  
        perror("destination file write error");  
        free(Buf);  
        fclose(fp_s);  
        return -1;  
    }  
    
		while(!feof(fp_s))	/*判断是否到文件尾*/
		{
				nbytes  =  fread(Buf, 1, 1024 * 10, fp_s);
			  if( nbytes != fwrite(Buf, 1, nbytes, fp_d))
				{
						perror("destination file write error");  
						free(Buf);  
						fclose(fp_s); 
					    fclose(fp_d); 
						return -1;  	
				}						
		}

    free(Buf);  
    fclose(fp_s);  
    fclose(fp_d);  
  
    return 0;  
}  