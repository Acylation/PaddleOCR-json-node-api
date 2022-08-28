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
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector>

#include <include/args.h>
#include <include/paddleocr.h>
#include <include/paddlestructure.h>

#include <include/tools.h>

#include <include/nlohmann_json.hpp>
using namespace nlohmann;

using namespace PaddleOCR;

void check_params() {
  if (FLAGS_det) {
    if (FLAGS_det_model_dir.empty()) {
      cout << "[ERROR] Use det, need {--det_model_dir}." << std::endl;
      exit_pause(1);
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
      cout << "[ERROR] Use rec, need {--rec_model_dir}." << std::endl;
      exit_pause(1);
    }
  }
  if (FLAGS_cls && FLAGS_use_angle_cls) {
    if (FLAGS_cls_model_dir.empty()) {
      cout << "[ERROR] Use cls, need {--cls_model_dir}." << std::endl;
      exit_pause(1);
    }
  }
  if (FLAGS_table) { // ��֧�ֱ��ʶ�� 
    cout << "[ERROR] Table structure is not supported. {--table} only 'false' is allow." << std::endl;
    exit_pause(1);
  }
  if (FLAGS_type != "ocr") { // ��֧�ֱ��ʶ�� 
    std::cerr << "[ERROR] Table structure is not supported. {--type} only 'ocr' is allow." << endl;
    exit_pause(1);
  }
  if (FLAGS_precision != "fp32" && FLAGS_precision != "fp16" &&
    FLAGS_precision != "int8") {
    cout << "[ERROR] {--precison} should be 'fp32'(default), 'fp16' or 'int8'. " << endl;
    exit_pause(1);
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

void print_ocr_fail(int code, const string& msg,const string& key2="", const string& msg2="") {
  json j;
  j["code"] = code;
  j["data"] = msg;
  if (key2 != "") {
    j[key2] = msg2;
  }
  print_json(j);
}

void run_ocr(PPOCR& ocr, string img_path) {
  // ��Ϊjson�ַ���������� 
  int imgstrlen = img_path.length();
  string logstr = "";
  bool is_image = false, is_hotupdate = false;
  if (imgstrlen > 2 && img_path[0] == '{' && img_path[imgstrlen - 1] == '}') {
    logstr = load_json_str(img_path, is_image, is_hotupdate);
    if (is_hotupdate) { // �ȸ���
      ocr.HotUpdate();
    }
  }
  // ִ��һ��ʶ��FLAGS_image_dirֻ���ǵ����ļ�·����
  if (!Utility::PathExists(img_path)) {
    string msg = "Image path not exist. Path:\"" + gbk_2_utf8(img_path) + "\"";
    if(is_hotupdate) print_ocr_fail(200, msg,"hotUpdate", logstr);
    else print_ocr_fail(200, msg);
    return;
  }

  std::vector<cv::String> cv_all_img_names({ img_path }); // һ�δ���һ���ļ� 

  std::vector<std::vector<OCRPredictResult>> ocr_results = // ִ�У���ȡ���
    ocr.ocr(cv_all_img_names, FLAGS_det, FLAGS_rec, FLAGS_cls);

  std::vector<OCRPredictResult> ocr_result = ocr_results[0];

  if (ocr_result.empty()) { // ͼ��������
    print_ocr_fail(101, "No text found in image. Path:\"" + gbk_2_utf8(img_path) + "\"");
    return;
  }
  else if (ocr_result[0].cls_label == -201) { // �޷���ȡͼƬ
    print_ocr_fail(201, "Image read failed. Path:\"" + gbk_2_utf8(img_path) + "\"");
    return;
  }

  json outJ;
  outJ["code"] = 100;
  outJ["data"] = json::array();
  if (is_hotupdate) {
    outJ["hotUpdate"] = logstr;
  }
  bool isEmpty = true;
  for (int i = 0; i < ocr_result.size(); i++) {
    json j;
    j["text"] = ocr_result[i].text;
    j["score"] = ocr_result[i].score;
    std::vector<std::vector<int>> b = ocr_result[i].box;
    if (b.empty() || j["text"] == "") // û�����֣��������� 
      continue;
    else
      j["box"] = { b[0][0], b[0][1], b[1][0], b[1][1],
                   b[2][0], b[2][1], b[3][0], b[3][1], };
    outJ["data"].push_back(j);
    isEmpty = false;
  }
  if (isEmpty)
    print_ocr_fail(101, "No text found in image. Path:\"" + gbk_2_utf8(img_path) + "\"");
  else {
    // ���з�ascii�ַ�ת��Ϊascii��������ı�������
    print_json(outJ);
    // ������ӻ��������ͼ�����ڵ��� 
    if (FLAGS_visualize && FLAGS_det) { 
      cv::Mat srcimg = cv::imread(img_path, cv::IMREAD_COLOR);
      if (!srcimg.data) {
        std::cerr << "[ERROR] Image read failed! image path: "
          << gbk_2_utf8(img_path) << endl;
        return;
      }
      std::string file_name = Utility::basename(img_path);

      Utility::VisualizeBboxes(srcimg, ocr_result,
        FLAGS_output + "/" + file_name); // ����������һ��cout 
    }
  }
}

int main(int argc, char** argv) {

  google::ParseCommandLineFlags(&argc, &argv, true); // �����������
  load_congif_file(); // ���������ļ� 
  check_params(); // �������Ϸ��� 
  PPOCR ocr = PPOCR(); // ��ʼ��ʶ���� 

  system("chcp 65001"); // ����̨��utf-8 
  cout << "OCR init completed." << endl; // �����ʾ
  // ������������ͼƬ����ִ��һ����ʶ�� 
  if (FLAGS_image_dir != "")
    run_ocr(ocr, FLAGS_image_dir);
  else // ��������ѭ����ȡͼƬ 
    while (1) {
      string img_path;
      getline(cin, img_path);
      // ȥ������̨�����ļ������ո�·���Զ��ӵ�˫���� 
      int imgstrlen = img_path.length();
      if (imgstrlen > 2 && img_path[0] == '\"' && img_path[imgstrlen - 1] == '\"') {
        img_path = img_path.substr(1, imgstrlen - 2);
      }
      run_ocr(ocr, img_path);
    }

  exit_pause(0);
  //return 0;
}


/* if (!Utility::PathExists(FLAGS_image_dir)) {
   std::cerr << "[ERROR] image path not exist! image_dir: " << FLAGS_image_dir
             << endl;
   system("pause");
   exit(1);
 }

 std::vector<cv::String> cv_all_img_names;
 cv::glob(FLAGS_image_dir, cv_all_img_names);
 std::cout << "total images num: " << cv_all_img_names.size() << endl;


 if (FLAGS_type == "ocr") {
   ocr(cv_all_img_names);
 } else if (FLAGS_type == "structure") {
   structure(cv_all_img_names);
 } else {
   std::cout << "only value in ['ocr','structure'] is supported" << endl;
 }*/