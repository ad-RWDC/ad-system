#include <iostream>
#include <sstream>
#include <direct.h>
#include <fstream>

#define USE_FACE
#include "NtKinect.h"

#include "DxLib.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

string faceProp[] = {
	"happy", "engaed", "glass", "leftEyeClosed",
	"rightEyeClosed", "mouseOpen", "mouseMoved", "lookingAway"};
string dstate[] = { "unknown", "no", "maybe", "yes" };

#include <time.h>
string now() {
	char s[1024];
	time_t t = time(NULL);
	struct tm lnow;
	localtime_s(&lnow, &t);
	sprintf_s(s, "%04d-%02d-%02d_%02d-%02d-%02d", lnow.tm_year + 1900, lnow.tm_mon + 1, lnow.tm_mday, lnow.tm_hour, lnow.tm_min, lnow.tm_sec);
	return string(s);
}




int WINAPI main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	NtKinect kinect;
	clock_t start, time_from_start;
	cv::VideoWriter vw;
	int scale = 1;
	cv::Size sz(1920 / scale, 1080 / scale);
	bool onSave = false;
	cv::Mat img;
	bool start_flag = false;
	int miss_flame = 0;
	double sum_looking_time = 0.0;
	string filename;
	string video_name;
	string log;
	int MovieGraphHandle;
	int video_number = 0;


	//ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1)    // ＤＸライブラリ初期化処理
	{
		return -1;    // エラーが発生したら終了
	}


	// ムービーファイルをロードします。
	MovieGraphHandle = LoadGraph("video\\narratage.mp4");


	// ムービーを再生状態にします
	PlayMovieToGraph(MovieGraphHandle);

	DrawExtendGraph(0, 0, 640, 480, MovieGraphHandle, FALSE);

	DeleteGraph(MovieGraphHandle);

	// ムービーファイルをロードします。
	MovieGraphHandle = LoadGraph("video\\black.mp4");

	// ムービーを再生状態にします
	PlayMovieToGraph(MovieGraphHandle);


	while (CheckHitKey(KEY_INPUT_ESCAPE) == 0) {


		if (onSave) {
			cv::resize(kinect.rgbImage, img, sz, 0, 0);
			cv::cvtColor(img, img, CV_BGRA2BGR);
			vw << img;
		}

		kinect.setRGB();
		kinect.setSkeleton();
		for (auto person : kinect.skeleton) {
			for (auto joint : person) {
				if (joint.TrackingState == TrackingState_NotTracked) continue;
				ColorSpacePoint cp;
				kinect.coordinateMapper->MapCameraPointToColorSpace(joint.Position, &cp);
				//cv::rectangle(kinect.rgbImage, cv::Rect((int)cp.X - 5, (int)cp.Y - 5, 10, 10), cv::Scalar(0, 0, 255), 2);
			}
		}
		kinect.setFace();
		for (cv::Rect r : kinect.faceRect) {
			//cv::rectangle(kinect.rgbImage, r, cv::Scalar(255, 255, 0), 2);
		}
		for (vector<PointF> vf : kinect.facePoint) {
			for (PointF p : vf) {
				//cv::rectangle(kinect.rgbImage, cv::Rect((int)p.X - 3, (int)p.Y - 3, 6, 6), cv::Scalar(0, 255, 255), 2);
			}
		}
		for (int p = 0; p < kinect.faceDirection.size(); p++) {
			if (!onSave) {
				DeleteGraph(MovieGraphHandle);
				filename = "D:\log\\" + now();
				_mkdir(filename.c_str());
				vw = cv::VideoWriter( filename + "\\video.avi", CV_FOURCC_MACRO('X', 'V', 'I', 'D'), 20.0, sz);

				if (!vw.isOpened()) throw runtime_error("cannot create video file");
				onSave = true;
				cv::imwrite( filename + "\\image.jpg", kinect.rgbImage);

				video_number = rand() % (5);

				if (video_number == 0) {
					// ムービーファイルをロードします。
					MovieGraphHandle = LoadGraph("video\\narratage.mp4");
					video_name = "0 narratage.mp4";
				}

				else if (video_number == 1) {
					// ムービーファイルをロードします。
					MovieGraphHandle = LoadGraph("video\\sensei.mp4");
					video_name = "1 sensei.mp4";
				}

				else if (video_number == 2) {
					// ムービーファイルをロードします。
					MovieGraphHandle = LoadGraph("video\\doraemon.mp4");
					video_name = "2 doraemon.mp4";
				}
				else if (video_number == 3) {
					// ムービーファイルをロードします。
					MovieGraphHandle = LoadGraph("video\\starwars.mp4");
					video_name = "3 starwars.mp4";
				}

				else if (video_number == 4) {
					// ムービーファイルをロードします。
					MovieGraphHandle = LoadGraph("video\\marvel.mp4");
					video_name = "4 marvel.mp4";
				}

				log = filename + "\n" + video_name + "\n";

				// ムービーを再生状態にします
				PlayMovieToGraph(MovieGraphHandle);
				start = clock();
				time_from_start = clock();
			}

			cv::Vec3f dir = kinect.faceDirection[p];
			cv::putText(kinect.rgbImage, "pitch : " + to_string(dir[0]), cv::Point(200 * p + 50, 30), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 0), 1, CV_AA);
			cv::putText(kinect.rgbImage, "yaw : " + to_string(dir[1]), cv::Point(200 * p + 50, 60), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 0), 1, CV_AA);
			cv::putText(kinect.rgbImage, "roll : " + to_string(dir[2]), cv::Point(200 * p + 50, 90), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 0), 1, CV_AA);

			if (!(dir[0] == 0.0 && dir[1] == 0.0 && dir[2] == 0.0)) {


				if (dir[1] >= -20 && dir[1] <= 20 && !start_flag) {
					start = clock();
					start_flag = true;
				}

				if (dir[1] >= -20 && dir[1] <= 20 && start_flag) {
					time_from_start = clock();
					cv::putText(kinect.rgbImage, "look : " + to_string(sum_looking_time + (time_from_start - start) / 1000.0), cv::Point(200 * p + 50, 120), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 255), 1, CV_AA);
				}

				if ((dir[1] < -20 || dir[1] > 20) && start_flag) {
					start_flag = false;
					//log = log + to_string(kinect.skeletonTrackingId[0]) + "looking_time : " + to_string((time_from_start - start) / 1000.0) + "\n";
					log = log + "looking_time : " + to_string((time_from_start - start) / 1000.0) + "\n";
					sum_looking_time = sum_looking_time + ((time_from_start - start) / 1000.0);
				}

				if ((dir[1] < -20 || dir[1] > 20) && !start_flag) {
					cv::putText(kinect.rgbImage, "look : " + to_string(sum_looking_time), cv::Point(200 * p + 50, 120), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 255), 1, CV_AA);
				}
			}
			else {
				if ((time_from_start - start)>0) {
					cv::putText(kinect.rgbImage, "look : " + to_string(sum_looking_time + (time_from_start - start) / 1000.0), cv::Point(200 * p + 50, 120), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 255), 1, CV_AA);
				}
				else {
					cv::putText(kinect.rgbImage, "look : " + to_string(sum_looking_time) , cv::Point(200 * p + 50, 120), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 255), 1, CV_AA);
				
				}
			}
			miss_flame = 0;
		}

		if (kinect.faceDirection.size() == 0) {
			miss_flame = miss_flame + 1;
		}

		if (onSave && miss_flame >= 30) {
			vw.release();
			onSave = false;
			if (start_flag == true) {
				start_flag = false;
				log = log + "looking_time : " + to_string((time_from_start - start) / 1000.0) + "\n";
				sum_looking_time = sum_looking_time + ((time_from_start - start) / 1000.0);
			}

			ofstream outputfile(filename + "\\log.txt");
			outputfile << log;
			outputfile << "looking_time_sum : " + to_string(sum_looking_time);

			if (sum_looking_time == 0.0) {
				DeleteFile(filename.c_str());
			}

			outputfile.close();
			cv::destroyAllWindows();
			// 読み込んだムービーファイルのグラフィックハンドルの削除
			DeleteGraph(MovieGraphHandle);

			// ムービーファイルをロードします。
			MovieGraphHandle = LoadGraph("video\\black.mp4");

			// ムービーを再生状態にします
			PlayMovieToGraph(MovieGraphHandle);

			sum_looking_time = 0.0;
			miss_flame = 0;
		}

		// ムービー映像を画面いっぱいに描画します
		DrawExtendGraph(0, 0, 640, 480, MovieGraphHandle, FALSE);
		
		//auto key = cv::waitKey(1);
		//if (key == 'q') break;
	}
	cv::destroyAllWindows();
	DxLib_End();        // ＤＸライブラリ使用の終了処理

	return 0;        // ソフトの終了
}