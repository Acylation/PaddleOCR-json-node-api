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

#include <string>
#include <fstream>
#include <include/utility.h>

#include <gflags/gflags.h>

// ����ģʽ 
DEFINE_string(image_path, "", "Set image_path to run a single task."); // ����д��ͼƬ·������ִ��һ��OCR�� 
DEFINE_int32(port, -1, "Set port to enable socket server mode."); // ����д�˶˿ںţ������׽��ַ��������������������ܵ�ģʽ�� 

// common args ���ò��� 
DEFINE_bool(use_gpu, false, "Infering with GPU or CPU."); // trueʱ����GPU����Ҫ�����֧�֣� 
DEFINE_bool(use_tensorrt, false, "Whether use tensorrt."); // trueʱ����tensorrt 
DEFINE_int32(gpu_id, 0, "Device id of GPU to execute."); // GPU id��ʹ��GPUʱ��Ч 
DEFINE_int32(gpu_mem, 4000, "GPU memory when infering with GPU."); // �����GPU�ڴ� 
DEFINE_int32(cpu_threads, 10, "Num of threads with CPU."); // CPU�߳� 
DEFINE_bool(enable_mkldnn, false, "Whether use mkldnn with CPU."); // trueʱ����mkldnn 
DEFINE_string(precision, "fp32", "Precision be one of fp32/fp16/int8"); // Ԥ��ľ��ȣ�֧��fp32, fp16, int8 3������ 
DEFINE_bool(benchmark, false, "Whether use benchmark."); // trueʱ����benchmark����Ԥ���ٶȡ��Դ�ռ�õȽ���ͳ�� 
DEFINE_string(output, "./output/", "Save benchmark log path."); // ���ӻ���������·�� TODO 
DEFINE_string(type, "ocr", "Perform ocr or structure, the value is selected in ['ocr','structure']."); // ��������
DEFINE_string(config_path, "", "Path of config file."); // �����ļ�·�� 
DEFINE_bool(ensure_ascii, false, "Path of config file."); // trueʱjson����asciiת�� 

// detection related DET������ 
DEFINE_string(det_model_dir, "", "Path of det inference model."); // detģ�Ϳ�·�� 
DEFINE_string(limit_type, "max", "limit_type of input image, the value is selected in ['max','min']."); // ��ͼƬ�ߴ����Ʋ��ó��߻��Ƕ̱� 
DEFINE_int32(limit_side_len, 960, "limit_side_len of input image."); // �Գ�/�̱�����ֵ 
DEFINE_double(det_db_thresh, 0.3, "Threshold of det_db_thresh."); // ���ڹ���DBԤ��Ķ�ֵ��ͼ������Ϊ0.-0.3�Խ��Ӱ�첻���� 
DEFINE_double(det_db_box_thresh, 0.6, "Threshold of det_db_box_thresh."); // DB�������box����ֵ�����������©��������������С 
DEFINE_double(det_db_unclip_ratio, 1.5, "Threshold of det_db_unclip_ratio."); // ��ʾ�ı���Ľ��³̶ȣ�ԽС���ı���������ı�
DEFINE_bool(use_dilation, false, "Whether use the dilation on output map."); // trueʱ�Էָ������������Ի�ȡ���ż��Ч��
DEFINE_string(det_db_score_mode, "slow", "Whether use polygon score, the value is selected in ['slow','fast']."); // slow:ʹ�ö���ο����bbox score��fast:ʹ�þ��ο���㡣���ο�����ٶȸ��죬����ο�������ı���������׼ȷ 
DEFINE_bool(visualize, false, "Whether show the detection results."); // trueʱ���ý�����п��ӻ���Ԥ����������output�ֶ�ָ�����ļ����º�����ͼ��ͬ����ͼ���ϡ� 

// classification related CLS���������� 
DEFINE_bool(use_angle_cls, false, "Whether use use_angle_cls."); // trueʱ���÷�������� 
DEFINE_string(cls_model_dir, "", "Path of cls inference model."); // clsģ�Ϳ�·�� 
DEFINE_double(cls_thresh, 0.9, "Threshold of cls_thresh."); // ����������ĵ÷���ֵ 
DEFINE_int32(cls_batch_num, 1, "cls_batch_num."); // ���������batchsize 

// recognition related REC�ı�ʶ����� 
DEFINE_string(rec_model_dir, "", "Path of rec inference model."); // recģ�Ϳ�·�� 
DEFINE_int32(rec_batch_num, 6, "rec_batch_num."); // ����ʶ��ģ��batchsize 
DEFINE_string(rec_char_dict_path, "../../ppocr/utils/ppocr_keys_v1.txt", "Path of dictionary."); // �ֵ�·�� 
DEFINE_int32(rec_img_h, 48, "rec image height"); // ����ʶ��ģ������ͼ��߶ȡ�V3ģ����48��V2Ӧ�ø�Ϊ32  
DEFINE_int32(rec_img_w, 320, "rec image width"); // ����ʶ��ģ������ͼ���ȡ�V3��V2һ�� 

