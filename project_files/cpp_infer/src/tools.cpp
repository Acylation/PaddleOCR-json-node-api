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


#include <fstream>
#include <iostream>
#include <windows.h>
#include <filesystem>
#include <include/args.h>
#include <include/nlohmann_json.hpp>
#include <include/tools_flags.h> // ��־
// ��ͼ���
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/opencv.hpp" // TODO ������ʾͼƬ�ã���ʽʱ����Ҫ
// ����ת��
#include<codecvt>
std::wstring_convert<std::codecvt_utf8<wchar_t>> conv_Ustr_Wstr; // string utf-8 �� wstring utf-16 ��˫��ת����

using namespace std;
using namespace nlohmann;


namespace tool {
  // ============== ִ��״̬ ======================

  int ToolCode = 0; // ���汾�غϴ�����
  string ToolMsg = ""; // ���汾�غϴ�����ʾ
  // ��ȡ״̬
  void get_state(int& code, string& msg) {
	code = ToolCode;
	msg = ToolMsg;
  }
  // ����״̬
  void set_state(int code = CODE_INIT, string msg = "") {
	ToolCode = code;
	ToolMsg = msg;
  }
  // ר��������Ϣ��wstringתstring��ת��ʧ��ʱ����Ĭ����ʾ����
  string msg_wstr_2_ustr(wstring& msg) {
	try {
	  string msgU8 = conv_Ustr_Wstr.to_bytes(msg); // ת��u8
	  return msgU8;
	}
	catch (...) {
	  return "wstring failed to convert to utf-8 string";
	}
  }

  // ================ �� ==========================

	// �˳�����ǰ��ͣ
	void exit_pause(int x = 1) {
	  cout << "OCR exit." << endl; // �˳���ʾ 
	  if (FLAGS_use_system_pause) system("pause");
	  exit(x);
	}
 
  // ���ֽ�ANSI�ַ�����ת���ַ�����
  wchar_t* char_2_wchar(char* c) {
	setlocale(LC_ALL, ""); // �������������Ϊwindowsϵͳ��ǰ����
	size_t lenWchar = mbstowcs(NULL, c, 0); // �õ�תΪ���ַ����ĳ���
	wchar_t* wc = new wchar_t[lenWchar + 1]; // ����ļ����Ŀ��ַ���
	int n = mbstowcs(wc, c, lenWchar + 1); // ���ֽ�ת���ַ�
	setlocale(LC_ALL, "C"); // ��ԭ��������ΪĬ��
	return wc;
  }

  // =============== ���� ==============================

  // ����һ��json
  std::string load_json_str(string& str_in, bool& is_image, bool& is_hotupdate) {
	is_image = false;
	is_hotupdate = false;
	string logstr = "";
	try {
	  auto j = json::parse(str_in); // תjson���� 
	  for (auto& el : j.items()) { // ������ֵ 
		string value = to_string(el.value());
		int vallen = value.length();
		if (vallen > 2 && value[0] == '\"' && value[vallen - 1] == '\"') {
		  value = value.substr(1, vallen - 2); // ɾȥnlohmann�ַ�������������
		}
		if (el.key() == "image_dir" || el.key() == "image_path") { // ͼƬ·��
		  //str_in = utf8_2_gbk(value);
		  str_in = value; // ֱ�ӷ���utf-8
		  is_image = true;
		}
		else { // ��������
		  // ����һ�����á��Զ������Ƿ���ںͺϷ��ԡ����ȼ���������������
		  string getlog = google::SetCommandLineOption(el.key().c_str(), value.c_str());
		  int getlen = getlog.length();
		  if (getlen > 0 && (getlog[getlen - 1] == '\n' || getlog[getlen - 1] == '\r\n')) {
			getlog = getlog.substr(0, getlen - 1); // ɾȥSetCommandLineOption��β����
		  }
		  logstr += getlog + ". ";
		  is_hotupdate = true;
		}
	  }
	  if (!is_image) { // תjson�ɹ�����json��û��ͼƬ·�� 
		str_in = "";
	  }
	}
	catch (...) {
	  logstr = "[Error] Load json fail.";
	  is_hotupdate = true;
	}
	return logstr;
  }

