#include "utility.h"


namespace ml_cam {

// Define Microsoft functions and data types to use in other platform
#if !defined(_WIN32)
typedef int errno_t;

// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/dupenv-s-wdupenv-s?view=vs-2017
error_t _dupenv_s(char **pValue, size_t *len, const char *pPath) {
    char *value;
    value = getenv(pPath);

    if (value != NULL) {
        *len = strlen(value);

        char *buf = (char *)malloc((*len + 1) * sizeof(char));
        // Check memory allocation
        if (buf == NULL) {  // Not enough memory
            return ENOMEM;
        }

        strncpy(buf, value, *len);
        buf[*len] = '\0';
        *pValue = buf;
    } else {
        return EINVAL;
    }

    return 0;
}
#endif  // _WIN32

void setLabel(cv::Mat &im, const std::string label, const cv::Point &origin) {
    int fontface = cv::FONT_HERSHEY_SIMPLEX;
    double scale = 0.5;
    int thickness = 1;
    int baseline = 0;

    cv::Size text =
        cv::getTextSize(label, fontface, scale, thickness, &baseline);
    cv::rectangle(im, origin + cv::Point(0, baseline),
                  origin + cv::Point(text.width, -text.height),
                  cv::Scalar(0, 255, 0), cv::FILLED);
    cv::putText(im, label, origin, fontface, scale, cv::Scalar(0, 0, 0),
                thickness, 8);
}

std::string getHomePath() {
    char *pValue;
    char *pValue2;
    size_t len;
    errno_t err = _dupenv_s(&pValue, &len, "HOME");
    errno_t err2;

    if (!err && pValue != NULL) {
        std::string pValueStdStr = pValue;
        free(pValue);
        return pValueStdStr;
    } else {
        err = _dupenv_s(&pValue, &len, "USERPROFILE");

        if (!err && pValue != NULL) {
            std::string pValueStdStr = pValue;
            free(pValue);
            return pValueStdStr;
        } else {
            err = _dupenv_s(&pValue, &len, "HOMEDRIVE");
            err2 = _dupenv_s(&pValue2, &len, "HOMEPATH");

            if (!err && !err2 && pValue != NULL && pValue2 != NULL) {
                std::string pValueStdStr = std::string(pValue) + pValue2;
                return pValueStdStr;
            }

            free(pValue);
            free(pValue2);
        }
    }

    return "";
}

QImage Mat2QImage(cv::Mat const &src) {
    cv::Mat temp;  // make the same cv::Mat
    cvtColor(src, temp,
             cv::COLOR_BGR2RGB);  // cvtColor Makes a copt, that what i need
    QImage dest((const uchar *)temp.data, static_cast<int>(temp.cols),
                static_cast<int>(temp.rows), static_cast<int>(temp.step),
                QImage::Format_RGB888);
    dest.bits();  // enforce deep copy, see documentation
    // of QImage::QImage ( const uchar * data, int width, int height, Format
    // format )
    return dest;
}

cv::Mat QImage2Mat(QImage const &src) {
    cv::Mat tmp(src.height(), src.width(), CV_8UC3, (uchar *)src.bits(),
                src.bytesPerLine());
    cv::Mat
        result;  // deep copy just in case (my lack of knowledge with open cv)
    cvtColor(tmp, result, cv::COLOR_BGR2RGB);
    return result;
}


void place_overlay(cv::Mat &image, const cv::Mat &overlay,
    int x, int y) {

    assert(x + overlay.cols < image.cols);
    assert(y + overlay.rows < image.rows);

    cv::Mat dest_roi;
    dest_roi = image(cv::Rect(x, y, overlay.cols, overlay.rows));
    overlay.copyTo(dest_roi);

}

}  // namespace ml_cam