_Pragma("once");
#include<opencv2/opencv.hpp>
#include<vector>
namespace PiKtures::Detector{
    using cv::Mat;
    class FaceDetector{
        protected:
            FaceDetector() = default;
        public:
            virtual ~FaceDetector(){}
            virtual void loadModule(const char* const) = 0;
            virtual void detect(const Mat&, std::vector<Mat>&) = 0;
            static Mat transformForDetection(const Mat&);
    };
    class OpenCVCascade: public FaceDetector{
        private:
            cv::CascadeClassifier cascade_classifier_;
        public:
            OpenCVCascade() = default;
            OpenCVCascade(const char* const);
            ~OpenCVCascade() = default;
            void loadModule(const char* const);
            void detect(const Mat&, std::vector<Mat>&);
    };
}