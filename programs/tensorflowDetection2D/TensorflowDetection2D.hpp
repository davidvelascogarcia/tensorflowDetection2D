#ifndef TENSORFLOWDETECTION2D_HPP
#define TENSORFLOWDETECTION2D_HPP

/*
 * ************************************************************
 *      Program: Tensorflow Detection 2D Module
 *      Type: tensorflowdetection2d.hpp
 *      Author: David Velasco Garcia @davidvelascogarcia
 * ************************************************************
 */

// Libraries

#include <iostream>
#include <string>

#include <yarp/os/BufferedPort.h>
#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#include <yarp/os/RFModule.h>
#include <yarp/sig/Image.h>

class tensorflowDetection2D
{
public:
    tensorflowDetection2D();
    void initDetector( std::string labels, std::string graph);
    int runDetector(yarp::os::Port tensorflowDetection2DImgOutput, yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *tensorflowDetection2DImgInput, yarp::os::Port tensorflowDetection2DDataOutput);

private:
    std::string modelPath;
    std::string labelPath;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *tensorflowDetection2DImgInputI;

};

#endif // TENSORFLOWDETECTION2D_HPP
