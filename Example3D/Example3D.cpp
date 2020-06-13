#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr size_t AMOUNTOFCUBES = 1024 * 8;
class MyGame : public bbe::Game
{
	struct CubeEntity
	{
		bbe::Cube cube;
		bbe::Vector3 rotationAxis;
		float rotationSpeed;
		float rotation;
	};
	CubeEntity cubeEntities[AMOUNTOFCUBES];
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::PointLight light;

	virtual void onStart() override
	{
		light.setPosition(bbe::Vector3(-1, -1, 1));
		light.setLightStrength(5);
		bbe::Random rand;
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			cubeEntities[i].rotationAxis = rand.randomVector3InUnitSphere();
			cubeEntities[i].rotation = rand.randomFloat() * bbe::Math::PI * 2;
			cubeEntities[i].rotationSpeed = rand.randomFloat() * bbe::Math::PI * 2 * 0.25f;
			cubeEntities[i].cube.set(rand.randomVector3InUnitSphere() * 100.0f, bbe::Vector3(1), cubeEntities[i].rotationAxis, cubeEntities[i].rotation);
		}
		
	}
	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << 1 / timeSinceLastFrame << std::endl;
		ccnc.update(timeSinceLastFrame);
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			cubeEntities[i].rotation += cubeEntities[i].rotationSpeed * timeSinceLastFrame;
			if (cubeEntities[i].rotation > bbe::Math::PI * 2)
			{
				cubeEntities[i].rotation -= bbe::Math::PI * 2;
			}
			cubeEntities[i].cube.setRotation(cubeEntities[i].rotationAxis, cubeEntities[i].rotation);
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			brush.fillCube(cubeEntities[i].cube);
		}
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "3D Test");

    return 0;
}

