#pragma once
#include "pch.h"

class Entity
{
public:
	Matrix GetWorldMatrix();
	void SetRotation(Quaternion rotation);
	void SetScale(Vector3 scale);
	void SetTranslation(Vector3 translation);
	void UpdateMatrix();
private:
	Vector3 m_Translation = { 0, 0, 0 };
	Vector3 m_Scale = { 1, 1, 1 };
	Quaternion m_Quaternion = Quaternion::Identity;
	Matrix m_WorldMatrix = Matrix::Identity;
};