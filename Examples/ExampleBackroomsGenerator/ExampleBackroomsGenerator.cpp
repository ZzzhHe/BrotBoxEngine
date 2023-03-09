#include "BBE/BrotBoxEngine.h"
#include "Rooms.h"
#include "AssetStore.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

namespace br
{
	class BackroomsGenerator : public bbe::Game
	{
	private:
		Rooms rooms;
		bbe::Vector2 cameraPos;

		int32_t expandRoom = 0;

		float roomAlpha = 1.f;
		float gateAlpha = 0.f;
		float gateConnectionsAlpha = 0.5f;

		int32_t hoveredRoom = -1;
		int32_t stopGenerationAtRooms = 1000000;

		int32_t currentSeed = 2;
		bool autoExpand = false;

		int renderMode = 1;
		int depth = 2;

		bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);

		bbe::List<size_t> interestingRooms;
	public:
		void newRooms()
		{
			rooms.clear();
			currentSeed++;
			rooms.setSeed(currentSeed);
			rooms.generateAtPoint(bbe::Vector2i(0, 0));
			expandRoom = 0;
			for (int i = 0; i < 13; i++)
			{
				expand();
			}
		}

		void expand()
		{
			rooms.connectGates(expandRoom);
			expandRoom++;
		}

		virtual void onStart() override
		{
			//int seed = 17126181 - 1;
			//size_t fit = 1000000;
			//while (true)
			//{
			//	seed++;
			//	Rooms rooms;
			//	rooms.setSeed(seed);
			//	rooms.generateAtPoint(bbe::Vector2i(0, 0));
			//
			//	for (int iterations = 0; iterations < 100; iterations++)
			//	{
			//		bbe::Rectanglei overArchingBounding = rooms.rooms[0].boundingBox;
			//		for (int i = 1; i < rooms.rooms.getLength(); i++)
			//		{
			//			overArchingBounding = overArchingBounding.combine(rooms.rooms[i].boundingBox);
			//		}
			//		size_t amountOfRooms = rooms.rooms.getLength() * overArchingBounding.getArea();
			//		if (!rooms.expandRoom(iterations))
			//		{
			//			amountOfRooms = rooms.rooms.getLength() * overArchingBounding.getArea();
			//			if (amountOfRooms < fit)
			//			{
			//
			//				fit = amountOfRooms;
			//				std::cout << "Seed: " << seed << " #Rooms: " << amountOfRooms << std::endl;
			//			}
			//			break;
			//		}
			//		if (amountOfRooms > fit)
			//		{
			//			break;
			//		}
			//	}
			//}

			newRooms();
		}

		void update2D(float timeSinceLastFrame)
		{
			if (renderMode != 0) return;

			//rooms.generateAtPoint(bbe::Vector2i(WINDOW_WIDTH / 2 + cameraPos.x, WINDOW_HEIGHT / 2 + cameraPos.y));

			for (int i = 0; i < 1024 * 1; i++)
			{
				if (!autoExpand || rooms.rooms.getLength() >= stopGenerationAtRooms) break;
				rooms.connectGates(expandRoom);
				expandRoom++;
			}

			std::cout << "# Rooms: " << rooms.rooms.getLength() << std::endl;

			float cameraSpeed = 300.f;
			if (isKeyDown(bbe::Key::LEFT_SHIFT))
			{
				cameraSpeed *= 10;
			}

			if (isKeyDown(bbe::Key::W))
			{
				cameraPos.y -= timeSinceLastFrame * cameraSpeed;
			}
			if (isKeyDown(bbe::Key::S))
			{
				cameraPos.y += timeSinceLastFrame * cameraSpeed;
			}
			if (isKeyDown(bbe::Key::A))
			{
				cameraPos.x -= timeSinceLastFrame * cameraSpeed;
			}
			if (isKeyDown(bbe::Key::D))
			{
				cameraPos.x += timeSinceLastFrame * cameraSpeed;
			}

			bbe::Vector2 mouseWorldPos = getMouse() + cameraPos;
			bbe::Vector2i mouseWorldPosi((int32_t)mouseWorldPos.x, (int32_t)mouseWorldPos.y);
			hoveredRoom = rooms.getRoomIndexAtPoint(mouseWorldPosi);
		}

