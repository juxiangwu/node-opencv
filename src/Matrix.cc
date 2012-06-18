#include "Contours.h"
#include "Matrix.h"
#include "OpenCV.h"

v8::Persistent<FunctionTemplate> Matrix::constructor;

cv::Scalar setColor(Local<Object> objColor);

//

void
Matrix::Init(Handle<Object> target) {
	HandleScope scope;

	//Class
	v8::Local<v8::FunctionTemplate> m = v8::FunctionTemplate::New(New);
	m->SetClassName(v8::String::NewSymbol("Matrix"));

	// Constructor
	constructor = Persistent<FunctionTemplate>::New(m);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Matrix"));

	// Prototype
	Local<ObjectTemplate> proto = constructor->PrototypeTemplate();


	NODE_SET_PROTOTYPE_METHOD(constructor, "row", Row);
	NODE_SET_PROTOTYPE_METHOD(constructor, "col", Col);


	NODE_SET_PROTOTYPE_METHOD(constructor, "empty", Empty);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", Get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "set", Set);
	NODE_SET_PROTOTYPE_METHOD(constructor, "width", Width);
	NODE_SET_PROTOTYPE_METHOD(constructor, "height", Height);
	NODE_SET_PROTOTYPE_METHOD(constructor, "channels", Channels);
	NODE_SET_PROTOTYPE_METHOD(constructor, "size", Size);
	NODE_SET_PROTOTYPE_METHOD(constructor, "toBuffer", ToBuffer);
	NODE_SET_PROTOTYPE_METHOD(constructor, "ellipse", Ellipse);
	NODE_SET_PROTOTYPE_METHOD(constructor, "save", Save);


	NODE_SET_PROTOTYPE_METHOD(constructor, "convertGrayscale", ConvertGrayscale);
	NODE_SET_PROTOTYPE_METHOD(constructor, "convertHSVscale", ConvertHSVscale);
	NODE_SET_PROTOTYPE_METHOD(constructor, "copy", Copy);
	NODE_SET_PROTOTYPE_METHOD(constructor, "ptr", Ptr);
	NODE_SET_PROTOTYPE_METHOD(constructor, "addWeighted", AddWeighted);
	NODE_SET_PROTOTYPE_METHOD(constructor, "split", Split);
	NODE_SET_PROTOTYPE_METHOD(constructor, "bla", Bla);
	NODE_SET_PROTOTYPE_METHOD(constructor, "canny", Canny);
	NODE_SET_PROTOTYPE_METHOD(constructor, "dilate", Dilate);

	NODE_SET_PROTOTYPE_METHOD(constructor, "findContours", FindContours);
	NODE_SET_PROTOTYPE_METHOD(constructor, "drawContour", DrawContour);
	NODE_SET_PROTOTYPE_METHOD(constructor, "drawAllContours", DrawAllContours);

	NODE_SET_METHOD(constructor, "Eye", Eye);


	target->Set(String::NewSymbol("Matrix"), m->GetFunction());
};


Handle<Value>
Matrix::New(const Arguments &args) {
	HandleScope scope;

	if (args.This()->InternalFieldCount() == 0)
		return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Cannot instantiate without new")));

	Matrix *mat;

	if (args.Length() == 0){
		mat = new Matrix;
	} else if (args.Length() == 2 && args[0]->IsInt32() && args[1]->IsInt32()){
			mat = new Matrix(args[0]->IntegerValue(), args[1]->IntegerValue());
	}

	mat->Wrap(args.Holder());
	return scope.Close(args.Holder());
}

Matrix::Matrix(): ObjectWrap() {
	mat = cv::Mat();
}

Matrix::Matrix(int w, int h): ObjectWrap() {

	//TODO:Parametrizar esto
	mat = cv::Mat(h, w, CV_8UC3);

}


Handle<Value>
Matrix::Empty(const Arguments& args){
	SETUP_FUNCTION(Matrix)

	return scope.Close(Boolean::New(self->mat.empty()));
}

Handle<Value>
Matrix::Get(const Arguments& args){
	SETUP_FUNCTION(Matrix)

	int i = args[0]->IntegerValue();
	int j = args[1]->IntegerValue();

	return scope.Close(Number::New(self->mat.at<double>(i,j)));
}


