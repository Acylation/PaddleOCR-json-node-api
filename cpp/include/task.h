// PaddleOCR-json
// https://github.com/hiroi-sora/PaddleOCR-json

#ifndef TASK_H
#define TASK_H

namespace PaddleOCR {

// ==================== ��־�� ==================== 
#define CODE_INIT                0   // ÿ�غϳ�ʼֵ���غϽ���ʱ��Ϊ�������ܹܿص�������δ���ִ���
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

// ==================== ��������� ==================== 
class Task {

public:
	int ocr(); // OCRͼƬ 

private:
	PPOCR ppocr; // OCR������� 
	int t_code; // ��������״̬�� 
	std::string t_msg; // ��������״̬��Ϣ 

	// ��������
	int single_image(); // ����ʶ��ģʽ 
	int socket_mode(); // �׽���ģʽ 
	int anonymous_pipe_mode(); // �����ܵ�ģʽ 

	// ���� 
	void set_state(int code = CODE_INIT, std::string msg = ""); // ����״̬ 
	std::string get_state_json(); // ��ȡ��ǰ״̬��json�ַ��� 

	// OpenCV���
	//    ����cv imread������utf-8�ַ������룬����Mat��ʧ��ʱ���ô����룬�����ؿ�Mat�� 
	cv::Mat imread_u8(std::string path, int flag = cv::IMREAD_COLOR);
#ifdef _WIN32
	//    ���� cv::imread ����·��pathW����һ��ͼƬ��pathW����Ϊunicode��wstring
	cv::Mat imread_wstr(std::wstring pathW, int flags = cv::IMREAD_COLOR);
#endif
	//    �Ӽ������ж�ȡͼƬ 
	cv::Mat imread_clipboard(int flag = cv::IMREAD_COLOR);

};

} // namespace PaddleOCR

#endif // TASK_H