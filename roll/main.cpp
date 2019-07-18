/*
different sequences need to modify parameters:
delta_roll
delta_pitch
delta_yaw
the name of modified video

*/



#include<opencv2\opencv.hpp>
#include<opencv2\core\types_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream> 
#include<fstream>
#include<vector>
#include<stdio.h>
#include<math.h>

using namespace std;
using namespace cv;
#define numframes 300
#define pix_width 4096
#define pix_height 2048
#define delta_roll  60 * CV_PI / 180 // + means video anticlockwise
#define delta_pitch  60 * CV_PI / 180// + means video up
#define delta_yaw 120*CV_PI/180 //+ means turn video right



void video2image(string video, string path)
{
	VideoCapture capture(video);
	if (!capture.isOpened())
	{
		cerr << "Failed to open a video" << endl;
		return;
	}

	Mat frame;

	int num = 1;
	string filename;
	char   temp_file[5];

	for (;;)
	{
		capture >> frame;
		if (frame.empty())
			break;

		_itoa_s(num, temp_file, 4, 10); //4表示字符长度  10表示十进制  实现整型转字符串
		filename = temp_file;
		filename = path + filename + ".bmp";
		num++;
		imwrite(filename, frame);
	}
	capture.release();

}



void imageroll(string Img, string imgn)
{
	double scaleX ;
	double scaleY ;
	//double delta = 60 * CV_PI / 180;
	int i, j, q;
	double dlat, dlon, dalpha, dbeta, dnewalpha, dnewbeta, dnewlat, dnewlon;
	double x, y, float_y, float_x;
	int floorx, floory, ceilx, ceily;
	double value_up_left, value_up_right, value_down_left, value_down_right;
	string filename, Nfilename;
	Mat frame,newframe;
	char   temp_file[5];
	//IplImage *frame,*newframe;

		for (int n = 1; n <= numframes; n++)
		{
			_itoa_s(n, temp_file, 4, 10); //4表示字符长度  10表示十进制  实现整型转字符串
			filename = temp_file;
			filename = Img  + filename + ".bmp";
			frame = imread(filename);
			//imshow(filename,frame);
			//waitKey(6000);
			newframe = imread(filename);
			//double delta_roll=n*45/600 * CV_PI / 180;  //缓慢变化至45度
			scaleX = 2 * CV_PI / frame.cols;
			scaleY = CV_PI / frame.rows;
			for (q = 0; q < frame.channels(); q++) {
				for (i = 0; i < frame.rows; i++) {
					for (j = 0; j < frame.cols; j++) {
						dlon = j *scaleX - 0.5*CV_PI;
						dlat = CV_PI / 2 - (i + 0.5)*scaleY;
						dalpha = dlon - CV_PI;
						dbeta = CV_PI / 2 - dlat;
						dnewalpha = atan(sin(dbeta)*sin(dalpha) / (cos(dbeta)*sin(delta_roll) + sin(dbeta)*cos(dalpha)*cos(delta_roll)));
						dnewbeta = acos(cos(delta_roll)*cos(dbeta) - sin(dbeta)*cos(dalpha)*sin(delta_roll));
						//if (j > 0 && j <= 1024 && dnewalpha > 0)
						if (j > 0 && j < frame.cols / 4 && dnewalpha > 0)
							dnewlon = CV_PI + dnewalpha;
						//else if (j > 0 && j <= 1024 && dnewalpha <= 0 || j > 1024 && j <= 3072 && dnewalpha >= 0)
						else if (j > 0 && j < frame.cols / 4 && dnewalpha < 0 || j >frame.cols / 4 && j < 3 * frame.cols / 4 && dnewalpha > 0)
							dnewlon = dnewalpha;
						//else if (j > 1024 && j <= 3072 && dnewalpha < 0 || j>3072 && dnewalpha >= 0)
						else if (j > frame.cols / 4 && j < 3 * frame.cols / 4 && dnewalpha < 0 || j>3 * frame.cols / 4 && dnewalpha > 0)
							dnewlon = dnewalpha + CV_PI;
						//else if (j > 3072 && dnewalpha < 0)
						else if (j > 3 * frame.cols / 4 && dnewalpha < 0)
							dnewlon = dnewalpha;


						dnewlat = CV_PI / 2 - dnewbeta;

						if (dnewlat < -CV_PI / 2) {
							dnewlat = -dnewlat - CV_PI;
							if (dnewlon >= CV_PI)
								dnewlon = dnewlon - CV_PI;
							else
								dnewlon = dnewlon + CV_PI;
						}
						else if (dnewlat > CV_PI / 2) {
							dnewlat = -dnewlat + CV_PI;
							if (dnewlon >= CV_PI)
								dnewlon = dnewlon - CV_PI;
							else
								dnewlon = dnewlon + CV_PI;
						}
						else
							dnewlat = dnewlat;

						x = (dnewlon + 0.5*CV_PI) / scaleX; //列数
						y = (-dnewlat + CV_PI / 2) / scaleY - 0.5;
						float_y = y - cvFloor(y);
						float_x = x - cvFloor(x);

						if (x > frame.cols-1)
							x = frame.cols-1;


						if (x >= 0 && x <= frame.cols-1 && y >= 0 && y <= frame.rows-1) {
							floory = cvFloor(y);
							floorx = cvFloor(x);
							ceilx = cvCeil(x);
							ceily = cvCeil(y);
							value_up_left = (1 - float_x)*(1 - float_y);
							value_up_right = (float_x)*(1 - float_y);
							value_down_left = (1 - float_x)*(float_y);
							value_down_right = (float_x)*(float_y);
							newframe.at<uchar>(i, j*newframe.channels() + q) = value_up_left*frame.at<uchar>(ceily, ceilx*newframe.channels() + q)
								+ value_up_right*frame.at<uchar>(ceily, floorx*newframe.channels() + q) +
								value_down_left*frame.at<uchar>(floory, ceilx*newframe.channels() + q) +
								value_down_right*frame.at<uchar>(floory, floorx*newframe.channels() + q);

						}
					}
				}
			}
			if (frame.empty())
			{
				cout << "could not load image" << endl;
				exit(0);
			}
			Nfilename = temp_file;
			Nfilename = imgn + "dancingroll" + Nfilename + ".bmp";
			imwrite(Nfilename, newframe);

		}
}


