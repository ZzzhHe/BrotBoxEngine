#include "BBE/Image.h"
#include "BBE/Exceptions.h"
#include "BBE/Math.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void bbe::Image::finishLoad(stbi_uc* pixels)
{
	m_format = ImageFormat::R8G8B8A8; // Is correct, even if texChannels == 3, because stbi is transforming the data for us on the fly.

	if (pixels == nullptr)
	{
		throw LoadException();
	}

	m_pdata = new byte[getSizeInBytes()]; //TODO use allocator
	memcpy(m_pdata, pixels, getSizeInBytes());

	stbi_image_free(pixels);
}

bbe::Image::Image()
{
}

bbe::Image::Image(const char * path)
{
	load(path);
}

bbe::Image::Image(const bbe::String& path)
{
	load(path);
}

bbe::Image::Image(int width, int height)
{
	load(width, height);
}

bbe::Image::Image(int width, int height, const Color & c)
{
	load(width, height, c);
}

bbe::Image::Image(int width, int height, const byte * data, ImageFormat format)
{
	load(width, height, data, format);
}

bbe::Image::Image(const Image& other)
{
	load(other.m_width, other.m_height, other.m_pdata, other.m_format);

	m_repeatMode = other.m_repeatMode;
	m_filterMode = other.m_filterMode;
	m_prendererData = other.m_prendererData;
}

bbe::Image::Image(Image&& other) noexcept
{
	this->operator=(std::move(other));
}

bbe::Image& bbe::Image::operator=(Image&& other) noexcept
{
	destroy();

	this->m_pdata = other.m_pdata;
	this->m_width = other.m_width;
	this->m_height = other.m_height;
	this->m_format = other.m_format;
	this->m_repeatMode = other.m_repeatMode;
	this->m_filterMode = other.m_filterMode;

	this->m_prendererData = other.m_prendererData;

	other.m_pdata = nullptr;
	other.m_width = 0;
	other.m_height = 0;
	other.m_format = (ImageFormat)0;
	other.m_repeatMode = (ImageRepeatMode)0;
	other.m_filterMode = (ImageFilterMode)0;

	other.m_prendererData = nullptr;

	return *this;
}

bbe::Image::~Image()
{
	destroy();
}

void bbe::Image::loadRaw(const bbe::List<unsigned char>& rawData)
{
	loadRaw(rawData.getRaw(), rawData.getLength());
}

void bbe::Image::loadRaw(const unsigned char* rawData, size_t dataLength)
{
	if (isLoaded())
	{
		throw AlreadyCreatedException();
	}

	int texChannels = 0;
	stbi_uc* pixels = stbi_load_from_memory(rawData, (int)dataLength, &m_width, &m_height, &texChannels, STBI_rgb_alpha);
	finishLoad(pixels);
}

void bbe::Image::load(const char * path)
{
	if (isLoaded())
	{
		throw AlreadyCreatedException();
	}

	int texChannels = 0;
	stbi_uc *pixels = stbi_load(path, &m_width, &m_height, &texChannels, STBI_rgb_alpha);
	finishLoad(pixels);
}

void bbe::Image::load(const bbe::String& path)
{
	load(path.getRaw());
}

void bbe::Image::load(int width, int height)
{
	load(width, height, Color());
}

void bbe::Image::load(int width, int height, const Color & c)
{
	m_width = width;
	m_height = height;
	m_format = ImageFormat::R8G8B8A8;

	const size_t size = getSizeInBytes();
	m_pdata = new byte[size]; //TODO use allocator
	for (int i = 0; i < size; i+=4)
	{
#ifdef _MSC_VER
		// MSVC doesn't understand that getSizeInBytes() will always
		// return a multiple of four, becuase m_format is R8G8B8A8.
#pragma warning( push )
#pragma warning( disable : 6386)
#endif
		m_pdata[i + 0] = (byte)(c.r * 255);
		m_pdata[i + 1] = (byte)(c.g * 255);
		m_pdata[i + 2] = (byte)(c.b * 255);
		m_pdata[i + 3] = (byte)(c.a * 255);
#ifdef _MSC_VER
#pragma warning( pop ) 
#endif
	}
}

void bbe::Image::load(int width, int height, const byte * data, ImageFormat format)
{
	destroy();

	m_width = width;
	m_height = height;
	m_format = format;

	m_pdata = new byte[getSizeInBytes()];
	memcpy(m_pdata, data, getSizeInBytes());
}

void bbe::Image::destroy()
{
	if (m_pdata != nullptr)
	{
		delete[] m_pdata;
		m_pdata = nullptr;
		m_width = 0;
		m_height = 0;
	}

	if(m_prendererData != nullptr)
	{
		m_prendererData = nullptr;
	}
}

int bbe::Image::getWidth() const
{
	return m_width;
}

int bbe::Image::getHeight() const
{
	return m_height;
}

bbe::Vector2 bbe::Image::getDimensions() const
{
	return Vector2(static_cast<float>(getWidth()), static_cast<float>(getHeight()));
}

size_t bbe::Image::getSizeInBytes() const
{
	return static_cast<size_t>(getWidth() * getHeight() * getAmountOfChannels() * getBytesPerChannel());
}

size_t bbe::Image::getAmountOfChannels() const
{
	switch (m_format)
	{
	case ImageFormat::R8:
		return 1;
	case ImageFormat::R8G8B8A8:
		return 4;
	case ImageFormat::R32FLOAT:
		return 1;
	default:
		throw FormatNotSupportedException();
	}
}

size_t bbe::Image::getBytesPerChannel() const
{
	switch (m_format)
	{
	case ImageFormat::R8:
		return 1;
	case ImageFormat::R8G8B8A8:
		return 1;
	case ImageFormat::R32FLOAT:
		return 4;
	default:
		throw FormatNotSupportedException();
	}
}

bbe::Color bbe::Image::getPixel(size_t x, size_t y) const
{
	if (m_pdata == nullptr)
	{
		throw NotInitializedException();
	}

	const size_t index = getIndexForRawAccess(x, y);
	switch(m_format)
	{
	case ImageFormat::R8:
		return Color(m_pdata[index] / 255.f, m_pdata[index] / 255.f, m_pdata[index] / 255.f, 1.0f);
	case ImageFormat::R8G8B8A8:
		return Color(m_pdata[index] / 255.f, m_pdata[index + 1] / 255.f, m_pdata[index + 2] / 255.f, m_pdata[index + 3] / 255.f);
	default:
		throw FormatNotSupportedException();
	}
	
}

size_t bbe::Image::getIndexForRawAccess(size_t x, size_t y) const
{
	return (y * m_width + x) * getAmountOfChannels();
}

bbe::ImageRepeatMode bbe::Image::getRepeatMode() const
{
	return m_repeatMode;
}

void bbe::Image::setRepeatMode(ImageRepeatMode irm)
{
	if (m_prendererData != nullptr)
	{
		throw AlreadyUploadedException();
	}

	m_repeatMode = irm;
}

bbe::ImageFilterMode bbe::Image::getFilterMode() const
{
	return m_filterMode;
}

void bbe::Image::setFilterMode(ImageFilterMode ifm)
{
	if (m_prendererData != nullptr)
	{
		throw AlreadyUploadedException();
	}

	m_filterMode = ifm;
}

bool bbe::Image::isLoaded() const
{
	return m_pdata != nullptr;
}
