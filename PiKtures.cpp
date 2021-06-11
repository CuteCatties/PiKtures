#include<PiKturesConfig.hpp>
#include<utility.hpp>
#include<command.hpp>
#include<detector.hpp>
#include<rowdy.hpp>
#include<enhancer.hpp>
#include<opencv2/opencv.hpp>
#include<iostream>
#include<vector>
#include<cstdlib>
using namespace std;
using namespace cv;
using namespace PiKtures::Command;
using namespace PiKtures::Detector;
using namespace PiKtures::Enhancer;
using namespace PiKtures::Rowdy;
using namespace PiKtures::Utility;

// constants
constexpr const char* image_window = "PiKture";
constexpr const char* spectrum_window[3] = {
    "spectrum_B", "spectrum_G", "spectrum_R"
};
constexpr const char* face_window = "face";
constexpr const char* watermark_window = "watermark";

// global variables
bool image_opened = false;
Mat operational_image;
bool show_spectrum = false;
Mat spectrum[3];
Mat face;
Mat watermark;

auto detector_holder = FaceDetector::getFaceDetector();
FaceDetector& detector = *detector_holder;
CommandParser top_cp("");
CommandParser noise_cp("noise: ");
CommandParser denoise_cp("denoise: ");
CommandParser enhance_cp("enhance: ");