Handle<Value> 
Matrix::Set(const Arguments& args){
	SETUP_FUNCTION(Matrix)

	int i = args[0]->IntegerValue();
	int j = args[1]->IntegerValue();
	double val = args[2]->NumberValue();

	if(args.Length() == 4) {
		self->mat.at<cv::Vec3b>(i,j)[args[3]->NumberValue()] = val;

	} else if(args.Length() == 3) {
		self->mat.at<cv::Vec3b>(i,j)[0] = val;
		self->mat.at<cv::Vec3b>(i,j)[1] = val;
		self->mat.at<cv::Vec3b>(i,j)[2] = val;

  } else {
		return ThrowException(Exception::TypeError(String::New("Invalid number of arguments")));
  }

	return scope.Close(Undefined());
}


Handle<Value> 
Matrix::Size(const Arguments& args){
	SETUP_FUNCTION(Matrix)

	v8::Local<v8::Array> arr = v8::Array::New(2);
	arr->Set(0, Number::New(self->mat.size().height));
	arr->Set(1, Number::New(self->mat.size().width));

	return scope.Close(arr);
}

Handle<Value> 
Matrix::Row(const Arguments& args){
	SETUP_FUNCTION(Matrix)

	int width = self->mat.size().width;
	int j = args[0]->IntegerValue();
	v8::Local<v8::Array> arr = v8::Array::New(width);

	for (int i=0; i<width; i++)
		arr->Set(i, Number::New(self->mat.at<double>(i, j)));

  return scope.Close(arr);
}

Handle<Value> 
Matrix::Col(const Arguments& args){
	SETUP_FUNCTION(Matrix)

	int width = self->mat.size().width;
	int j = args[0]->IntegerValue();
	v8::Local<v8::Array> arr = v8::Array::New(width);

	for (int i=0; i<width; i++)
		arr->Set(i, Number::New(self->mat.at<double>(j, i)));

	return scope.Close(arr);
}

Handle<Value> 
Matrix::Width(const Arguments& args){
	SETUP_FUNCTION(Matrix)

	return scope.Close(Number::New(self->mat.size().width));
}

Handle<Value> 
Matrix::Height(const Arguments& args){
	SETUP_FUNCTION(Matrix)

	return scope.Close(Number::New(self->mat.size().height));
}

Handle<Value> 
Matrix::Channels(const Arguments& args){
	 SETUP_FUNCTION(Matrix)

	return scope.Close(Number::New(self->mat.channels()));
}


Handle<Value>
Matrix::ToBuffer(const v8::Arguments& args){
	SETUP_FUNCTION(Matrix)

	std::vector<uchar> vec(0);
	std::vector<int> params(0);//CV_IMWRITE_JPEG_QUALITY 90

	cv::imencode(".jpg", self->mat, vec, params);

	node::Buffer *buf = node::Buffer::New(vec.size());
	uchar* data = (uchar*) Buffer::Data(buf);
	memcpy(data, &vec[0], vec.size());

	v8::Local<v8::Object> globalObj = v8::Context::GetCurrent()->Global();
	v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(v8::String::New("Buffer")));
	v8::Handle<v8::Value> constructorArgs[3] = {buf->handle_, v8::Integer::New(vec.size()), v8::Integer::New(0)};
	v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

	return scope.Close(actualBuffer);
}


Handle<Value> 
Matrix::Ellipse(const v8::Arguments& args){
	SETUP_FUNCTION(Matrix)

	int x = args[0]->Uint32Value();
	int y = args[1]->Uint32Value();
	int width = args[2]->Uint32Value();
	int height = args[3]->Uint32Value();    

	cv::ellipse(self->mat, cv::Point(x, y), cv::Size(width, height), 0, 0, 360, cv::Scalar( 255, 0, 255 ), 4, 8, 0);
	return scope.Close(v8::Null());
}


Handle<Value>
Matrix::Save(const v8::Arguments& args){
	HandleScope scope;

	if (!args[0]->IsString())
		return ThrowException(Exception::TypeError(String::New("filename required")));

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());
	String::AsciiValue filename(args[0]);
	int res = cv::imwrite(*filename, self->mat);
	return scope.Close(Number::New(res));
}


Handle<Value> 
Matrix::Eye(const v8::Arguments& args){
	HandleScope scope;

	int w = args[0]->Uint32Value();
	int h = args[1]->Uint32Value();

	Local<Object> im_h = Matrix::constructor->GetFunction()->NewInstance();
	Matrix *img = ObjectWrap::Unwrap<Matrix>(im_h);
	cv::Mat mat = cv::Mat::eye(w, h, CV_64FC1);

	img->mat = mat;
	return scope.Close(im_h);
}


