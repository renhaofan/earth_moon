#include "GLCamera.hpp"

GLCamera::GLCamera() {
	_u = Eigen::Vector3f(1, 0, 0);
	_v = Eigen::Vector3f(0, 1, 0);
	_w = Eigen::Vector3f(0, 0, 1);
	_e = Eigen::Vector3f(0, 0, 0);
	_view_matrix.setIdentity();
	_view_rotate_matrix.setIdentity();
	_view_translate_matrix.setIdentity();
}

GLCamera::~GLCamera() {

}

void GLCamera::PrintInfo() {
	std::cout << "Camera position: \n" << _e << "\n";
	std::cout << "u: " << _u.transpose() << "\n";
	std::cout << "v: " << _v.transpose() << "\n";
	std::cout << "w: " << _w.transpose() << "\n";
	std::cout << " _view_rotate_matrix: \n" << _view_rotate_matrix << "\n";
	std::cout << " _view_translate_matrix:\n " << _view_translate_matrix << "\n";
	std::cout << " _view_matrix:\n " << _view_matrix << "\n";

}


#pragma region(setters)
void GLCamera::SetViewTranslateMatrix(const Eigen::Vector3f& m) {
	_view_translate_matrix.block<3, 1>(0, 3) = m;
	UpdateViewMatrix();
}
void GLCamera::SetViewTranslateMatrix(const Eigen::Matrix4f& m) {
	_view_translate_matrix = m;
	UpdateViewMatrix();
}

void GLCamera::SetViewRotateMatrix(const Eigen::Matrix3f& m) {
	_view_rotate_matrix.block<3, 3>(0, 0) = m;
	UpdateViewMatrix();
}
void GLCamera::SetViewRotateMatrix(const Eigen::Matrix4f& m) {
	_view_rotate_matrix = m;
	UpdateViewMatrix();
}
void GLCamera::SetViewRotateMatrix(const Eigen::Vector3f& u, const Eigen::Vector3f& v, const Eigen::Vector3f& w) {
	Eigen::Vector3f tmp_u, tmp_v, tmp_w;
	tmp_u = u; tmp_v = v; tmp_w = w;
	tmp_u.normalize(); tmp_v.normalize(); tmp_w.normalize();
	_view_rotate_matrix.block<1, 3>(0, 0) = _u = tmp_u;
	_view_rotate_matrix.block<1, 3>(1, 0) = _v = tmp_v;
	_view_rotate_matrix.block<1, 3>(2, 0) = _w = tmp_w;

	UpdateViewMatrix();
}

void GLCamera::SetViewMatrix(const Eigen::Matrix4f& m) {
	Eigen::Matrix4f r_view; r_view.setIdentity();
	r_view.block<3, 3>(0, 0) = m.block<3, 3>(0, 0);
	Eigen::Vector4f t_view;
	t_view = m.block<4, 1>(0, 3);
	t_view = r_view.transpose() * t_view;

	SetViewTranslateMatrix(ToNonhomogeneous(t_view));
	SetViewRotateMatrix(r_view);
}
void GLCamera::SetViewMatrix(const Eigen::Vector3f& u, const Eigen::Vector3f& v, const Eigen::Vector3f& w, const Eigen::Vector3f& e) {
	Eigen::Vector3f tmp_e, tmp_u, tmp_v, tmp_w;
	tmp_u = u; tmp_v = v; tmp_w = w; tmp_e = e;

	tmp_u.normalize(); tmp_v.normalize(); tmp_w.normalize();

	/*_e = e;
	_view_rotate_matrix.block<1, 3>(0, 0) = _u = tmp_u;
	_view_rotate_matrix.block<1, 3>(1, 0) = _v = tmp_v;
	_view_rotate_matrix.block<1, 3>(2, 0) = _w = tmp_w;
	_view_translate_matrix.block<3, 1>(0, 3) = -e;
	*/
	SetViewTranslateMatrix(Eigen::Vector3f(tmp_e.x(), tmp_e.y(), tmp_e.z()));
	SetViewRotateMatrix(tmp_u, tmp_v, tmp_w);
}