// commands
vector<PiKtures::Command::CommandSpecifier> top_commands({
    {
        "open",
        "Open a image. Discard any unsaved changes in currently opened image.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(argc < 2){
                out<<pfx<<"please specify the path to the image to be opened.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }else if(argc > 2){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            operational_image = imread(argv[1]);
            if(!image_opened) namedWindow(image_window, WINDOW_NORMAL);
            image_opened = true;
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "save",
        "Save the opened image. Overwrite if specified file exists.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 2){
                out<<pfx<<"please specify the path to save the image.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }else if(argc > 2){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            imwrite(argv[1], operational_image);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "spectrum",
        "Show or hide spectrum.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc > 1){
                out<<pfx<<"no parameter is required.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            show_spectrum = !show_spectrum;
            if(show_spectrum){
                for(int i = 0; i < 3; ++i){
                     namedWindow(spectrum_window[i], WINDOW_NORMAL);
                }
            }else{
                for(int i = 0; i < 3; ++i){
                     destroyWindow(spectrum_window[i]);
                }
            }
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "face",
        "Detect face in opened image.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc > 2){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            if(argc == 2){
                detector.loadModule(argv[1]);
            }
            if(!detector.ready()){
                out<<pfx<<"no face detection module loaded.\n";
                return static_cast<unsigned int>(ErrorCode::NO_ACTIVE_MODULE);
            }
            vector<Mat> result;
            detector.detect(operational_image, result);
            if(result.size() == 0){
                out<<pfx<<"no face detected. but we are *not* sorry.\n";
            }else{
                face = operational_image.clone();
                applyToEachChannel(
                    [](Mat& channel, const Mat& selector){
                        channel = channel.mul(selector);
                    },
                    face, face, result[0]
                );
                imshow(face_window, face);
                waitKey(0);
                destroyWindow(face_window);
            }
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "noise",
        "Make image noisy.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            return noise_cp.parse(argv[1], out, argc - 1, argv + 1);
        },
        nullptr
    },
    {
        "denoise",
        "Try to make image quiet(?)",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            return denoise_cp.parse(argv[1], out, argc - 1, argv + 1);
        },
        nullptr
    },
    {
        "enhance",
        "Enhance the image",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            return enhance_cp.parse(argv[1], out, argc - 1, argv + 1);
        },
        nullptr
    },
    {
        "help",
        "Show help message",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            out<<pfx<<endl;
            top_cp.listCommands("", out, "\t", true, 10);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "quit",
        "terminate this program",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            cv::destroyAllWindows();
            std::exit(0);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "exit",
        "terminate this program",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            cv::destroyAllWindows();
            std::exit(0);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    }
});
vector<PiKtures::Command::CommandSpecifier> noise_commands({
    {
        "salt",
        "Add salt noise.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 3){
                out<<pfx<<"please specify the probability of both black and white pixels.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 3){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            salt(operational_image, strtod(argv[1], nullptr), strtod(argv[2], nullptr));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "random",
        "Add random noise.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 2){
                out<<pfx<<"please specify the probability of pixels to be changed.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 2){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            random(operational_image, strtod(argv[1], nullptr));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "gaussian",
        "Add gaussian noise.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 3){
                out<<pfx<<"please specify both expectation and deviation of the noise.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 3){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            gaussian(operational_image, strtod(argv[1], nullptr), strtod(argv[2], nullptr));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "watermark",
        "Add blind watermark to image.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 4){
                out<<pfx<<"please specify the content, size(scale) and power of watermark.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 4){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            blindWatermark(operational_image, argv[1], strtod(argv[2], nullptr), strtod(argv[3], nullptr));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "uncover",
        "Uncover blind watermark in image.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc > 1){
                out<<pfx<<"uncover takes no parameter.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            watermark = operational_image.clone();
            uncoverWatermark(watermark);
            imshow(watermark_window, watermark);
            waitKey(0);
            destroyWindow(watermark_window);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "help",
        "Show help message",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            out<<pfx<<endl;
            noise_cp.listCommands("", out, "\t", true, 10);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    }
});
vector<PiKtures::Command::CommandSpecifier> denoise_commands({
    {
        "median",
        "Apply median filter.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 2){
                out<<pfx<<"please specify the size of kernel.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 2){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            medianFilter(operational_image, strtol(argv[1], nullptr, 10));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "mean",
        "Apply mean filter.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 2){
                out<<pfx<<"please specify the size of kernel.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 2){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            meanFilter(operational_image, strtol(argv[1], nullptr, 10));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "gaussian",
        "Apply gaussian filter.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 3){
                out<<pfx<<"please specify the size of kernel and sigma.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 3){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            gaussianFilter(operational_image, strtol(argv[1], nullptr, 10), strtod(argv[2], nullptr));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "bilateral",
        "Apply bilateral filter.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 3){
                out<<pfx<<"please specify the sigma of color and space.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 3){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            bilateralFilter(operational_image, strtod(argv[1], nullptr), strtod(argv[2], nullptr));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "butterworthLowpass",
        "Apply butterworth low-pass filter.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 3){
                out<<pfx<<"please specify the radius and rank.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 3){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            butterworthLowpassFilter(operational_image, strtod(argv[1], nullptr), strtod(argv[2], nullptr));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "gaussianLowpass",
        "Apply gaussian low-pass filter.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 2){
                out<<pfx<<"please specify the radius.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 2){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            gaussianLowpassFilter(operational_image, strtod(argv[1], nullptr));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "help",
        "Show help message",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            out<<pfx<<endl;
            denoise_cp.listCommands("", out, "\t", true, 10);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    }
});
vector<PiKtures::Command::CommandSpecifier> enhance_commands({
    {
        "dermabrasion",
        "Apply dermabrasion within the area of face. Apply to whole image in case no face detected.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 3){
                out<<pfx<<"please specify the filter strength of color and space.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 3){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            if(detector.ready()){
                vector<Mat> result;
                detector.detect(operational_image, result);
                if(result.size() == 0){
                    dermabrasion(operational_image, strtod(argv[1], nullptr), strtod(argv[2], nullptr));
                }else{
                    Mat f = operational_image.clone();
                    Mat r = operational_image.clone();
                    applyToEachChannel(
                        [](Mat& channel, const Mat& selector){
                            channel = channel.mul(selector);
                        },
                        f, f, result[0]
                    );
                    result[0] = reverseSelector(result[0]);
                    applyToEachChannel(
                        [](Mat& channel, const Mat& selector){
                            channel = channel.mul(selector);
                        },
                        r, r, result[0]
                    );
                    dermabrasion(f, strtod(argv[1], nullptr), strtod(argv[2], nullptr));
                    operational_image = f + r;
                }
            }else{
                dermabrasion(operational_image, strtod(argv[1], nullptr), strtod(argv[2], nullptr));
            }
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "luminance",
        "Alter luminance within the area of face. Apply to whole image in case no face detected.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 3){
                out<<pfx<<"please specify the rate and bonus of luminance.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 3){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            if(detector.ready()){
                vector<Mat> result;
                detector.detect(operational_image, result);
                if(result.size() == 0){
                    luminance(operational_image, strtod(argv[1], nullptr), strtol(argv[2], nullptr, 10));
                }else{
                    Mat f = operational_image.clone();
                    Mat r = operational_image.clone();
                    applyToEachChannel(
                        [](Mat& channel, const Mat& selector){
                            channel = channel.mul(selector);
                        },
                        f, f, result[0]
                    );
                    result[0] = reverseSelector(result[0]);
                    applyToEachChannel(
                        [](Mat& channel, const Mat& selector){
                            channel = channel.mul(selector);
                        },
                        r, r, result[0]
                    );
                    luminance(f, strtod(argv[1], nullptr), strtol(argv[2], nullptr, 10));
                    operational_image = f + r;
                }
            }else{
                luminance(operational_image, strtod(argv[1], nullptr), strtol(argv[2], nullptr, 10));
            }
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "sharpen",
        "sharpen the whole image.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 3){
                out<<pfx<<"please specify the radius and power of sharpen.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 3){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            Mat sharpen_temp = operational_image.clone();
            gaussianHighpassFilter(sharpen_temp, strtod(argv[1], nullptr));
            sharpen_temp.convertTo(sharpen_temp, CV_32F);
            operational_image.convertTo(operational_image, CV_32F);
            scaleAdd(sharpen_temp, strtod(argv[2], nullptr), operational_image, operational_image);
            operational_image.convertTo(operational_image, CV_8U);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "highpass",
        "highpass",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc < 2){
                out<<pfx<<"please specify the radius.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
            }
            if(argc > 2){
                out<<pfx<<"too many parameters provided.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            gaussianHighpassFilter(operational_image, strtod(argv[1], nullptr));
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "help",
        "Show help message",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            out<<pfx<<endl;
            enhance_cp.listCommands("", out, "\t", true, 10);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    }
});

