#ifndef TENSORFLOWDETECTOR_HPP
#define TENSORFLOWDETECTOR_HPP

// Libraries

#include <string.h>
#include <vector>

#include <opencv2/core/mat.hpp>

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"



tensorflow::Status readLabelsMapFile(const tensorflow::string &fileName, std::map<int, tensorflow::string> &labelsMap);

tensorflow::Status loadGraph(const tensorflow::string &graph_file_name, std::unique_ptr<tensorflow::Session> *session);

tensorflow::Status readTensorFromMat(const cv::Mat &mat, tensorflow::Tensor &outTensor);

std::string drawBoundingBoxOnImage(cv::Mat &image, double xMin, double yMin, double xMax, double yMax, double score, std::string label, bool scaled);

std::string drawBoundingBoxesOnImage(cv::Mat &image, tensorflow::TTypes<float>::Flat &scores, tensorflow::TTypes<float>::Flat &classes, tensorflow::TTypes<float,3>::Tensor &boxes, std::map<int, tensorflow::string> &labelsMap, std::vector<size_t> &idxs);

double IOU(cv::Rect box1, cv::Rect box2);

std::vector<size_t> filterBoxes(tensorflow::TTypes<float>::Flat &scores, tensorflow::TTypes<float, 3>::Tensor &boxes, double thresholdIOU, double thresholdScore);

#endif // TENSORFLOWDETECTOR_HPP
