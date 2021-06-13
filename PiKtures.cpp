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
#include<stack>
#include<filesystem>
using namespace std;
using namespace cv;
using namespace std::filesystem;
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
unsigned int bound_width = 30;
unsigned int bound_gaussian_size = 9;
double bound_gaussian_sigma = 10;

// global variables
bool image_opened = false;
Mat operational_image;
bool show_spectrum = false;
Mat spectrum[3];
Mat face;
Mat watermark;
stack<Mat> past_stack;
stack<Mat> future_stack;

auto detector_holder = FaceDetector::getFaceDetector();
FaceDetector& detector = *detector_holder;
CommandParser top_cp("");
CommandParser noise_cp("noise: ");
CommandParser denoise_cp("denoise: ");
CommandParser enhance_cp("enhance: ");

// functions to be registered
unsigned int terminatePiKtures(
    ostream& out,
    const char* pfx,
    const int argc,
    const char** argv
){
    destroyAllWindows();
    exit(0);
    return static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
}

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
            if(!is_regular_file(argv[1])){
                out<<pfx<<"cannot open specified file.\n";
                return static_cast<unsigned int>(ErrorCode::FILE_UNACCESSIABLE);
            }
            operational_image = imread(argv[1]);
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
            try{
                path target(argv[1]);
                create_directories(target.parent_path());
            }catch(exception& e){
                out<<pfx<<e.what()<<endl;
                out<<pfx<<"!!!! IMAGE IS NOT SAVED !!!!\n";
                return static_cast<unsigned int>(ErrorCode::FILESYSTEM_ERROR);
            }
            imwrite(argv[1], operational_image);
            return static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
        },
        nullptr
    },
    {
        "undo",
        "Cancel last change made to opened image.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc > 2){
                out<<pfx<<"undo takes no parameters.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            if(past_stack.size() <= 1){
                out<<pfx<<"no operation history.\n";
                return static_cast<unsigned int>(ErrorCode::EMPTY_PAST_STACK);
            }
            future_stack.push(past_stack.top());
            past_stack.pop();
            operational_image = past_stack.top();
            past_stack.pop();
            return static_cast<unsigned int>(ErrorCode::NO_STACK_MODIFICATION);
        },
        nullptr
    },
    {
        "redo",
        "Cancel last undo.",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(!image_opened){
                out<<pfx<<"no image currently opened.\n";
                return static_cast<unsigned int>(ErrorCode::NO_IMAGE_OPENED);
            }
            if(argc > 2){
                out<<pfx<<"redo takes no parameters.\n";
                return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_MUCH);
            }
            if(future_stack.empty()){
                out<<pfx<<"no cancelable undo operations.\n";
                return static_cast<unsigned int>(ErrorCode::EMPTY_FUTURE_STACK);
            }
            //imshow("stacktop", future_stack.top());
            operational_image = future_stack.top();
            future_stack.pop();
            return static_cast<unsigned int>(ErrorCode::NO_STACK_MODIFICATION);
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
            return static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
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
            }
            return static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
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
            return static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
        },
        nullptr
    },
    {
        "quit",
        "terminate this program",
        terminatePiKtures,
        nullptr
    },
    {
        "exit",
        "terminate this program",
        terminatePiKtures,
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
            return static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
        },
        nullptr
    },
    {
        "help",
        "Show help message",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            out<<pfx<<endl;
            noise_cp.listCommands("", out, "\t", true, 10);
            return static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
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
            return static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
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
                    for(auto& res: result){
                        Mat f;
                        applyToEachChannel(
                            [](Mat& channel, const Mat& selector){
                                channel = channel.mul(selector);
                            },
                            f, operational_image, res
                        );
                        Mat r = operational_image - f;
                        dermabrasion(f, strtod(argv[1], nullptr), strtod(argv[2], nullptr));
                        operational_image = f + r;
                        Mat sf = getBoundarySelector(res, bound_width);
                        Mat sr = reverseSelector(sf);
                        Mat preserve;
                        applyToEachChannel(
                            [](Mat& channel, const Mat& selector){
                                channel = channel.mul(selector);
                            },
                            preserve, operational_image, sr
                        );
                        gaussianFilter(operational_image, bound_gaussian_size, bound_gaussian_sigma);
                        applyToEachChannel(
                            [](Mat& channel, const Mat& selector){
                                channel = channel.mul(selector);
                            },
                            operational_image, operational_image, sf
                        );
                        operational_image += preserve;
                    }
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
                    for(auto& res: result){
                        Mat f;
                        applyToEachChannel(
                            [](Mat& channel, const Mat& selector){
                                channel = channel.mul(selector);
                            },
                            f, operational_image, res
                        );
                        Mat r = operational_image - f;
                        luminance(f, strtod(argv[1], nullptr), strtol(argv[2], nullptr, 10));
                        operational_image = f + r;
                        Mat sf = getBoundarySelector(res, bound_width);
                        Mat sr = reverseSelector(sf);
                        Mat preserve;
                        applyToEachChannel(
                            [](Mat& channel, const Mat& selector){
                                channel = channel.mul(selector);
                            },
                            preserve, operational_image, sr
                        );
                        gaussianFilter(operational_image, bound_gaussian_size, bound_gaussian_sigma);
                        applyToEachChannel(
                            [](Mat& channel, const Mat& selector){
                                channel = channel.mul(selector);
                            },
                            operational_image, operational_image, sf
                        );
                        operational_image += preserve;
                    }
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
            return static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
        },
        nullptr
    }
});

int main(int argc, char** argv){
    enhance_cp.insertCommand(enhance_commands);
    denoise_cp.insertCommand(denoise_commands);
    noise_cp.insertCommand(noise_commands);
    top_cp.insertCommand(top_commands);
    string read_buffer;
    vector<string> split_buffer;
    unsigned int r = static_cast<unsigned int>(ErrorCode::NO_MODIFYCATION);
    bool side = 1;
    const char** args = nullptr;
    cv::startWindowThread();
    cout<<">>>> PiKtures v"<<PiKtures_VERSION_MAJOR<<"."<<PiKtures_VERSION_MINOR<<" by CuteKitties powered by OpenCV4 <<<<\n";
    while(true){
        if(image_opened){
            past_stack.push(operational_image.clone());
            imshow(image_window, operational_image);
        }
        if(show_spectrum) for(int i = 0; i < 3; ++i){
            spectrum[i] = calculateSpectrum(operational_image, i);
            imshow(spectrum_window[i], spectrum[i]);
        }
        if(isError(r)){
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
        }else{
            args = new const char*[split_buffer.size() + 1];
            for(decltype(split_buffer.size()) i = 0; i < split_buffer.size(); ++i) args[i] = split_buffer.at(i).data();
            args[split_buffer.size()] = nullptr;
            r = top_cp.parse(split_buffer.at(0).c_str(), cout, split_buffer.size(), args);
            if(args != nullptr) delete[] args;
            args = nullptr;
        }
        if(maintainStacks(r)){
            if(isModified(r)){
                while(!future_stack.empty()) future_stack.pop();
            }else{
                if(!past_stack.empty()) past_stack.pop();
            }
        }
    }
    return 0;
}