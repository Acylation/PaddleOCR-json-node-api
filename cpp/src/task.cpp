
#include "include/paddleocr.h"
#include "include/args.h"
#include "include/task.h"
#include "include/nlohmann/json.hpp" // json�� 

namespace PaddleOCR {

// OCR ����ѭ��
int Task::ocr(){
	// ��ʼ������
	ppocr = new PPOCR(); // ����������� 
	std::cout << "OCR init completed." << std::endl;

	// ����ͼƬʶ��ģʽ 
	if (!FLAGS_image_path.empty()) {
		return single_image();
	}
	//// �׽��ַ�����ģʽ 
	//else if (FLAGS_port != -1) {
	//	return socket_mode();
	//}
	//// �����ܵ�ģʽ
	//else {
	//	return anonymous_pipe_mode();
	//}

	return 0;
}

// ����״̬
void Task::set_state(int code, std::string msg) {
	t_code = code;
	t_msg = msg;
}

// jsonת�ַ��� 
std::string json_dump(nlohmann::json j) {
	try {
		std::string str = j.dump(-1, ' ', FLAGS_ensure_ascii);
		return str;
	}
	catch (...) {
		nlohmann::json j2;
		j2["code"] = CODE_ERR_JSON_DUMP;
		j2["data"] = MSG_ERR_JSON_DUMP;
		std::string str = j2.dump(-1, ' ', FLAGS_ensure_ascii);
		return str;
	}
}

// ��ȡ״̬json�ַ��� 
std::string Task::get_state_json(int code, std::string msg) {
	nlohmann::json j;
	if (code == CODE_INIT && msg.empty()) { // ���գ���䵱ǰ״̬ 
		code = t_code;
		msg = t_msg;
	}
	j["code"] = code;
	j["data"] = msg;
	return json_dump(j);
}

// ��OCR���ת��Ϊjson�ַ��� 
std::string Task::get_ocr_result_json(const std::vector<OCRPredictResult>& ocr_result) {
	nlohmann::json outJ;
	outJ["code"] = 100;
	outJ["data"] = nlohmann::json::array();
	bool isEmpty = true;
	for (int i = 0; i < ocr_result.size(); i++) {
		nlohmann::json j;
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
	// ���1��ʶ��ɹ��������֣�recδ����� 
	if (isEmpty) {
		return "";
	}
	// ���2��ʶ��ɹ��������� 
	return json_dump(outJ);
}

// ����ͼƬʶ��ģʽ 
int Task::single_image() {
	set_state();
	cv::Mat img = imread_u8(FLAGS_image_path);
	if (img.empty()) { // ��ͼʧ�� 
		std::cout << get_state_json() << std::endl;
		return 0;
	}
	// ִ��OCR 
	std::vector<OCRPredictResult> res_ocr = ppocr->ocr(img, FLAGS_det, FLAGS_rec, FLAGS_cls);
	// ��ȡ��� 
	std::string res_json = get_ocr_result_json(res_ocr);
	// ���1��ʶ��ɹ��������֣�recδ����� 
	if (res_json.empty()) {
		std::cout << get_state_json(CODE_OK_NONE, MSG_OK_NONE(FLAGS_image_path)) << std::endl;
	}
	// ���2��ʶ��ɹ��������� 
	else {
		std::cout << res_json << std::endl;
	}
	return 0;
}

}