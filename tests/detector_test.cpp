#include<detector.hpp>
#include<vector>
#include<string>
using namespace std;
using namespace cv;
using PiKtures::Detector::OpenCVCascade;
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
    Mat buffer[3];
    split(srcImage, buffer);
    detector.detect(srcImage, results);
    for(size_t i = 0; i < results.size(); ++i){
        buffer[0] = buffer[0].mul(results[i]);
        buffer[1] = buffer[1].mul(results[i]);
        buffer[2] = buffer[2].mul(results[i]);
        merge(buffer, 3, results[i]);
        imwrite(string("result") + to_string(i) + ".jpg", results[i]);
    }
    return 0;
}