  // ����һ�������ı��еļ���ֵ���ɹ�����true�� 
  bool get_line_config(string s, string& key, string& value) {
	int i = 0;
	// ȥ����һ���ո񣬽�ȡ��һ��key
	while (s[i] != '\0' && s[i] != ' ')  ++i;
	if (s[i] == '\0' || i == 0) return false;
	key = s.substr(0, i);

	// ȥ���ո����������ȡ�ڶ���value
	while (s[i] != '\0' && s[i] == ' ')  ++i;
	if (s[i] == '\0') return false;
	value = s.substr(i);
	return true;
  }

  // ��image_dir���������ļ�������д��FLAGS
  void load_congif_file() {
	// ��ȡ����Flags
	std::vector<google::CommandLineFlagInfo> allFlags;
	GetAllFlags(&allFlags);

	// ���������ļ�
	bool isDefaultDir = false;
	if (FLAGS_config_path == "") { // δ���������ļ�·������Ĭ��·��Ϊ ������+��׺ 
	  string exe_name = google::ProgramInvocationShortName();
	  string::size_type dPos = exe_name.rfind('.'); // ���һ����λ��
	  FLAGS_config_path = exe_name.substr(0, dPos) + (string)"_config.txt";
	  isDefaultDir = true;
	}
	ifstream inConf(FLAGS_config_path);
	if (!inConf) { // ���������ļ�������ʱ����
	  if (!isDefaultDir) std::cerr << "[ERROR] config path not exist! config_dir: " << FLAGS_config_path << endl;
	  return;
	}

	// ���������ļ�
	string s;
	while (getline(inConf, s)) // ���ж�����Ϣ 
	{
	  if (s[0] == '#') continue; // �ų�ע�� 
	  string key, value;
	  if (!get_line_config(s, key, value)) continue; // ����һ������ 

	   // ����һ�����á��Զ������Ƿ���ںͺϷ��ԡ����ȼ���������������
	  google::SetCommandLineOptionWithMode(key.c_str(), value.c_str(), google::SET_FLAG_IF_DEFAULT);
	}
	inConf.close();
  }

  // ================ ��ͼ =============================

  // ���·��pathW�Ƿ�Ϊ�ļ����Ƿ���true
  bool is_exists_wstr(wstring pathW) {
	struct _stat buf;
	int result = _wstat((wchar_t*)pathW.c_str(), &buf);
	if (result != 0) { // ��������
	  return false;
	}
	if (S_IFREG & buf.st_mode) { // ���ļ�
	  return true;
	}
	// else if (S_IFDIR & buf.st_mode) { // ��Ŀ¼
	   //return false;
	// }
	return false;
  }