void imagepitch(string Img, string imgn)
{
	double scaleX ;
	double scaleY ;
	//double delta_pitch = 30 * CV_PI / 180;
	int i, j, q;
	double dlat, dlon, dalpha, dbeta, dnewalpha, dnewbeta, dnewlat, dnewlon;
	double x, y, float_y, float_x;
	int floorx, floory, ceilx, ceily;
	double value_up_left, value_up_right, value_down_left, value_down_right;
	string filename, Nfilename;
	Mat frame, newframe;
	char   temp_file[5];
	//IplImage *frame,*newframe;
	for (int i = 1; i <= numframes; i++)
	{
		_itoa_s(i, temp_file, 4, 10); //4表示字符长度  10表示十进制  实现整型转字符串  
		filename = temp_file;
		filename = Img + "img" + filename + ".bmp";
		frame = imread(filename);
		//imshow(filename,frame);
		//waitKey(6000);
		newframe = imread(filename);
		scaleX = 2 * CV_PI / frame.cols;
		scaleY = CV_PI / frame.rows;
		for (q = 0; q < frame.channels(); q++) {
			for (i = 0; i < frame.rows; i++) {
				for (j = 0; j < frame.cols; j++) {
					dlon = j *scaleX;
					dlat = CV_PI / 2 - (i + 0.5)*scaleY;
					dalpha = dlon - CV_PI;
					dbeta = CV_PI / 2 - dlat;
					dnewalpha = atan(sin(dbeta)*sin(dalpha) / (cos(dbeta)*sin(delta_pitch) + sin(dbeta)*cos(dalpha)*cos(delta_pitch)));
					dnewbeta = acos(cos(delta_pitch)*cos(dbeta) - sin(dbeta)*cos(dalpha)*sin(delta_pitch));
					//if (j > 0 && j <= 2048 && dnewalpha <0 || j>2048 && dnewalpha>0) 4096*2048
					if (j > 0 && j <= frame.cols / 2 && dnewalpha <0 || j>frame.cols/2 && dnewalpha>0)
						dnewlon = CV_PI + dnewalpha;
					//else if (j>0 && j <= 2048 && dnewalpha>0)
					else if (j>0 && j <= frame.cols/2 && dnewalpha>0)
						dnewlon = dnewalpha;
					//else if (j>2048 && dnewalpha<0)
					else if (j>frame.cols/2 && dnewalpha<0)
						dnewlon = dnewalpha + 2 * CV_PI;

					dnewlat = CV_PI / 2 - dnewbeta;

					if (dnewlat < -CV_PI / 2) {
						dnewlat = -dnewlat - CV_PI;
						if (dnewlon >= CV_PI)
							dnewlon = dnewlon - CV_PI;
						else
							dnewlon = dnewlon + CV_PI;
					}
					else if (dnewlat > CV_PI / 2) {
						dnewlat = -dnewlat + CV_PI;
						if (dnewlon >= CV_PI)
							dnewlon = dnewlon - CV_PI;
						else
							dnewlon = dnewlon + CV_PI;
					}
					else
						dnewlat = dnewlat;

					x = dnewlon / scaleX; //列数
					y = (-dnewlat + CV_PI / 2) / scaleY - 0.5;
					float_y = y - cvFloor(y);
					float_x = x - cvFloor(x);

					if (x > frame.cols-1)
						x = frame.cols-1;


					if (x >= 0 && x <=frame.cols-1 && y >= 0 && y <= frame.rows-1) {
						floory = cvFloor(y);
						floorx = cvFloor(x);
						ceilx = cvCeil(x);
						ceily = cvCeil(y);
						value_up_left = (1 - float_x)*(1 - float_y);
						value_up_right = (float_x)*(1 - float_y);
						value_down_left = (1 - float_x)*(float_y);
						value_down_right = (float_x)*(float_y);
						newframe.at<uchar>(i, j*newframe.channels() + q) = value_up_left*frame.at<uchar>(ceily, ceilx*newframe.channels() + q)
							+ value_up_right*frame.at<uchar>(ceily, floorx*newframe.channels() + q) +
							value_down_left*frame.at<uchar>(floory, ceilx*newframe.channels() + q) +
							value_down_right*frame.at<uchar>(floory, floorx*newframe.channels() + q);

					}
				}
			}
		}
		if (frame.empty())
		{
			cout << "could not load image" << endl;
			exit(0);
		}
		Nfilename = temp_file;
		Nfilename = imgn + "pitch" + Nfilename + ".bmp";
		imwrite(Nfilename, newframe);
		
	}
}