		void update3D(float timeSinceLastFrame)
		{
			if (renderMode != 1) return;

			ccnc.update(timeSinceLastFrame * 0.2f);
			bbe::Vector3 camPos = ccnc.getCameraPos();
			//camPos.x = 0.0f;
			//camPos.y = 50.0f;
			camPos.z = 1.8f;
			ccnc.setCameraPos(camPos);
			//ccnc.setCameraForward(bbe::Vector3(1, 0, 0));
			float fps = (1 / timeSinceLastFrame);
			static float minFps = 100000;
			if (fps < minFps)
			{
				minFps = fps;
				std::cout << fps << std::endl;
			}
			//std::cout << fps << std::endl;
			interestingRooms = rooms.generateAtPointMulti(bbe::Vector2i((int32_t)camPos.x, (int32_t)camPos.y), depth);
		}

		virtual void update(float timeSinceLastFrame) override
		{
			update2D(timeSinceLastFrame);
			update3D(timeSinceLastFrame);
		}

		void drawImgui()
		{
			const char* const modes[] = {"2D", "3D"};
			ImGui::Combo("Render Mode", &renderMode, modes, 2, 10);
			if (renderMode == 0)
			{
				ImGui::SliderFloat("roomAlpha: ", &roomAlpha, 0.f, 1.f);
				ImGui::SliderFloat("GateAlpha: ", &gateAlpha, 0.f, 1.f);
				ImGui::SliderFloat("gateConnectionsAlpha: ", &gateConnectionsAlpha, 0.f, 1.f);
				ImGui::LabelText("Seed: ", "%i", currentSeed);
				ImGui::LabelText("HoveredId: ", "%i", hoveredRoom);
				ImGui::LabelText("Expand: ", "%i", expandRoom);
				ImGui::Checkbox("autoExpand", &autoExpand);
				if (ImGui::Button("New Seed"))
				{
					newRooms();
					return;
				}
				if (ImGui::Button("Expand"))
				{
					expand();
					return;
				}

				ImGui::InputInt("stopGenerationAtRooms", &stopGenerationAtRooms);
			}
			else if (renderMode == 1)
			{
				ImGui::InputInt("depth", &depth);
			}
			else
			{
				throw bbe::IllegalStateException();
			}
		}

		virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
		{
			if (renderMode != 1) return;
			brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
			//brush.setCamera(bbe::Vector3(0, 0, 1.7f), bbe::Vector3(-1, 0, 1.7f));

			int lightCount = 0;
			int wallCount = 0;
			for (size_t roomi : interestingRooms)
			{
				const Room& r = rooms.rooms[roomi];
				brush.setColorHSV(r.hue, r.saturation, r.value);
				const bbe::Rectanglei &bounding = r.boundingBox;
				bbe::Vector3 translationVec = bbe::Vector3(bounding.x + bounding.width / 2.f, bounding.y + bounding.height / 2.f, 0);
				bbe::Matrix4 translation = bbe::Matrix4::createTranslationMatrix(translationVec);
				bbe::Matrix4 scale = bbe::Matrix4::createScaleMatrix({ (float)bounding.width, (float)bounding.height, 1 });
				brush.fillRectangle(translation * scale, nullptr, nullptr, nullptr, assetStore::Floor());

				translationVec.z = 2.5f;
				translation = bbe::Matrix4::createTranslationMatrix(translationVec);
				bbe::Matrix4 rotationMat = bbe::Matrix4::createRotationMatrix(bbe::Math::PI, bbe::Vector3(1, 0, 0));
				brush.fillRectangle(translation * rotationMat * scale, nullptr, nullptr, nullptr, assetStore::Ceiling());

				brush.setColor(1, 1, 1, 1);
				brush.fillModel(bbe::Matrix4(), r.wallsModel, nullptr, nullptr, nullptr, assetStore::Wall());
				for (const bbe::PointLight& light : r.lights)
				{
					brush.fillCube(bbe::Cube(light.pos + bbe::Vector3(0.05f, 0.05f, 0.5f), bbe::Vector3(0.9f, 0.9f, 0.01f)), nullptr, nullptr, &bbe::Image::white());
					brush.addLight(light);
					lightCount++;
				}
			}
			//std::cout << "Lights: " << lightCount << " Walls: " << wallCount << std::endl;
		}

		virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
		{
			drawImgui();

			if (renderMode != 0)
			{
				bbe::String fps = "FPS: ";
				fps += 1.f / getAverageFrameTime();
				brush.fillText(25, 25, fps);
				return;
			}

			bbe::Rectanglei cameraRect(cameraPos.x, cameraPos.y, WINDOW_WIDTH, WINDOW_HEIGHT);
			for (const Room& r : rooms.rooms)
			{
				if (cameraRect.intersects(r.boundingBox))
				{
					if (roomAlpha > 0)
					{
						//TODO Debug code, remove me
						if (r.id == 8 || r.id == 9)
						{
							static int hue = 0;
							brush.setColorHSV(hue, r.saturation, r.value, roomAlpha);
							hue++;
						}
						else
						{ 
							brush.setColorHSV(r.hue, r.saturation, r.value, roomAlpha);
						}
						brush.fillRect(r.boundingBox.x - cameraPos.x, r.boundingBox.y - cameraPos.y, r.boundingBox.width, r.boundingBox.height);
					}

					if (gateAlpha > 0)
					{
						for (const Neighbor& n : r.neighbors)
						{
							const Room& neighborRoom = rooms.rooms[n.neighborId];
							brush.setColorHSV(neighborRoom.hue, neighborRoom.saturation, neighborRoom.value, gateAlpha);
							for (const Gate& g : n.gates)
							{
								brush.fillRect(g.ownGatePos.x - cameraPos.x, g.ownGatePos.y - cameraPos.y, 1, 1);
							}
						}
					}

					if (gateConnectionsAlpha > 0)
					{
						brush.setColorRGB(1, 1, 1, gateConnectionsAlpha);
						for (int32_t x = 0; x < r.walkable.getWidth(); x++)
						{
							for (int32_t y = 0; y < r.walkable.getHeight(); y++)
							{
								if (r.walkable[x][y])
								{
									brush.fillRect(r.boundingBox.x + x - cameraPos.x, r.boundingBox.y + y - cameraPos.y, 1, 1);
								}
							}
						}
					}
				}
			}

			if (hoveredRoom >= 0)
			{
				const Room& room = rooms.rooms[hoveredRoom];
				brush.setColorRGB(1, 1, 1, 0.2f);
				brush.fillRect(room.boundingBox.x - cameraPos.x, room.boundingBox.y - cameraPos.y, room.boundingBox.width, room.boundingBox.height);

				brush.setColorRGB(0, 0, 0, 0.5f);
				for (const Neighbor& n : room.neighbors)
				{
					const Room& neighborRoom = rooms.rooms[n.neighborId];
					auto myCenter = room.boundingBox.getCenter();
					bbe::Vector2 myCenterf = bbe::Vector2(myCenter.x, myCenter.y);
					auto otherCenter = neighborRoom.boundingBox.getCenter();
					bbe::Vector2 otherCenterf = bbe::Vector2(otherCenter.x, otherCenter.y);
					brush.fillLine(myCenterf - cameraPos, otherCenterf - cameraPos);
				}
			}

		}

		virtual void onEnd() override
		{
		}
	};
}

int main()
{
	br::BackroomsGenerator *mg = new br::BackroomsGenerator();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Backrooms Generator");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
