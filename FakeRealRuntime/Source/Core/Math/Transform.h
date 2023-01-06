#pragma once

#include "Core/Math/Matrix.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Quaternion.h"

namespace FakeReal {
	class Transform
	{
	public:
		Transform() {}
		Transform(const Vector3& scale, const Vector3& rotation, const Vector3& position);
		~Transform() {}

		Transform(const Transform&) = default;
		Transform& operator = (const Transform&) = default;

		Transform(Transform&&) = default;
		Transform& operator = (Transform&&) = default;

	public:
		const Vector3& GetScale() const;
		const Vector3& GetRotation() const;
		const Quaternion& GetRotationQ() const;
		const Vector3& GetPosition() const;
		Vector3& GetPosition();
		Vector3 GetRightAxis() const;
		Vector3 GetUpAxis() const;
		Vector3 GetForwardAxis() const;

		Matrix4x4 GetLocalToWorldMatrix() const;

		Matrix4x4 GetWorldToLocalMatrix() const;

		void SetScale(const Vector3& scale);
		void SetScale(float x, float y, float z);

		void SetRotation(const Vector3& eulerAnglesInRadian);
		void SetRotation(float  x, float y, float z);
		void SetRotation(const Quaternion& quaternion);
		void SetRotation(float x, float y, float z, float w);

		void SetPosition(const Vector3& position);
		void SetPosition(float x, float y, float z);

		void Rotate(const Vector3& eulerAnglesInRadian);
		void RotateAxis(const Vector3& axis, float radian);
		void RotateAround(const Vector3& point, const Vector3& axis, float radian);

		void Translate(const Vector3& direction, float val);

		void LookAt(const Vector3& target, const Vector3& up = Vector3(0.f, 1.f, 0.f));
		void LookTo(const Vector3& direction, const Vector3& up = Vector3(0.f, 1.f, 0.f));

		void Product(const Transform& t1, const Transform& t2);

		void Inverse(Transform& out) const;

		void SetMatrix(const Matrix4x4& matrix);
	private:
		Vector3 GetEulerAnglesFromRotationMatrix(const Matrix4x4& rotation_mx);

	private:
		Vector3 m_Scale = { 1.f, 1.f, 1.f };
		Vector3 m_Rotation = { 0.0f, 0.0f, 0.0f };
		Vector3 m_Position = { 0.f, 0.f, 0.f };
		Quaternion m_RotationQ;
	};
}
