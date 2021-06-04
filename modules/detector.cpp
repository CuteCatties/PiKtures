#include<detector.hpp>
#include<stdexcept>
cv::Mat PiKtures::Detector::FaceDetector::transformForDetection(const Mat& origin){
    Mat result;
    cv::cvtColor(origin, result, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(result, result);
    return result;
}
PiKtures::Detector::OpenCVCascade::OpenCVCascade(
    const char* const file_path
):cascade_classifier_(file_path){
    if(cascade_classifier_.empty()) throw std::runtime_error("panic: Failed to load module in constructor.");
}
void PiKtures::Detector::OpenCVCascade::loadModule(const char* const file_path){
    cascade_classifier_.load(file_path);
    if(cascade_classifier_.empty()) throw std::runtime_error("panic: Failed to load module in loadModule().");
}
void PiKtures::Detector::OpenCVCascade::detect(const Mat& image, std::vector<Mat>& result){
    if(cascade_classifier_.empty()) throw std::runtime_error("panic: No module loaded.");
    std::vector<cv::Rect> faces;
    cascade_classifier_.detectMultiScale(
        PiKtures::Detector::FaceDetector::transformForDetection(image),
        faces
    );
    result.clear();
    for(unsigned int index = 0; index < faces.size(); ++index){
        cv::Rect& r = faces[index];
        result.push_back(Mat::zeros(image.size(), CV_8U));
        Mat& target = result[index];
        target(r) = Mat::ones(r.size(), CV_8U);
    }
}