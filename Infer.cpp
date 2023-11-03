#include "Infer.h"

static constexpr float inv255 = 1.0f / 255.0f;
using namespace cv;

Infer::Infer(Net_config config)
{
	this->confThreshold = config.confThreshold;
	this->nmsThreshold = config.nmsThreshold;

	string classesFile = "label.names";
	string model_path = config.modelpath;
	std::wstring widestr = std::wstring(model_path.begin(), model_path.end());
	//OrtStatus* status = OrtSessionOptionsAppendExecutionProvider_CUDA(sessionOptions, 0);
	
	OrtTensorRTProviderOptions * trt_options = new OrtTensorRTProviderOptions();
	trt_options->trt_fp16_enable = 1;
	trt_options->trt_engine_cache_enable = 1;
	trt_options->trt_max_workspace_size = 11811160064;
	//trt_options->trt_max_workspace_size = 8589934592;
	//trt_options->trt_max_workspace_size = 4294967296;
	trt_options->trt_engine_cache_path = "./cache";
	sessionOptions.AppendExecutionProvider_TensorRT(*trt_options);
	sessionOptions.SetExecutionMode(ORT_SEQUENTIAL);
	sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
	sessionOptions.EnableMemPattern();
	sessionOptions.EnableCpuMemArena();

	ort_session = new Session(env, widestr.c_str(), sessionOptions);
	size_t numInputNodes = ort_session->GetInputCount();
	size_t numOutputNodes = ort_session->GetOutputCount();
	
	for (int i = 0; i < numInputNodes; i++)
	{
		input_names_allocated.push_back(ort_session->GetInputNameAllocated(i, allocator));
		Ort::TypeInfo input_type_info = ort_session->GetInputTypeInfo(i);
		auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
		auto input_dims = input_tensor_info.GetShape();
		input_node_dims.push_back(input_dims);
		input_names.push_back( input_names_allocated[i].get() );
	}

	for (int i = 0; i < numOutputNodes; i++)
	{
		output_names_allocated.push_back(ort_session->GetOutputNameAllocated(i, allocator));
		Ort::TypeInfo output_type_info = ort_session->GetOutputTypeInfo(i);
		auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
		auto output_dims = output_tensor_info.GetShape();
		output_node_dims.push_back(output_dims);
		output_names.push_back(output_names_allocated[i].get());
	}
	
	//cout << output_names[0] << endl;

	this->inpHeight = input_node_dims[0][2];
	this->inpWidth = input_node_dims[0][3];
	//this->nout = output_node_dims[0][2];  //garbage?
	this->num_proposal = output_node_dims[0][1];
	this->input_shape_ = { 1, 3, this->inpHeight, this->inpWidth };

	this->class_names = { "enemies","friends" };
	this->num_class = class_names.size();
	
	this->input_image_.resize(416 * 416 * 3);

	this->input_image_size = this->input_image_.size();
	this->input_shape_size = this->input_shape_.size();
	this->output_names_size = this->output_names.size();
	
}

/*
void Infer::normalize_(Mat img)
{
	// img.convertTo(img, CV_32F);
	int row = img.rows;
	int col = img.cols;
	this->input_image_.resize(row * col * img.channels());
	for (int c = 0; c < 3; c++)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				float pix = img.ptr<uchar>(i)[j * 3 + 2 - c];
				this->input_image_[c * row * col + i * col + j] = pix / 255.0;
			}
		}
	}
}
*/


/*
inline void Infer::normalize_(const Mat& img)
{
	//static constexpr int row = 416;
	//cout << img.rows << endl;
	//static constexpr int col = 416;
	//cout << img.cols << endl;
	//this->input_image_.resize(row * col * 3);

	// Precompute 1.0 / 255.0 to avoid division in the loop
	// const float inv255 = 1.0f / 255.0f;
	//#pragma omp parallel
	//#pragma omp for nowait
	for (int i = 0; i < 416; i++)
	{
		const uchar* rowPtr = img.ptr<uchar>(i);
		for (int j = 0; j < 416; j++)
		{
			for (int c = 0; c < 3; c++)
			{
				const float pix = static_cast<float>(rowPtr[j * 3 + 2 - c]) * inv255;
				//this->input_image_[(c * row + i) * col + j] = pix;
				this->input_image_[(c * 416 + i) * 416 + j] = pix;
			}
		}
	}
}
*/