//俯仰角方向，一帧旋转恒定度数
void imagepitchDelta(string Img, string imgn)
{
	double scaleX;
	double scaleY;
	//double delta_pitch = 30 * CV_PI / 180;
	int i, j, q;
	double dlat, dlon, dalpha, dbeta, dnewalpha, dnewbeta, dnewlat, dnewlon;
	double x, y, float_y, float_x;
	int floorx, floory, ceilx, ceily;
	double value_up_left, value_up_right, value_down_left, value_down_right;
	string filename, Nfilename;
	Mat frame, newframe;
	char   temp_file[5];
	//IplImage *frame,*newframe;
	for (int n = 0; n < numframes; n++)
	{
		_itoa_s(n, temp_file, 4, 10); //4表示字符长度  10表示十进制  实现整型转字符串  
		filename = temp_file;
		filename = Img + filename + ".bmp";
		frame = imread(filename);
		//imshow(filename,frame);
		//waitKey(6000);
		newframe = imread(filename);
		scaleX = 2 * CV_PI / frame.cols;
		scaleY = CV_PI / frame.rows;
		double  delta_pitch_fr = 0;
		delta_pitch_fr = 1.0* n*delta_pitch / (numframes - 1);
		for (q = 0; q < frame.channels(); q++) {
			for (i = 0; i < frame.rows; i++) {
				for (j = 0; j < frame.cols; j++) {
					dlon = j *scaleX;
					dlat = CV_PI / 2 - (i + 0.5)*scaleY;
					dalpha = dlon - CV_PI;
					dbeta = CV_PI / 2 - dlat;
					dnewalpha = atan(sin(dbeta)*sin(dalpha) / (cos(dbeta)*sin(delta_pitch_fr) + sin(dbeta)*cos(dalpha)*cos(delta_pitch_fr)));
					dnewbeta = acos(cos(delta_pitch_fr)*cos(dbeta) - sin(dbeta)*cos(dalpha)*sin(delta_pitch_fr));
					//if (j > 0 && j <= 2048 && dnewalpha <0 || j>2048 && dnewalpha>0) 4096*2048
					if (j > 0 && j <= frame.cols / 2 && dnewalpha <0 || j>frame.cols / 2 && dnewalpha>0)
						dnewlon = CV_PI + dnewalpha;
					//else if (j>0 && j <= 2048 && dnewalpha>0)
					else if (j>0 && j <= frame.cols / 2 && dnewalpha>0)
						dnewlon = dnewalpha;
					//else if (j>2048 && dnewalpha<0)
					else if (j>frame.cols / 2 && dnewalpha<0)
						dnewlon = dnewalpha + 2 * CV_PI;

					dnewlat = CV_PI / 2 - dnewbeta;

					if (dnewlat < -CV_PI / 2) {
						dnewlat = -dnewlat - CV_PI;
						if (dnewlon >= CV_PI)
							dnewlon = dnewlon - CV_PI;
						else
							dnewlon = dnewlon + CV_PI;
					}
					else if (dnewlat > CV_PI / 2) {
						dnewlat = -dnewlat + CV_PI;
						if (dnewlon >= CV_PI)
							dnewlon = dnewlon - CV_PI;
						else
							dnewlon = dnewlon + CV_PI;
					}
					else
						dnewlat = dnewlat;

					x = dnewlon / scaleX; //列数
					y = (-dnewlat + CV_PI / 2) / scaleY - 0.5;
					float_y = y - cvFloor(y);
					float_x = x - cvFloor(x);

					if (x > frame.cols - 1)
						x = frame.cols - 1;


					if (x >= 0 && x <= frame.cols - 1 && y >= 0 && y <= frame.rows - 1) {
						floory = cvFloor(y);
						floorx = cvFloor(x);
						ceilx = cvCeil(x);
						ceily = cvCeil(y);
						value_up_left = (1 - float_x)*(1 - float_y);
						value_up_right = (float_x)*(1 - float_y);
						value_down_left = (1 - float_x)*(float_y);
						value_down_right = (float_x)*(float_y);
						newframe.at<uchar>(i, j*newframe.channels() + q) = value_up_left*frame.at<uchar>(ceily, ceilx*newframe.channels() + q)
							+ value_up_right*frame.at<uchar>(ceily, floorx*newframe.channels() + q) +
							value_down_left*frame.at<uchar>(floory, ceilx*newframe.channels() + q) +
							value_down_right*frame.at<uchar>(floory, floorx*newframe.channels() + q);

					}
				}
			}
		}
		if (frame.empty())
		{
			cout << "could not load image" << endl;
			exit(0);
		}
		Nfilename = temp_file;
		Nfilename = imgn + "pitch" + Nfilename + ".bmp";
		imwrite(Nfilename, newframe);

	}
}

