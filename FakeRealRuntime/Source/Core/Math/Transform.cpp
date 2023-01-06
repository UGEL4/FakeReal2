#include "FRPch.h"
#include "Core/Math/Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
namespace FakeReal {

	Transform::Transform(const Vector3& scale, const Vector3& rotation, const Vector3& position)
		:m_Scale(scale), m_Rotation(rotation), m_Position(position)
	{
		m_RotationQ = Quaternion(m_Rotation);
	}

	const Vector3& Transform::GetScale() const
	{
		return m_Scale;
	}

	const Vector3& Transform::GetRotation() const
	{
		return m_Rotation;
	}

	const Quaternion& Transform::GetRotationQ() const
	{
		return m_RotationQ;
	}

	const Vector3& Transform::GetPosition() const
	{
		return m_Position;
	}

	Vector3& Transform::GetPosition()
	{
		return m_Position;
	}

	Vector3 Transform::GetRightAxis() const
	{
		Matrix4x4 mat = glm::eulerAngleXYZ(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		return Vector3(mat[0][0], mat[0][1], mat[0][2]);
	}

	Vector3 Transform::GetUpAxis() const
	{
		Matrix4x4 mat = glm::eulerAngleXYZ(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		return Vector3(mat[1][0], mat[1][1], mat[1][2]);
	}

	Vector3 Transform::GetForwardAxis() const
	{
		Matrix4x4 mat = glm::eulerAngleXYZ(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		return Vector3(-mat[2][0], -mat[2][1], -mat[2][2]);
	}

	Matrix4x4 Transform::GetLocalToWorldMatrix() const
	{
		//t
		Matrix4x4 t_mat(1.f);
		t_mat = glm::translate(t_mat, m_Position);
		//q
		Matrix4x4 r_mat = glm::mat4_cast(m_RotationQ);
		//s
		Matrix4x4 s_mat(1.f);
		s_mat = glm::scale(s_mat, m_Scale);
		t_mat = t_mat * r_mat * s_mat;

		return t_mat;
	}

	Matrix4x4 Transform::GetWorldToLocalMatrix() const
	{
		Matrix4x4 mat = GetLocalToWorldMatrix();
		return glm::inverse(mat);
	}

	void Transform::SetScale(const Vector3& scale)
	{
		SetScale(scale.x, scale.y, scale.z);
	}

	void Transform::SetScale(float x, float y, float z)
	{
		m_Scale.x = x;
		m_Scale.y = y;
		m_Scale.z = z;
	}

	void Transform::SetRotation(const Vector3& eulerAnglesInRadian)
	{
		SetRotation(eulerAnglesInRadian.x, eulerAnglesInRadian.y, eulerAnglesInRadian.z);
	}

	void Transform::SetRotation(float x, float y, float z)
	{
		m_Rotation.x = x;
		m_Rotation.y = y;
		m_Rotation.z = z;
		m_RotationQ = Quaternion(m_Rotation);
	}

	void Transform::SetRotation(const Quaternion& quaternion)
	{
		SetRotation(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
	}

	void Transform::SetRotation(float x, float y, float z, float w)
	{
		m_RotationQ.x = x;
		m_RotationQ.y = y;
		m_RotationQ.z = z;
		m_RotationQ.w = w;
		m_Rotation = glm::eulerAngles(m_RotationQ);
	}

	void Transform::SetPosition(const Vector3& position)
	{
		SetPosition(position.x, position.y, position.z);
	}

	void Transform::SetPosition(float x, float y, float z)
	{
		m_Position.x = x;
		m_Position.y = y;
		m_Position.z = z;
	}

	void Transform::Rotate(const Vector3& eulerAnglesInRadian)
	{
		m_Rotation += eulerAnglesInRadian;
		m_RotationQ = Quaternion(m_Rotation);
	}

	void Transform::RotateAxis(const Vector3& axis, float radian)
	{
		m_RotationQ = glm::rotate(m_RotationQ, radian, axis);
		m_Rotation = glm::eulerAngles(m_RotationQ);
	}

	void Transform::RotateAround(const Vector3& point, const Vector3& axis, float radian)
	{
		Matrix4x4 mat(1.0f);
		mat = glm::translate(mat, m_Position - point);
		mat = mat * glm::mat4_cast(m_RotationQ);
		mat = mat * glm::rotate(Matrix4x4(1.f), radian, axis);
		mat = glm::translate(mat, point);

		m_RotationQ = glm::quat_cast(mat);
		m_Rotation	= glm::eulerAngles(m_RotationQ);
		m_Position	= Vector3(mat[3][0], mat[3][1], mat[3][2]);
	}

	void Transform::Translate(const Vector3& direction, float val)
	{
		m_Position += glm::normalize(direction) * val;
	}

	void Transform::LookAt(const Vector3& target, const Vector3& up /*= Vector3( 0.f, 1.f, 0.f )*/)
	{
		Matrix4x4 lookAtMat = glm::lookAtRH(m_Position, target, glm::normalize(up));

		m_RotationQ = glm::quat_cast(lookAtMat);
		m_Rotation	= glm::eulerAngles(m_RotationQ);
	}

	void Transform::LookTo(const Vector3& direction, const Vector3& up /*= Vector3( 0.f, 1.f, 0.f )*/)
	{
		Matrix4x4 lookAtMat = glm::lookAtRH(m_Position, direction, glm::normalize(up));
		lookAtMat = glm::inverse(lookAtMat);

		m_RotationQ = glm::quat_cast(lookAtMat);
		m_Rotation	= glm::eulerAngles(m_RotationQ);
	}

	//ÁÐÖ÷Ðò,Ë³ÐòÎªt2 * t1
	void Transform::Product(const Transform& t1, const Transform& t2)
	{
		Matrix4x4 mat = t2.GetLocalToWorldMatrix() * t1.GetLocalToWorldMatrix();
		SetMatrix(mat);
		/*m_Position = mat[3];

		glm::vec3 scale;
		glm::mat3 RMat;
		for (int i = 0; i < 3; i++)
		{
			glm::vec3 Col = mat[i];
			scale[i] = Col.length();
			RMat[i] = { Col.x / scale[i], Col.y / scale[i], Col.z / scale[i] };
		}

		m_Scale = scale;

		m_RotationQ = glm::quat_cast(RMat);
		m_Rotation = glm::eulerAngles(m_RotationQ);*/
	}

	void Transform::Inverse(Transform& out) const
	{
		Matrix4x4 invMat = GetWorldToLocalMatrix();
		out.SetMatrix(invMat);
		/*out.m_Position = invMat[3];
		glm::vec3 scale;
		glm::mat3 RMat;
		for (int i = 0; i < 3; i++)
		{
			glm::vec3 Col = invMat[i];
			scale[i] = Col.length();
			RMat[i] = { Col.x / scale[i], Col.y / scale[i], Col.z / scale[i] };
		}

		out.m_Scale = scale;

		out.m_RotationQ = glm::quat_cast(RMat);
		out.m_Rotation = glm::eulerAngles(out.m_RotationQ);*/
	}

	void Transform::SetMatrix(const Matrix4x4& matrix)
	{
		/*glm::vec3 skew;
		glm::vec4 prespective;
		glm::decompose(matrix, m_Scale, m_RotationQ, m_Position, skew, prespective);
		m_RotationQ = glm::conjugate(m_RotationQ);
		m_Rotation = glm::eulerAngles(m_RotationQ);*/
		m_Position = matrix[3];

		Vector3 scale;
		Matrix3x3 RMat;
		for (int i = 0; i < 3; i++)
		{
			Vector3 Col = matrix[i];
			//scale[i] = Col.length();
			scale[i] = sqrt(Col[0] * Col[0] + Col[1] * Col[1] + Col[2] * Col[2]);
			//RMat[i] = { Col.x / scale[i], Col.y / scale[i], Col.z / scale[i] };
			RMat[i] = Col / scale[i];
		}

		m_Scale = scale;

		m_RotationQ = glm::quat_cast(RMat);
		m_Rotation	= glm::eulerAngles(m_RotationQ);
	}

	Vector3 Transform::GetEulerAnglesFromRotationMatrix(const Matrix4x4& rotation_mx)
	{
		Quaternion quaternion = glm::quat_cast(rotation_mx);
		return glm::eulerAngles(quaternion);
	}

}
