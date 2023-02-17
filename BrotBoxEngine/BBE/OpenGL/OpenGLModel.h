#pragma once

#include "BBE/glfwWrapper.h"
#include "BBE/ManuallyRefCountable.h"
#include "BBE/Model.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			class OpenGLModel :
				public ManuallyRefCountable
			{
			private:
				GLuint m_vbo = 0;
				GLuint m_ibo = 0;
				size_t m_amountOfIndices = 0;
			public:
				OpenGLModel(const bbe::Model& model);
				virtual ~OpenGLModel();

				GLuint getVbo();
				GLuint getIbo();

				size_t getAmountOfIndices();
			};
		}
	}
}
