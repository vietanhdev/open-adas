#ifndef LANE_DETECTOR_H
#define LANE_DETECTOR_H

#include <Python.h>
#include <opencv2/opencv.hpp>
#include <pyboostcvconverter/pyboostcvconverter.hpp>

class LaneDetector {
   private:
   public:
    LaneDetector() {

        setenv("PYTHONPATH", "lanenet_trt_python", 1);

        Py_Initialize();
        if( !Py_IsInitialized() ){
            printf("Initialize failed\n");
            exit(1);
        }
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('./')");

        PyObject *pName,*pModule,*pDict,*pFunc;
        
        // PyString_FromString for python2.x
        // PyUnicode_DecodeFSDefault for python3.x
        pName = PyUnicode_DecodeFSDefault("vgg_model");
        
        pModule = PyImport_Import(pName);
        if ( !pModule ){
            printf("Can't find Module\n");
            PyErr_Print();
            exit(1);
        }
        pDict = PyModule_GetDict(pModule);
        if ( !pDict ){
            exit(1);
        }
        pFunc = PyDict_GetItemString(pDict, "predict");
        if ( !pFunc || !PyCallable_Check(pFunc) ){
            printf("can't find function [predict]\n");
            exit(1);
        }


        printf(" ===========> START CALL PYTHON SCRIPT <===========\n");

        printf(" ===========> 1st CALL <===========\n");
        PyObject_CallObject(pFunc,NULL);
        printf(" ===========> 2nd CALL <===========\n");
        PyObject_CallObject(pFunc,NULL);
        printf(" ===========> 3rd CALL <===========\n");
        PyObject_CallObject(pFunc,NULL);
        printf(" ===========> 4th CALL <===========\n");
        PyObject_CallObject(pFunc,NULL);

        printf(" ===========> CALLING FINISHED <===========\n");

        Py_DECREF(pName);
        Py_DECREF(pModule);

        // close Python
        Py_Finalize();
    }

    cv::Mat inference(const cv::Mat& img) {
        // PyObject* pInt;
        // Py_Initialize();
        // PyRun_SimpleString("from .lanenet_trt_python import
        // lanenet_tensor_trt");

        // Py_Finalize();
    }
};

#endif