#include<detector.hpp>
#include<utility.hpp>
#include<vector>
#include<string>
using namespace std;
using namespace cv;
using PiKtures::Detector::OpenCVCascade;
using PiKtures::Utility::applyToEachChannel;
using PiKtures::Utility::reverseSelector;
int main(int argc, char** argv){
    if(argc < 2){
        cerr<<"Please specify the path to pre-trained module.\n";
        return 1;
    }else if(argc > 2){
        cerr<<"Attention: extra parameters ignored.\n";
    }
    OpenCVCascade detector(argv[1]);
    Mat srcImage = imread("smp.jpg");
    if(!srcImage.data){
		cerr<<"Failed to read image.\n";
		return -1;
	}
    vector<Mat> results;
    Mat buffer;
    Mat buffer2;
    detector.detect(srcImage, results);
    for(size_t i = 0; i < results.size(); ++i){
        applyToEachChannel(
            [](Mat& channel, const Mat& selector)->void{
                channel = channel.mul(selector);
            },
            buffer, srcImage, results[i]
        );
        imwrite(string("+result") + to_string(i) + ".jpg", buffer);
        results[i] = reverseSelector(results[i]);
        applyToEachChannel(
            [](Mat& channel, const Mat& selector)->void{
                channel = channel.mul(selector);
            },
            buffer2, srcImage, results[i]
        );
        imwrite(string("-result") + to_string(i) + ".jpg", buffer2);
        buffer = buffer + buffer2;
        imwrite("origin.jpg", buffer);
    }
    return 0;
}