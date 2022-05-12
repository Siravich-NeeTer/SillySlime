#pragma once

#include "GameObject.h"

class Bullet : public GameObject
{
	private:
		float ATK;
		GameObject* shooter;
	public:
		// Getter
		Bullet();
		float getATK() const;
		GameObject* getShooter() const;

		// Setter
		void setATK(const float& atk);
		void setShooter(GameObject* character);
		void updatePosition(const glm::vec3& position) override;
};