void imageyaw(string Img, string imgn)
{
	double scaleX;
	double scaleY;
	//double delta = 60 * CV_PI / 180;
	int i, j, q;
	double dlon, dalpha,  dnewalpha,  dnewlon;
	double x, y, float_x;
	int floorx, ceilx;
	double value_left, value_right;
	string filename, Nfilename;
	Mat frame, newframe;
	char   temp_file[5];
	//IplImage *frame,*newframe;

	for (int n = 1; n <= numframes; n++)
	{
		_itoa_s(n, temp_file, 4, 10); //4表示字符长度  10表示十进制  实现整型转字符串
		filename = temp_file;
		filename = Img +"pic_"+ filename + ".jpg";
		frame = imread(filename);
		//imshow(filename,frame);
		//waitKey(6000);
		newframe = imread(filename);
		//double delta_roll=n*45/600 * CV_PI / 180;  //缓慢变化至45度
		scaleX = 2 * CV_PI / frame.cols;
		scaleY = CV_PI / frame.rows;
		for (q = 0; q < frame.channels(); q++) {
			for (i = 0; i < frame.rows; i++) {
				for (j = 0; j < frame.cols; j++) {
					dlon = j *scaleX ;
					dnewalpha = dlon +delta_yaw;
					
					if (dnewalpha > 2 * CV_PI)
						dnewlon = dnewalpha - 2 *CV_PI;
					else if (dnewalpha < 0)
						dnewlon = dnewalpha + 2 * CV_PI;
					else
						dnewlon = dnewalpha;

					x = (dnewlon ) / scaleX; //列数
					y = i;
					float_x = x - cvFloor(x);

					if (x > frame.cols - 1)
						x = frame.cols - 1;

					if (x >= 0 && x <= frame.cols - 1 ) {
						floorx = cvFloor(x);
						ceilx = cvCeil(x);
						value_left = 1 - float_x;
						value_right = float_x;

						newframe.at<uchar>(i, j*newframe.channels() + q) = value_left*frame.at<uchar>(y, ceilx*newframe.channels() + q)
							+ value_right*frame.at<uchar>(y, floorx*newframe.channels() + q) ;

					}
				}
			}
		}
		if (frame.empty())
		{
			cout << "could not load image" << endl;
			exit(0);
		}
		Nfilename = temp_file;
		Nfilename = imgn + "yaw" + Nfilename + ".bmp";
		imwrite(Nfilename, newframe);

	}
}