  // ���� cv::imread ����·��pathW����һ��ͼƬ��pathW����Ϊunicode��wstring
  cv::Mat imread_wstr(wstring pathW, int flags = cv::IMREAD_COLOR) {
    string pathU8 = msg_wstr_2_ustr(pathW); // ��ת��utf-8���Ա��������
    // �� �����������Ҫ��������CF_UNICODETEXT�ȸ��ã����ܵ��÷�ֻ���ṩwstring�����Զ��һ��ת��һ�Ρ�
    if (!is_exists_wstr(pathW)) { // ·��������
      set_state(CODE_ERR_PATH_EXIST, MSG_ERR_PATH_EXIST(pathU8)); // ����״̬��·�����������޷����
      return cv::Mat();
    }
    FILE* fp = _wfopen((wchar_t*)pathW.c_str(), L"rb"); // wpathǿ������ת����whar_t�����Դ��ļ�
    if (!fp) { // ��ʧ��
      set_state(CODE_ERR_PATH_READ, MSG_ERR_PATH_READ(pathU8)); // ����״̬���޷���ȡ
      return cv::Mat();
    }
    // ���ļ������ڴ�
    fseek(fp, 0, SEEK_END); // ������ fp ���ļ�λ��Ϊ SEEK_END �ļ���ĩβ
    long sz = ftell(fp); // ��ȡ�� fp �ĵ�ǰ�ļ�λ�ã����ܴ�С���ֽڣ�
    char* buf = new char[sz]; // ����ļ��ֽ�����
    fseek(fp, 0, SEEK_SET); // ������ fp ���ļ�λ��Ϊ SEEK_SET �ļ��Ŀ�ͷ
    long n = fread(buf, 1, sz, fp); // �Ӹ����� fp ��ȡ���ݵ� buf ��ָ��������У����سɹ���ȡ��Ԫ������
    cv::_InputArray arr(buf, sz); // ת��ΪOpenCV����
    cv::Mat img = cv::imdecode(arr, flags); // �����ڴ����ݣ����cv::Mat����
    delete[] buf; // �ͷ�buf�ռ�
    fclose(fp); // �ر��ļ�
    if (!img.data) {
      set_state(CODE_ERR_PATH_DECODE, MSG_ERR_PATH_DECODE(pathU8)); // ����״̬������ʧ��
    }
    return img;
  }

