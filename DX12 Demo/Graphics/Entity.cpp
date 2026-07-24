#include "pch.h"
#include "Entity.h"

Matrix Entity::GetWorldMatrix()
{
	return m_WorldMatrix;
}

void Entity::SetRotation(Quaternion rotation)
{
	m_Quaternion = rotation;
	UpdateMatrix();
}

void Entity::SetScale(Vector3 scale)
{
	m_Scale = scale;
	UpdateMatrix();
}

void Entity::SetTranslation(Vector3 translation)
{
	m_Translation = translation;
	UpdateMatrix();
}

void Entity::UpdateMatrix()
{
	m_WorldMatrix = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
		XMMatrixRotationQuaternion(m_Quaternion) *
		XMMatrixTranslation(m_Translation.x, m_Translation.y, m_Translation.z);
}
