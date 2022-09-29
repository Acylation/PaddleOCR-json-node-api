// Copyright (c) 2022 hiroi-sora. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// https://github.com/hiroi-sora/PaddleOCR-json

#ifndef _tools_flags_
#define _tools_flags_

// �����ڲ�ʹ�ã�������ı�־��
#define CODE_INIT                0   // ÿ�غϳ�ʼֵ���غϽ���ʱ��Ϊ�������ܹܿص�������δ���ִ���
#define CODE_ERR_MAT_NULL       -999 // OCR������matΪ�ա����� cls_label �����ݡ�

// ����ı�־��

// ʶ��ɹ�
#define CODE_OK                 100 // �ɹ�����ʶ�������
#define CODE_OK_NONE            101 // �ɹ�����δʶ������
#define MSG_OK_NONE(p)          "No text found in image. Path: \""+p+"\""
// ��·����ͼ��ʧ��
#define CODE_ERR_PATH_EXIST     200 // ͼƬ·��������
#define MSG_ERR_PATH_EXIST(p)   "Image path dose not exist. Path: \""+p+"\""
#define CODE_ERR_PATH_CONV      201 // ͼƬ·��string�޷�ת����wstring
#define MSG_ERR_PATH_CONV(p)    "Image path failed to convert to utf-16 wstring. Path: \""+p+"\""
#define CODE_ERR_PATH_READ      202 // ͼƬ·�����ڣ����޷����ļ�
#define MSG_ERR_PATH_READ(p)    "Image open failed. Path: \""+p+"\""
#define CODE_ERR_PATH_DECODE    203 // ͼƬ�򿪳ɹ�������ȡ���������޷���opencv����
#define MSG_ERR_PATH_DECODE(p)  "Image decode failed. Path: \""+p+"\""
// �������ͼ��ʧ��
#define CODE_ERR_CLIP_OPEN      210 // �������ʧ�� ( OpenClipboard )
#define MSG_ERR_CLIP_OPEN       "Clipboard open failed."
#define CODE_ERR_CLIP_EMPTY     211 // ������Ϊ�� ( GetPriorityClipboardFormat NULL )
#define MSG_ERR_CLIP_EMPTY      "Clipboard is empty."
#define CODE_ERR_CLIP_FORMAT    212 // ������ĸ�ʽ��֧�� ( GetPriorityClipboardFormat -1 )
#define MSG_ERR_CLIP_FORMAT     "Clipboard format is not valid."
#define CODE_ERR_CLIP_DATA      213 // �������ȡ���ݾ��ʧ�ܣ�ͨ���ɱ�ĳ���ռ�ü��������� ( GetClipboardData NULL )
#define MSG_ERR_CLIP_DATA       "Getting clipboard data handle failed."
#define CODE_ERR_CLIP_FILES     214 // �������ѯ�����ļ���������Ϊ1 ( DragQueryFile != 1 )
#define MSG_ERR_CLIP_FILES(n)   "Clipboard number of query files is not valid. Number: "+std::to_string(n)
#define CODE_ERR_CLIP_GETOBJ    215 // ���������ͼ�ζ�����Ϣʧ�� ( GetObject NULL )
#define MSG_ERR_CLIP_GETOBJ     "Clipboard get bitmap object failed."
#define CODE_ERR_CLIP_BITMAP    216 // �������ȡλͼ����ʧ�� ( GetBitmapBits �����ֽ�Ϊ�� )
#define MSG_ERR_CLIP_BITMAP     "Getting clipboard bitmap bits failed."
#define CODE_ERR_CLIP_CHANNEL   217 // ��������λͼ��ͨ������֧�� ( nChannels ��Ϊ1��3��4 )
#define MSG_ERR_CLIP_CHANNEL(n) "Clipboard number of image channels is not valid. Number: "+std::to_string(n)
// δ֪
#define CODE_ERR_UNKNOW        299 // δ֪�쳣
#define MSG_ERR_UNKNOW         "An unknown error has occurred."



#endif