//每一帧旋转同样的角度
void imageyawDelta(string Img, string imgn)
{
	double scaleX;
	double scaleY;
	//double delta = 60 * CV_PI / 180;
	int i, j, q;
	double dlon, dalpha, dnewalpha, dnewlon;
	double x, y, float_x;
	int floorx, ceilx;
	double value_left, value_right;
	string filename, Nfilename;
	Mat frame, newframe;
	char   temp_file[5];
	//IplImage *frame,*newframe;

	for (int n = 0; n < numframes; n++)
	{
		_itoa_s(n, temp_file, 4, 10); //4表示字符长度  10表示十进制  实现整型转字符串
		filename = temp_file;
		filename = Img  + filename + ".bmp";
		frame = imread(filename);
		//imshow(filename,frame);
		//waitKey(6000);
		newframe = imread(filename);
		//double delta_roll=n*45/600 * CV_PI / 180;  //缓慢变化至45度
		scaleX = 2 * CV_PI / frame.cols;
		scaleY = CV_PI / frame.rows;
	//缓慢变化yaw_delta
			double delta_yaw_fr = 0;
			delta_yaw_fr =1.0* n*delta_yaw / (numframes-1);
			for (q = 0; q < frame.channels(); q++) {
				for (i = 0; i < frame.rows; i++) {
					for (j = 0; j < frame.cols; j++) {
						dlon = j *scaleX;
						dnewalpha = dlon + delta_yaw_fr;

						if (dnewalpha > 2 * CV_PI)
							dnewlon = dnewalpha - 2 * CV_PI;
						else if (dnewalpha < 0)
							dnewlon = dnewalpha + 2 * CV_PI;
						else
							dnewlon = dnewalpha;

						x = (dnewlon) / scaleX; //列数
						y = i;
						float_x = x - cvFloor(x);

						if (x > frame.cols - 1)
							x = frame.cols - 1;

						if (x >= 0 && x <= frame.cols - 1) {
							floorx = cvFloor(x);
							ceilx = cvCeil(x);
							value_left = 1 - float_x;
							value_right = float_x;

							newframe.at<uchar>(i, j*newframe.channels() + q) = value_right*frame.at<uchar>(y, ceilx*newframe.channels() + q)
								+ value_left*frame.at<uchar>(y, floorx*newframe.channels() + q);

						}
					}
				}
			}
	
		if (frame.empty())
		{
			cout << "could not load image" << endl;
			exit(0);
		}
		Nfilename = temp_file;
		Nfilename = imgn + "yaw" + Nfilename + ".bmp";
		imwrite(Nfilename, newframe);

	}
}

