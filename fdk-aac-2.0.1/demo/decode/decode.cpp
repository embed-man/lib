/*************************************************************************
    > File Name: main.cpp
    > Author: xzy
    > Mail: 
    > Created Time: 2021年06月23日 星期三 11时24分00秒
 ************************************************************************/

#include<iostream>
#include "stdio.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include "stdlib.h"
#include "string.h"
#include "aacdecoder_lib.h"
#define D printf
using namespace std;
unsigned int get_aac_len(char* str)
{
    if ( !str ) {
        return 0;
    }
    unsigned int len = 0;
    unsigned char f_bit = str[3] & 0xff;
    unsigned char m_bit = str[4] & 0xff;
    unsigned char b_bit = str[5] & 0xff;
    len += (b_bit>>5);
    len += (m_bit<<3);
    len += ((f_bit&3)<<11);
    return len;
}
int get_file_len(int fd)
{
    struct stat st;
    fstat(fd, &st);
    return (st.st_size);
}
void d_atds_head(void *p)
{
    if (p == NULL) {
        return ;
    }
    char *atds_head = (char*)p;
    D("1:0x%x\n", atds_head[0]);
    D("2:0x%x\n", atds_head[1]);
    D("3:0x%x\n", atds_head[2]);
    D("4:0x%x\n", atds_head[3]);
    D("5:0x%x\n", atds_head[4]);
    D("6:0x%x\n", atds_head[5]);
    D("7:0x%x\n", atds_head[6]);
}
int main()
{
    int pcm_fd = -1;
    int aac_fd = -1;
    int pcm_file_len = 0x00;
    int aac_file_len = 0x00;
    int aac_len = 0x00;
    int aac_offset = 0x00;
    unsigned int aac_valid = 0;
    unsigned int aac_size = 0;
    int aac_index = 0;
    char *mmap_fp = NULL;

    unsigned char *decoder_fill_p = NULL;
	HANDLE_AACDECODER handle;
	handle = aacDecoder_Open(TT_MP4_ADTS , 1);
	AAC_DECODER_ERROR err;
    unsigned int decsize=16* 2048 * sizeof(INT_PCM);
    unsigned char* decdata=(unsigned char*)malloc(decsize);

    pcm_fd = open("./audio.pcm", O_WRONLY);
    aac_fd = open("./audio.aac", O_RDWR);
    if (aac_fd < 0 || pcm_fd < 0) {
        D("aac fd:%d  pcm fd:%d\n", aac_fd, pcm_fd); 
        return -1;
    }
    aac_file_len = get_file_len(aac_fd);
    D("aac_file_len1:%d\n", aac_file_len);
    mmap_fp = (char*)mmap(NULL, aac_file_len, PROT_READ | PROT_WRITE, MAP_PRIVATE, aac_fd, 0);
    if (mmap_fp == NULL) {
        return -1;
    }
    while (aac_file_len > aac_offset) {

        d_atds_head(mmap_fp + aac_offset); 
        aac_len = get_aac_len(mmap_fp + aac_offset);
        aac_valid = aac_len ;
        aac_size = aac_len ;
#if 1

        decoder_fill_p = (unsigned char*)(mmap_fp + aac_offset);
		err=aacDecoder_Fill(handle, (unsigned char**)&decoder_fill_p, &aac_size, &aac_valid);
		if(err > 0)
			cout<<"fill err:"<<err<<endl;
		err = aacDecoder_DecodeFrame(handle, (INT_PCM *)decdata,decsize / sizeof(INT_PCM), 0);
		if(err> 0)
			cout<<"dec err:"<<err<<endl;
		CStreamInfo *info = aacDecoder_GetStreamInfo(handle);
		cout<<"channels"<<info->numChannels<<endl;
		cout<<"sampleRate"<<info->sampleRate<<endl;
		cout<<"frameSize"<<info->frameSize<<endl;

        if (write(pcm_fd, decdata, info->numChannels*info->frameSize*2) <= 0) {
            perror("write pcm_fd error");
            return -1;
        }

#endif
        aac_offset += aac_len;
        D("frame_num:%d--aac_len:%d--aac_offset:%d\n", aac_index, aac_len, aac_offset);
        aac_index++;
        //aac_file_len -= aac_offset;
    }

    if (mmap_fp) {
        munmap(mmap_fp, aac_file_len);
    }
    free(decdata);
    return 0;
}
