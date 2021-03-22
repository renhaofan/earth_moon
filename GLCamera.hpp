#pragma once


//　matrix store element is row-major

/*****************************************************************
   Implement viewMatrix with transformation for OpenGL to utilize
*****************************************************************/

/*****************************************************************

		 z|                                w|
	x     |          M_{view}        u      |
	------0       ------------>      -------e
		   \								 \
			\y								  \v
Note: gazing direction is -w, not w, u is up-direction
*****************************************************************/



#include <iostream>
#include <stack>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Dense>


class GLCamera {
public:
	GLCamera();
	~GLCamera();
	// print info
	void PrintInfo();
	// setters
	void SetViewTranslateMatrix(const Eigen::Vector3f& m);
	void SetViewTranslateMatrix(const Eigen::Matrix4f& m);
	void SetViewTranslateMatrix(float tx, float ty, float tz) {
		SetViewTranslateMatrix(Eigen::Vector3f(tx, ty, tz));
	}

	void SetViewRotateMatrix(const Eigen::Matrix3f& m);
	void SetViewRotateMatrix(const Eigen::Matrix4f& m);
	void SetViewRotateMatrix(const Eigen::Vector3f& u, const Eigen::Vector3f& v, const Eigen::Vector3f& w);

	void SetViewMatrix(const Eigen::Matrix4f& m);
	void SetViewMatrix(const Eigen::Vector3f& u, const Eigen::Vector3f& v, const Eigen::Vector3f& w, const Eigen::Vector3f& e);



	void SetCamera(const Eigen::Vector3f& pos, const Eigen::Vector3f& tar);
	void SetCamera(const Eigen::Vector3f& pos, const Eigen::Vector3f& tar, const Eigen::Vector3f& upDir);
	void SetCamera(float px, float py, float pz, float tx, float ty, float tz) {
		SetCamera(Eigen::Vector3f(px, py, pz), Eigen::Vector3f(tx, ty, tz));
	}
	void SetCamera(float px, float py, float pz, float tx, float ty, float tz, float ux, float uy, float uz) {
		SetCamera(Eigen::Vector3f(px, py, pz), Eigen::Vector3f(tx, ty, tz), Eigen::Vector3f(ux, uy, uz));
	}
	// similar to gluLookAt()
	void LookAt(float px, float py, float pz, float tx, float ty, float tz, float ux, float uy, float uz) {
		SetCamera(px, py, pz, tx, ty, tz, ux, uy, uz);
	}


	// update
	void UpdateViewMatrix() {
		_u = _view_rotate_matrix.block<1, 3>(0, 0);
		_v = _view_rotate_matrix.block<1, 3>(1, 0);
		_w = _view_rotate_matrix.block<1, 3>(2, 0);
		_e = _view_translate_matrix.block<3, 1>(0, 3);
		_e = -_e;
		_view_matrix = _view_rotate_matrix * _view_translate_matrix;
	}


	//getters
	Eigen::Matrix4f GetViewMatrix() {
		return _view_matrix;
	}
	Eigen::Vector3f GetViewTranslate() {
		return _view_translate_matrix.block<3, 1>(0, 3);
	}
	Eigen::Vector3f GetCameraU() {
		return _u;
	}
	Eigen::Vector3f GetCameraV() {
		return _v;
	}
	Eigen::Vector3f GetCameraW() {
		return _w;
	}
	Eigen::Vector3f GetCameraE() {
		return _e;
	}
	// load 
	void LoadToGLMatrix(float* m);

	// translate to position t, similiar with glTranslate();
	void TranslateTo(const Eigen::Vector3f& t) {
		SetViewTranslateMatrix((Eigen::Vector3f)(-t));
	}
	void TranslateTo(float tx, float ty, float tz) {
		SetViewTranslateMatrix(-tx, -ty, -tz);
	}

	// delta > 0, shift direction---camera coordinates: right(u), up(v), backward(w);
	void ShiftLeft(float deltaLeft);
	void ShiftRight(float deltaRight);
	void ShiftUp(float deltaUp);
	void ShiftDown(float deltaDown);
	void ShiftForward(float deltaForward);
	void ShiftBackward(float deltaBackward);

	// delta > 0, move direction---world coordinates: right(x), up(y), backward(z);
	void MoveLeft(float deltaLeft);
	void MoveRight(float deltaRight);
	void MoveUp(float deltaUp);
	void MoveDown(float deltaDown);
	void MoveForward(float deltaForward);
	void MoveBackward(float deltaBackward);

	// rotate aroud axis, u, v, w
	void PitchU(float angle);
	void YawV(float angle);
	void RollW(float angle);

	// for multiple view
	void PushViewMatrix();
	void PopViewMatrix();


	// to vector homogeneous coordinates
	Eigen::Vector4f ToHomogeneous(const Eigen::Vector3f& v) {
		return Eigen::Vector4f(v.x(), v.y(), v.z(), 0);
	}
	// to vector nonhomogeneous coordinates
	Eigen::Vector3f ToNonhomogeneous(const Eigen::Vector4f& v) {
		if (std::abs(v[3]) < 1e-5) {
			return Eigen::Vector3f(v.x(), v.y(), v.z());
		}
		else {
			return Eigen::Vector3f(v.x() / v.w(), v.y() / v.w(), v.z() / v.w());
		}
	}


protected:

private:
	// Camera e is camera 
	Eigen::Vector3f _u, _v, _w, _e;		   // right-hand corresponding to x,y,z,orign(0, 0, 0)

	// 4x4 transform matrices
	Eigen::Matrix4f _view_matrix;
	Eigen::Matrix4f _view_rotate_matrix; // x,y,z  row-major
	Eigen::Matrix4f _view_translate_matrix;

	std::stack<Eigen::Matrix4f> view_stack;
};