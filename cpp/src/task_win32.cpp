
// Windows ƽ̨����������� 

#ifdef _WIN32

#include <windows.h>

#include "include/paddleocr.h"
#include "include/args.h"
#include "include/task.h"

// ����ת��
#include<codecvt>
std::wstring_convert<std::codecvt_utf8<wchar_t>> conv_Ustr_Wstr; // string utf-8 �� wstring utf-16 ��˫��ת����

namespace PaddleOCR {

// ==================== ���ߺ��� ====================

// ���ֽ�ANSI�ַ�����ת���ַ�����
wchar_t* char_2_wchar(char* c) {
    setlocale(LC_ALL, ""); // �������������Ϊwindowsϵͳ��ǰ����
    size_t lenWchar = mbstowcs(NULL, c, 0); // �õ�תΪ���ַ����ĳ���
    wchar_t* wc = new wchar_t[lenWchar + 1]; // ����ļ����Ŀ��ַ���
    int n = mbstowcs(wc, c, lenWchar + 1); // ���ֽ�ת���ַ�
    setlocale(LC_ALL, "C"); // ��ԭ��������ΪĬ��
    return wc;
}

// ר��������Ϣ��wstringתstring��ת��ʧ��ʱ����Ĭ����ʾ����
std::string msg_wstr_2_ustr(std::wstring& msg) {
    try {
        std::string msgU8 = conv_Ustr_Wstr.to_bytes(msg); // ת��u8
        return msgU8;
    }
    catch (...) {
        return "wstring failed to convert to utf-8 string";
    }
}

// ���·��pathW�Ƿ�Ϊ�ļ����Ƿ���true
bool is_exists_wstr(std::wstring pathW) {
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
cv::Mat Task::imread_wstr(std::wstring pathW, int flag) {
    std::string pathU8 = msg_wstr_2_ustr(pathW); // ��ת��utf-8���Ա��������
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
    cv::Mat img = cv::imdecode(arr, flag); // �����ڴ����ݣ����cv::Mat����
    delete[] buf; // �ͷ�buf�ռ�
    fclose(fp); // �ر��ļ�
    if (!img.data) {
        set_state(CODE_ERR_PATH_DECODE, MSG_ERR_PATH_DECODE(pathU8)); // ����״̬������ʧ��
    }
    return img;
}

// ==================== ���ʵ�� ====================

// ����cv imread������utf-8�ַ������룬����Mat�� 
cv::Mat Task::imread_u8(std::string pathU8, int flag) {
    if (pathU8 == u8"clipboard") { // ��Ϊ����������
        return imread_clipboard(flag);
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


// �Ӽ��������һ��ͼƬ������Mat��ע��flag�Լ������ڴ�ͼƬ��Ч�����Լ�����·��ͼƬ��Ч�� 
cv::Mat Task::imread_clipboard(int flag) {
    // �ο��ĵ��� https://docs.microsoft.com/zh-cn/windows/win32/dataxchg/using-the-clipboard

    // ���Դ򿪼����壬��������ֹ����Ӧ�ó����޸ļ���������
    if (!OpenClipboard(NULL)) {
        set_state(CODE_ERR_CLIP_OPEN, MSG_ERR_CLIP_OPEN); // ����״̬���������ʧ��
    }
    else {
        static UINT auPriorityList[] = {  // �������ļ������ʽ��
          CF_BITMAP,                      // λͼ
          CF_HDROP,                       // �ļ��б������ļ�������ѡ���ļ����ƣ�
        };
        int auPriorityLen = sizeof(auPriorityList) / sizeof(auPriorityList[0]); // �б���
        int uFormat = GetPriorityClipboardFormat(auPriorityList, auPriorityLen); // ��ȡ��ǰ���������ݵĸ�ʽ
        // ���ݸ�ʽ���䲻ͬ����
        //     ������ɹ����ͷ�ȫ����Դ���رռ����壬����ͼƬmat��
        //     ������ʧ�ܣ��ͷ��Ѵ򿪵���Դ����������״̬������switch��ͳһ�رռ�����ͷ��ؿ�mat
        switch (uFormat)
        {

        case CF_BITMAP: { // 1. λͼ ===================================================================
            HBITMAP hbm = (HBITMAP)GetClipboardData(uFormat); // 1.1. �Ӽ�������¼��ָ�룬�õ��ļ����
            if (hbm) {
                // GlobalLock(hbm); // ����ֵ������Ч�ģ���λͼ�ƺ�����Ҫ����
              // https://social.msdn.microsoft.com/Forums/vstudio/en-US/d2a6aa71-68d7-4db0-8b1f-5d1920f9c4ce/globallock-and-dib-transform-into-hbitmap-issue?forum=vcgeneral
                BITMAP bmp; // ���ָ�򻺳�����ָ�룬�����������й�ָ��ͼ�ζ������Ϣ
                GetObject(hbm, sizeof(BITMAP), &bmp); // 1.2. ��ȡͼ�ζ������Ϣ������ͼƬ���ݱ���
                if (!hbm) {
                    set_state(CODE_ERR_CLIP_GETOBJ, MSG_ERR_CLIP_GETOBJ); // ����״̬������ͼ�ζ�����Ϣʧ��
                    break;
                }
                int nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel / 8; // ����ɫ�����ͨ������32bitΪ4��24bitΪ3
                // 1.3. �����hbm�е�λͼ���Ƶ�������
                long sz = bmp.bmHeight * bmp.bmWidth * nChannels; // ͼƬ��С���ֽڣ�
                cv::Mat mat(cv::Size(bmp.bmWidth, bmp.bmHeight), CV_MAKETYPE(CV_8U, nChannels));  // ����վ��󣬴���λͼ��С�����
                long getsz = GetBitmapBits(hbm, sz, mat.data); // �����hbm��sz���ֽڸ��Ƶ�������img.data
                if (!getsz) {
                    set_state(CODE_ERR_CLIP_BITMAP, MSG_ERR_CLIP_BITMAP); // ����״̬����ȡλͼ����ʧ��
                    break;
                }
                CloseClipboard();  // �ͷ���Դ
                // 1.4. ���غ��ʵ�ͨ��
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
                cv::Mat mat = imread_wstr(nameW, flag); // 2.7. ���Զ�ȡ�ļ�
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


} // namespace PaddleOCR



#endif