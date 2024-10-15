#include "BBE/ChatGPTComm.h"
#include "BBE/Logging.h"
#include "BBE/SimpleUrlRequest.h"
#include "BBE/SimpleFile.h"
#include "BBE/Async.h"

static bbe::List<char> sendRequestBinary(const std::string& url, const bbe::String& key, const std::string& jsonInput) {
	auto response = bbe::simpleUrlRequest::urlRequest(
		url.c_str(),
		{ "Authorization: Bearer " + key, "Content-Type: application/json" },
		jsonInput.c_str()
	);

	return response.dataContainer;
}

static std::string sendRequest(const std::string& url, const bbe::String& key, const std::string& jsonInput) {
	return sendRequestBinary(url, key, jsonInput).getRaw();
}

bbe::ChatGPTComm::ChatGPTComm(const bbe::String& key) : key(key)
{
}

bool bbe::ChatGPTComm::isKeySet() const
{
	std::lock_guard _(mutex);
	return key.isEmpty() == false;
}

bbe::ChatGPTQueryResponse bbe::ChatGPTComm::query(const bbe::String& msg)
{
	std::unique_lock ul(mutex);
	// Add user's message to the conversation
	history.push_back({ {"role", "user"}, {"content", msg.getRaw()}});

	std::vector<nlohmann::json> sendMessages = {
		{{"role", "system"}, {"content", role.getRaw()}}
	};
	for (size_t i = 0; i < history.size(); i++)
	{
		sendMessages.push_back(history[i]);
	}

	// Prepare the JSON data for the request
	nlohmann::json jsonData = {
		{"model", model.getRaw()},
		{"messages", sendMessages}
	};

	// Send the request
	bbe::String keyCopy = key;
	ul.unlock(); // Making the actual sendRequest call outside the lock. This is what actually hangs and we shouldn't hold the lock longer than needed.
	std::string response = sendRequest("https://api.openai.com/v1/chat/completions", keyCopy, jsonData.dump(-1, ' ', false, nlohmann::detail::error_handler_t::ignore));
	ul.lock();

	// Parse and display the response
	try {
		ChatGPTQueryResponse retVal;
		auto jsonResponse = nlohmann::json::parse(response);
		retVal.message = jsonResponse["choices"][0]["message"]["content"].get<std::string>().c_str();

		retVal.inputTokens = jsonResponse["usage"]["prompt_tokens"];
		retVal.outputTokens = jsonResponse["usage"]["completion_tokens"];
		retVal.totalTokens = jsonResponse["usage"]["total_tokens"];
		totalTokens += retVal.totalTokens;

		// Add assistant's message to the conversation
		history.push_back({ {"role", "assistant"}, {"content", retVal.message.getRaw()}});
		return retVal;
	}
	catch (const std::exception& e) {
		BBELOGLN("Error parsing API response: " << e.what());
	}

	bbe::Crash(bbe::Error::IllegalState, "Error parsing API response");
}

std::future<bbe::ChatGPTQueryResponse> bbe::ChatGPTComm::queryAsync(const bbe::String& msg)
{
	return bbe::async(&bbe::ChatGPTComm::query, this, msg);
}

bbe::Sound bbe::ChatGPTComm::synthesizeSpeech(const bbe::String& text)
{
	std::unique_lock ul(mutex);

	nlohmann::json jsonData = {
		{"model", ttsModel.getRaw()},
		{"input", text.getRaw()},
		{"voice", voice.getRaw()}
	};

	bbe::String keyCopy = key;
	ul.unlock(); // Making the actual sendRequest call outside the lock. This is what actually hangs and we shouldn't hold the lock longer than needed.
	bbe::List<char> soundData = sendRequestBinary("https://api.openai.com/v1/audio/speech", keyCopy, jsonData.dump());
	ul.lock();
	bbe::Sound sound;
	sound.load(soundData, bbe::SoundLoadFormat::MP3);
	return sound;
}

std::future<bbe::Sound> bbe::ChatGPTComm::synthesizeSpeechAsync(const bbe::String& text)
{
	return bbe::async(&bbe::ChatGPTComm::synthesizeSpeech, this, text);
}

bbe::String bbe::ChatGPTComm::transcribe(const bbe::Sound& sound)
{
	std::map<bbe::String, bbe::String> formFields = {
		{"model", "whisper-1"}
	};

	const bbe::ByteBuffer file = sound.toWav();

	auto response = bbe::simpleUrlRequest::urlFile(
		"https://api.openai.com/v1/audio/transcriptions",
		{ "Authorization: Bearer " + key },
		formFields,
		&file,
		"file",
		"audio.wav",
		true,
		false
	);

	return bbe::String(reinterpret_cast<const char*>(response.dataContainer.getRaw()));
}


std::future<bbe::String> bbe::ChatGPTComm::transcribeAsync(const bbe::Sound& sound)
{
	return bbe::async(&bbe::ChatGPTComm::transcribe, this, sound);
}

void bbe::ChatGPTComm::purgeMemory()
{
	std::lock_guard _(mutex);
	history.clear();
}

bbe::List<bbe::String> bbe::ChatGPTComm::getAvailableModels() const
{
	std::unique_lock ul(mutex);
	std::string url = "https://api.openai.com/v1/models";

	bbe::String keyCopy = key;
	ul.unlock(); // Making the actual sendRequest call outside the lock. This is what actually hangs and we shouldn't hold the lock longer than needed.
	std::string response = sendRequest(url, keyCopy, "");
	ul.unlock();

	bbe::List<bbe::String> models;
	try {
		auto jsonResponse = nlohmann::json::parse(response);
		for (const auto& model : jsonResponse["data"]) {
			models.add(model["id"].get<std::string>().c_str());
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
	}

	if (models.isEmpty()) {
		bbe::Crash(bbe::Error::IllegalState, "No models available or failed to retrieve models.");
	}

	return models;
}
