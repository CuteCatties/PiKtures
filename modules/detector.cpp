#include<detector.hpp>
#include<utility.hpp>
namespace PiKtures::Detector{
    using PiKtures::Utility::ErrorCode;
}
cv::Mat PiKtures::Detector::FaceDetector::transformForDetection(const Mat& origin){
    Mat result;
    cv::cvtColor(origin, result, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(result, result);
    return result;
}
PiKtures::Detector::FaceDetector&& PiKtures::Detector::FaceDetector::getFaceDetector(){
    return OpenCVCascade();
}
PiKtures::Detector::FaceDetector&& PiKtures::Detector::FaceDetector::getFaceDetector(const char* const module){
    return OpenCVCascade(module);
}
PiKtures::Detector::OpenCVCascade::OpenCVCascade(
    const char* const file_path
):cascade_classifier_(file_path){
    if(cascade_classifier_.empty()) throw ErrorCode::MODULE_LOAD_FAILED;
}
void PiKtures::Detector::OpenCVCascade::loadModule(const char* const file_path){
    cascade_classifier_.load(file_path);
    if(cascade_classifier_.empty()) throw ErrorCode::MODULE_LOAD_FAILED;
}
void PiKtures::Detector::OpenCVCascade::detect(const Mat& image, std::vector<Mat>& result){
    if(cascade_classifier_.empty()) throw ErrorCode::NO_ACTIVE_MODULE;
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
bool PiKtures::Detector::OpenCVCascade::ready()const{
    return (!cascade_classifier_.empty());
}