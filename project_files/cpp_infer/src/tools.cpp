
#include<fstream>
#include <iostream>
#include <windows.h>
#include <filesystem>
#include <include/args.h>
#include <include/nlohmann_json.hpp>
// ��ͼ���
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
// ����ת��
#include<codecvt>
std::wstring_convert<std::codecvt_utf8<wchar_t>> convU8toWstr; // string u8"" ��wstring��ת����

using namespace std;
using namespace nlohmann;

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
string utf8_2_gbk(const string& str_utf8){
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

// ����һ��json
string load_json_str(string& str_in, bool& is_image, bool& is_hotupdate) {
  is_image = false;
  is_hotupdate = false;
  string logstr = "";
  try{
    auto j = json::parse(str_in); // תjson���� 
	for (auto& el : j.items()) { // ������ֵ 
	  string value = to_string(el.value());
	  int vallen = value.length();
	  if (vallen > 2 && value[0] == '\"' && value[vallen-1] == '\"') {
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
		if (getlen > 0 && (getlog[getlen - 1] == '\n'|| getlog[getlen - 1] == '\r\n')) {
		  getlog = getlog.substr(0, getlen - 1); // ɾȥSetCommandLineOption��β����
		}
		logstr += getlog +". ";
		is_hotupdate = true;
	  }
	}
	if (!is_image) { // תjson�ɹ�����json��û��ͼƬ·�� 
	  str_in = "";
	}
  }
  catch (...){
	logstr = "[Error] Load json fail.";
	is_hotupdate = true;
    //cout << "json parse fail" << endl;
  }
  return logstr;
}

// ����һ�������ı��еļ���ֵ���ɹ�����true�� 
bool get_line_config(string s,string &key, string &value) { 
  int i = 0;
  // ȥ����һ���ո񣬽�ȡ��һ��key
  while (s[i] != '\0' && s[i] != ' ')  ++i; 
  if (s[i] == '\0' || i==0) return false;
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
    if(!isDefaultDir) std::cerr << "[ERROR] config path not exist! config_dir: " << FLAGS_config_path << endl;
    return;
  }

  // ���������ļ�
  string s;
  while (getline(inConf, s)) // ���ж�����Ϣ 
  {
    if (s[0] == '#') continue; // �ų�ע�� 
    string key, value;
    if ( !get_line_config(s, key, value) ) continue; // ����һ������ 

     // ����һ�����á��Զ������Ƿ���ںͺϷ��ԡ����ȼ���������������
     google::SetCommandLineOptionWithMode(key.c_str(), value.c_str(), google::SET_FLAG_IF_DEFAULT);
  }
  inConf.close();
}

// ���� cv::imread ����·��path����һ��ͼƬ��path����Ϊutf-8��string
cv::Mat imreadU8(string pathU8, int flags = cv::IMREAD_COLOR) {
  // string u8 ת wchar_t
  std::wstring wpath;
  try {
	wpath = convU8toWstr.from_bytes(pathU8); // ����ת����ת��
  }catch (...) {
	return cv::Mat();
  }
  // ���ļ�
  FILE* fp = _wfopen((wchar_t*)wpath.c_str(), L"rb"); // wpathǿ������ת����whar_t
  if (!fp) { // ��ʧ��
	return cv::Mat();
  }
  // ���ļ������ڴ�
  fseek(fp, 0, SEEK_END); // ������ fp ���ļ�λ��Ϊ SEEK_END �ļ���ĩβ
  long sz = ftell(fp); // ��ȡ�� fp �ĵ�ǰ�ļ�λ�ã����ܴ�С���ֽڣ�
  char* buf = new char[sz]; // ����ļ��ֽ�����
  fseek(fp, 0, SEEK_SET); // ������ fp ���ļ�λ��Ϊ SEEK_SET �ļ��Ŀ�ͷ
  long n = fread(buf, 1, sz, fp); // �Ӹ����� fp ��ȡ���ݵ� buf ��ָ��������У����سɹ���ȡ��Ԫ������
  cv::_InputArray arr(buf, sz); // ��ֻ���������鴫�ݵ�OpenCV����
  cv::Mat img = cv::imdecode(arr, flags); // �����ڴ����ݣ����cv::Mat����
  delete[] buf; // �ͷ�buf�ռ�
  fclose(fp); // �ر��ļ�
  return img;
}

// �˳�����ǰ��ͣ
void exit_pause(int x=1) {
	cout << "OCR exit." << endl; // �˳���ʾ 
	if (FLAGS_use_system_pause) system("pause");
	exit(x);
}


//cv::Mat readByImread(string path) {
//  return cv::imread(path, cv::IMREAD_COLOR);
//}

//cv::Mat readByImdecode(string path) {
//  std::ifstream imgStream(path.c_str(), std::ios::binary);
//  imgStream.seekg(0, std::ios::end);
//  size_t fileSize = imgStream.tellg();
//  imgStream.seekg(0, std::ios::beg);
//  if (fileSize == 0) {
//	return cv::Mat();
//  }
//  std::vector<unsigned char> data(fileSize);
//  imgStream.read(reinterpret_cast<char*>(&data[0]), sizeof(unsigned char) * fileSize);
//  if (!imgStream) {
//	return cv::Mat();
//  }
//  return cv::imdecode(cv::Mat(data), cv::IMREAD_COLOR);
//}