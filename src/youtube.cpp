#include <fstream>

#include "util.h"
#include "web.h"
#include "youtube.h"

#define APIKEY "AIzaSyDxmk_iusdpHuj5VfFnqyvweW1Lep0j2oc"

using json = nlohmann::json;
using namespace std;

// YouTube API https://developers.google.com/youtube/v3/docs/search/list
tuple<string, string> search_youtube_for_song(const string& song, bool verbose) {
    // Top result isn't guaranteed to be the correct result
    static const int MAX_NUM_RESULTS_PER_SONG = 3;

    json request;
    request["part"] = "snippet";
    request["topicId"] = "/m/04rlf"; // music
    request["maxResults"] = to_string(MAX_NUM_RESULTS_PER_SONG);
    request["type"] = "video";
    request["q"] = urlify(song);
    request["key"] = APIKEY;

    cout<<"Searching YouTube for song: \""<<song<<"\""<<endl;
    if (verbose) {
        cout<<"Song title in url: "<<request["q"]<<endl;
    }

    string query = construct_query(request, {"part", "topicId", "maxResults", "type", "q", "key"});
    string url = "https://www.googleapis.com/youtube/v3/search?" + query;
    if (verbose) {
        cout<<TAB<<"api call: "<<url<<endl;
    }
    
    auto response = cpr::Get(cpr::Url{url}, cpr::VerifySsl{false});
    if (check_successful_response(response, "YouTube")) {
        json resp = json::parse(response.text);

        int num_downloads = min<int>(resp["items"].size(), MAX_NUM_RESULTS_PER_SONG);
        if (verbose) {
            cout<<"Retreiving Ids for top "<<num_downloads<<" results..."<<endl;
        }
        
        float lowest_dist = 1.0;
        string winner = "";
        json winner_title = "";

        for (int i = 0; i < num_downloads; ++i) {
            json videoId = resp["items"][i]["id"]["videoId"];
            json title = resp["items"][i]["snippet"]["title"];
            float dist = title_distance(song, title);

            if (verbose) {
                cout<<TAB<<videoId<<" -> "<<title<<" ("<<dist<<")"<<endl;
            }

            winner = dist < lowest_dist ? videoId.get<string>() : winner;
            winner_title = dist < lowest_dist ? title : winner_title;
            lowest_dist = min(dist, lowest_dist);
        }
        if (verbose) {
            cout<<"The following video was chosen: "<<winner_title<<endl;
        }
        return make_tuple(winner, winner_title.get<string>());
    }
    return make_tuple("", "");
}
