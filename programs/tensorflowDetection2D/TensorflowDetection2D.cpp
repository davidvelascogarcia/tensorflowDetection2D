/*
 * ************************************************************
 *      Program: Tensorflow Detection 2D Module
 *      Type: tensorflowdetection2d.cpp
 *      Author: David Velasco Garcia @davidvelascogarcia
 * ************************************************************
 */

// Libraries

#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

#include <yarp/os/BufferedPort.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Port.h>

#include "MainDetector.hpp"
#include "TensorflowDetection2D.hpp"
#include "TensorflowDetector.hpp"


tensorflowDetection2D::tensorflowDetection2D()
{
}

void tensorflowDetection2D::initDetector(std::string labels, std::string graph)
{
  // Get paths
  modelPath=graph;
  labelPath=labels;
}

int tensorflowDetection2D::runDetector(yarp::os::Port tensorflowDetection2DImgOutput, yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *tensorflowDetection2DImgInput, yarp::os::Port tensorflowDetection2DDataOutput){

  // Connect image input ports objects
  tensorflowDetection2DImgInputI=tensorflowDetection2DImgInput;

  // Init main detector
  maindetector detectorModule;
  detectorModule.runDetection(labelPath, modelPath, tensorflowDetection2DImgOutput, tensorflowDetection2DImgInputI, tensorflowDetection2DDataOutput);
  return 0;
}