//直接每一帧转过一定的像素
void imageyawpix(string Img, string imgn)
{
	double scaleX;
	double scaleY;
	//double delta = 60 * CV_PI / 180;
	int i, j, q;
	double dlon, dalpha, dnewalpha, dnewlon;
	double x, y, float_x;
	int floorx, ceilx;
	double value_left, value_right;
	string filename, Nfilename;
	Mat frame, newframe;
	char   temp_file[5];
	//IplImage *frame,*newframe;

	for (int n = 0; n < numframes; n++)
	{
		_itoa_s(n, temp_file, 4, 10); //4表示字符长度  10表示十进制  实现整型转字符串
		filename = temp_file;
		filename = Img + filename + ".bmp";
		frame = imread(filename);
		//imshow(filename,frame);
		//waitKey(6000);
		newframe = imread(filename);
		//double delta_roll=n*45/600 * CV_PI / 180;  //缓慢变化至45度
		scaleX = 2 * CV_PI / frame.cols;
		scaleY = CV_PI / frame.rows;
		//缓慢变化yaw_delta
		double delta_yaw_fr = 0;
		delta_yaw_fr = 6.8* n;
		for (q = 0; q < frame.channels(); q++) {
			for (i = 0; i < frame.rows; i++) {
				for (j = 0; j < frame.cols; j++) {
					dlon = j + delta_yaw_fr;
					if (dlon >= frame.cols)
						dnewlon = dlon - frame.cols;
					else
						dnewlon = dlon;

					//newframe.at<uchar>(i, j*newframe.channels() + q) = frame.at<uchar>(i, dnewlon*newframe.channels() + q);

					x = dnewlon;
					float_x = x - cvFloor(x);
					if (x >= 0 && x <= frame.cols - 1) {
						floorx = cvFloor(x);
						ceilx = cvCeil(x);
						value_left = 1 - float_x;
						value_right = float_x;

						newframe.at<uchar>(i, j*newframe.channels() + q) = value_right*frame.at<uchar>(i, ceilx*newframe.channels() + q)
							+ value_left*frame.at<uchar>(i, floorx*newframe.channels() + q);
					}

				}
			}
		}


		if (frame.empty())
		{
			cout << "could not load image" << endl;
			exit(0);
		}
		Nfilename = temp_file;
		Nfilename = imgn + "yaw" + Nfilename + ".bmp";
		imwrite(Nfilename, newframe);

	}
}




