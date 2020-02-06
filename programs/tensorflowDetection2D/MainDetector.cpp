
// Libraries
#include <time.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string.h>
#include <utility>
#include <vector>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Port.h>
#include <yarp/os/PortReader.h>

#include <cv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "tensorflow/cc/ops/const_op.h"
#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/command_line_flags.h"

#include "MainDetector.hpp"
#include "TensorflowDetector.hpp"



int maindetector::runDetection(std::string labels, std::string graph, yarp::os::Port tensorflowDetection2DImgOutput, yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *tensorflowDetection2DImgInput, yarp::os::Port tensorflowDetection2DDataOutput) {

    // Recieve paths
    std::string LABELS = labels;
    std::string GRAPH = graph;

    // Set nodes input/output names
    tensorflow::string inputLayer = "image_tensor:0";
    std::vector<std::string> outputLayer = {"detection_boxes:0", "detection_scores:0", "detection_classes:0", "num_detections:0"};

    // Load an check .pb frozen model
    std::unique_ptr<tensorflow::Session> session;
    tensorflow::string graphPath = GRAPH;
    std::cout<<"The graph it´s going to be loaded:" << graphPath<<"."<<std::endl;
    std::cout<<"Loading graph..."<<std::endl;

    tensorflow::Status loadGraphStatus = loadGraph(graphPath, &session);
    if (!loadGraphStatus.ok())
    {
        std::cout<<std::endl;
        std::cout<<std::endl;
        std::cout<<"Fail loading graph "<<graphPath<<"."<<std::endl;
        return -1;
    } else
        std::cout<<std::endl;
        std::cout<<std::endl;
        std::cout<<"Graph "<<graphPath<<" loaded correctly."<<std::endl;


    // Load and check labels
    std::map<int, std::string> labelsMap = std::map<int,std::string>();
    std::cout<<std::endl;
    std::cout<<std::endl;
    std::cout<<"Labels "<<LABELS<<" are going to be loaded."<<std::endl;
    tensorflow::Status readLabelsMapStatus = readLabelsMapFile(LABELS, labelsMap);

    if (!readLabelsMapStatus.ok())
    {
        std::cout<<"Fail loading labels "<<LABELS<<"."<<std::endl;
        return -1;
    } else
        std::cout<<"Labels "<<LABELS<<" loaded correctly."<<std::endl;
        std::cout<<labelsMap.size()<<" labels have been loaded."<<std::endl;

    std::cout<<std::endl;
    std::cout<<std::endl;

    // Configure Tensor
    std::cout<<"Video source frames are going to be taken."<<std::endl;
    cv::Mat frame;
    tensorflow::Tensor tensor;
    std::vector<tensorflow::Tensor> outputs;
    double thresholdScore = 0.5;
    double thresholdIOU = 0.8;

    // Count FPS
    int nFrames = 25;
    int iFrame = 0;
    double fps = 0.;
    time_t start, end;
    time(&start);

    // Get image
    yarp::sig::ImageOf<yarp::sig::PixelRgb> *tensorflowDetection2DImgInputShape = tensorflowDetection2DImgInput->read();
    yarp::os::Bottle bottle;

    // Configure shape
    tensorflow::TensorShape shape = tensorflow::TensorShape();
    shape.AddDim(1);
    shape.AddDim((tensorflow::int64)tensorflowDetection2DImgInputShape->height());
    shape.AddDim((tensorflow::int64)tensorflowDetection2DImgInputShape->width());
    shape.AddDim(3);

    std::cout<<std::endl;
    std::cout<<std::endl;
    std::cout<<"Taking frames..."<<std::endl;
    tensorflowDetection2DImgInputI=tensorflowDetection2DImgInput;

    while (true) {

	// Clear Bottle
	bottle.clear();

        // Recieve image
        yarp::sig::ImageOf<yarp::sig::PixelRgb> *tensorflowDetection2DImg = tensorflowDetection2DImgInputI->read();
        cv::Mat inImg = cv::cvarrToMat((IplImage *)tensorflowDetection2DImg->getIplImage());
        frame=inImg;
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        // Count frames
        std::cout << "Frame: " << iFrame << std::endl;
        if (nFrames % (iFrame + 1) == 0) {
            time(&end);
            fps = 1. * nFrames / difftime(end, start);
            time(&start);
        }
        iFrame++;

        // Mat -> Tensor and check Tensor
        tensor = tensorflow::Tensor(tensorflow::DT_FLOAT, shape);
        tensorflow::Status readTensorStatus = readTensorFromMat(frame, tensor);
        if (!readTensorStatus.ok()) {;
            std::cout<<std::endl;
            std::cout<<std::endl;
            std::cout<<"Mat OpenCV -> Tensor : FAIL"<<std::endl;
            return -1;
        }

        // Execute graph
        outputs.clear();
        tensorflow::Status runStatus = session->Run({{inputLayer, tensor}}, outputLayer, {}, &outputs);
        if (!runStatus.ok()) {
            std::cout<<std::endl;
            std::cout<<std::endl;
            std::cout<<"Running model status: FAIL"<<std::endl;
            return -1;
        }

        // Extract results
        tensorflow::TTypes<float>::Flat scores = outputs[1].flat<float>();
        tensorflow::TTypes<float>::Flat classes = outputs[2].flat<float>();
        tensorflow::TTypes<float>::Flat numDetections = outputs[3].flat<float>();
        tensorflow::TTypes<float, 3>::Tensor boxes = outputs[0].flat_outer_dims<float,3>();

        // Print results
        std::vector<size_t> goodIdxs = filterBoxes(scores, boxes, thresholdIOU, thresholdScore);
        for (size_t i = 0; i < goodIdxs.size(); i++)
        {
        std::cout<<std::endl;
        std::cout<<std::endl;
        std::cout<<"Detection: "<<labelsMap[classes(goodIdxs.at(i))]<< " -> Score: "<<scores(goodIdxs.at(i))<<std::endl;

        // Get score detection
        double score_detection=scores(goodIdxs.at(i));
        std::string class_name=std::string(labelsMap[classes(goodIdxs.at(i))]);

        std::cout<<std::endl;
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        // Draw detection and get coordinates
        std::string coordXY;
        coordXY=drawBoundingBoxesOnImage(frame, scores, classes, boxes, labelsMap, goodIdxs);
        cv::putText(frame, std::to_string(fps).substr(0, 5), cv::Point(0, frame.rows), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255));

        // Prepare data bottle
        bottle.addString(" Detection number: ");
        bottle.addInt(goodIdxs.size());
        bottle.addString(" Detection: ");
        bottle.addString(class_name);
        bottle.addString(" Score: ");
        bottle.addDouble(score_detection);
        bottle.addString(" Coordinates: ");
        bottle.addString(coordXY);

        // Print coordinates
        std::cout<<"Coordinates: "<<coordXY<<std::endl;
        std::cout<<std::endl;
    }

    // Publish results
    yarp::sig::ImageOf<yarp::sig::PixelBgr> imgOutput;
    imgOutput.setExternal(frame.data,frame.size[1],frame.size[0]);
    tensorflowDetection2DImgOutput.write(imgOutput);
    tensorflowDetection2DDataOutput.write(bottle);

    // Show image directly by openCV
    /*cv::imshow("Video source: Processed", frame);
    cv::waitKey(5);*/

}
    //cv::destroyAllWindows();

    return 0;
}
