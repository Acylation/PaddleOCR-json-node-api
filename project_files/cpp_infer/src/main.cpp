// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

// ��������� https://github.com/PaddlePaddle/PaddleOCR
// ���ο��� by https://github.com/hiroi-sora/PaddleOCR-json


// �汾��Ϣ
#define PROJECT_VER "v1.2.1"
#define PROJECT_NAME "PaddleOCR-json " PROJECT_VER


#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector>

#include <include/args.h>
#include <include/paddleocr.h>
#include <include/paddlestructure.h>

#include <include/tools.h>
#include <include/tools_flags.h> // ��־λ

#include <nlohmann/json.hpp>
using namespace nlohmann;

using namespace PaddleOCR;

void check_params() {
  if (FLAGS_use_debug) {
    FLAGS_ensure_ascii = false;
    FLAGS_use_system_pause = true;
    FLAGS_ensure_chcp = true;
  }
  if (FLAGS_det) {
    if (FLAGS_det_model_dir.empty()) {
      cerr << "[ERROR] Use det, need {--det_model_dir}." << endl;
      tool::exit_pause(1);
    }
  }
  if (FLAGS_rec) {
    //cout
    //    << "In PP-OCRv3, rec_image_shape parameter defaults to '3, 48, 320',"
    //       "if you are using recognition model with PP-OCRv2 or an older "
    //       "version, "
    //       "please set --rec_image_shape='3,32,320"
    //    << std::endl;
    if (FLAGS_rec_model_dir.empty()) {
      cerr << "[ERROR] Use rec, need {--rec_model_dir}." << endl;
      tool::exit_pause(1);
    }
  }
  if (FLAGS_cls && FLAGS_use_angle_cls) {
    if (FLAGS_cls_model_dir.empty()) {
      cerr << "[ERROR] Use cls, need {--cls_model_dir}." << endl;
      tool::exit_pause(1);
    }
  }
  if (FLAGS_table) { // ��֧�ֱ��ʶ�� 
    cerr << "[ERROR] Table structure is not supported. {--table} only 'false' is allow." << endl;
    tool::exit_pause(1);
  }
  if (FLAGS_type != "ocr") { // ��֧�ֱ��ʶ�� 
    cerr << "[ERROR] Table structure is not supported. {--type} only 'ocr' is allow." << endl;
    tool::exit_pause(1);
  }
  if (FLAGS_precision != "fp32" && FLAGS_precision != "fp16" &&
    FLAGS_precision != "int8") {
    cerr << "[ERROR] {--precison} should be 'fp32'(default), 'fp16' or 'int8'. " << endl;
    tool::exit_pause(1);
  }
}

void print_json(const json &j) {
  try {
    cout << j.dump(-1, ' ', FLAGS_ensure_ascii) << endl;
  }
  catch (...) {
    json j2;
    j2["code"] = 300;
    j2["data"] = "JSON dump failed. Coding error.";
    cout << j2.dump(-1, ' ', FLAGS_ensure_ascii) << endl;
  }
}

void print_ocr_fail(int code, const string& msg, const string& hotUpdate ="") {
  json j;
  j["code"] = code;
  j["data"] = msg;
  if (hotUpdate != "") {
    j["hotUpdate"] = hotUpdate;
  }
  print_json(j);
}