//假定图像集已经按帧号顺序排列  
void images2video(string _Imagespath, string &_videoTosave)
{
	int newnumframes = 1500; //需要合成视频的帧数  
	int fourcc = CV_FOURCC('D', 'I', 'V', 'X'); //编解码类型  
	double fps = 30;
	bool iscolor = true;
	//int frameWidth = 4096; //帧宽度  
	//int frameHeight = 2048; //帧高度  
	VideoWriter Writer;
/*
	//多个视频
	string filename,videoname;
	char   temp_video[5],temp_file[5];
	Mat frame;

	for (int i = 1; i <= 88; i++) {
		Writer = VideoWriter(_videoTosave, fourcc, fps, Size(pix_width, pix_height), iscolor);
		_itoa_s(i, temp_video, 4, 10);
		videoname = _videoTosave + temp_video + ".mp4";
		for (int j = 1; j <= 30; j++) {

			filename = _Imagespath + temp_video + ".bmp";
			frame = imread(filename);
			if (frame.empty())
			{
				cout << "could not load image" << endl;
				exit(0);

			}

			Writer.write(frame);
		}

	}
	*/

	//单个视频
	Writer = VideoWriter(_videoTosave, fourcc, fps, Size(pix_width, pix_height), iscolor);
	string filename;
	char   temp_file[5];
	Mat frame;

	for (int j = 0; j <newnumframes; j++) {


		_itoa_s(j, temp_file, 5, 10); //4表示字符长度  10表示十进制  实现整型转字符串
		filename = temp_file;
		filename = _Imagespath + "yaw_"+filename + ".bmp";

		frame = imread(filename);
		if (frame.empty())
		{
			cout << "could not load image" << endl;
			exit(0);

		}
		Writer.write(frame);

	}
}
	/*
	//循环播放，增强序列长度
	for (int i = numframes; i >=1 ; i--)
	{
		_itoa_s(i, temp_file, 4, 10); //4表示字符长度  10表示十进制  实现整型转字符串
		filename = temp_file;
		filename = _Imagespath + "dancingroll" + filename + ".bmp";
		frame = imread(filename);
		if (frame.empty())
		{
			cout << "could not load image" << endl;
			exit(0);
		}
		Writer.write(frame);
	}
	*/



int main(int argc, char** argv)
{
	string videoFromfile = "E:\\lab\\graduation_project\\test_video\\MP4\\skateboard_in_lot_4096x2048_30fps_8bit_P420_erp.mp4";  //读取视频  
	string Imagespath = "W:\\opencv_code\\pic_Harbor\\";    //保存图片集路径  
	string Imageroll = "E:\\lab\\graduation_project\\open_CV\\yuv2rgb\\yuv2rgb\\";		//左右倾斜
	string Imagepitch = "C:\\Users\\yangminxia\\test_sequences\\roll\\";	//俯仰角变换
	string ImagepitchDelta = "E:\\lab\\graduation_project\\test_video\\pic_pitch\\";	//俯仰角变换
	string Imageyaw = "E:\\lab\\graduation_project\\open_CV\\yuv2rgb\\yuv2rgb\\";		//前后旋转
	string ImageyawDelta = "E:\\lab\\graduation_project\\test_video\\pic_yaw2\\";		//前后旋转且缓慢变化
	string Imageyawpix = "E:\\lab\\graduation_project\\test_video\\pic_yaw3\\";		//前后旋转且缓慢变化
	string videoTosave = "E:\\lab\\graduation_project\\test_video\\Harbor_yaw.mp4"; //保存视频路径  
	//video2image(videoFromfile, Imagespath);
	//imageroll(Imagespath, Imageroll);
	//imagepitch(Imagespath, Imagepitch);
	//imagepitchDelta(Imagespath, ImagepitchDelta);
	//imageyaw(Imagespath, Imageyaw);
	//imageyawDelta(Imagespath, ImageyawDelta);
	//imageyawpix(Imagespath, Imageyawpix);
	images2video(Imagespath, videoTosave);
	
	return 0;
}