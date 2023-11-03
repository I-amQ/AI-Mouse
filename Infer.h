#pragma once
#include <fstream>
#include <sstream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <onnxruntime_cxx_api.h>

using namespace std;
using namespace Ort;

struct Net_config
{
	float confThreshold; // Confidence threshold
	float nmsThreshold;  // Non-maximum suppression threshold
	string modelpath;
};

typedef struct BoxInfo
{
	float x1;
	float y1;
	float x2;
	float y2;
	//float score;
	//int label;
} BoxInfo;

class Infer
{
public:
	Infer(Net_config config);
	float * detect(const cv::Mat& frame);
	float confThreshold;
	void free_output(float* output);

private:
	int inpWidth;
	int inpHeight;
	int nout;
	int num_proposal;
	vector<string> class_names;
	int num_class;
	float nmsThreshold;
	vector<float> input_image_;
	inline void normalize_(const cv::Mat & img);
	//void nms(vector<BoxInfo>& input_boxes);
	

	Env env = Env(ORT_LOGGING_LEVEL_ERROR, "Infer");
	Ort::Session* ort_session = nullptr;
	SessionOptions sessionOptions = SessionOptions();
	AllocatorWithDefaultOptions allocator;

	vector<char*> input_names;
	vector<char*> output_names;

	std::vector<AllocatedStringPtr> input_names_allocated;
	std::vector<AllocatedStringPtr> output_names_allocated;
	//behavior when using newer onnxruntime version

	vector<vector<int64_t>> input_node_dims; // >=1 outputs
	vector<vector<int64_t>> output_node_dims; // >=1 outputs

	array<int64_t, 4> input_shape_;
	size_t input_image_size;
	size_t input_shape_size;
	size_t output_names_size;

	MemoryInfo allocator_info = MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

	vector<Value> ort_outputs;

};