// layout model related ���������� 
DEFINE_string(layout_model_dir, "", "Path of table layout inference model."); // �������ģ��inference model·�� 
DEFINE_string(layout_dict_path, "../../ppocr/utils/dict/layout_dict/layout_publaynet_dict.txt", "Path of dictionary."); // �����ֵ�·�� 
DEFINE_double(layout_score_threshold, 0.5, "Threshold of score."); // ����ķ�����ֵ 
DEFINE_double(layout_nms_threshold, 0.5, "Threshold of nms."); // nms����ֵ 
// structure model related ���ṹ��� 
DEFINE_string(table_model_dir, "", "Path of table struture inference model."); // ���ʶ��ģ��inference model·�� 
DEFINE_int32(table_max_len, 488, "max len size of input image."); // ���ʶ��ģ������ͼ�񳤱ߴ�С 
DEFINE_int32(table_batch_num, 1, "table_batch_num.");
DEFINE_bool(merge_no_span_structure, true, "Whether merge <td> and </td> to <td></td>"); // trueʱ��<td>��</td>�ϲ���<td></td> 
DEFINE_string(table_char_dict_path, "../../ppocr/utils/dict/table_structure_dict_ch.txt", "Path of dictionary."); // ����ֵ�·�� 

// ocr forward related ǰ������� 
DEFINE_bool(det, true, "Whether use det in forward.");
DEFINE_bool(rec, true, "Whether use rec in forward.");
DEFINE_bool(cls, false, "Whether use cls in forward.");
DEFINE_bool(table, false, "Whether use table structure in forward.");
DEFINE_bool(layout, false, "Whether use layout analysis in forward.");

// �������ļ��ж�ȡ���ã�������־�ַ����� 
std::string read_config() {
    if (!PaddleOCR::Utility::PathExists(FLAGS_config_path)) {
        return ("config_path [" + FLAGS_config_path + "] does not exist. ");
    }
    std::ifstream infile(FLAGS_config_path);
    if (!infile) {
        return ("[WARNING] Unable to open config_path [" + FLAGS_config_path + "]. ");
    }
    std::string msg = "Load config from ["+ FLAGS_config_path+"] : ";
    std::string line;
    int num = 0;
    while (getline(infile, line)) {
        int length = line.length();
        if (length < 3 || line[0] == '#') // �������к�ע��
            continue;
        int split = 0; // ��ֵ�Եķָ��� 
        for (; split < length; split++) {
            if (line[split] == ' ' || line[split] == '=')
                break;
        }
        if (split >= length-1 || split==0) // �������Ȳ���ļ�ֵ�� 
            continue;
        std::string key = line.substr(0, split);
        std::string value = line.substr(split+1);
        // �������ã����ȼ����������д�������� 
        std::string res = google::SetCommandLineOptionWithMode(key.c_str(), value.c_str(), google::SET_FLAG_IF_DEFAULT);
        if (!res.empty()) {
            num++;
            msg += res.substr(0, res.length()-1);
        }
    }
    infile.close();
    if (num == 0)
        msg += "No valid config found.";
    else
        msg += ". ";
    return msg;
}

// ���һ��·��path�Ƿ���ڣ�����Ϣд��msg 
void check_path(const std::string &path, const std::string &name, std::string &msg) {
    if (path.empty())
        msg += (name+" is empty. ");
    else if (!PaddleOCR::Utility::PathExists(path)) {
        msg += (name + " [" + path + "] does not exist. ");
    }
}

// �������Ϸ��ԡ��ɹ����ؿ��ַ�����ʧ�ܷ��ر�����Ϣ�ַ����� 
std::string check_flags() {
    std::string msg = "";
    if (FLAGS_det) { // ���det
        check_path(FLAGS_det_model_dir, "det_model_dir", msg);
    }
    if (FLAGS_rec) { // ���rec 
        check_path(FLAGS_rec_model_dir, "rec_model_dir", msg);
    }
    if (FLAGS_cls && FLAGS_use_angle_cls) { // ���cls 
        check_path(FLAGS_cls_model_dir, "cls_model_dir", msg);
    }
    if (FLAGS_table) { // ���table 
        check_path(FLAGS_table_model_dir, "table_model_dir", msg);
        if(!FLAGS_det)
            check_path(FLAGS_det_model_dir, "det_model_dir", msg);
        if (!FLAGS_rec)
            check_path(FLAGS_rec_model_dir, "rec_model_dir", msg);
    }
    if (FLAGS_layout) { // ���� 
        check_path(FLAGS_layout_model_dir, "layout_model_dir", msg);
    }
    // ���ö��ֵ 
    if (FLAGS_precision != "fp32" && FLAGS_precision != "fp16" && FLAGS_precision != "int8") {
        msg += "precison should be 'fp32'(default), 'fp16' or 'int8', not " + FLAGS_precision + ". ";
    }
    if (FLAGS_type != "ocr" && FLAGS_type != "structure") {
        msg += "type should be 'ocr'(default) or 'structure', not " + FLAGS_type + ". ";
    }
    if (FLAGS_limit_type != "max" && FLAGS_limit_type != "min") {
        msg += "limit_type should be 'max'(default) or 'min', not " + FLAGS_limit_type + ". ";
    }
    if (FLAGS_det_db_score_mode != "slow" && FLAGS_det_db_score_mode != "fast") {
        msg += "limit_type should be 'slow'(default) or 'fast', not " + FLAGS_det_db_score_mode + ". ";
    }
    return msg;
}