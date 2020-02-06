#ifndef MAINDETECTOR_HPP
#define MAINDETECTOR_HPP

// Libraries
#include <fstream>
#include <iostream>
#include <time.h>
#include <utility>
#include <vector>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/ConnectionReader.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Port.h>
#include <yarp/os/PortReader.h>

#include <cv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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

#include "TensorflowDetector.hpp"

class maindetector
{
public:
int runDetection(std::string labels, std::string graph, yarp::os::Port tensorflowDetection2DImgOutput, yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *tensorflowDetection2DImgInput, yarp::os::Port tensorflowDetection2DDataOutput);

private:
yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *tensorflowDetection2DImgInputI;
};
#endif //MAINDETECTOR_HPP
