#define _GNU_SOURCE
#include <Python.h>
#include <stdio.h>
#include <stdbool.h>
#define XPLM200
#define XPLM210
#include <XPLM/XPLMDefs.h>
#include <XPLM/XPLMPlugin.h>
#include "utils.h"

PyObject *XPLMGetMyIDFun(PyObject *self, PyObject *args)
{
  (void) self;
  (void) args;
  return PyLong_FromLong(XPLMGetMyID());
}

PyObject *XPLMCountPluginsFun(PyObject *self, PyObject *args)
{
  (void) self;
  (void) args;
  return PyLong_FromLong(XPLMCountPlugins());
}

PyObject *XPLMGetNthPluginFun(PyObject *self, PyObject *args)
{
  (void) self;
  int inIndex;
  if(!PyArg_ParseTuple(args, "i", &inIndex)){
    return NULL;
  }
  return PyLong_FromLong(XPLMGetNthPlugin(inIndex));
}

PyObject *XPLMFindPluginByPathFun(PyObject *self, PyObject *args)
{
  (void) self;
  const char *inPath;
  if(!PyArg_ParseTuple(args, "s", &inPath)){
    return NULL;
  }
  return PyLong_FromLong(XPLMFindPluginByPath(inPath));
}

PyObject *XPLMFindPluginBySignatureFun(PyObject *self, PyObject *args)
{
  (void) self;
  const char *inSignature;
  if(!PyArg_ParseTuple(args, "s", &inSignature)){
    return NULL;
  }
  return PyLong_FromLong(XPLMFindPluginBySignature(inSignature));
}

PyObject *XPLMGetPluginInfoFun(PyObject *self, PyObject *args)
{
  (void) self;
  int inPlugin;
  PyObject *outNameObj;
  PyObject *outFilePathObj;
  PyObject *outSignatureObj;
  PyObject *outDescriptionObj;
  if(!PyArg_ParseTuple(args, "iOOOO", &inPlugin, &outNameObj, &outFilePathObj, &outSignatureObj, &outDescriptionObj)){
    return NULL;
  }
  char outName[512];
  char outFilePath[512];
  char outSignature[512];
  char outDescription[512];
  XPLMGetPluginInfo(inPlugin, outName, outFilePath, outSignature, outDescription);
  if(outNameObj != Py_None){
    PyList_Append(outNameObj, PyUnicode_DecodeUTF8(outName, strlen(outName), NULL));
  }
  if(outFilePathObj != Py_None){
    PyList_Append(outFilePathObj, PyUnicode_DecodeUTF8(outFilePath, strlen(outFilePath), NULL));
  }
  if(outSignatureObj != Py_None){
    PyList_Append(outSignatureObj, PyUnicode_DecodeUTF8(outSignature, strlen(outSignature), NULL));
  }
  if(outDescriptionObj != Py_None){
    PyList_Append(outDescriptionObj, PyUnicode_DecodeUTF8(outDescription, strlen(outDescription), NULL));
  }
  Py_RETURN_NONE;
}

PyObject *XPLMIsPluginEnabledFun(PyObject *self, PyObject *args)
{
  (void) self;
  int inPluginID;
  if(!PyArg_ParseTuple(args, "i", &inPluginID)){
    return NULL;
  }
  return PyLong_FromLong(XPLMIsPluginEnabled(inPluginID));
}

PyObject *XPLMEnablePluginFun(PyObject *self, PyObject *args)
{
  (void) self;
  int inPluginID;
  if(!PyArg_ParseTuple(args, "i", &inPluginID)){
    return NULL;
  }
  return PyLong_FromLong(XPLMEnablePlugin(inPluginID));
}

PyObject *XPLMDisablePluginFun(PyObject *self, PyObject *args)
{
  (void) self;
  int inPluginID;
  if(!PyArg_ParseTuple(args, "i", &inPluginID)){
    return NULL;
  }
  XPLMDisablePlugin(inPluginID);
  Py_RETURN_NONE;
}

PyObject *XPLMReloadPluginsFun(PyObject *self, PyObject *args)
{
  (void) self;
  (void) args;
  XPLMReloadPlugins();
  Py_RETURN_NONE;
}

PyObject *XPLMSendMessageToPluginFun(PyObject *self, PyObject *args)
{
  (void) self;
  int inPluginID;
  int inMessage;
  PyObject *inParam;
  if(!PyArg_ParseTuple(args, "iiO", &inPluginID, &inMessage, &inParam)){
    return NULL;
  }
  XPLMSendMessageToPlugin(inPluginID, inMessage, inParam);
  Py_RETURN_NONE;
}

PyObject *XPLMHasFeatureFun(PyObject *self, PyObject *args)
{
  (void) self;
  const char *inFeature;
  if(!PyArg_ParseTuple(args, "s", &inFeature)){
    return NULL;
  }
  return PyLong_FromLong(XPLMHasFeature(inFeature));
}

PyObject *XPLMIsFeatureEnabledFun(PyObject *self, PyObject *args)
{
  (void) self;
  const char *inFeature;
  if(!PyArg_ParseTuple(args, "s", &inFeature)){
    return NULL;
  }
  return PyLong_FromLong(XPLMIsFeatureEnabled(inFeature));
}