void run_ocr(PPOCR& ocr, string img_path) {
  // img_path����Ϊ���±��룺����nlohmann_json��ʶ���json���� ��  UTF-8 ����ַ

  // 1. ǰ����
  int imgstrlen = img_path.length();
  string hotupdateLog = ""; // ����json�����õ�����־
  bool is_image = false, is_hotupdate = false;
  // 1.1. ��Ϊjson�ַ���������� 
  if (imgstrlen > 2 && img_path[0] == '{' && img_path[imgstrlen - 1] == '}') {
    hotupdateLog = tool::load_json_str(img_path, is_image, is_hotupdate);
    // ����jsonת���� img_path ������ԭ����ʲô�������� u8 string
    if (is_hotupdate) { // �ȸ���
      ocr.HotUpdate();
    }
  } // ����һ����img_pathΪ utf-8 ��·���ַ���
 
  // 2. ִ��OCR
  // 2.1. ת�����б�
  std::vector<cv::String> cv_all_img_names({ img_path }); // һ�δ���һ���ļ� 
  // 2.2. ִ��OCR����ȡ���
  std::vector<std::vector<OCRPredictResult>> ocr_results = 
    ocr.ocr(cv_all_img_names, FLAGS_det, FLAGS_rec, FLAGS_cls);
  std::vector<OCRPredictResult> ocr_result = ocr_results[0]; // ��ȡ��һ�������Ҳֻ��һ����

  // 3. ���
  // 3.1. �����ʶ��ɹ��������֣�detδ�����
  if (ocr_result.empty()) {
    print_ocr_fail(CODE_OK_NONE, MSG_OK_NONE(img_path), hotupdateLog);
    return;
  }
  // 3.2. �����ʶ��ʧ��
  if (ocr_result[0].cls_label == CODE_ERR_MAT_NULL) { // �����ǩ
    // �����ͼ���̵��ܹܿ����� �Ƿ��б����쳣���
    int code;
    string msg;
    tool::get_state(code, msg);
    if (code != CODE_INIT) { // �б����쳣
      print_ocr_fail(code, msg, hotupdateLog); // ���json
      return;
    }
    print_ocr_fail(CODE_ERR_UNKNOW, MSG_ERR_UNKNOW, hotupdateLog); // δ֪����
    return;
  }
  // 3.3. ��������
  json outJ;
  outJ["code"] = 100;
  outJ["data"] = json::array();
  if (is_hotupdate) {
    outJ["hotUpdate"] = hotupdateLog;
  }
  bool isEmpty = true;
  for (int i = 0; i < ocr_result.size(); i++) {
    json j;
    j["text"] = ocr_result[i].text;
    j["score"] = ocr_result[i].score;
    std::vector<std::vector<int>> b = ocr_result[i].box;
    // �ް�Χ��
    if (b.empty()) {
      if (FLAGS_det) // ����det���ް�Χ�У���������
        continue;
      else // δ��det�����հ�Χ��
        for (int bi = 0; bi < 4; bi++)
          b.push_back(std::vector<int> {-1, -1});
    }
    // ������rec��û�����֣��������� 
    if (FLAGS_rec && (j["score"] <= 0 || j["text"] == "")) {
      continue;
    }
    else {
      j["box"] = { {b[0][0], b[0][1]}, {b[1][0], b[1][1]},
        {b[2][0], b[2][1] }, { b[3][0], b[3][1] } };
    }
    outJ["data"].push_back(j);
    isEmpty = false;
  }
  // 3.4. �����ʶ��ɹ��������֣�recδ�����
  if (isEmpty) {
    print_ocr_fail(CODE_OK_NONE, MSG_OK_NONE(img_path), hotupdateLog);
    return;
  }
  // 3.5. ���������������
  else {
    // ���з�ascii�ַ�ת��Ϊascii��������ı�������
    print_json(outJ);
    // 3.4.2. ������ӻ��������ͼ�����ڵ��� 
    if (FLAGS_visualize && FLAGS_det) { 
      //cv::Mat srcimg = cv::imread(img_path, cv::IMREAD_COLOR);
      cv::Mat srcimg = tool::imread_utf8(img_path);
      if (!srcimg.data) {
        //std::cerr << "[ERROR] Image read failed. Path: "
        //  << gbk_2_utf8(img_path) << endl;
        return;
      }
      std::string file_name = Utility::basename(img_path);

      Utility::VisualizeBboxes(srcimg, ocr_result,
        FLAGS_output + "/" + file_name);
    }
  }
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true); // �����������
  tool::load_congif_file(); // ���������ļ� 
  if (FLAGS_cpu_threads <= 0) {
    std::cerr << "[ERROR] Failed to get hardware concurrency. Set cpu_threads to 10." << std::endl;
    FLAGS_cpu_threads = 10;
  }
  check_params(); // �������Ϸ��� 
  PPOCR ocr = PPOCR(); // ��ʼ��ʶ���� 
  if (FLAGS_ensure_chcp) {
    system("chcp 65001"); // ����̨��utf-8 
  }
  if (FLAGS_use_debug) {
    cout << "Debug Mode" << endl; // debug������ʾ
  }
  cout << PROJECT_NAME << endl; // �汾��ʾ
  cout << "OCR init completed." << endl; // �����ʾ
  // ������������ͼƬ����ִ��һ����ʶ�� 
  if (FLAGS_image_dir != "")
    run_ocr(ocr, FLAGS_image_dir);
  else // ��������ѭ����ȡͼƬ 
    while (1) {
      string img_path;
      getline(cin, img_path);
      run_ocr(ocr, img_path);
    }

  tool::exit_pause(0);
  //return 0;
}