void GLCamera::SetCamera(const Eigen::Vector3f& pos, const Eigen::Vector3f& tar) {
	Eigen::Vector3f tmp_e, tmp_u, tmp_v, tmp_w;
	tmp_e = pos;
	tmp_w = pos - tar; tmp_w.normalize();
	if (abs(tmp_e.x()) < 1e-5 && abs(tmp_e.z()) < 1e-5) {
		if (tmp_w.y() > 0) {
			tmp_v = Eigen::Vector3f(0, 0, -1);
		}
		else {
			tmp_v = Eigen::Vector3f(0, 0, 1);
		}
	}
	else
	{
		tmp_v = Eigen::Vector3f(0, 1, 0);
	}

	tmp_u = tmp_v.cross(tmp_w); tmp_u.normalize();
	tmp_v = tmp_w.cross(tmp_u); tmp_v.normalize();

	tmp_e = -tmp_e;
	SetViewTranslateMatrix(Eigen::Vector3f(tmp_e.x(), tmp_e.y(), tmp_e.z()));
	SetViewRotateMatrix(tmp_u, tmp_v, tmp_w);
}
void GLCamera::SetCamera(const Eigen::Vector3f& pos, const Eigen::Vector3f& tar, const Eigen::Vector3f& upDir) {
	Eigen::Vector3f tmp_e, tmp_u, tmp_v, tmp_w;
	tmp_e = pos;
	tmp_w = pos - tar; tmp_w.normalize();
	tmp_v = upDir; tmp_v.normalize();
	tmp_u = tmp_v.cross(tmp_w); tmp_u.normalize();

	tmp_e = -tmp_e;
	SetViewTranslateMatrix(Eigen::Vector3f(tmp_e.x(), tmp_e.y(), tmp_e.z()));

	SetViewRotateMatrix(tmp_u, tmp_v, tmp_w);
}


#pragma endregion(setters)


#pragma region(shift)

void GLCamera::ShiftLeft(float deltaLeft) {
	if (deltaLeft <= 0) {
		ShiftRight(-deltaLeft);
	}
	else {
		SetViewTranslateMatrix((Eigen::Vector3f)(-(_e - _u * deltaLeft)));
	}
}
void GLCamera::ShiftRight(float deltaRight) {
	if (deltaRight <= 0) {
		ShiftLeft(-deltaRight);
	}
	else {
		SetViewTranslateMatrix((Eigen::Vector3f)(-(_e + _u * deltaRight)));
	}
}

void GLCamera::ShiftUp(float deltaUp) {
	if (deltaUp <= 0) {
		ShiftDown(-deltaUp);
	}
	else {
		SetViewTranslateMatrix((Eigen::Vector3f)(-(_e + _v * deltaUp)));
	}
}
void GLCamera::ShiftDown(float deltaDown) {
	if (deltaDown <= 0) {
		ShiftUp(-deltaDown);
	}
	else {
		SetViewTranslateMatrix((Eigen::Vector3f)(-(_e - _v * deltaDown)));
	}
}

void GLCamera::ShiftForward(float deltaForward) {
	if (deltaForward <= 0) {
		ShiftBackward(-deltaForward);
	}
	else {
		SetViewTranslateMatrix((Eigen::Vector3f)(-(_e - _w * deltaForward)));
	}
}
void GLCamera::ShiftBackward(float deltaBackward) {
	if (deltaBackward <= 0) {
		ShiftForward(-deltaBackward);
	}
	else {
		SetViewTranslateMatrix((Eigen::Vector3f)(-(_e + _w * deltaBackward)));
	}
}
#pragma endregion(shift)