int main(){
    enhance_cp.insertCommand(enhance_commands);
    denoise_cp.insertCommand(denoise_commands);
    noise_cp.insertCommand(noise_commands);
    top_cp.insertCommand(top_commands);
    string read_buffer;
    vector<string> split_buffer;
    unsigned int r = 0;
    bool side = 1;
    const char** args = nullptr;
    cv::startWindowThread();
    cout<<">>>> PiKtures v"<<PiKtures_VERSION_MAJOR<<"."<<PiKtures_VERSION_MINOR<<" by CuteKitties powered by OpenCV4 <<<<\n";
    while(true){
        if(show_spectrum) for(int i = 0; i < 3; ++i){
            spectrum[i] = calculateSpectrum(operational_image, i);
            imshow(spectrum_window[i], spectrum[i]);
        }
        if(image_opened){
            imshow(image_window, operational_image);
        }
        if(r != 0){
            #ifdef __linux__
            cout<<"\033[6;31m"<<r<<"\033[0m";
            #else
            cout<<r;
            #endif
            side = !side;
            #ifdef __linux__
            cout<<"\033[1;36m";
            #endif
            if(side) cout<<"m(=.ω·=)o~ ";
            else cout<<"m(=·ω.=)o~ ";
            #ifdef __linux__
            cout<<"\033[0m";
            #endif
        }else{
            #ifdef __linux__
            cout<<"\033[1;36m";
            #endif
            cout<<"m(=·ω·=)o~ ";
            #ifdef __linux__
            cout<<"\033[0m";
            #endif
        }
        getline(cin, read_buffer);
        splitCommandLine(read_buffer, split_buffer);
        if(split_buffer.size() == 0){
            cout<<"say something...\n";
            r = static_cast<unsigned int>(ErrorCode::EMPTY_COMMAND);
        }
        args = new const char*[split_buffer.size() + 1];
        for(decltype(split_buffer.size()) i = 0; i < split_buffer.size(); ++i) args[i] = split_buffer.at(i).data();
        args[split_buffer.size()] = nullptr;
        r = top_cp.parse(split_buffer.at(0).c_str(), cout, split_buffer.size(), args);
        if(args != nullptr) delete[] args;
        args = nullptr;
    }
    return 0;
}