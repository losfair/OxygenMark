#include <node.h>
#include <string>
#include <map>
#include "Parser.h"
#include "Tokenizer.h"

using namespace std;
using namespace v8;

extern "C" OxygenMark::Document * loadDocumentFromSource(const char *src_c);
extern "C" OxygenMark::Document * loadDocument(const char *filename);
extern "C" void setDocumentParam(OxygenMark::Document *doc, const char *key, const char *value);
extern "C" void clearDocumentParams(OxygenMark::Document *doc);
extern "C" char * renderToHtml(OxygenMark::Document *doc, bool isWholePage);
extern "C" char * generateJavascriptRenderer(OxygenMark::Document *doc, bool isWholePage);
extern "C" void destroyDocument(OxygenMark::Document *doc);

map<int, OxygenMark::Document *> loadedDocuments;
int currentDocumentId = 1;

static void onLoadDocumentFromSource(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    if(args.Length() != 1) return;

    OxygenMark::Document *doc = loadDocumentFromSource(*String::Utf8Value(args[0] -> ToString()));
    if(doc == NULL) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Unable to load document"));
        return;
    }

    loadedDocuments[currentDocumentId] = doc;
    Local<Number> docId = Number::New(isolate, currentDocumentId);
    currentDocumentId++;

    args.GetReturnValue().Set(docId);
}

static void onLoadDocumentFromBinaryFile(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    if(args.Length() != 1) return;

    OxygenMark::Document *doc = loadDocument(*String::Utf8Value(args[0] -> ToString()));
    if(doc == NULL) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Unable to load document"));
        return;
    }

    loadedDocuments[currentDocumentId] = doc;
    Local<Number> docId = Number::New(isolate, currentDocumentId);
    currentDocumentId++;

    args.GetReturnValue().Set(docId);
}

static void onSetDocumentParam(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    if(args.Length() != 3) return;

    auto itr = loadedDocuments.find((int) args[0] -> NumberValue());
    if(itr == loadedDocuments.end()) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Unable to find target document"));
        return;
    }

    OxygenMark::Document *doc = itr -> second;
    setDocumentParam(doc, *String::Utf8Value(args[1] -> ToString()), *String::Utf8Value(args[2] -> ToString()));
}

static void onClearDocumentParams(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    if(args.Length() != 1) return;

    auto itr = loadedDocuments.find((int) args[0] -> NumberValue());
    if(itr == loadedDocuments.end()) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Unable to find target document"));
        return;
    }

    OxygenMark::Document *doc = itr -> second;
    clearDocumentParams(doc);
}

static void onRenderToHtml(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    if(args.Length() < 1) return;

    auto itr = loadedDocuments.find((int) args[0] -> NumberValue());
    if(itr == loadedDocuments.end()) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Document not found"));
        return;
    }

    OxygenMark::Document *doc = itr -> second;
    
    bool isWholePage = true;

    if(args.Length() >= 2 && args[1] -> NumberValue() == 0) isWholePage = false;

    char *result = renderToHtml(doc, isWholePage);

    if(result == NULL) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Unable to render document"));
        return;
    }

    Local<String> ret = String::NewFromUtf8(isolate, result);
    delete[] result;

    args.GetReturnValue().Set(ret);
}

static void onGenerateRenderer(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    if(args.Length() < 1) return;

    auto itr = loadedDocuments.find((int) args[0] -> NumberValue());
    if(itr == loadedDocuments.end()) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Document not found"));
        return;
    }

    OxygenMark::Document *doc = itr -> second;
    
    bool isWholePage = true;

    if(args.Length() >= 2 && args[1] -> NumberValue() == 0) isWholePage = false;

    char *result = generateJavascriptRenderer(doc, isWholePage);

    if(result == NULL) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Unable to generate renderer"));
        return;
    }

    Local<String> ret = String::NewFromUtf8(isolate, result);
    delete[] result;

    args.GetReturnValue().Set(ret);
}

static void onDestroyDocument(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    if(args.Length() != 1) return;

    auto itr = loadedDocuments.find((int) args[0] -> NumberValue());
    if(itr == loadedDocuments.end()) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Document not found"));
        return;
    }

    OxygenMark::Document *doc = itr -> second;
    loadedDocuments.erase(itr);

    delete doc;
}

static void moduleInit(Local<Object> exports) {
    //Isolate *isolate = Isolate::GetCurrent();

    NODE_SET_METHOD(exports, "loadDocumentFromSource", onLoadDocumentFromSource);
    NODE_SET_METHOD(exports, "loadDocumentFromBinaryFile", onLoadDocumentFromBinaryFile);
    NODE_SET_METHOD(exports, "setDocumentParam", onSetDocumentParam);
    NODE_SET_METHOD(exports, "clearDocumentParams", onClearDocumentParams);
    NODE_SET_METHOD(exports, "renderToHtml", onRenderToHtml);
    NODE_SET_METHOD(exports, "generateRenderer", onGenerateRenderer);
    NODE_SET_METHOD(exports, "destroyDocument", onDestroyDocument);
}

NODE_MODULE(OxygenMark, moduleInit)