PyObject *XPLMEnableFeatureFun(PyObject *self, PyObject *args)
{
  (void) self;
  const char *inFeature;
  int inEnable;
  if(!PyArg_ParseTuple(args, "si", &inFeature, &inEnable)){
    return NULL;
  }
  XPLMEnableFeature(inFeature, inEnable);
  Py_RETURN_NONE;
}

static PyObject *feDict;
static intptr_t feCntr;

static void featureEnumerator(const char *inFeature, void *inRef)
{
  PyObject *key = PyLong_FromVoidPtr(inRef);
  PyObject *callbackInfo = PyDict_GetItem(feDict, key);
  Py_XDECREF(key);
  if(callbackInfo == NULL){
    printf("Unknown feature enumeration callback requested! (inFeature = '%s' inRef = %p)\n", inFeature, inRef);
    return;
  }
  PyObject *res = PyObject_CallFunction(PySequence_GetItem(callbackInfo, 1), "(sO)",
                                        inFeature, PySequence_GetItem(callbackInfo, 2));
  PyObject *err = PyErr_Occurred();
  if(err){
    printf("Error occured during the feature enumeration callback(inFeature = '%s' inRef = %p):\n", inFeature, inRef);
    PyErr_Print();
  }
  Py_XDECREF(res);
}


PyObject *XPLMEnumerateFeaturesFun(PyObject *self, PyObject *args)
{
  (void) self;
  PyObject *fun;
  PyObject *ref;
  PyObject *pluginSelf;
  if(!PyArg_ParseTuple(args, "OOO", &pluginSelf, &fun, &ref)){
    return NULL;
  }
  PyObject *key = PyLong_FromLong(feCntr++);
  void *inRef = PyLong_AsVoidPtr(key);
  PyDict_SetItem(feDict, key, args);

  XPLMEnumerateFeatures(featureEnumerator, inRef);
  Py_RETURN_NONE;
}


static PyMethodDef XPLMPluginMethods[] = {
  {"XPLMGetMyID", XPLMGetMyIDFun, METH_VARARGS, ""},
  {"XPLMCountPlugins", XPLMCountPluginsFun, METH_VARARGS, ""},
  {"XPLMGetNthPlugin", XPLMGetNthPluginFun, METH_VARARGS, ""},
  {"XPLMFindPluginByPath", XPLMFindPluginByPathFun, METH_VARARGS, ""},
  {"XPLMFindPluginBySignature", XPLMFindPluginBySignatureFun, METH_VARARGS, ""},
  {"XPLMGetPluginInfo", XPLMGetPluginInfoFun, METH_VARARGS, ""},
  {"XPLMIsPluginEnabled", XPLMIsPluginEnabledFun, METH_VARARGS, ""},
  {"XPLMEnablePlugin", XPLMEnablePluginFun, METH_VARARGS, ""},
  {"XPLMDisablePlugin", XPLMDisablePluginFun, METH_VARARGS, ""},
  {"XPLMReloadPlugins", XPLMReloadPluginsFun, METH_VARARGS, ""},
  {"XPLMSendMessageToPlugin", XPLMSendMessageToPluginFun, METH_VARARGS, ""},
  {"XPLMHasFeature", XPLMHasFeatureFun, METH_VARARGS, ""},
  {"XPLMIsFeatureEnabled", XPLMIsFeatureEnabledFun, METH_VARARGS, ""},
  {"XPLMEnableFeature", XPLMEnableFeatureFun, METH_VARARGS, ""},
  {"XPLMEnumerateFeatures", XPLMEnumerateFeaturesFun, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL}
};

static struct PyModuleDef XPLMPluginModule = {
  PyModuleDef_HEAD_INIT,
  "XPLMPlugin",
  NULL,
  -1,
  XPLMPluginMethods,
  NULL,
  NULL,
  NULL,
  NULL
};

PyMODINIT_FUNC
PyInit_XPLMPlugin(void)
{
  if(!(feDict = PyDict_New())){
    return NULL;
  }
  PyObject *mod = PyModule_Create(&XPLMPluginModule);
  if(mod){
    PyModule_AddIntConstant(mod, "XPLM_MSG_PLANE_CRASHED", XPLM_MSG_PLANE_CRASHED);
    PyModule_AddIntConstant(mod, "XPLM_MSG_PLANE_LOADED", XPLM_MSG_PLANE_LOADED);
    PyModule_AddIntConstant(mod, "XPLM_MSG_AIRPORT_LOADED", XPLM_MSG_AIRPORT_LOADED);
    PyModule_AddIntConstant(mod, "XPLM_MSG_SCENERY_LOADED", XPLM_MSG_SCENERY_LOADED);
    PyModule_AddIntConstant(mod, "XPLM_MSG_AIRPLANE_COUNT_CHANGED", XPLM_MSG_AIRPLANE_COUNT_CHANGED);
    PyModule_AddIntConstant(mod, "XPLM_MSG_PLANE_UNLOADED", XPLM_MSG_PLANE_UNLOADED);
    PyModule_AddIntConstant(mod, "XPLM_MSG_WILL_WRITE_PREFS", XPLM_MSG_WILL_WRITE_PREFS);
    PyModule_AddIntConstant(mod, "XPLM_MSG_LIVERY_LOADED", XPLM_MSG_LIVERY_LOADED);
  }

  return mod;
}

