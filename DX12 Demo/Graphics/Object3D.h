#pragma once
#include "pch.h"

class Object3D
{
public:
	void AdjustPosition(float deltaX, float deltaY, float deltaZ);
	void AdjustPosition(Vector3 deltaPosition);
	void SetPosition(float x, float y, float z);
	void SetPosition(Vector3 position);

	Vector3& GetPosition();
	void SetDimensionId(uint32_t dimension);
	uint32_t GetDimensionId();

	const Vector3 GetLeftVector();
	const Vector3 GetRightVector();
	const Vector3 GetForwardVector();
	const Vector3 GetBackVector();
	const Vector3 GetUpVector();
	const Vector3 GetDownVector();

protected:
	virtual void UpdateMatrix();

	Vector3 m_Position = { 0, 0, 0 };
};