Handle<Value>
Matrix::ConvertGrayscale(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());
	if(self->mat.channels() != 3)
		return ThrowException(String::New("Image is no 3-channel"));

	cv::Mat gray;

	cv::cvtColor(self->mat, gray, CV_BGR2GRAY);
	gray.copyTo(self->mat);


	return scope.Close(v8::Null());
}


Handle<Value>
Matrix::ConvertHSVscale(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());
	if(self->mat.channels() != 3)
		return ThrowException(String::New("Image is no 3-channel"));

	cv::Mat hsv;

	cv::cvtColor(self->mat, hsv, CV_BGR2HSV);
	hsv.copyTo(self->mat);

	return scope.Close(v8::Null());
}


Handle<Value>
Matrix::Copy(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());

	Local<Object> img_to_return = Matrix::constructor->GetFunction()->NewInstance();
	Matrix *img = ObjectWrap::Unwrap<Matrix>(img_to_return);
	self->mat.copyTo(img->mat);

	return scope.Close(img_to_return);
}


Handle<Value>
Matrix::Ptr(const v8::Arguments& args) {
	HandleScope scope;
	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());
	int line = args[0]->Uint32Value();


	char* data = self->mat.ptr<char>(line);
	//uchar* data = self->mat.data;

/*
  char *mydata = "Random raw data\0";
*/
	node::Buffer *return_buffer = Buffer::New((char *)data, self->mat.step);
	return scope.Close( return_buffer->handle_ );

//  return Undefined();
}

Handle<Value>
Matrix::Bla(const v8::Arguments& args) {
	HandleScope scope;
	int i = args[1]->Uint32Value();

	int div = 64;

	if (Buffer::HasInstance(args[0])){

		char *buf = (char *) Buffer::Data(args[0]->ToObject());
		unsigned len = Buffer::Length(args[0]->ToObject());

	}
	return Undefined();

}


Handle<Value>
Matrix::AddWeighted(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());

	Matrix *src1 = ObjectWrap::Unwrap<Matrix>(args[0]->ToObject());
	Matrix *src2 = ObjectWrap::Unwrap<Matrix>(args[2]->ToObject());

	float alpha = args[1]->NumberValue();
	float beta = args[3]->NumberValue();
	int gamma = 0;

	cv::addWeighted(src1->mat, alpha, src2->mat, beta, gamma, self->mat);
	

	return scope.Close(v8::Null());
}


Handle<Value>
Matrix::Split(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());

	return scope.Close(v8::Null());
}


Handle<Value>
Matrix::Canny(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());
	int lowThresh = args[0]->NumberValue();
	int highThresh = args[1]->NumberValue();

	cv::Canny(self->mat, self->mat, lowThresh, highThresh);

	return scope.Close(v8::Null());
}


Handle<Value>
Matrix::Dilate(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());
	int niters = args[0]->NumberValue();

	cv::dilate(self->mat, self->mat, cv::Mat(), cv::Point(-1, -1), niters);

	return scope.Close(v8::Null());
}


Handle<Value>
Matrix::FindContours(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());

	Local<Object> conts_to_return= Contour::constructor->GetFunction()->NewInstance();
	Contour *contours = ObjectWrap::Unwrap<Contour>(conts_to_return);

	cv::findContours(self->mat, contours->contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	return scope.Close(conts_to_return);

}


Handle<Value>
Matrix::DrawContour(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());
	Contour *cont = ObjectWrap::Unwrap<Contour>(args[0]->ToObject());
	int pos = args[1]->NumberValue();

	cv::Scalar color(0, 0, 255);

	if(args[2]->IsArray()) {
		Local<Object> objColor = args[2]->ToObject();
		color = setColor(objColor);
	}

	cv::drawContours(self->mat, cont->contours, pos, color, 1);

	return Undefined();
}


Handle<Value>
Matrix::DrawAllContours(const v8::Arguments& args) {
	HandleScope scope;

	Matrix *self = ObjectWrap::Unwrap<Matrix>(args.This());
	Contour *cont = ObjectWrap::Unwrap<Contour>(args[0]->ToObject());

	cv::Scalar color(0, 0, 255);

	if(args[1]->IsArray()) {
		Local<Object> objColor = args[1]->ToObject();
		color = setColor(objColor);
	}

	cv::drawContours(self->mat, cont->contours, -1, color, 1);

	return Undefined();
}

cv::Scalar setColor(Local<Object> objColor) {

	Local<Value> valB = objColor->Get(0);
	Local<Value> valG = objColor->Get(1);
	Local<Value> valR = objColor->Get(2);

	cv::Scalar color = cv::Scalar(valB->IntegerValue(), valG->IntegerValue(), valR->IntegerValue());
	return color;

}