inline void Infer::normalize_(const Mat& img)
{
	//static constexpr int row = 416;
	//cout << img.rows << endl;
	//static constexpr int col = 416;
	//cout << img.cols << endl;
	//this->input_image_.resize(row * col * 3);

	// Precompute 1.0 / 255.0 to avoid division in the loop
	// const float inv255 = 1.0f / 255.0f;
	for (int i = 0; i < 416; i++)
	{
		const uchar* rowPtr = img.ptr<uchar>(i);
		for (int j = 0; j < 416; j++)
		{
			//const float pix0
			this->input_image_[(0 * 416 + i) * 416 + j] = static_cast<float>(rowPtr[j * 3 + 2]) * inv255;

			//const float pix1 
			this->input_image_[(1 * 416 + i) * 416 + j] = static_cast<float>(rowPtr[j * 3 + 1]) * inv255;

			//const float pix2
			this->input_image_[(2 * 416 + i) * 416 + j] = static_cast<float>(rowPtr[j * 3 + 0]) * inv255;
		}
	}
}


/*

void Infer::nms(vector<BoxInfo>& input_boxes)
{
	sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
	vector<float> vArea(input_boxes.size());
	for (int i = 0; i < int(input_boxes.size()); ++i)
	{
		vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
			* (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
	}

	vector<bool> isSuppressed(input_boxes.size(), false);
	for (int i = 0; i < int(input_boxes.size()); ++i)
	{
		if (isSuppressed[i]) { continue; }
		for (int j = i + 1; j < int(input_boxes.size()); ++j)
		{
			if (isSuppressed[j]) { continue; }
			float xx1 = (max)(input_boxes[i].x1, input_boxes[j].x1);
			float yy1 = (max)(input_boxes[i].y1, input_boxes[j].y1);
			float xx2 = (min)(input_boxes[i].x2, input_boxes[j].x2);
			float yy2 = (min)(input_boxes[i].y2, input_boxes[j].y2);

			float w = (max)(float(0), xx2 - xx1 + 1);
			float h = (max)(float(0), yy2 - yy1 + 1);
			float inter = w * h;
			float ovr = inter / (vArea[i] + vArea[j] - inter);

			if (ovr >= this->nmsThreshold)
			{
				isSuppressed[j] = true;
			}
		}
	}
	// return post_nms;
	int idx_t = 0;
	input_boxes.erase(remove_if(input_boxes.begin(), input_boxes.end(), [&idx_t, &isSuppressed](const BoxInfo& f) { return isSuppressed[idx_t++]; }), input_boxes.end());
}
*/

float * Infer::detect(const Mat& frame)
{
	//Mat dstimg;
	//resize(frame, dstimg, Size(this->inpWidth, this->inpHeight));
	//auto per_last = std::chrono::system_clock::now();
	this->normalize_(frame);
	//std::cout << chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - per_last).count() << endl;
	//array<int64_t, 4> input_shape_{ 1, 3, this->inpHeight, this->inpWidth };

	//auto allocator_info = MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

	const Value input_tensor_ = Value::CreateTensor<float>(this->allocator_info,
		this->input_image_.data(),
		this->input_image_size,
		this->input_shape_.data(),
		this->input_shape_size);
	
	ort_outputs = this->ort_session->Run(RunOptions{ nullptr },
		&this->input_names[0],
		&input_tensor_, 1,
		this->output_names.data(),
		this->output_names_size);

	//float* pdata = ort_outputs[0].GetTensorMutableData<float>();
	
	/////generate proposals
	
	//float ratioh = (float)frame.rows / this->inpHeight, ratiow = (float)frame.cols / this->inpWidth;
	
	/*
	vector<BoxInfo> generate_boxes;
	if (pdata != nullptr) {
		int n = 0;
		do {

			//if(pdata[n+6] > confThreshold and !pdata[n+5]) generate_boxes.push_back(BoxInfo{pdata[n + 1],pdata[n + 2],pdata[n + 3],pdata[n + 4],pdata[n + 6],(int)pdata[n + 5]});
			if (pdata[n + 6] > confThreshold and !pdata[n + 5]) generate_boxes.emplace_back(BoxInfo{ pdata[n + 1],pdata[n + 2],pdata[n + 3],pdata[n + 4] });
			//xmin ymin xmax ymax score class
			//remove n+5 class and n+6 score for faster push back;

			n += 7;

		} while (pdata[n + 6]);
	}
	*/

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	// nms(generate_boxes);
	/*
	
	for (size_t i = 0; i < generate_boxes.size(); ++i)
	{
		int xmin = int(generate_boxes[i].x1);
		int ymin = int(generate_boxes[i].y1);
		rectangle(frame, Point(xmin, ymin), Point(int(generate_boxes[i].x2), int(generate_boxes[i].y2)), Scalar(0, 0, 255), 2);
		string label = format("%.2f", generate_boxes[i].score);
		label = this->class_names[generate_boxes[i].label] + ":" + label;
		putText(frame, label, Point(xmin, ymin - 5), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 1);
	}

	*/

	return ort_outputs[0].GetTensorMutableData<float>();
	
	
};
/*
void Infer::free_output(float * output) {
	this->allocator.Free(output);
}
*/