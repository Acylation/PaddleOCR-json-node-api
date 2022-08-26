
#include<fstream>
#include <iostream>
#include <filesystem>

#include <include/args.h>
using namespace std;

bool get_line_config(string s,string &key, string &value) { // ����һ���ı��еļ���ֵ���ɹ�����true�� 
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

//if ( find_if(allFlags.begin(), allFlags.end(), [&](auto v) {return v.name == key;} ) == allFlags.end())
//  continue;  // �ų������ļ��в��Ϸ���������Flags�ڣ���key
//if ( !google::GetCommandLineFlagInfoOrDie(key.c_str()).is_default )
//  continue;  // �ų����������д��ڵ�flags�������������ȼ����������ļ��� 
 //cout << key << " : " << value << endl;