#pragma region(move)
void GLCamera::MoveLeft(float deltaLeft) {
	if (deltaLeft <= 0) {
		MoveRight(-deltaLeft);
	}
	else {
		Eigen::Vector3f move = GetViewTranslate();
		move.x() += deltaLeft;
		SetViewTranslateMatrix(move);
	}
}
void GLCamera::MoveRight(float deltaRight) {
	if (deltaRight <= 0) {
		MoveLeft(-deltaRight);
	}
	else {
		Eigen::Vector3f move = GetViewTranslate();
		move.x() -= deltaRight;
		SetViewTranslateMatrix(move);
	}

}

void GLCamera::MoveUp(float deltaUp) {
	if (deltaUp <= 0) {
		MoveDown(-deltaUp);
	}
	else {
		Eigen::Vector3f move = GetViewTranslate();
		move.y() -= deltaUp;
		SetViewTranslateMatrix(move);
	}
}
void GLCamera::MoveDown(float deltaDown) {
	if (deltaDown <= 0) {
		MoveUp(-deltaDown);
	}
	else {
		Eigen::Vector3f move = GetViewTranslate();
		move.y() += deltaDown;
		SetViewTranslateMatrix(move);
	}
}

void GLCamera::MoveForward(float deltaForward) {
	if (deltaForward <= 0) {
		MoveBackward(-deltaForward);
	}
	else {
		Eigen::Vector3f move = GetViewTranslate();
		move.z() += deltaForward;
		SetViewTranslateMatrix(move);
	}
}
void GLCamera::MoveBackward(float deltaBackward) {
	if (deltaBackward <= 0) {
		MoveForward(-deltaBackward);
	}
	else {
		Eigen::Vector3f move = GetViewTranslate();
		move.z() -= deltaBackward;
		SetViewTranslateMatrix(move);
	}
}
#pragma endregion(move)

#pragma region(rotation)
void GLCamera::PitchU(float angle) {
	Eigen::Matrix4f rotate;
	rotate.setIdentity();

	rotate(1, 1) = std::cos(angle);
	rotate(2, 2) = std::cos(angle);
	rotate(1, 2) = -std::sin(angle);
	rotate(2, 1) = std::sin(angle);

	_view_rotate_matrix = rotate * _view_rotate_matrix;
	UpdateViewMatrix();
}
void GLCamera::YawV(float angle) {
	Eigen::Matrix4f rotate;
	rotate.setIdentity();

	rotate(0, 0) = std::cos(angle);
	rotate(2, 2) = std::cos(angle);
	rotate(2, 0) = -std::sin(angle);
	rotate(0, 2) = std::sin(angle);

	_view_rotate_matrix = rotate * _view_rotate_matrix;
	UpdateViewMatrix();
}
void GLCamera::RollW(float angle) {
	Eigen::Matrix4f rotate;
	rotate.setIdentity();

	rotate(0, 0) = std::cos(angle);
	rotate(1, 1) = std::cos(angle);
	rotate(1, 0) = std::sin(angle);
	rotate(0, 1) = -std::sin(angle);

	_view_rotate_matrix = rotate * _view_rotate_matrix;
	UpdateViewMatrix();

}

#pragma endregion(rotation)



// load OpenGL matrix is col-major
void GLCamera::LoadToGLMatrix(float* m) {
	m[0] = _view_matrix(0, 0); m[4] = _view_matrix(0, 1);  m[8] = _view_matrix(0, 2);  m[12] = _view_matrix(0, 3);
	m[1] = _view_matrix(1, 0); m[5] = _view_matrix(1, 1);  m[9] = _view_matrix(1, 2);  m[13] = _view_matrix(1, 3);
	m[2] = _view_matrix(2, 0); m[6] = _view_matrix(2, 1);  m[10] = _view_matrix(2, 2); m[14] = _view_matrix(2, 3);
	m[3] = _view_matrix(3, 0); m[7] = _view_matrix(3, 1);  m[11] = _view_matrix(3, 2);  m[15] = _view_matrix(3, 3);
}

void GLCamera::PushViewMatrix() {
	view_stack.push(_view_matrix);
}
void GLCamera::PopViewMatrix() {
	if (!view_stack.empty()) {
		SetViewMatrix(view_stack.top());
		view_stack.pop();
	}
}