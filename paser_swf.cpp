/*
 *  paser_swf.cpp
 *
 *  Created on: Oct 20, 2011
 *  Author : sijiewang
 *  Email  : lnmcc@hotmail.com
 *  Site   : lnmcc.net
 */

#include <bitset>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
//#include <error.h>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <zlib.h>

#define MAX_LENGTH 17

using namespace std;


typedef struct 
{
	char m_FileSpec[4];
	bool m_Compressed;
	int  m_Version;
	unsigned long  m_FileSize;
	unsigned long  m_RectLen;
	int m_xmin;
	int m_xmax;
	int m_ymin;
	int m_ymax;
	int m_Width;
	int m_Height;
	float m_FrameRate;
	unsigned int m_FrameCounts;
}FlashHead;



int main(int argc ,char** argv)
{
	FlashHead fh;
	ifstream file;
	fh.m_Compressed=0;
	file.open(argv[1],ios_base::binary);
	unsigned int header;
	file.read((char*)&header,4);

	fh.m_Version=(header >> 24) & 255;

	if((header & 0x00FFFFFF)!=0x00535746 && (header & 0x00FFFFFF)!=0x00535743)
	{
		printf("file does not start with a SWF header\n");
		return -1;
	}
	
	file.seekg(4,ios::beg);
	file.read((char*)&fh.m_FileSize,4);

	const bool compressed=(header & 255)=='C';
	unsigned char* pBytes=new  unsigned char[fh.m_FileSize];
	unsigned char* pDestBytes=new unsigned  char[fh.m_FileSize*2];
	file.seekg(8,ios::beg);
	file.read((char*)pBytes,fh.m_FileSize-8);
	if(compressed)
	{
		fh.m_Compressed=1;
		//printf("The swf is compressed\n");
		int nReturn=uncompress((unsigned char*)pDestBytes,&fh.m_FileSize,(unsigned char*)pBytes,fh.m_FileSize-8);
		if(nReturn)
		{
			cout<<"uncompress failed"<<endl;
            file.close();
			return -1;
		}
	}
	else
		memcpy(pDestBytes,pBytes,sizeof(pBytes));
	bitset<1024> bt;
	bitset<8> bt_tmp;
	bt.reset();
	bt_tmp.reset();
	memcpy((char*)&bt,pDestBytes,sizeof(bt));
//	cout<<"bt="<<bt<<endl;

	for(int i=1;i<128;i++)
	{
		for(int j=(i-1)*8,k=0;j<=(i*8-1);j++,k++)
		{
			bt_tmp[k]=bt[j];
		}
		
		for(int j=(i-1)*8,k=7;j<=(i*8-1);j++,k--)
		{
			bt[j]=bt_tmp[k];	
		}
	}

//	cout<<"new bt="<<bt<<endl;
	bt_tmp.reset();	
	for(int i=0;i<5;i++)
		bt_tmp[4-i]=bt[i];

	fh.m_RectLen=bt_tmp.to_ulong();
	
	bitset<32> bt_32;
	bt_32.reset();
	bt=(bt>>5);
	for(int i=0;i<fh.m_RectLen;i++)
		bt_32[fh.m_RectLen-1-i]=bt[i];
	fh.m_xmin=bt_32.to_ulong();	


	bt=(bt>>fh.m_RectLen);
	bt_32.reset();
	for(int i=0;i<fh.m_RectLen;i++)
		bt_32[fh.m_RectLen-1-i]=bt[i];
	fh.m_xmax=bt_32.to_ulong();	


	bt=(bt>>fh.m_RectLen);
	bt_32.reset();
	for(int i=0;i<fh.m_RectLen;i++)
		bt_32[fh.m_RectLen-1-i]=bt[i];
	fh.m_ymin=bt_32.to_ulong();	


	bt=(bt>>fh.m_RectLen);
	bt_32.reset();
	for(int i=0;i<fh.m_RectLen;i++)
		bt_32[fh.m_RectLen-1-i]=bt[i];
	fh.m_ymax=bt_32.to_ulong();	


	int need=ceil((fh.m_RectLen*4+5) / 8.0f);
	//printf("need=%d\n",need);
	bt=(bt>>(8*need-3*fh.m_RectLen-5));

	bt=(bt>>8);
	bt_32.reset();
	for(int i=0;i<8;i++)
		bt_32[8-1-i]=bt[i];
	fh.m_FrameRate=bt_32.to_ulong();

	bt=(bt>>8);
	bt_32.reset();
	for(int i=0;i<8;i++)
		bt_32[8-1-i]=bt[i];
	bt=(bt>>8);
	for(int i=0;i<8;i++)
		bt_32[16-1-i]=bt[i];
	fh.m_FrameCounts=bt_32.to_ulong();
	
	fh.m_Width=(fh.m_xmax-fh.m_xmin)/20;
	fh.m_Height=(fh.m_ymax-fh.m_ymin)/20;

	file.close();

    if (argc == 2) {
        cout<<"Version="<<fh.m_Version<<endl;
        cout<<"Xmin="<<fh.m_xmin<<endl;
        cout<<"Xmax="<<fh.m_xmax<<endl;
        cout<<"Ymin="<<fh.m_ymin<<endl;
        cout<<"Ymax="<<fh.m_ymax<<endl;
        cout<<"FrameRate="<<fh.m_FrameRate<<endl;
        cout<<"FrameCounts="<<fh.m_FrameCounts<<endl;
        cout<<"Compressed="<<fh.m_Compressed<<endl;
        cout<<"Width="<<fh.m_Width<<endl;
        cout<<"Height="<<fh.m_Height<<endl;
    } else if (argc == 3) { 
        cout<<"{"<<endl;
        cout<<"\"frame_rate\" : \""<<fh.m_FrameRate<<"\","<<endl;
        cout<<"\"frame_count\": \""<<fh.m_FrameCounts<<"\","<<endl;
        cout<<"\"compressed\" : \""<<fh.m_Compressed<<"\","<<endl;
        cout<<"\"width\"      : \""<<fh.m_Width<<"\","<<endl;
        cout<<"\"height\"     : \""<<fh.m_Height<<"\""<<endl;
        cout<<"}"<<endl;
    }
	return 0;
}
