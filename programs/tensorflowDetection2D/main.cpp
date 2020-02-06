/*
 * ************************************************************
 *      Program: Tensorflow Detection 2D Module
 *      Type: main.cpp
 *      Author: David Velasco Garcia @davidvelascogarcia
 * ************************************************************
 */

/*
  *
  * | INPUT PORT                      | CONTENT                                                 |
  * |---------------------------------|---------------------------------------------------------|
  * | /tensorflowDetection2D/img:i    | Input image                                             |
  *
  *
  * | OUTPUT PORT                     | CONTENT                                                 |
  * |---------------------------------|---------------------------------------------------------|
  * | /tensorflowDetection2D/img:o    | Output image with object detection                      |
  * | /tensorflowDetection2D/data:o   | Output result, object, score, coordinates and number    |
  *
  */

// Libraries

#include <yarp/os/BufferedPort.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Port.h>

#include <cstdlib>
#include <iostream>

#include "TensorflowDetection2D.hpp"


// Variable

int yarpServerCheck=0;
std::string labels;
std::string graph;

int main(int argc, char ** argv){

  // Welcome message
  std::cout<<"**************************************************************************"<<std::endl;
  std::cout<<"**************************************************************************"<<std::endl;
  std::cout<<"                     Program: Tensorflow Detector 2D                      "<<std::endl;
  std::cout<<"                     Author: David Velasco García                         "<<std::endl;
  std::cout<<"                             @davidvelascogarcia                          "<<std::endl;
  std::cout<<"**************************************************************************"<<std::endl;
  std::cout<<"**************************************************************************"<<std::endl;

  std::cout<<std::endl;
  std::cout<<"Starting system..."<<std::endl;
  std::cout<<std::endl;
  std::cout<<"Initializing ..."<<std::endl;
  std::cout<<std::endl;
  std::cout<<"Loading TensorFlow 2D detector module..."<<std::endl;

  // Init YARP network
  yarp::os::Network yarp;

  std::cout<<std::endl;
  std::cout<<"Initializing YARP network..."<<std::endl;

  // Opening image input port
  std::cout<<"Opening image input port with the name /tensorflowDetection2D/img:i."<<std::endl;
  yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > tensorflowDetection2DImgInput;
  tensorflowDetection2DImgInput.open("/tensorflowDetection2D/img:i");

  // Opening Sender Port
  yarp::os::Port tensorflowDetection2DImgOutput;
  std::cout<<std::endl;
  std::cout<<"Opening sender ports..."<<std::endl;
  std::cout<<std::endl;
  std::cout<<"Opening post-processed port with the name /tensorflowDetection2D/img:o."<<std::endl;
  tensorflowDetection2DImgOutput.open("/tensorflowDetection2D/img:o");

  // Opening results data ports
  yarp::os::Port tensorflowDetection2DDataOutput;
  std::cout<<std::endl;
  std::cout<<"Opening results port with the name /tensorflowDetection2D/results."<<std::endl;
  tensorflowDetection2DDataOutput.open("/tensorflowDetection2D/data:o");

  // Check YARP server
  std::cout<<std::endl;
  std::cout<<"Checking yarpserver status..."<<std::endl;
  while(yarpServerCheck==0){

  if (!yarp::os::Network::checkNetwork())
  {

      std::cout<<std::endl;
      std::cout<<"YARPSERVER status: FAIL"<<std::endl;
      std::cout<<"Please, start yarpserver or connect to yarpserver already running..."<<std::endl;


  }else{
      std::cout<<std::endl;
      std::cout<<"YARPSERVER status: OK"<<std::endl;
      std::cout<<std::endl;
      yarpServerCheck=1;
  }
  }

  std::cout<<"Locating pre-trained model and labels map..."<<std::endl;

  // YARP ResourceFinder method
  /*yarp::os::ResourceFinder rf;
  rf.setVerbose(true);
  rf.setDefaultContext("tensorflowDetection2D");
  rf.setDefaultConfigFile("tensorflowDetection2D.ini");
  rf.configure(argc, argv);
  std::string pathToModel = rf.check("pathToModel", yarp::os::Value(""), "documentation").asString();
  labels = rf.findFileByName("labels_map.pbtxt");
  graph = rf.findFileByName("frozen_inference_graph.pb");*/

  labels="./../models/labels_map.pbtxt";
  graph="./../models/frozen_inference_graph.pb";

  // Object detector
  tensorflowDetection2D tensorflowDetector;

  // Init detector
  tensorflowDetector.initDetector(labels, graph);
  tensorflowDetector.runDetector(tensorflowDetection2DImgOutput, &tensorflowDetection2DImgInput, tensorflowDetection2DDataOutput);

  std::cout<<std::endl;
  std::cout<<"Closing Tensorflow 2D detector module..."<<std::endl;

  return 0;
}
