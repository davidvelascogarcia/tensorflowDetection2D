
// Librerias

#include <fstream>
#include <utility>
#include <vector>
#include <iostream>
#include <time.h>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Time.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cv.hpp>
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
#include "MainDetector.hpp"
//#include "tensorflowdetection2d.hpp" Da error de ambiguedad int64 int_64

using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;

// Espacios de nombres

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::draw;
using namespace std;
using namespace cv;



int maindetector::detect(string labels, string graph, string video_source, Port puerto_pre, Port puerto_post) {


    system("clear");
    cout<<endl;
    cout<<endl;

    // Path
    string ROOTDIR = "../";
    string LABELS = labels;
    string GRAPH = graph;
    string source_video=video_source;

    // Set  nombres nodos entrada y salida
    string inputLayer = "image_tensor:0";
    vector<string> outputLayer = {"detection_boxes:0", "detection_scores:0", "detection_classes:0", "num_detections:0"};

    // Cargar modelo del .pb
    std::unique_ptr<tensorflow::Session> session;
    string graphPath = tensorflow::io::JoinPath(ROOTDIR, GRAPH);
    cout<<"Se cargará el graph:" << graphPath<<"."<<endl;
    cout<<"Cargando graph..."<<endl;
    Time::delay(1);
    //LOG(INFO) << "Cargando graph:" << graphPath<<" ...";

    Status loadGraphStatus = loadGraph(graphPath, &session);
    if (!loadGraphStatus.ok()) {
        system("clear");
        cout<<endl;
        cout<<endl;
        cout<<"Fallo al cargar el graph "<<graphPath<<"."<<endl;
        //LOG(ERROR) << "Carga del graph: FAIL" << loadGraphStatus;
        Time::delay(1);
        return -1;
    } else
        system("clear");
        cout<<endl;
        cout<<endl;
        cout<<"Graph "<<graphPath<<" cargado correctamente."<<endl;
        //LOG(INFO) << "Carga del graph: OK" << endl;
        Time::delay(1);


    // Cargar etiquetas
    std::map<int, std::string> labelsMap = std::map<int,std::string>();
    system("clear");
    cout<<endl;
    cout<<endl;
    cout<<"Se cargarán las etiquetas "<<LABELS<<"."<<endl;
    Time::delay(1);
    Status readLabelsMapStatus = readLabelsMapFile(tensorflow::io::JoinPath(ROOTDIR, LABELS), labelsMap);
    if (!readLabelsMapStatus.ok()) {
        //LOG(ERROR) << "readLabelsMapFile(): ERROR" << loadGraphStatus;
        cout<<"Fallo al cargar las etiquetas "<<LABELS<<"."<<endl;
        //LOG(INFO) << "Carga del graph: OK" << endl;
        Time::delay(1);
        return -1;
    } else
        //LOG(INFO) << "readLabelsMapFile(): labels map loaded with " << labelsMap.size() << " label(s)" << endl;
        cout<<"Etiquetas "<<LABELS<<" cargadas correctamente."<<endl;
        cout<<labelsMap.size()<<" equiquetas han sido cargadas."<<endl;
        Time::delay(1);

    system("clear");
    cout<<endl;
    cout<<endl;
    cout<<"Se capturarán frames de la fuente de video."<<endl;
    Time::delay(1);
    Mat frame;
    Tensor tensor;
    std::vector<Tensor> outputs;
    double thresholdScore = 0.5;
    double thresholdIOU = 0.8;

    // Cuenta FPS
    int nFrames = 25;
    int iFrame = 0;
    double fps = 0.;
    time_t start, end;
    time(&start);

    // Adquiere imagen de la fuente de video
    VideoCapture cap(source_video);

    tensorflow::TensorShape shape = tensorflow::TensorShape();
    shape.AddDim(1);
    shape.AddDim((int64)cap.get(CAP_PROP_FRAME_HEIGHT));
    shape.AddDim((int64)cap.get(CAP_PROP_FRAME_WIDTH));
    shape.AddDim(3);
    system("clear");
    cout<<endl;
    cout<<endl;
    cout<<"Capturando..."<<endl;
    Time::delay(1);
    //tensorflowDetection2D yarp_sender;
    while (cap.isOpened()) {
        cap >> frame;

        // Enviar imagen preprocesada por yarp
        //yarp_sender.send_post(frame, puerto_pre);
        // A mano
        ImageOf<PixelBgr> B;
        B.setExternal(frame.data,frame.size[1],frame.size[0]);
        puerto_pre.write(B);

        cvtColor(frame, frame, COLOR_BGR2RGB);
        cout << "Frame: " << iFrame << endl;

        if (nFrames % (iFrame + 1) == 0) {
            time(&end);
            fps = 1. * nFrames / difftime(end, start);
            time(&start);
        }
        iFrame++;

        // Pasar Mat a Tensor
        tensor = Tensor(tensorflow::DT_FLOAT, shape);
        Status readTensorStatus = readTensorFromMat(frame, tensor);
        if (!readTensorStatus.ok()) {
            //LOG(ERROR) << "Mat->Tensor conversion failed: " << readTensorStatus;
            system("clear");
            cout<<endl;
            cout<<endl;
            cout<<"Conversión Mat OpenCV -> Tensor : FAIL"<<endl;
            Time::delay(1);
            return -1;
        }

        // Ejecutar graph
        outputs.clear();
        Status runStatus = session->Run({{inputLayer, tensor}}, outputLayer, {}, &outputs);
        if (!runStatus.ok()) {
            //LOG(ERROR) << "Running model failed: " << runStatus;
            system("clear");
            cout<<endl;
            cout<<endl;
            cout<<"Ejecución del modelo: FAIL"<<endl;
            Time::delay(1);
            return -1;
        }

        // Extraer resultados del vector
        tensorflow::TTypes<float>::Flat scores = outputs[1].flat<float>();
        tensorflow::TTypes<float>::Flat classes = outputs[2].flat<float>();
        tensorflow::TTypes<float>::Flat numDetections = outputs[3].flat<float>();
        tensorflow::TTypes<float, 3>::Tensor boxes = outputs[0].flat_outer_dims<float,3>();

        vector<size_t> goodIdxs = filterBoxes(scores, boxes, thresholdIOU, thresholdScore);
        for (size_t i = 0; i < goodIdxs.size(); i++){
        cout<<endl;
        cout<<endl;
        cout<<"Detección: "<<labelsMap[classes(goodIdxs.at(i))]<< " -> Precisón: "<<scores(goodIdxs.at(i))<<endl;

        /* LOG(INFO) << "score:" << scores(goodIdxs.at(i)) << ",class:" << labelsMap[classes(goodIdxs.at(i))]
                      << " (" << classes(goodIdxs.at(i)) << "), box:" << "," << boxes(0, goodIdxs.at(i), 0) << ","
                      << boxes(0, goodIdxs.at(i), 1) << "," << boxes(0, goodIdxs.at(i), 2) << ","
                      << boxes(0, goodIdxs.at(i), 3);*/


        cout<<endl;

        cvtColor(frame, frame, COLOR_BGR2RGB);
        drawBoundingBoxesOnImage(frame, scores, classes, boxes, labelsMap, goodIdxs);

        putText(frame, to_string(fps).substr(0, 5), Point(0, frame.rows), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255));
        //yarp_sender.send_post(frame, puerto_post);
        // A mano
        ImageOf<PixelBgr> C;
        C.setExternal(frame.data,frame.size[1],frame.size[0]);
        puerto_post.write(C);
        imshow("Fuente de video: Procesada", frame);
        waitKey(5);
    }}
    destroyAllWindows();

    return 0;
}
