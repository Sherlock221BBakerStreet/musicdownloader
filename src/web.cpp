#include <stack>

#include <cpr/util.h>

#include "util.h"
#include "web.h"

using json = nlohmann::json;
using namespace std;

// Is server even the right name for that argument?
bool check_successful_response(const cpr::Response& response, const string& server) {
	if (!response.status_code) {
		cout<<"Error occured ("<<(int)response.error.code<<"):"<<endl
			<<response.error.message<<endl
			<<endl
			<<"Exiting program"<<endl
			<<endl;
		exit(0xBAD);
	} else if (response.status_code/100 == 4) {
		cout<<server<<" response ("<<response.status_code<<"):"<<endl
			<<response.text<<endl
			<<endl
			<<"Exiting program"<<endl
			<<endl;
		exit(0xBAD);
	} else if (response.status_code/100 != 2) {
		cout<<server<<" response ("<<response.status_code<<"):"<<endl
			<<response.text<<endl
			<<endl;
		return false;
	}
	return true;
}

string construct_query(const json& request, const vector<string>& keys) {
	string query = "";
	for (const auto key : keys) {
		string value = request[key];
		query += key + "=" + value + "&";
	}
	return query;
}

tuple<bool, string> download_song(const string& url) {
	static const int MAX_NUM_ATTEMPTS = 100;

	static const auto doAgain = [](const string& mp3) {
		// Magic number is hopefully not so magic
		return starts_with(trim(mp3), "<html>") || mp3.size() < 100*1024;
	};

	auto response = cpr::Get(cpr::Url{url}); bool fail;
	for (int i = 0; fail = doAgain(response.text) && check_successful_response(response, "YouTubeInMP3") && i < MAX_NUM_ATTEMPTS; ++i) {
		response = cpr::Get(cpr::Url{url});
	}
	// check_successful_response here may not be needed
	return make_tuple(!fail && check_successful_response(response, "YouTubeInMP3"), response.text);
}

string search_duckduckgo(const string& query) {
	string url = "https://duckduckgo.com/html/?q=" + urlify(query);

	cout<<"DuckDuckGo URL: "<<url<<endl;
	auto response = cpr::Get(cpr::Url{url}, cpr::VerifySsl{false});
	return check_successful_response(response, "DuckDuckGo") ? response.text : "";
}

string remove_html_tags(const string& html, const string& rpl) {
	string tagless = html;

	int pos = 0;
	// Terrible way to do this, I think
	while ((pos = tagless.find('<', pos)) != string::npos) {
		int end = tagless.find('>', pos);

		string tag = tagless.substr(pos, end-pos+1);
		tagless.replace(pos, end-pos+1, tag == "<br>" ? "" : rpl);
	}
	return trim(tagless);
}