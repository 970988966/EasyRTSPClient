/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
#include <stdio.h>
#include <string.h>
#include "EasyRTSPClientAPI.h"

#ifdef _WIN32
#define KEY "6A59754D6A3469576B5A7541736E56587149704B7065314659584E35556C525455454E73615756756443356C65475570567778576F50306C34456468646D6C754A6B4A68596D397A595541794D4445325257467A65555268636E6470626C526C5957316C59584E35"
#else
#define KEY "6A59754D6A354F576B597141736E56587149704B7066466C59584E35636E527A63474E736157567564434E58444661672F535867523246326157346D516D466962334E68514449774D545A4659584E355247467964326C75564756686257566863336B3D"
#endif

char fRTSPURL[256] = { 0 };

Easy_RTSP_Handle fRTSPHandle = 0;

/* RTSPClient���ݻص� */
int Easy_APICALL __RTSPClientCallBack( int _chid, void *_chPtr, int _frameType, char *_pBuf, RTSP_FRAME_INFO* _frameInfo)
{
	if (_frameType == EASY_SDK_VIDEO_FRAME_FLAG)//�ص���Ƶ���ݣ�����00 00 00 01ͷ
	{
		if (_frameInfo->codec == EASY_SDK_VIDEO_CODEC_H264)
		{

			/*::fwrite(_pBuf, 1, _frameInfo->length, f264);*/
			/* 
				ÿһ��I�ؼ�֡����SPS+PPS+IDR�����
				|---------------------|----------------|-------------------------------------|
				|                     |                |                                     |
				0-----------------reserved1--------reserved2-------------------------------length
			*/
			if (_frameInfo->type == EASY_SDK_VIDEO_FRAME_I)
			{
				char sps[2048] = { 0 };
				char pps[2048] = { 0 };
				char* IFrame = NULL;
				unsigned int spsLen,ppsLen,iFrameLen = 0;

				spsLen = _frameInfo->reserved1;							// SPS���ݳ���
				ppsLen = _frameInfo->reserved2 - _frameInfo->reserved1;	// PPS���ݳ���
				iFrameLen = _frameInfo->length - spsLen - ppsLen;		// IDR���ݳ���

				memcpy(sps, _pBuf, spsLen);			//SPS���ݣ�����00 00 00 01
				memcpy(pps, _pBuf+spsLen, ppsLen);	//PPS���ݣ�����00 00 00 01
				IFrame = _pBuf + spsLen + ppsLen;	//IDR���ݣ�����00 00 00 01

				printf("Get I H264 SPS/PPS/IDR Len:%d/%d/%d \ttimestamp:%u.%u\n", spsLen, ppsLen, iFrameLen, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
			}
			else if (_frameInfo->type == EASY_SDK_VIDEO_FRAME_P)
			{
				printf("Get P H264 Len:%d \ttimestamp:%u.%u\n", _frameInfo->length, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
			}
		}
		else if (_frameInfo->codec == EASY_SDK_VIDEO_CODEC_MJPEG)
			printf("Get MJPEG Len:%d \ttimestamp:%u.%u\n", _frameInfo->length, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
		else if (_frameInfo->codec == EASY_SDK_VIDEO_CODEC_MPEG4)
			printf("Get MPEG4 Len:%d \ttimestamp:%u.%u\n", _frameInfo->length, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
	}
	else if (_frameType == EASY_SDK_AUDIO_FRAME_FLAG)//�ص���Ƶ����
	{
		if (_frameInfo->codec == EASY_SDK_AUDIO_CODEC_AAC)
			printf("Get AAC Len:%d \ttimestamp:%u.%u\n", _frameInfo->length, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
		else if (_frameInfo->codec == EASY_SDK_AUDIO_CODEC_G711A)
			printf("Get PCMA Len:%d \ttimestamp:%u.%u\n", _frameInfo->length, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
		else if (_frameInfo->codec == EASY_SDK_AUDIO_CODEC_G711U)
			printf("Get PCMU Len:%d \ttimestamp:%u.%u\n", _frameInfo->length, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
		else if (_frameInfo->codec == EASY_SDK_AUDIO_CODEC_G726)
			printf("Get G.726 Len:%d \ttimestamp:%u.%u\n", _frameInfo->length, _frameInfo->timestamp_sec, _frameInfo->timestamp_usec);
	}
	else if (_frameType == EASY_SDK_EVENT_FRAME_FLAG)//�ص�����״̬�¼�
	{
		// ��ʼ���ӻ�������ʧ���ٴν�������
		if (NULL == _pBuf && NULL == _frameInfo)
		{
			printf("Connecting:%s ...\n", fRTSPURL);
		}

		// �ж�֡���!
		else if (NULL != _frameInfo && _frameInfo->type == 0xF1)
		{
			printf("Lose Packet:%s ...\n", fRTSPURL);
		}

		// ���ӶϿ�
		else if (NULL != _frameInfo && _frameInfo->codec == EASY_SDK_EVENT_CODEC_ERROR)
		{
			printf("Error:%s��%d ...\n", fRTSPURL, EasyRTSP_GetErrCode(fRTSPHandle));
		}

		else if (NULL != _frameInfo && _frameInfo->codec == EASY_SDK_EVENT_CODEC_EXIT)
		{
			printf("Exit:%s��%d ...\n", fRTSPURL, EasyRTSP_GetErrCode(fRTSPHandle));
		}


	}
	else if (_frameType == EASY_SDK_MEDIA_INFO_FLAG)//�ص���ý����Ϣ
	{
		if(_pBuf != NULL)
		{
			EASY_MEDIA_INFO_T mediainfo;
			memset(&mediainfo, 0x00, sizeof(EASY_MEDIA_INFO_T));
			memcpy(&mediainfo, _pBuf, sizeof(EASY_MEDIA_INFO_T));
			printf("RTSP DESCRIBE Get Media Info: video:%u fps:%u audio:%u channel:%u sampleRate:%u \n", 
				mediainfo.u32VideoCodec, mediainfo.u32VideoFps, mediainfo.u32AudioCodec, mediainfo.u32AudioChannel, mediainfo.u32AudioSamplerate);
		}
	}
	return 0;
}

void usage(char const* progName) 
{
  printf("Usage: %s <rtsp-url> \n", progName);
}

int main(int argc, char** argv)
{
	int isActivated = 0;
	// We need at least one "rtsp://" URL argument:
	if (argc < 2) 
	{
		usage(argv[0]);
		printf("Press Enter exit...\n");
		getchar();
		return 1;
	}

	isActivated = EasyRTSP_Activate(KEY);
	switch(isActivated)
	{
	case EASY_ACTIVATE_INVALID_KEY:
		printf("KEY is EASY_ACTIVATE_INVALID_KEY!\n");
		break;
	case EASY_ACTIVATE_TIME_ERR:
		printf("KEY is EASY_ACTIVATE_TIME_ERR!\n");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_LEN_ERR:
		printf("KEY is EASY_ACTIVATE_PROCESS_NAME_LEN_ERR!\n");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_ERR:
		printf("KEY is EASY_ACTIVATE_PROCESS_NAME_ERR!\n");
		break;
	case EASY_ACTIVATE_VALIDITY_PERIOD_ERR:
		printf("KEY is EASY_ACTIVATE_VALIDITY_PERIOD_ERR!\n");
		break;
	case EASY_ACTIVATE_PLATFORM_ERR:
		printf("EASY_ACTIVATE_PLATFORM_ERR!\n");
		break;
	case EASY_ACTIVATE_COMPANY_ID_LEN_ERR:
		printf("EASY_ACTIVATE_COMPANY_ID_LEN_ERR!\n");
		break;
	case EASY_ACTIVATE_SUCCESS:
		printf("KEY is EASY_ACTIVATE_SUCCESS!\n");
		break;
	}

	if(EASY_ACTIVATE_SUCCESS != isActivated)
	{
		getchar();
		return -1;
	}

	sprintf(fRTSPURL, "%s", argv[1]);

	//����RTSPSource
	EasyRTSP_Init(&fRTSPHandle);

	// ���Ը���fRTSPHanlde�жϣ�Ҳ���Ը���EasyRTSP_Init�Ƿ񷵻�0�ж�
	if (NULL == fRTSPHandle) return 0;
	
	// �������ݻص�����
	EasyRTSP_SetCallback(fRTSPHandle, __RTSPClientCallBack);

	// ���ý�����Ƶ����Ƶ����
	unsigned int mediaType = EASY_SDK_VIDEO_FRAME_FLAG | EASY_SDK_AUDIO_FRAME_FLAG;

	// ��RTSP��
	EasyRTSP_OpenStream(fRTSPHandle, 0, fRTSPURL, EASY_RTP_OVER_TCP, mediaType, 0, 0, NULL, 1000, 0, 0, 3);

	printf("Press Enter exit...\n");
	getchar();
   
	// �ر�RTSPClient
	EasyRTSP_CloseStream(fRTSPHandle);

	// �ͷ�RTSPHandle
	EasyRTSP_Deinit(&fRTSPHandle);
	fRTSPHandle = NULL;

    return 0;
}