  // �Ӽ��������һ��ͼƬ��
  cv::Mat imread_clipboard(int flags = cv::IMREAD_COLOR) {
    // �ο��ĵ��� https://docs.microsoft.com/zh-cn/windows/win32/dataxchg/using-the-clipboard

    // ���Դ򿪼����壬��������ֹ����Ӧ�ó����޸ļ���������
    if (!OpenClipboard(NULL)) {
      set_state(CODE_ERR_CLIP_OPEN, MSG_ERR_CLIP_OPEN); // ����״̬���������ʧ��
    }
    else {
      static UINT auPriorityList[] = {  // �������ļ������ʽ��
        CF_UNICODETEXT,                 // unicode�ַ���
        CF_HDROP,                       // �ļ��б������ļ�������ѡ���ļ����ƣ�
        CF_BITMAP,                      // λͼ
      };
      int auPriorityLen = sizeof(auPriorityList) / sizeof(auPriorityList[0]); // �б���
      int uFormat = GetPriorityClipboardFormat(auPriorityList, auPriorityLen); // ��ȡ��ǰ���������ݵĸ�ʽ
      // ���ݸ�ʽ���䲻ͬ����
      //     ������ɹ����ͷ�ȫ����Դ���رռ����壬����ͼƬmat��
      //     ������ʧ�ܣ��ͷ��Ѵ򿪵���Դ����������״̬������switch��ͳһ�رռ�����ͷ��ؿ�mat
      switch (uFormat)
      {
      case CF_UNICODETEXT: { // 1. unicode�ַ��� ==========================================================
        HANDLE hClip = GetClipboardData(uFormat); // 1.1. ��ָ����ʽ�Ӽ������м������ݣ�����ָ����ʽ�ļ��������ľ��
        if (hClip) { // ��ȡ�ɹ�
          wchar_t* pBuf = (wchar_t*)GlobalLock(hClip); // 1.2. ����ȫ���ڴ���󣬲�����ָ��ö�����ڴ��ĵ�һ���ֽڵ�ָ��
          wstring wpath = pBuf;
          // �ͷ���Դ
          GlobalUnlock(hClip); // 1.x.1. �ͷ��ڴ���󣬵ݼ���GMEM_MOVEABLEһ�������ڴ�����������������
          CloseClipboard(); // 1.x.2. �رռ����壬ʹ���������ܹ��������ʼ�����
          return imread_wstr(wpath); // 1.�ɹ������Ը�������ַ�����ȡ�ļ�
        }
        set_state(CODE_ERR_CLIP_DATA, MSG_ERR_CLIP_DATA); // ����״̬����ȡ����������ʧ��
        break;
      }

      case CF_HDROP: { // 2. �ļ��б��� =========================================================== 
        HDROP hClip = (HDROP)GetClipboardData(uFormat); // 2.1. �õ��ļ��б�ľ��
        if (hClip) {
          // https://docs.microsoft.com/zh-CN/windows/win32/api/shellapi/nf-shellapi-dragqueryfilea
          GlobalLock(hClip); // 2.2. ����ȫ���ڴ����
          int iFiles = DragQueryFile(hClip, 0xFFFFFFFF, NULL, 0); // 2.3. 0xFFFFFFFF��ʾ�����ļ��б�ļ���
          if (iFiles != 1) { // ֻ����1���ļ�
            GlobalUnlock(hClip);
            set_state(CODE_ERR_CLIP_FILES, MSG_ERR_CLIP_FILES(iFiles)); // ����״̬���ļ���������Ϊ1
            break;
          }
          //for (int i = 0; i < iFiles; i++) {
          int i = 0; // ֻȡ��1���ļ�
          UINT lenChar = DragQueryFile(hClip, i, NULL, 0); // 2.4. �õ���i���ļ����������軺�����Ĵ�С���ֽڣ�
          char* nameC = new char[lenChar + 1]; // ����ļ������ֽ�����
          DragQueryFile(hClip, i, nameC, lenChar + 1); // 2.5. �����i���ļ���
          wchar_t* nameW = char_2_wchar(nameC); // 2.6. �ļ���תΪ���ֽ�����
          cv::Mat mat = imread_wstr(nameW); // 2.7. ���Զ�ȡ�ļ�
          // �ͷ���Դ
          delete[] nameC;
          delete[] nameW;
          GlobalUnlock(hClip); // 2.x.1 �ͷ��ļ��б���
          CloseClipboard(); // 2.x.2 �رռ�����
          return mat;
        }
        set_state(CODE_ERR_CLIP_DATA, MSG_ERR_CLIP_DATA); // ����״̬����ȡ����������ʧ��
        break;
      }

      case CF_BITMAP: { // 3. λͼ ===================================================================
        HBITMAP hbm = (HBITMAP)GetClipboardData(uFormat); // 3.1. �Ӽ�������¼��ָ�룬�õ��ļ����
        if (hbm) {
          // GlobalLock(hbm); // ����ֵ������Ч�ģ���λͼ�ƺ�����Ҫ����
        // https://social.msdn.microsoft.com/Forums/vstudio/en-US/d2a6aa71-68d7-4db0-8b1f-5d1920f9c4ce/globallock-and-dib-transform-into-hbitmap-issue?forum=vcgeneral
          BITMAP bmp; // ���ָ�򻺳�����ָ�룬�����������й�ָ��ͼ�ζ������Ϣ
          GetObject(hbm, sizeof(BITMAP), &bmp); // 3.3. ��ȡͼ�ζ������Ϣ������ͼƬ���ݱ���
          if (!hbm) {
            set_state(CODE_ERR_CLIP_GETOBJ, MSG_ERR_CLIP_GETOBJ); // ����״̬������ͼ�ζ�����Ϣʧ��
            break;
          }
          int nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel / 8; // ����ɫ�����ͨ������32bitΪ4��24bitΪ3
          // 3.4. �����hbm�е�λͼ���Ƶ�������
          long sz = bmp.bmHeight * bmp.bmWidth * nChannels; // ͼƬ��С���ֽڣ�
          cv::Mat mat(cv::Size(bmp.bmWidth, bmp.bmHeight), CV_MAKETYPE(CV_8U, nChannels));  // ����վ��󣬴���λͼ��С�����
          long getsz = GetBitmapBits(hbm, sz, mat.data); // �����hbm��sz���ֽڸ��Ƶ�������img.data
          if (!getsz) {
            set_state(CODE_ERR_CLIP_BITMAP, MSG_ERR_CLIP_BITMAP); // ����״̬����ȡλͼ����ʧ��
            break;
          }
          CloseClipboard();  // �ͷ���Դ
          // 3.5. ���غ��ʵ�ͨ��
          if (mat.data) {
            if (nChannels == 1 || nChannels == 3) { // 1��3ͨ����PPOCR��ʶ��ֱ�ӷ���
              return mat;
            }
            else if (nChannels == 4) { // 4ͨ����PPOCR����ʶ��ɾȥalphaת3ͨ���ٷ���
              cv::Mat mat_c3;
              cv::cvtColor(mat, mat_c3, cv::COLOR_BGRA2BGR); // ɫ�ʿռ�ת��
              return mat_c3;
            }
            set_state(CODE_ERR_CLIP_CHANNEL, MSG_ERR_CLIP_CHANNEL(nChannels)); // ����״̬��ͨ�����쳣
            break;
          }
          // ���������� !getsz �Ѿ� break �ˣ������ߵ������������ٱ���һ��
          set_state(CODE_ERR_CLIP_BITMAP, MSG_ERR_CLIP_BITMAP); // ����״̬����ȡλͼ����ʧ��
          break;
        }
        set_state(CODE_ERR_CLIP_DATA, MSG_ERR_CLIP_DATA); // ����״̬����ȡ����������ʧ��
        break;
      }

      case NULL: // ������Ϊ��
        set_state(CODE_ERR_CLIP_EMPTY, MSG_ERR_CLIP_EMPTY); // ����״̬��������Ϊ��
        break;
      case -1: // ������֧�ֵĸ�ʽ
      default: // δ֪
        set_state(CODE_ERR_CLIP_FORMAT, MSG_ERR_CLIP_FORMAT); // ����״̬�� ������ĸ�ʽ��֧��
        break;
      }
      CloseClipboard(); // Ϊbreak������رռ����壬ʹ���������ܹ��������ʼ����塣
    }
    return cv::Mat();
  }

