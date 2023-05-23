
#include <include/paddleocr.h>
#include <include/args.h>
#include <include/task.h>

namespace PaddleOCR {

// OCR ����ѭ��
int Task::ocr(){
	// ��ʼ������
	ppocr = PPOCR();
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

// ��ȡ״̬json 
std::string get_state_json() {

}

// ����ͼƬʶ��ģʽ
int Task::single_image() {
	set_state();
	cv::Mat img = imread_u8(FLAGS_image_path);
	if (img.empty()) { // ��ͼʧ�� 
		std::string str = get_state_json(); // ��ȡ״̬json 
		return 0;
	}
	//if (!PaddleOCR::Utility::PathExists(FLAGS_image_path)) {
	//	set_state(CODE_ERR_PATH_EXIST, MSG_ERR_PATH_EXIST(FLAGS_image_path)); // ����״̬��·�������� 
	//	return 1;
	//}


	return 0;
}

}