  // ===== ÿ�غϵ���ڣ���ȡͼƬ =============
  // ���� cv::imread ����·��pathU8����һ��ͼƬ��pathU8����Ϊutf-8��string
  cv::Mat imread_utf8(string pathU8, int flags = cv::IMREAD_COLOR) {
    set_state(); // ����״̬����ʼ��
    if (pathU8 == u8"clipboard") { // ��Ϊ����������
      return imread_clipboard(flags);
    }
    // string u8 ת wchar_t
    std::wstring wpath;
    try {
      wpath = conv_Ustr_Wstr.from_bytes(pathU8); // ����ת����ת��
    }
    catch (...) {
      set_state(CODE_ERR_PATH_CONV, MSG_ERR_PATH_CONV(pathU8)); // ����״̬��תwstringʧ��
      return cv::Mat();
    }
    return imread_wstr(wpath);
  }
}


/* ����
* 

D:\ͼƬ\Screenshots\t1.png

--image_dir=clipboard

  // ���·��pathU8�������ڣ�����true
  bool is_exists(string pathU8) {
	cout << u8"��ʼ��飺 " << pathU8 << endl;
	struct _stat buf;
	int result = _stat(pathU8.c_str(), &buf);
	if (result == 0) {
	  cout << u8"���0�� " << pathU8 << endl;
	  return false;
	}
	if (S_IFREG & buf.st_mode) {
	  cout << u8"���Ϊ�ļ��� " << pathU8 << endl;
	  return true;
	}
	else if (S_IFDIR & buf.st_mode) {
	  cout << u8"���ΪĿ¼�� " << pathU8 << endl;
	  return false;
	}
	cout << u8"���ʧ�ܣ� " << pathU8 << endl;
  }


cv::Mat readByImread(string path) {
  return cv::imread(path, cv::IMREAD_COLOR);
}

cv::Mat readByImdecode(string path) {
  std::ifstream imgStream(path.c_str(), std::ios::binary);
  imgStream.seekg(0, std::ios::end);
  size_t fileSize = imgStream.tellg();
  imgStream.seekg(0, std::ios::beg);
  if (fileSize == 0) {
	return cv::Mat();
  }
  std::vector<unsigned char> data(fileSize);
  imgStream.read(reinterpret_cast<char*>(&data[0]), sizeof(unsigned char) * fileSize);
  if (!imgStream) {
	return cv::Mat();
  }
  return cv::imdecode(cv::Mat(data), cv::IMREAD_COLOR);
}

  // �ж��ַ����Ƿ�utf-8
  bool is_utf8(const string& str)
  {
	char nBytes = 0; // UFT8����1-6���ֽڱ���,ASCII��һ���ֽ�
	unsigned char chr;
	bool bAllAscii = true; // ���ȫ������ASCII, ˵������UTF-8
	for (int i = 0; i < str.length(); i++)
	{
	  chr = str[i];
	  // �ж��Ƿ�ASCII����,�������,˵���п�����UTF-8,ASCII��7λ����,
	  // ����һ���ֽڴ�,���λ���Ϊ0,o0xxxxxxx
	  if ((chr & 0x80) != 0)
		bAllAscii = false;
	  // �������ASCII��,Ӧ���Ƕ��ֽڷ�,�����ֽ���
	  if (nBytes == 0)
	  {
		if (chr >= 0x80)
		{
		  if (chr >= 0xFC && chr <= 0xFD)   nBytes = 6;
		  else if (chr >= 0xF8)         nBytes = 5;
		  else if (chr >= 0xF0)         nBytes = 4;
		  else if (chr >= 0xE0)         nBytes = 3;
		  else if (chr >= 0xC0)         nBytes = 2;
		  else {
			return false;
		  }
		  nBytes--;
		}
	  }
	  else //���ֽڷ��ķ����ֽ�,ӦΪ 10xxxxxx
	  {
		if ((chr & 0xC0) != 0x80)
		  return false;
		nBytes--;
	  }
	}

	if (nBytes > 0) //Υ������
	  return false;

	if (bAllAscii) //���ȫ������ASCII, ˵������UTF-8
	  return false;

	return true;
  }
  // �ж��ַ����Ƿ�bgk
  bool is_gbk(const string& str)
  {
	unsigned int nBytes = 0;//GBK����1-2���ֽڱ���,�������� ,Ӣ��һ��
	unsigned char chr = str.at(0);
	bool bAllAscii = true; //���ȫ������ASCII,

	for (unsigned int i = 0; str[i] != '\0'; ++i) {
	  chr = str.at(i);
	  if ((chr & 0x80) != 0 && nBytes == 0) {// �ж��Ƿ�ASCII����,�������,˵���п�����GBK
		bAllAscii = false;
	  }

	  if (nBytes == 0) {
		if (chr >= 0x80) {
		  if (chr >= 0x81 && chr <= 0xFE) {
			nBytes = +2;
		  }
		  else {
			return false;
		  }
		  nBytes--;
		}
	  }
	  else {
		if (chr < 0x40 || chr>0xFE) {
		  return false;
		}
		nBytes--;
	  }//else end
	}

	if (nBytes != 0) { // Υ������
	  return false;
	}

	if (bAllAscii) { // ���ȫ������ASCII, Ҳ��GBK
	  return true;
	}

	return true;
  }
  // �ַ���gbkתutf-8
  string gbk_2_utf8(const string& str_gbk)
  {
	if (str_gbk == "" || is_utf8(str_gbk) || !is_gbk(str_gbk)) {
	  return str_gbk; // �Ѿ���utf-8���߷�gbk����ת
	}
	string str_utf8 = "";
	WCHAR* str1;
	int n = MultiByteToWideChar(CP_ACP, 0, str_gbk.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, str_gbk.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char* str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	str_utf8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return str_utf8;
  }
  // �ַ���utf-8תgbk
  string utf8_2_gbk(const string& str_utf8) {
	if (str_utf8 == "")
	  return str_utf8;
	const char* strUTF8 = str_utf8.c_str();
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	std::string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
  }

*/